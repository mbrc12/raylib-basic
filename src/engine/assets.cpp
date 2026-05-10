#include "assets.hpp"

#include <toml++/toml.hpp>

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

namespace engine {
namespace {

std::string assetPath(const char* relativePath) {
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
bool gIndexLoaded = false;

void loadIndex() {
    if (gIndexLoaded)
        return;

    std::string indexText = readFile(assetPath("assets/index.toml").c_str());
    auto table = toml::parse(indexText);

    if (table.contains("shaders")) {
        auto shaders = *table["shaders"].as_table();
        for (const auto& [key, value] : shaders) {
            if (auto entry = value.as_table()) {
                ShaderPaths paths;
                if (auto v = (*entry)["vert"].as_string()) {
                    paths.vert = assetPath(v->get().c_str());
                }
                if (auto v = (*entry)["frag"].as_string()) {
                    paths.frag = assetPath(v->get().c_str());
                }
                gShaderPaths[std::string(key)] = paths;
            }
        }
    }

    auto loadSimpleSection = [](const toml::table& tbl, const char* section,
                                std::unordered_map<std::string, std::string>& out) {
        if (tbl.contains(section)) {
            auto entries = *tbl[section].as_table();
            for (const auto& [key, value] : entries) {
                if (auto path = value.as_string()) {
                    out[std::string(key)] = assetPath(path->get().c_str());
                }
            }
        }
    };

    loadSimpleSection(table, "textures", gTexturePaths);
    loadSimpleSection(table, "fonts", gFontPaths);

    gIndexLoaded = true;
}

} // namespace

std::string readFile(const char* path) {
    std::ifstream file(path);
    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

Texture2D loadTexture(const char* name) {
    loadIndex();
    auto it = gTexturePaths.find(name);
    if (it == gTexturePaths.end()) {
        TraceLog(LOG_WARNING, "ASSETS: Texture '%s' not found in index", name);
        return {};
    }
    return LoadTexture(it->second.c_str());
}

Image loadImage(const char* name) {
    loadIndex();
    auto it = gTexturePaths.find(name);
    if (it == gTexturePaths.end()) {
        TraceLog(LOG_WARNING, "ASSETS: Image '%s' not found in index", name);
        return {};
    }
    return LoadImage(it->second.c_str());
}

ShaderSources loadShader(const char* name) {
    loadIndex();
    auto it = gShaderPaths.find(name);
    if (it == gShaderPaths.end()) {
        TraceLog(LOG_WARNING, "ASSETS: Shader '%s' not found in index", name);
        return {};
    }
    return {readFile(it->second.vert.c_str()), readFile(it->second.frag.c_str())};
}

Font loadFont(const char* name) {
    loadIndex();
    auto it = gFontPaths.find(name);
    if (it == gFontPaths.end()) {
        TraceLog(LOG_WARNING, "ASSETS: Font '%s' not found in index", name);
        return {};
    }
    return LoadFont(it->second.c_str());
}

} // namespace engine
