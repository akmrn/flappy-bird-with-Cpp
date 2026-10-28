#pragma once
#include <SDL3/SDL.h>
#include <string>
#include "RAII_SDL.hpp"
#include "TextLabel.hpp"

class ScoreSystem {
public:
    ScoreSystem(SDL_Renderer* renderer, raii::Font font, const std::string& savePath);

    ScoreSystem(const ScoreSystem&) = delete;
    ScoreSystem& operator=(const ScoreSystem&) = delete;

    void addPoint();
    void reset();
    // Persist the high score once (called on game over, not on every point)
    void commit();
    void render(SDL_Renderer* renderer);

    int score() const { return m_score; }
    int highScore() const { return m_highScore; }

private:
    void loadHighScore();
    void saveHighScore() const;
    void refreshLabel();

    int m_score = 0;
    int m_highScore = 0;
    bool m_dirty = false; // high score changed since last save

    SDL_Renderer* m_renderer = nullptr; // non-owning
    std::string m_savePath;
    raii::Font m_font;
    TextLabel m_label;
};
