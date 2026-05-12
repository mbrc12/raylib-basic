#include "assets.hpp"
#include "engine/util.hpp"

#include <toml++/toml.hpp>

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

namespace engine::assets {
std::string readFileInternal(const char* path);

namespace {

std::string resolvePath(const char* relativePath) {
#if defined(__EMSCRIPTEN__)
    return relativePath;
#else
    return std::string(GetApplicationDirectory()) + relativePath;
#endif
}

struct ShaderPaths {
    std::string vert;
    std::string frag;
};

std::unordered_map<std::string, ShaderPaths> gShaderPaths;
std::unordered_map<std::string, std::string> gTexturePaths;
struct FontPath {
    std::string path;
    int size = 16;
};
std::unordered_map<std::string, FontPath> gFontPaths;
std::unordered_map<std::string, std::string> gModelPaths;
std::unordered_map<std::string, std::string> gTextPaths;

std::unordered_map<std::string, Shader*> gShaderCache;
std::unordered_map<std::string, Texture2D*> gTextureCache;
std::unordered_map<std::string, Font*> gFontCache;
std::unordered_map<std::string, Model*> gModelCache;

std::unordered_map<std::string, d2::Sprite*> gSprites;

bool gInitialized = false;

std::string loadIndexText() {
    std::string path = resolvePath("assets/index.toml");
    return readFileInternal(path.c_str());
}

template <typename Cache, typename Loader>
typename Cache::mapped_type loadCached(Cache& cache, const char* name, Loader&& loader) {
    if (auto it = cache.find(name); it != cache.end()) {
        return it->second;
    }

    auto* resource = loader();
    if (resource == nullptr) {
        return nullptr;
    }

    cache.emplace(name, resource);
    return resource;
}

template <typename Map>
const typename Map::mapped_type* findPath(const Map& paths, const char* name, const char* kind) {
    auto it = paths.find(name);
    if (it == paths.end()) {
        auto err = std::format("ASSETS: {} '{}' not found in index", kind, name);
        throw std::runtime_error(err);
        return nullptr;
    }

    return &it->second;
}

template <typename T> std::optional<T> toml_get_opt(toml::table t, std::string_view key) {
    assert(t.contains(key));
    auto value = t[key];
    if constexpr (std::is_same_v<T, std::string>) {
        if (!value.is_string()) {
            return std::nullopt;
        }
        return value.as_string()->get();
    } else if constexpr (std::is_same_v<T, int>) {
        if (!value.is_integer()) {
            return std::nullopt;
        }
        return static_cast<int>(value.as_integer()->get());
    } else if constexpr (std::is_same_v<T, toml::table>) {
        if (!value.is_table()) {
            return std::nullopt;
        }
        return *value.as_table();
    } else if constexpr (std::is_same_v<T, bool>) {
        if (!value.is_boolean()) {
            return std::nullopt;
        }
        return value.as_boolean()->get();
    } else {
        return std::nullopt;
    }
}

template <typename T> T toml_get(toml::table t, std::string_view key) {
    auto opt = toml_get_opt<T>(t, key);
    if (!opt.has_value()) {
        auto err = std::format("ASSETS: Invalid type for key '{}' in index", key);
        throw std::runtime_error(err);
        return {};
    }
    return *opt;
}

void parseIndex(const std::string& indexText) {
    if (indexText.empty())
        return;

    toml::table table;
    try {
        table = toml::parse(indexText);
    } catch (const toml::parse_error& e) {
        throw std::runtime_error(std::format("ASSETS: Failed to parse index file: {}", e.what()));
        return;
    }

    auto loadPathSection = [](const toml::table& tbl, const char* section,
                              std::unordered_map<std::string, std::string>& out, const char* subdir) {
        if (!tbl.contains(section)) {
            return;
        }

        std::string prefix = resolvePath(subdir);
        auto entries = toml_get<toml::table>(tbl, section);
        for (const auto& [key, value] : entries) {
            auto key_ = std::string(key);
            out[key_] = prefix + toml_get<std::string>(entries, key.str());
            dbg("ASSETS: Loaded %s '%s' at path '%s'", section, key_.c_str(), out[key_].c_str());
        }
    };

    if (table.contains("shaders")) {
        std::string prefix = resolvePath("assets/shaders/");
        auto shaders = toml_get<toml::table>(table, "shaders");
        for (const auto& [key, _] : shaders) {
            auto key_ = std::string(key);
            auto shaderTable = toml_get<toml::table>(shaders, key_);
            auto vert = prefix + toml_get<std::string>(shaderTable, "vert");
            auto frag = prefix + toml_get<std::string>(shaderTable, "frag");
            gShaderPaths[key_] = {.vert = vert, .frag = frag};
            dbg("ASSETS: Loaded shader '%s' with vert path '%s' and frag path '%s'", key_.c_str(), vert.c_str(),
                frag.c_str());
        }
    }

    if (table.contains("fonts")) {
        std::string prefix = resolvePath("assets/fonts/");
        auto fonts = toml_get<toml::table>(table, "fonts");
        for (const auto& [key, _] : fonts) {
            auto key_ = std::string(key);
            auto fontTable = toml_get<toml::table>(fonts, key_);
            auto path = prefix + toml_get<std::string>(fontTable, "path");
            auto size = toml_get<int>(fontTable, "size");
            gFontPaths[key_] = {.path = path, .size = size};
            dbg("ASSETS: Loaded font '%s' at path '%s' with size %d", key_.c_str(), path.c_str(), size);
        }
    }

    loadPathSection(table, "textures", gTexturePaths, "assets/textures/");

    if (table.contains("sprites")) {
        auto sprites = toml_get<toml::table>(table, "sprites");
        for (const auto& [key, _] : sprites) {
            auto key_ = std::string(key);
            auto spriteTable = toml_get<toml::table>(sprites, key_);
            auto texture = toml_get<std::string>(spriteTable, "texture");
            auto x = toml_get<int>(spriteTable, "x");
            auto y = toml_get<int>(spriteTable, "y");
            auto w = toml_get<int>(spriteTable, "w");
            auto h = toml_get<int>(spriteTable, "h");
            auto center = toml_get_opt<bool>(spriteTable, "center").value_or(false);
            auto sprite = new d2::Sprite(texture, x, y, w, h, center);

            dbg("ASSETS: Loaded sprite '%s' from texture '%s' at (%d, %d, %d, %d)", key_.c_str(), texture.c_str(), x, y,
                w, h);
            
            if (spriteTable.contains("ninepatch")) {
                auto ninepatchTable = toml_get<toml::table>(spriteTable, "ninepatch");
                auto l = toml_get<int>(ninepatchTable, "l");
                auto r = toml_get<int>(ninepatchTable, "r");
                auto t = toml_get<int>(ninepatchTable, "t");
                auto b = toml_get<int>(ninepatchTable, "b");
                sprite->configureNinepatch(l, r, t, b);
                dbg("ASSETS: Configured ninepatch for sprite '%s' with l=%d, r=%d, t=%d, b=%d", key_.c_str(), l, r, t, b);
            }

            gSprites[key_] = sprite;
        }
    }

    loadPathSection(table, "models", gModelPaths, "assets/models/");
    loadPathSection(table, "texts", gTextPaths, "assets/");
}

} // namespace

void init() {
    if (gInitialized)
        return;

    std::string indexText = loadIndexText();
    if (indexText.empty()) {
        assert(false && "ASSETS: Failed to load index file");
    } else {
        parseIndex(indexText);
    }

    gInitialized = true;
}

void manual_unload() {
    for (auto& [name, ptr] : gShaderCache) {
        ptr->unload();
        delete ptr;
    }
    gShaderCache.clear();

    for (auto& [name, ptr] : gTextureCache) {
        UnloadTexture(*ptr);
        delete ptr;
    }
    gTextureCache.clear();

    for (auto& [name, ptr] : gFontCache) {
        UnloadFont(*ptr);
        delete ptr;
    }
    gFontCache.clear();

    for (auto& [name, ptr] : gSprites) {
        delete ptr;
    }
    gSprites.clear();

    for (auto& [name, ptr] : gModelCache) {
        UnloadModel(*ptr);
        delete ptr;
    }
    gModelCache.clear();
}

Shader* shader(const char* name) {
    init();

    const ShaderPaths* paths = findPath(gShaderPaths, name, "Shader");
    if (paths == nullptr) {
        return nullptr;
    }

    return loadCached(gShaderCache, name, [&]() {
        const std::string vertSource = readFileInternal(paths->vert.c_str());
        const std::string fragSource = readFileInternal(paths->frag.c_str());
        return new Shader(vertSource.c_str(), fragSource.c_str());
    });
}

Texture2D* texture(const char* name) {
    init();

    const std::string* path = findPath(gTexturePaths, name, "Texture");
    if (path == nullptr) {
        return nullptr;
    }

    return loadCached(gTextureCache, name, [&]() { return new Texture2D(LoadTexture(path->c_str())); });
}

Font* font(const char* name) {
    init();

    const FontPath* path = findPath(gFontPaths, name, "Font");
    if (path == nullptr) {
        return nullptr;
    }

    return loadCached(gFontCache, name,
                      [&]() { return new Font(LoadFontEx(path->path.c_str(), path->size, nullptr, 0)); });
}

Model* model(const char* name) {
    init();

    const std::string* path = findPath(gModelPaths, name, "Model");
    if (path == nullptr) {
        return nullptr;
    }

    return loadCached(gModelCache, name, [&]() { return new Model(LoadModel(path->c_str())); });
}

d2::Sprite* sprite(const char* name) {
    init();

    auto it = gSprites.find(name);
    if (it == gSprites.end()) {
        throw std::runtime_error(std::format("ASSETS: Sprite '{}' not found in index", name));
    }

    return it->second;
}

Image image(const char* name) {
    init();

    const std::string* path = findPath(gTexturePaths, name, "Image");
    if (path == nullptr) {
        return {};
    }

    return LoadImage(path->c_str());
}

std::string readFileInternal(const char* path) {
    std::ifstream file(path);
    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

std::string text(const char* name) {
    init();

    const std::string* path = findPath(gTextPaths, name, "Text");
    if (path == nullptr) {
        return {};
    }

    return readFileInternal(path->c_str());
}

} // namespace engine::assets
