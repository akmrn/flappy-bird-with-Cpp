#pragma once
#include "RAII_SDL.hpp"
#include "Assets.hpp"
#include <map>
#include <set>
#include <string>

// Loads each image once and owns it. Others only keep raw, non-owning pointers.
class TextureCache {
public:
    explicit TextureCache(SDL_Renderer* renderer) : m_renderer(renderer) {}

    TextureCache(const TextureCache&) = delete;
    TextureCache& operator=(const TextureCache&) = delete;

    // Required image: throws if it cannot be loaded.
    SDL_Texture* get(const std::string& file) {
        auto it = m_textures.find(file);
        if (it != m_textures.end()) return it->second.get();

        raii::Texture tex = assets::loadTexture(m_renderer, file); // throws on failure
        SDL_Texture* raw = tex.get();
        m_textures.emplace(file, std::move(tex));
        return raw;
    }

    // Optional image: returns nullptr if it is missing (the failure is remembered).
    SDL_Texture* tryGet(const std::string& file) {
        if (m_failed.count(file)) return nullptr;
        try {
            return get(file);
        } catch (const std::exception&) {
            m_failed.insert(file);
            return nullptr;
        }
    }

private:
    SDL_Renderer* m_renderer; // non-owning
    std::map<std::string, raii::Texture> m_textures;
    std::set<std::string> m_failed;
};
