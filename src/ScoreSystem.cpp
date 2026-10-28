#include "ScoreSystem.hpp"
#include <fstream>
#include <iostream>
#include <utility>

ScoreSystem::ScoreSystem(SDL_Renderer* renderer, raii::Font font, const std::string& savePath)
    : m_renderer(renderer), m_savePath(savePath), m_font(std::move(font)) {
    loadHighScore();
    refreshLabel();
}

void ScoreSystem::addPoint() {
    ++m_score;
    if (m_score > m_highScore) {
        m_highScore = m_score;
        m_dirty = true;
    }
    refreshLabel();
}

void ScoreSystem::reset() {
    m_score = 0;
    refreshLabel();
}

void ScoreSystem::commit() {
    if (m_dirty) {
        saveHighScore();
        m_dirty = false;
    }
}

void ScoreSystem::render(SDL_Renderer* renderer) {
    m_label.render(renderer, 10.0f, 10.0f);
}

void ScoreSystem::refreshLabel() {
    m_label.setText(m_renderer, m_font.get(),
                    "Score: " + std::to_string(m_score) + "  Best: " + std::to_string(m_highScore),
                    SDL_Color{20, 20, 20, 255});
}

void ScoreSystem::loadHighScore() {
    std::ifstream file(m_savePath);
    int value = 0;
    if (file >> value && value > 0) {
        m_highScore = value;
    }
}

void ScoreSystem::saveHighScore() const {
    std::ofstream file(m_savePath, std::ios::trunc);
    if (!(file << m_highScore)) {
        std::cerr << "Warning: could not save high score to " << m_savePath << "\n";
    }
}
