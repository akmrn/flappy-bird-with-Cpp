#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "scoreSystem.h"

inline bool init()
{
    // initialize SDL
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

    return true;
}

inline void quit(SDL_Window * window, SDL_Renderer * renderer, TTF_Font* gameOverFont, TTF_Font* restartFont, SDL_Texture * playerFrame1, SDL_Texture * playerFrame2, SDL_Texture* gameOverTexture, SDL_Texture* restartTextTexture, ScoreSystem scoreSystem)
{
    scoreSystem.cleanup();

    if (restartTextTexture)
        SDL_DestroyTexture(restartTextTexture);

    if (gameOverTexture)
        SDL_DestroyTexture(gameOverTexture);

    if (playerFrame1)
        SDL_DestroyTexture(playerFrame1);
    if (playerFrame2)
        SDL_DestroyTexture(playerFrame2);

    if (restartFont)
        TTF_CloseFont(restartFont);

    if (gameOverFont)
        TTF_CloseFont(gameOverFont);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}
