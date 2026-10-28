#include "Game.hpp"
#include "Assets.hpp"
#include <algorithm>
#include <stdexcept>
#include <string>

namespace {
    raii::Window createWindow() {
        raii::Window window(SDL_CreateWindow("Flappy Bird", cfg::worldW, cfg::worldH,
                                             SDL_WINDOW_RESIZABLE));
        if (!window) {
            throw std::runtime_error(std::string("Failed to create window: ") + SDL_GetError());
        }
        return window;
    }

    raii::Renderer createRenderer(SDL_Window* window) {
        raii::Renderer renderer(SDL_CreateRenderer(window, nullptr));
        if (!renderer) {
            throw std::runtime_error(std::string("Failed to create renderer: ") + SDL_GetError());
        }
        SDL_SetRenderVSync(renderer.get(), 1);

        // Fixed logical world scaled to any window size, keeping aspect ratio
        SDL_SetRenderLogicalPresentation(renderer.get(), cfg::worldW, cfg::worldH,
                                         SDL_LOGICAL_PRESENTATION_LETTERBOX);
        SDL_SetRenderDrawBlendMode(renderer.get(), SDL_BLENDMODE_BLEND);
        return renderer;
    }

    bool intersects(const SDL_FRect& a, const SDL_FRect& b) {
        return a.x < b.x + b.w && a.x + a.w > b.x &&
               a.y < b.y + b.h && a.y + a.h > b.y;
    }

    constexpr SDL_Color white{255, 255, 255, 255};
}

Game::Game()
    : m_window(createWindow()),
      m_renderer(createRenderer(m_window.get())),
      m_scoreSystem(m_renderer.get(), assets::loadFont("Yuyu.ttf", 28.0f),
                    assets::savePath("highscore.txt")),
      m_player(m_renderer.get()),
      m_titleFont(assets::loadFont("Bitcount.ttf", 72.0f)),
      m_hintFont(assets::loadFont("Anton.ttf", 22.0f)) {

    // Static labels are rendered once and cached
    m_gameOverLabel.setText(m_renderer.get(), m_titleFont.get(), "GAME OVER", {255, 50, 50, 255});
    m_restartLabel.setText(m_renderer.get(), m_hintFont.get(),
                           "Press Space or Left Click to Restart", white);
    m_readyLabel.setText(m_renderer.get(), m_hintFont.get(),
                         "Press Space or Click to Start  (P: pause, F11: fullscreen)", white);
    m_pausedLabel.setText(m_renderer.get(), m_titleFont.get(), "PAUSED", white);

    spawnInitialPipes();
}

void Game::spawnInitialPipes() {
    m_pipes.clear();
    m_pipes.reserve(cfg::pipeCount);
    const float startX = static_cast<float>(cfg::worldW) + 100.0f;
    for (int i = 0; i < cfg::pipeCount; ++i) {
        m_pipes.emplace_back(startX + static_cast<float>(i) * cfg::pipeSpacing);
    }
}

float Game::currentPipeSpeed() const {
    // Difficulty grows with score up to a cap
    const float speed = cfg::pipeBaseSpeed +
                        static_cast<float>(m_scoreSystem.score()) * cfg::pipeSpeedPerPoint;
    return std::min(speed, cfg::pipeMaxSpeed);
}

void Game::run() {
    // High-resolution timer + fixed-step physics for deterministic behavior
    const double freq = static_cast<double>(SDL_GetPerformanceFrequency());
    Uint64 last = SDL_GetPerformanceCounter();
    float accumulator = 0.0f;

    while (m_isRunning) {
        const Uint64 now = SDL_GetPerformanceCounter();
        float frameTime = static_cast<float>(static_cast<double>(now - last) / freq);
        last = now;
        frameTime = std::min(frameTime, cfg::maxFrameTime);

        processEvents();

        accumulator += frameTime;
        while (accumulator >= cfg::fixedStep) {
            update(cfg::fixedStep);
            accumulator -= cfg::fixedStep;
        }

        render();
    }
}

void Game::processEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_QUIT:
            m_isRunning = false;
            break;

        case SDL_EVENT_KEY_DOWN:
            if (event.key.repeat) break; // ignore held keys
            switch (event.key.key) {
            case SDLK_SPACE:
            case SDLK_UP:     handleAction(); break;
            case SDLK_P:      togglePause(); break;
            case SDLK_F11:    toggleFullscreen(); break;
            case SDLK_ESCAPE: m_isRunning = false; break;
            default: break;
            }
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (event.button.button == SDL_BUTTON_LEFT) handleAction();
            break;

        case SDL_EVENT_WINDOW_FOCUS_LOST:
            if (m_state == State::Playing) m_state = State::Paused;
            break;

        default: break;
        }
    }
}

