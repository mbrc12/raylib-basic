#include "assets.hpp"

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

bool gInitialized = false;

std::string loadIndexText() {
    std::string path = resolvePath("assets/index.toml");
    return readFileInternal(path.c_str());
}

template<typename Cache, typename Loader>
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

template<typename Map>
const typename Map::mapped_type* findPath(const Map& paths, const char* name, const char* kind) {
    auto it = paths.find(name);
    if (it == paths.end()) {
        TraceLog(LOG_WARNING, "ASSETS: %s '%s' not found in index", kind, name);
        return nullptr;
    }

    return &it->second;
}

void parseIndex(const std::string& indexText) {
    if (indexText.empty())
        return;

    toml::table table;
    try {
        table = toml::parse(indexText);
    } catch (const toml::parse_error& e) {
        TraceLog(LOG_WARNING, "ASSETS: Failed to parse index: %s", e.what());
        return;
    }

    if (table.contains("shaders")) {
        auto shaders = *table["shaders"].as_table();
        for (const auto& [key, value] : shaders) {
            if (auto entry = value.as_table()) {
                ShaderPaths paths;
                std::string prefix = resolvePath("assets/shaders/");
                if (auto v = (*entry)["vert"].as_string()) {
                    paths.vert = prefix + v->get();
                }
                if (auto v = (*entry)["frag"].as_string()) {
                    paths.frag = prefix + v->get();
                }
                gShaderPaths[std::string(key)] = paths;
            }
        }
    }

    auto loadSection = [](const toml::table& tbl, const char* section,
                          std::unordered_map<std::string, std::string>& out, const char* subdir) {
        if (tbl.contains(section)) {
            std::string prefix = resolvePath(subdir);
            auto entries = *tbl[section].as_table();
            for (const auto& [key, value] : entries) {
                if (auto path = value.as_string()) {
                    out[std::string(key)] = prefix + path->get();
                }
            }
        }
    };

    loadSection(table, "textures", gTexturePaths, "assets/textures/");

    if (table.contains("fonts")) {
        std::string prefix = resolvePath("assets/fonts/");
        auto fonts = *table["fonts"].as_table();
        for (const auto& [key, value] : fonts) {
            FontPath entry;
            if (auto path = value.as_string()) {
                entry.path = prefix + path->get();
            } else if (auto fontTable = value.as_table()) {
                if (auto path = (*fontTable)["path"].as_string()) {
                    entry.path = prefix + path->get();
                }
                if (auto size = (*fontTable)["size"].as_integer()) {
                    entry.size = static_cast<int>(size->get());
                }
            }
            if (!entry.path.empty()) {
                gFontPaths[std::string(key)] = entry;
            }
        }
    }

    loadSection(table, "models", gModelPaths, "assets/models/");
    loadSection(table, "texts", gTextPaths, "assets/");
}

} // namespace

void init() {
    if (gInitialized)
        return;

    std::string indexText = loadIndexText();
    if (indexText.empty()) {
        TraceLog(LOG_WARNING, "ASSETS: Could not read index.toml");
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

    return loadCached(gTextureCache, name, [&]() {
        return new Texture2D(LoadTexture(path->c_str()));
    });
}

Font* font(const char* name) {
    init();

    const FontPath* path = findPath(gFontPaths, name, "Font");
    if (path == nullptr) {
        return nullptr;
    }

    return loadCached(gFontCache, name, [&]() {
        return new Font(LoadFontEx(path->path.c_str(), path->size, nullptr, 0));
    });
}

Model* model(const char* name) {
    init();

    const std::string* path = findPath(gModelPaths, name, "Model");
    if (path == nullptr) {
        return nullptr;
    }

    return loadCached(gModelCache, name, [&]() {
        return new Model(LoadModel(path->c_str()));
    });
}

Image image(const char* name) {
    init();

    auto pathIt = gTexturePaths.find(name);
    if (pathIt == gTexturePaths.end()) {
        TraceLog(LOG_WARNING, "ASSETS: Image '%s' not found in index", name);
        return {};
    }

    return LoadImage(pathIt->second.c_str());
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
