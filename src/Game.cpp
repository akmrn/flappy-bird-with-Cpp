#include "Game.hpp"
#include <SDL3_image/SDL_image.h>
#include <stdexcept>
#include <iostream>
#include <cstddef>

namespace {
    raii::Window createWindow() {
        SDL_Window* window = SDL_CreateWindow(
            "Flappy Bird",
            win_W,
            win_H,
            SDL_WINDOW_RESIZABLE
        );
        if (!window) {
            throw std::runtime_error(std::string("Failed to create SDL_Window: ") + SDL_GetError());
        }
        return raii::Window(window);
    }

    raii::Renderer createRenderer(SDL_Window* window) {
        SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
        if (!renderer) {
            throw std::runtime_error(std::string("Failed to create SDL_Renderer: ") + SDL_GetError());
        }
        SDL_SetRenderVSync(renderer, 1);

        // Disable fixed logical presentation to allow true full-window dynamic scaling
        SDL_SetRenderLogicalPresentation(
            renderer,
            0,
            0,
            SDL_LOGICAL_PRESENTATION_DISABLED
        );

        return raii::Renderer(renderer);
    }
}

Game::Game()
    : m_sdlContext(),
      m_ttfContext(),
      m_window(createWindow()),
      m_renderer(createRenderer(m_window.get())),
      m_scoreSystem(m_renderer.get(), "assets/Yuyu.ttf", 28.0f, "highscore.txt"),
      m_player(m_renderer.get())
{
    // Load fonts for Game Over HUD
    TTF_Font* goFont = TTF_OpenFont("assets/Bitcount.ttf", 72.0f);
    TTF_Font* rFont  = TTF_OpenFont("assets/Anton.ttf", 22.0f);
    if (!goFont || !rFont) {
        throw std::runtime_error("Failed to load game over / restart fonts!");
    }
    m_gameOverFont.reset(goFont);
    m_restartFont.reset(rFont);

    // Spawn multiple pipes spaced horizontally (6 pipes with comfortable spacing)
    const int initialPipeCount = 6;
    const float pipeSpacing    = 380.0f;
    const float gapSize        = 240.0f;
    const float startOffsetX   = static_cast<float>(win_W) + 100.0f;

    for (int i = 0; i < initialPipeCount; ++i) {
        float spawnX = startOffsetX + (i * pipeSpacing);
        m_pipes.emplace_back(
            spawnX,
            80.0f,
            gapSize,
            -200.0f,
            static_cast<float>(win_H)
        );
    }
}

void Game::run() {
    Uint64 lastTime = SDL_GetTicks();

    while (m_isRunning) {
        Uint64 currentTime = SDL_GetTicks();
        float deltaTime = static_cast<float>(currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        if (deltaTime > 0.1f) {
            deltaTime = 0.1f;
        }

        processEvents();
        update(deltaTime);
        render();
    }
}

void Game::processEvents() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            m_isRunning = false;
        } else if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            // Update win_W and win_H dynamically when window is resized or maximized
            int w = 0;
            int h = 0;
            SDL_GetWindowSize(m_window.get(), &w, &h);
            win_W = w;
            win_H = h;
        } else if (event.type == SDL_EVENT_KEY_DOWN) {
            if (event.key.key == SDLK_SPACE) {
                if (m_isGameOver) {
                    resetGame();
                } else {
                    m_player.jump();
                }
            }
        } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                if (m_isGameOver) {
                    resetGame();
                } else {
                    m_player.jump();
                }
            }
        }
    }
}

void Game::update(float deltaTime) {
    if (m_isGameOver) {
        return;
    }

    m_player.update(deltaTime);

    SDL_FRect playerBounds = m_player.getBounds();

    if (playerBounds.y <= 0.0f ||
        playerBounds.y + playerBounds.h >= static_cast<float>(win_H)) {
        m_isGameOver = true;
    }

    for (std::size_t i = 0; i < m_pipes.size();) {
        Pipe& pipe = m_pipes[i];

        pipe.update(
            deltaTime,
            static_cast<float>(win_W),
            static_cast<float>(win_H)
        );

        if (!pipe.isPassed() &&
            pipe.x() + pipe.width() < playerBounds.x) {
            pipe.setPassed(true);
            m_scoreSystem.addPoint();
        }

        SDL_FRect topRect = pipe.topRect();
        SDL_FRect bottomRect = pipe.bottomRect(static_cast<float>(win_H));

        auto checkIntersect = [](const SDL_FRect& a, const SDL_FRect& b) {
            return (
                a.x < b.x + b.w &&
                a.x + a.w > b.x &&
                a.y < b.y + b.h &&
                a.y + a.h > b.y
            );
        };

        if (checkIntersect(playerBounds, topRect) ||
            checkIntersect(playerBounds, bottomRect)) {
            m_isGameOver = true;
        }

        // Properly recycle and space out pipes when they exit the screen
        if (pipe.x() + pipe.width() < 0.0f) {
            m_pipes.erase(m_pipes.begin() + static_cast<std::ptrdiff_t>(i));

            // Find the furthest pipe's X coordinate to maintain consistent spacing
            float maxX = static_cast<float>(win_W);

            for (const auto& remainingPipe : m_pipes) {
                if (remainingPipe.x() > maxX) {
                    maxX = remainingPipe.x();
                }
            }

            const float pipeSpacing = 380.0f;
            const float gapSize     = 240.0f;

            m_pipes.emplace_back(
                maxX + pipeSpacing,
                80.0f,
                gapSize,
                -200.0f,
                static_cast<float>(win_H)
            );
        } else {
            ++i;
        }
    }
}

