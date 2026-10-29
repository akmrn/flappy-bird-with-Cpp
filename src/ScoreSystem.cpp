#include "ScoreSystem.hpp"
#include <algorithm>
#include <fstream>
#include <stdexcept>

namespace {
    // Panel placement in the top-left corner of the logical screen
    constexpr float kBoxLeft  = 8.0f;
    constexpr float kBoxTop   = 8.0f;
    constexpr float kBoxWidth = 230.0f;

    // Center and height of the dark plate inside scoreBox.webp, as fractions of the image
    constexpr float kPlateCenterX = 0.595f;
    constexpr float kPlateCenterY = 0.563f;
    constexpr float kPlateHeight  = 0.335f;

    // Keep the text a bit smaller than the plate so it does not touch the edges
    constexpr float kTextFill     = 0.80f;
    constexpr float kShadowOffset = 1.5f;

    // Renders white text with alpha, so color mod can turn it into a shadow
    raii::Texture makeText(SDL_Renderer* renderer, TTF_Font* font,
                           const std::string& text, float& outW, float& outH) {
        raii::Texture tex;
        raii::Surface surface(TTF_RenderText_Blended(font, text.c_str(), 0,
                                                     SDL_Color{255, 255, 255, 255}));
        if (!surface) return tex;
        outW = static_cast<float>(surface->w);
        outH = static_cast<float>(surface->h);
        tex.reset(SDL_CreateTextureFromSurface(renderer, surface.get()));
        return tex;
    }
}

ScoreSystem::ScoreSystem(SDL_Renderer* renderer, raii::Font font,
                         const std::string& savePath, SDL_Texture* box)
    : m_renderer(renderer),
      m_box(box),
      m_font(std::move(font)),
      m_savePath(savePath) {
    if (!m_font) {
        throw std::runtime_error("Failed to load score font");
    }

    if (m_box) {
        float boxW = 0.0f, boxH = 0.0f;
        SDL_GetTextureSize(m_box, &boxW, &boxH);
        if (boxW > 0.0f) m_boxH = kBoxWidth * boxH / boxW;
    }

    loadHighScore();
    rebuildText();
}

void ScoreSystem::addPoint() {
    ++m_score;
    if (m_score > m_highScore) m_highScore = m_score;
    rebuildText();
}

void ScoreSystem::reset() {
    m_score = 0;
    rebuildText();
}

void ScoreSystem::commit() {
    saveHighScore();
}

void ScoreSystem::rebuildText() {
    const std::string text = std::to_string(m_score) + " / " + std::to_string(m_highScore);
    m_text = makeText(m_renderer, m_font.get(), text, m_textW, m_textH);
}

void ScoreSystem::render(SDL_Renderer* renderer) const {
    const float boxH = m_box ? m_boxH : 40.0f;
    SDL_FRect boxRect{kBoxLeft, kBoxTop, kBoxWidth, boxH};

    if (m_box) {
        SDL_RenderTexture(renderer, m_box, nullptr, &boxRect);
    } else {
        // Fallback when the panel image is missing
        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 200);
        SDL_RenderFillRect(renderer, &boxRect);
    }

    SDL_Texture* tex = m_text.get();
    if (!tex) return;

    // Fit the text inside the plate
    const float plateCx = kBoxLeft + kBoxWidth * kPlateCenterX;
    const float plateCy = kBoxTop + boxH * kPlateCenterY;
    const float maxH = boxH * kPlateHeight * kTextFill;
    const float scale = std::min(1.0f, maxH / m_textH);
    const float w = m_textW * scale;
    const float h = m_textH * scale;

    SDL_FRect dst{plateCx - w / 2.0f, plateCy - h / 2.0f, w, h};

    // Shadow: same texture drawn in black, slightly offset
    SDL_SetTextureColorMod(tex, 0, 0, 0);
    SDL_FRect shadow{dst.x + kShadowOffset, dst.y + kShadowOffset, w, h};
    SDL_RenderTexture(renderer, tex, nullptr, &shadow);

    SDL_SetTextureColorMod(tex, 255, 255, 255);
    SDL_RenderTexture(renderer, tex, nullptr, &dst);
}

void ScoreSystem::loadHighScore() {
    std::ifstream file(m_savePath);
    if (file.is_open()) {
        file >> m_highScore;
        if (m_highScore < 0) m_highScore = 0;
    }
}

void ScoreSystem::saveHighScore() const {
    std::ofstream file(m_savePath);
    if (file.is_open()) {
        file << m_highScore;
    }
}
