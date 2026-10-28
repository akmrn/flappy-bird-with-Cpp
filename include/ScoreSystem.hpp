#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include "RAII_SDL.hpp"

class ScoreSystem {
public:
    // Constructor acquires resources immediately (RAII)
    ScoreSystem(SDL_Renderer* renderer, const std::string& fontPath, float fontSize, const std::string& savePath);

    // Destructor is default because smart pointers handle cleanup automatically
    ~ScoreSystem() = default;

    // Disallow copy to prevent double deletion of resources
    ScoreSystem(const ScoreSystem&) = delete;
    ScoreSystem& operator=(const ScoreSystem&) = delete;

    void addPoint();
    void reset();
    void render(SDL_Renderer* renderer);

    int getScore() const { return m_score; }
    int getHighScore() const { return m_highScore; }

private:
    void loadHighScore();
    void saveHighScore();
    void updateTexture();

    int m_score = 0;
    int m_highScore = 0;
    
    // Non-owning renderer pointer (Game owns the renderer)
    SDL_Renderer* m_renderer = nullptr;

    std::string m_savePath;

  

    // RAII-managed resources (No manual cleanup needed)
    raii::Font m_font;
    raii::Texture m_scoreTexture;

    int m_texW = 0;
    int m_texH = 0;
};
