#include "assets.hpp"

#include <toml++/toml.hpp>

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

namespace engine::assets {
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
std::unordered_map<std::string, std::string> gFontPaths;

std::unordered_map<std::string, Resource<Shader>> gShaderCache;
std::unordered_map<std::string, Resource<Texture2D>> gTextureCache;
std::unordered_map<std::string, Resource<Font>> gFontCache;

bool gInitialized = false;

std::string loadIndexText() {
    std::string path = resolvePath("assets/index.toml");
    return readFile(path.c_str());
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
    loadSection(table, "fonts", gFontPaths, "assets/fonts/");
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

Resource<Shader> shader(const char* name) {
    init();

    auto it = gShaderCache.find(name);
    if (it != gShaderCache.end())
        return it->second;

    auto pathIt = gShaderPaths.find(name);
    if (pathIt == gShaderPaths.end()) {
        TraceLog(LOG_WARNING, "ASSETS: Shader '%s' not found in index", name);
        return Resource<Shader>();
    }

    std::string vertSource = readFile(pathIt->second.vert.c_str());
    std::string fragSource = readFile(pathIt->second.frag.c_str());
    auto res = Resource<Shader>(new Shader(vertSource.c_str(), fragSource.c_str()));
    gShaderCache[name] = res;
    return res;
}

Resource<Texture2D> texture(const char* name) {
    init();

    auto it = gTextureCache.find(name);
    if (it != gTextureCache.end())
        return it->second;

    auto pathIt = gTexturePaths.find(name);
    if (pathIt == gTexturePaths.end()) {
        TraceLog(LOG_WARNING, "ASSETS: Texture '%s' not found in index", name);
        return Resource<Texture2D>();
    }

    Texture2D t = LoadTexture(pathIt->second.c_str());
    Resource<Texture2D> res(t);
    gTextureCache[name] = res;
    return res;
}

Resource<Font> font(const char* name) {
    init();

    auto it = gFontCache.find(name);
    if (it != gFontCache.end())
        return it->second;

    auto pathIt = gFontPaths.find(name);
    if (pathIt == gFontPaths.end()) {
        TraceLog(LOG_WARNING, "ASSETS: Font '%s' not found in index", name);
        return Resource<Font>();
    }

    Font f = LoadFont(pathIt->second.c_str());
    Resource<Font> res(f);
    gFontCache[name] = res;
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

std::string readFile(const char* path) {
    std::ifstream file(path);
    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

} // namespace engine::assets