void Game::handleAction() {
    switch (m_state) {
    case State::Ready:
        m_state = State::Playing;
        m_player.jump();
        break;
    case State::Playing:
        m_player.jump();
        break;
    case State::Paused:
        m_state = State::Playing;
        break;
    case State::GameOver:
        if (m_cooldown <= 0.0f) resetGame();
        break;
    }
}

void Game::togglePause() {
    if (m_state == State::Playing)      m_state = State::Paused;
    else if (m_state == State::Paused)  m_state = State::Playing;
}

void Game::toggleFullscreen() {
    const bool isFullscreen = (SDL_GetWindowFlags(m_window.get()) & SDL_WINDOW_FULLSCREEN) != 0;
    SDL_SetWindowFullscreen(m_window.get(), !isFullscreen);
}

void Game::gameOver() {
    m_state = State::GameOver;
    m_cooldown = cfg::restartCooldown;
    m_scoreSystem.commit(); // save high score once
}

void Game::update(float dt) {
    if (m_state == State::GameOver) {
        m_cooldown = std::max(0.0f, m_cooldown - dt);
        return;
    }
    if (m_state != State::Playing) return;

    m_player.update(dt);
    const SDL_FRect box = m_player.hitbox();

    // Ground collision
    if (box.y + box.h >= cfg::playHeight) {
        gameOver();
        return;
    }

    const float speed = currentPipeSpeed();

    for (Pipe& pipe : m_pipes) {
        pipe.update(dt, speed);

        if (!pipe.isPassed() && pipe.right() < box.x) {
            pipe.setPassed(true);
            m_scoreSystem.addPoint();
        }

        if (intersects(box, pipe.topRect()) || intersects(box, pipe.bottomRect())) {
            gameOver();
            return;
        }
    }

    // Recycle off-screen pipes behind the furthest one (no erase / realloc)
    for (Pipe& pipe : m_pipes) {
        if (!pipe.isOffscreen()) continue;

        float maxX = 0.0f;
        for (const Pipe& other : m_pipes) maxX = std::max(maxX, other.x());
        pipe.reset(maxX + cfg::pipeSpacing);
    }
}

void Game::render() {
    SDL_Renderer* r = m_renderer.get();
    const float cx = static_cast<float>(cfg::worldW) / 2.0f;
    const float cy = static_cast<float>(cfg::worldH) / 2.0f;

    // Clear the whole target (letterbox bars included)
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderClear(r);

    // Sky
    SDL_SetRenderDrawColor(r, 200, 250, 255, 255);
    SDL_FRect sky{0.0f, 0.0f, static_cast<float>(cfg::worldW), static_cast<float>(cfg::worldH)};
    SDL_RenderFillRect(r, &sky);

    for (const Pipe& pipe : m_pipes) pipe.render(r);

    // Ground
    SDL_SetRenderDrawColor(r, 222, 184, 135, 255);
    SDL_FRect ground{0.0f, cfg::playHeight, static_cast<float>(cfg::worldW), cfg::groundHeight};
    SDL_RenderFillRect(r, &ground);

    m_player.render(r);
    m_scoreSystem.render(r);

    if (m_state == State::Ready) {
        m_readyLabel.renderCentered(r, cx, cy + 100.0f);
    }

    if (m_state == State::Paused || m_state == State::GameOver) {
        SDL_SetRenderDrawColor(r, 0, 0, 0, 150);
        SDL_FRect overlay{0.0f, 0.0f, static_cast<float>(cfg::worldW),
                          static_cast<float>(cfg::worldH)};
        SDL_RenderFillRect(r, &overlay);

        if (m_state == State::Paused) {
            m_pausedLabel.renderCentered(r, cx, cy - 60.0f);
        } else {
            m_gameOverLabel.renderCentered(r, cx, cy - 60.0f);
            m_restartLabel.renderCentered(r, cx, cy + 20.0f);
        }
    }

    SDL_RenderPresent(r);
}

void Game::resetGame() {
    m_player.reset();
    spawnInitialPipes();
    m_scoreSystem.reset();
    m_state = State::Ready;
}