void Game::render() {
    SDL_SetRenderDrawColor(m_renderer.get(), 200, 250, 255, 255);
    SDL_RenderClear(m_renderer.get());

    for (auto& pipe : m_pipes) {
        pipe.render(m_renderer.get(), static_cast<float>(win_H));
    }

    m_player.render(m_renderer.get());
    m_scoreSystem.render(m_renderer.get());

    if (m_isGameOver) {
        SDL_SetRenderDrawBlendMode(m_renderer.get(), SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(m_renderer.get(), 0, 0, 0, 150);

        // Fullscreen dynamic overlay based on live win_W and win_H
        SDL_FRect overlay{
            0.0f,
            0.0f,
            static_cast<float>(win_W),
            static_cast<float>(win_H)
        };
        SDL_RenderFillRect(m_renderer.get(), &overlay);

        if (!m_gameOverTex.get()) {
            SDL_Surface* goSurf = TTF_RenderText_Blended(
                m_gameOverFont.get(),
                "GAME OVER",
                0,
                {255, 50, 50, 255}
            );

            if (goSurf) {
                m_gameOverTexW = goSurf->w;
                m_gameOverTexH = goSurf->h;
                m_gameOverTex.reset(
                    SDL_CreateTextureFromSurface(m_renderer.get(), goSurf)
                );
                SDL_DestroySurface(goSurf);
            }
        }

        if (!m_restartTex.get()) {
            SDL_Surface* rSurf = TTF_RenderText_Blended(
                m_restartFont.get(),
                "Press Space or Left Click to Restart",
                0,
                {255, 255, 255, 255}
            );

            if (rSurf) {
                m_restartTexW = rSurf->w;
                m_restartTexH = rSurf->h;
                m_restartTex.reset(
                    SDL_CreateTextureFromSurface(m_renderer.get(), rSurf)
                );
                SDL_DestroySurface(rSurf);
            }
        }

        // Perfectly centered Game Over HUD elements
        if (m_gameOverTex.get()) {
            SDL_FRect dstGo{
                (static_cast<float>(win_W) -
                 static_cast<float>(m_gameOverTexW)) / 2.0f,
                (static_cast<float>(win_H) / 2.0f) - 60.0f,
                static_cast<float>(m_gameOverTexW),
                static_cast<float>(m_gameOverTexH)
            };

            SDL_RenderTexture(
                m_renderer.get(),
                m_gameOverTex.get(),
                nullptr,
                &dstGo
            );
        }

        if (m_restartTex.get()) {
            SDL_FRect dstRestart{
                (static_cast<float>(win_W) -
                 static_cast<float>(m_restartTexW)) / 2.0f,
                (static_cast<float>(win_H) / 2.0f) + 20.0f,
                static_cast<float>(m_restartTexW),
                static_cast<float>(m_restartTexH)
            };

            SDL_RenderTexture(
                m_renderer.get(),
                m_restartTex.get(),
                nullptr,
                &dstRestart
            );
        }
    }

    SDL_RenderPresent(m_renderer.get());
}

void Game::resetGame() {
    m_player.reset();
    m_pipes.clear();

    const int initialPipeCount = 6;
    const float pipeSpacing    = 380.0f;
    const float gapSize        = 240.0f;
    const float startOffsetX   = static_cast<float>(win_W) + 100.0f;

    for (int i = 0; i < initialPipeCount; ++i) {
        float spawnX = startOffsetX + (i * pipeSpacing);

        m_pipes.emplace_back(
            spawnX,
            80.0f,
            gapSize,
            -200.0f,
            static_cast<float>(win_H)
        );
    }

    m_scoreSystem.reset();
    m_isGameOver = false;
}
