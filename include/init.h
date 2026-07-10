#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "ScoreSystem.h"

class GameApp
{
    public:
        GameApp() : window(nullptr), renderer(nullptr), gameOverFont(nullptr),
        restartFont(nullptr), playerFrame1(nullptr), playerFrame2(nullptr),
        gameOverTexture(nullptr), restartTextTexture(nullptr) {}

        ~GameApp()
        {
            cleanup();
        }

        bool init(const char* title, int width, int height)
        {
            if(!SDL_Init(SDL_INIT_VIDEO))
            {
                SDL_Log("INIT Failed: %s", SDL_GetError());
                return false;
            }

        // initialize SDL_ttf
        if(!TTF_Init())
        {
            SDL_Log("TTF_Init Failed: %s", SDL_GetError());
            SDL_Quit();
            return false;
        }

        window = SDL_CreateWindow(title, width, height, 0);
                if (!window)
                {
                    SDL_Log("Window creation failed: %s", SDL_GetError());
                    return false;
                }

        renderer = SDL_CreateRenderer(window, nullptr);
                if (!renderer) {
                    SDL_Log("Renderer creation failed: %s", SDL_GetError());
                    return false;
                }

        return true;
        }

        void cleanup()
        {
            scoreSystem.cleanup();

            if (restartTextTexture)
                    {
                        SDL_DestroyTexture(restartTextTexture);
                        restartTextTexture = nullptr;
                    }

            if (gameOverTexture)
                    {
                        SDL_DestroyTexture(gameOverTexture);
                        gameOverTexture = nullptr;
                    }

            if (playerFrame1)
                    {
                        SDL_DestroyTexture(playerFrame1);
                        playerFrame1 = nullptr;
                    }

            if (playerFrame2)
                    {
                        SDL_DestroyTexture(playerFrame2);
                        playerFrame2 = nullptr;
                    }

            if (restartFont)
                    {
                        TTF_CloseFont(restartFont);
                        restartFont = nullptr;
                    }

            if (gameOverFont)
                    {
                        TTF_CloseFont(gameOverFont);
                        gameOverFont = nullptr;
                    }

            if (renderer)
                    {
                        SDL_DestroyRenderer(renderer);
                        renderer = nullptr;
                    }

            if (window)
                    {
                        SDL_DestroyWindow(window);
                        window = nullptr;
                    }

            TTF_Quit();
            SDL_Quit();
        }

        SDL_Window* getWindow() const { return window; }
        SDL_Renderer* getRenderer() const { return renderer; }
        // Player textures
            void setPlayerFrame1(SDL_Texture* texture) { playerFrame1 = texture; }

            void setPlayerFrame2(SDL_Texture* texture) { playerFrame2 = texture; }

            SDL_Texture* getPlayerFrame1() const { return playerFrame1; }

            SDL_Texture* getPlayerFrame2() const{ return playerFrame2; }

            // Fonts
            void setGameOverFont(TTF_Font* font) { gameOverFont = font; }

            void setRestartFont(TTF_Font* font) { restartFont = font; }

            TTF_Font* getGameOverFont() const { return gameOverFont; }

            TTF_Font* getRestartFont() const { return restartFont; }

            // Text textures
            void setGameOverTexture(SDL_Texture* texture) { gameOverTexture = texture; }

            void setRestartTextTexture(SDL_Texture* texture) { restartTextTexture = texture; }

            SDL_Texture* getGameOverTexture() const { return gameOverTexture; }

            SDL_Texture* getRestartTextTexture() const { return restartTextTexture; }

            // Score system
            ScoreSystem& getScoreSystem() { return scoreSystem; }

    private:
        SDL_Window* window;
        SDL_Renderer* renderer;
        TTF_Font* gameOverFont;
        TTF_Font* restartFont;
        SDL_Texture* playerFrame1;
        SDL_Texture* playerFrame2;
        SDL_Texture* gameOverTexture;
        SDL_Texture* restartTextTexture;
        ScoreSystem scoreSystem;
};
