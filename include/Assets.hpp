#pragma once
#include "RAII_SDL.hpp"
#include <memory>
#include <stdexcept>
#include <string>

namespace assets {
    // Resolve assets relative to the executable, not the working directory.
    inline std::string path(const std::string& file) {
        const char* base = SDL_GetBasePath(); // owned by SDL, do not free
        return std::string(base ? base : "") + "assets/" + file;
    }

    inline raii::Font loadFont(const std::string& file, float size) {
        raii::Font font(TTF_OpenFont(path(file).c_str(), size));
        if (!font) {
            throw std::runtime_error("Failed to load font '" + file + "': " + SDL_GetError());
        }
        return font;
    }

    inline raii::Texture loadTexture(SDL_Renderer* renderer, const std::string& file) {
        raii::Texture tex(IMG_LoadTexture(renderer, path(file).c_str()));
        if (!tex) {
            throw std::runtime_error("Failed to load image '" + file + "': " + SDL_GetError());
        }
        return tex;
    }

    // Per-user writable location for the save file.
    inline std::string savePath(const std::string& file) {
        char* pref = SDL_GetPrefPath("FlappyOrg", "FlappyBird");
        std::string result = pref ? std::string(pref) + file : file;
        SDL_free(pref);
        return result;
    }
}
