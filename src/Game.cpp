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

    // Axis-aligned rectangle overlap test
    bool intersects(const SDL_FRect& a, const SDL_FRect& b) {
        return a.x < b.x + b.w && a.x + a.w > b.x &&
               a.y < b.y + b.h && a.y + a.h > b.y;
    }

    // Color used to clear the window, including the letterbox bars.
    // Set it close to your background image's dominant color so bars are invisible.
    constexpr SDL_Color letterboxColor{82, 102, 106, 255};
}

Game::Game()
    : m_window(createWindow()),
      m_renderer(createRenderer(m_window.get())),
      m_textures(m_renderer.get()),
      m_pipeTex(m_textures.tryGet(skin::pipe)),
      m_backgroundTex(m_textures.tryGet(skin::background)),
      m_groundTex(m_textures.tryGet(skin::ground)),
      m_scoreBoxTex(m_textures.tryGet(skin::scoreBox)),
      m_background(m_backgroundTex, 0.3f),
      m_ground(m_groundTex),
      m_scoreSystem(m_renderer.get(), assets::loadFont("Yuyu.ttf", 22.0f),
                    assets::savePath("highscore.txt"), m_scoreBoxTex),
      m_player(m_textures.get(skin::bird1), m_textures.get(skin::bird2)),
      m_titleFont(assets::loadFont("Bitcount.ttf", 72.0f)),
      m_hintFont(assets::loadFont("Anton.ttf", 22.0f))
{
    // Static labels are rendered once and cached
    m_gameOverLabel.setText(m_renderer.get(), m_titleFont.get(), "GAME OVER", cfg::red);
    m_restartLabel.setText(m_renderer.get(), m_hintFont.get(),
                           "Press Space or Left Click to Restart", cfg::white);
    m_readyLabel.setText(m_renderer.get(), m_hintFont.get(),
                         "Press Space or Click to Start  (P: pause, F11: fullscreen)", cfg::black);
    m_pausedLabel.setText(m_renderer.get(), m_titleFont.get(), "PAUSED", cfg::white);

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
        frameTime = std::min(frameTime, cfg::maxFrameTime); // avoid spiral of death

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
            // Auto-pause when the window loses focus
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
        // Everything is frozen after a crash; only the restart cooldown ticks
        m_cooldown = std::max(0.0f, m_cooldown - dt);
        return;
    }
    if (m_state == State::Ready) {
        // Slow idle scroll so the world feels alive before the first jump
        m_ground.update(dt, cfg::pipeBaseSpeed);
        m_background.update(dt, cfg::pipeBaseSpeed);
        return;
    }
    if (m_state != State::Playing) return; // Paused: nothing moves

    const float speed = currentPipeSpeed();

    // Ground scrolls at exactly the same speed as the pipes
    m_ground.update(dt, speed);
    m_background.update(dt, speed); // parallax layer, slower than the pipes (factor set in Background)

    m_player.update(dt);
    const SDL_FRect box = m_player.hitbox();

    // Ground collision
    if (box.y + box.h >= cfg::playHeight) {
        gameOver();
        return;
    }

    for (Pipe& pipe : m_pipes) {
        pipe.update(dt, speed);

        // Score when the pipe has fully passed the bird
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
    const float worldW = static_cast<float>(cfg::worldW);
    const float worldH = static_cast<float>(cfg::worldH);
    const float cx = worldW / 2.0f;
    const float cy = worldH / 2.0f;

    SDL_SetRenderDrawColor(r, letterboxColor.r, letterboxColor.g, letterboxColor.b, 255);
    SDL_RenderClear(r);

    // Background, pipes, ground, then bird
    m_background.render(r);

    for (const Pipe& pipe : m_pipes) pipe.render(r, m_pipeTex);

    m_ground.render(r);

    m_player.render(r);
    m_scoreSystem.render(r);

    if (m_state == State::Ready) {
        m_readyLabel.renderCentered(r, cx, cy + 100.0f);
    }

    if (m_state == State::Paused || m_state == State::GameOver) {
        SDL_SetRenderDrawColor(r, 0, 0, 0, 150);
        SDL_FRect overlay{0.0f, 0.0f, worldW, worldH};
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
    m_ground.reset();
    m_background.reset();
    spawnInitialPipes();
    m_scoreSystem.reset();
    m_state = State::Ready;
}
