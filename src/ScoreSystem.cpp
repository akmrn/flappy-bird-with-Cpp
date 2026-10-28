#include "ScoreSystem.hpp"
#include <fstream>
#include <stdexcept>
#include <string>

ScoreSystem::ScoreSystem(SDL_Renderer* renderer, const std::string& fontPath, float fontSize, const std::string& savePath)
    : m_renderer(renderer), m_savePath(savePath) {
    
    // Load high score from disk
    loadHighScore();

    // Load font using RAII wrapper
    TTF_Font* rawFont = TTF_OpenFont(fontPath.c_str(), fontSize);
    if (!rawFont) {
        throw std::runtime_error(std::string("Failed to load score font: ") + SDL_GetError());
    }
    m_font.reset(rawFont);

    // Generate initial texture for "Score: 0"
    updateTexture();
}

void ScoreSystem::addPoint() {
    m_score++;
    if (m_score > m_highScore) {
        m_highScore = m_score;
        saveHighScore();
    }
    updateTexture();
}

void ScoreSystem::reset() {
    m_score = 0;
    updateTexture();
}

void ScoreSystem::render(SDL_Renderer* renderer) {
    if (!m_scoreTexture) return;

    SDL_FRect dstRect = { 10.0f, 10.0f, static_cast<float>(m_texW), static_cast<float>(m_texH) };
    SDL_RenderTexture(renderer, m_scoreTexture.get(), nullptr, &dstRect);
}

void ScoreSystem::updateTexture() {
    std::string text = "Score: " + std::to_string(m_score) + "  High: " + std::to_string(m_highScore);
    SDL_Color color = { 0, 0, 0, 255 };

    // Create temporary surface
    SDL_Surface* rawSurface = TTF_RenderText_Blended(m_font.get(), text.c_str(), 0, color);
    if (!rawSurface) return;

    // Manage surface with RAII so it frees automatically when out of scope
    raii::Surface surface(rawSurface);
    m_texW = surface->w;
    m_texH = surface->h;

    SDL_Texture* rawTexture = SDL_CreateTextureFromSurface(m_renderer, surface.get());
    if (!rawTexture) return;

    // Transfer ownership to m_scoreTexture (automatically destroys previous texture!)
    m_scoreTexture.reset(rawTexture);
}

void ScoreSystem::loadHighScore() {
    std::ifstream file(m_savePath);
    if (file.is_open()) {
        file >> m_highScore;
    }
}

void ScoreSystem::saveHighScore() {
    std::ofstream file(m_savePath);
    if (file.is_open()) {
        file << m_highScore;
    }
}
