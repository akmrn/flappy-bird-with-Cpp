#pragma once
#include "RAII_SDL.hpp"
#include <string>

// Caches a rendered text texture; re-renders only when the text changes.
class TextLabel {
public:
    void setText(SDL_Renderer* renderer, TTF_Font* font,
                 const std::string& text, SDL_Color color) {
        if (text == m_text && m_texture) return;
        m_text = text;

        raii::Surface surface(TTF_RenderText_Blended(font, text.c_str(), 0, color));
        if (!surface) return;
        m_w = static_cast<float>(surface->w);
        m_h = static_cast<float>(surface->h);
        m_texture.reset(SDL_CreateTextureFromSurface(renderer, surface.get()));
    }

    void render(SDL_Renderer* renderer, float x, float y) const {
        if (!m_texture) return;
        SDL_FRect dst{x, y, m_w, m_h};
        SDL_RenderTexture(renderer, m_texture.get(), nullptr, &dst);
    }

    void renderCentered(SDL_Renderer* renderer, float centerX, float y) const {
        render(renderer, centerX - m_w / 2.0f, y);
    }

private:
    std::string m_text;
    raii::Texture m_texture;
    float m_w = 0.0f;
    float m_h = 0.0f;
};
