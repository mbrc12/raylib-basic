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

    auto it = gShaderCache.find(name);
    if (it != gShaderCache.end())
        return it->second;

    auto pathIt = gShaderPaths.find(name);
    if (pathIt == gShaderPaths.end()) {
        TraceLog(LOG_WARNING, "ASSETS: Shader '%s' not found in index", name);
        return nullptr;
    }

    std::string vertSource = readFileInternal(pathIt->second.vert.c_str());
    std::string fragSource = readFileInternal(pathIt->second.frag.c_str());
    auto* res = new Shader(vertSource.c_str(), fragSource.c_str());
    gShaderCache[name] = res;
    return res;
}

Texture2D* texture(const char* name) {
    init();

    auto it = gTextureCache.find(name);
    if (it != gTextureCache.end())
        return it->second;

    auto pathIt = gTexturePaths.find(name);
    if (pathIt == gTexturePaths.end()) {
        TraceLog(LOG_WARNING, "ASSETS: Texture '%s' not found in index", name);
        return nullptr;
    }

    auto* res = new Texture2D(LoadTexture(pathIt->second.c_str()));
    gTextureCache[name] = res;
    return res;
}

Font* font(const char* name) {
    init();

    auto it = gFontCache.find(name);
    if (it != gFontCache.end())
        return it->second;

    auto pathIt = gFontPaths.find(name);
    if (pathIt == gFontPaths.end()) {
        TraceLog(LOG_WARNING, "ASSETS: Font '%s' not found in index", name);
        return nullptr;
    }

    int fontSize = pathIt->second.size;
    auto* res = new Font(LoadFontEx(pathIt->second.path.c_str(), fontSize, nullptr, 0));
    gFontCache[name] = res;
    return res;
}

Model* model(const char* name) {
    init();

    auto it = gModelCache.find(name);
    if (it != gModelCache.end())
        return it->second;

    auto pathIt = gModelPaths.find(name);
    if (pathIt == gModelPaths.end()) {
        TraceLog(LOG_WARNING, "ASSETS: Model '%s' not found in index", name);
        return nullptr;
    }

    auto* res = new Model(LoadModel(pathIt->second.c_str()));
    gModelCache[name] = res;
    return res;
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

    auto pathIt = gTextPaths.find(name);
    if (pathIt == gTextPaths.end()) {
        TraceLog(LOG_WARNING, "ASSETS: Text '%s' not found in index", name);
        return {};
    }

    return readFileInternal(pathIt->second.c_str());
}

} // namespace engine::assets
