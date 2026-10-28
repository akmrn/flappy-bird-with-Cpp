#pragma once

#include <vector>
#include <string>
#include "RAII_SDL.hpp"
#include "Pipe.hpp"
#include "ScoreSystem.hpp"
#include "player.hpp"
#include "constants.hpp"

class Game {
public:
    Game();
    ~Game() = default;

    // Prevent copy/move semantics for the top-level application
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    void run();

private:
    void processEvents();
    void update(float deltaTime);
    void render();
    void resetGame();

    // 1. Subsystem contexts (Initialized first, destroyed last)
    raii::SdlContext m_sdlContext;
    raii::TtfContext m_ttfContext;

    // 2. Core SDL resources
    raii::Window m_window;
    raii::Renderer m_renderer;

    // 3. Game subsystems & entities (Initialized AFTER renderer)
    ScoreSystem m_scoreSystem;
    Player m_player;
    std::vector<Pipe> m_pipes;

    // 4. Fonts and UI Textures for Game Over HUD
    raii::Font m_gameOverFont;
    raii::Font m_restartFont;
    raii::Texture m_gameOverTex;
    raii::Texture m_restartTex;
    int m_gameOverTexW = 0;
    int m_gameOverTexH = 0;
    int m_restartTexW = 0;
    int m_restartTexH = 0;

    // 5. Game state & timing
    bool m_isRunning = true;
    bool m_isGameOver = false;
};
