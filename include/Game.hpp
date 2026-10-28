#pragma once

#include <vector>
#include "RAII_SDL.hpp"
#include "Pipe.hpp"
#include "Ground.hpp"
#include "ScoreSystem.hpp"
#include "player.hpp"
#include "TextLabel.hpp"
#include "TextureCache.hpp"
#include "Skin.hpp"
#include "constants.hpp"

class Game {
public:
    Game();
    ~Game() = default;

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    void run();

private:
    enum class State { Ready, Playing, Paused, GameOver };

    void processEvents();
    void handleAction();      // jump / start / resume / restart
    void togglePause();
    void toggleFullscreen();
    void update(float dt);
    void render();
    void resetGame();
    void gameOver();
    void spawnInitialPipes();
    float currentPipeSpeed() const;

    // Declaration order matters: members are initialized in this exact order.

    // 1. Subsystem contexts (initialized first, destroyed last)
    raii::SdlContext m_sdlContext;
    raii::TtfContext m_ttfContext;

    // 2. Core SDL resources
    raii::Window m_window;
    raii::Renderer m_renderer;

    // 3. Image cache (must come before anything that borrows its textures)
    TextureCache m_textures;

    // Optional images (non-owning, may be nullptr)
    SDL_Texture* m_pipeTex = nullptr;
    SDL_Texture* m_backgroundTex = nullptr;
    SDL_Texture* m_groundTex = nullptr;

    // 4. Game objects (Ground must come after m_groundTex)
    Ground m_ground;
    ScoreSystem m_scoreSystem;
    Player m_player;
    std::vector<Pipe> m_pipes;

    // 5. Fonts and cached labels
    raii::Font m_titleFont;
    raii::Font m_hintFont;
    TextLabel m_gameOverLabel;
    TextLabel m_restartLabel;
    TextLabel m_readyLabel;
    TextLabel m_pausedLabel;

    // 6. State
    State m_state = State::Ready;
    bool m_isRunning = true;
    float m_cooldown = 0.0f; // blocks accidental instant restart
};
