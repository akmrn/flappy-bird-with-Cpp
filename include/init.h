#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

inline int init()
{
    // initialize SDL
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("INIT Failed: %s", SDL_GetError());
        return 1;
    }

        // initialize SDL_ttf
    if(!TTF_Init())
    {
        SDL_Log("TTF_Init Failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    
    return 0;
}

inline void quit()
{
    TTF_Quit();
    SDL_Quit();
}