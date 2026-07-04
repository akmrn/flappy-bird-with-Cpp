/*
- system                                               SYSTEM
    - initialize SDL                                   [X]
    - create window                                     [X]
    - create renderer                                   [X]
    - VSync (synchronization with monitor refresh rate) [X]
    - game running flag                                 [X]
    - game loop                                         [X]
        - event loop                                    [X]
-----------------------------------------------------------------------------
- player                                               OBJECT
    - data
        - position (x, y)                               [X]
        - size (w, h)                                   [X]
        - velocityY                                     [X]
        - texture                                       [X]
    - setup
        - load the player image                         [X]
    - render
        - draw the image                                [X]
    - cleanup
        - destroy player texture                        [X]
-----------------------------------------------------------------------------
- input                                                INPUT
    - keyboard input                                    [X]
        - detect space key press                        [X]
    - mouse input                                       [X]
        - detect left mouse click                       [X]
-----------------------------------------------------------------------------
- jump trigger                                         ACTION
    - jump request bool                                 [X]
    - space requests jump                               [X]
    - left click requests jump                          [X]
-----------------------------------------------------------------------------
- jump gives player upward velocity                     [X] PHYSICS
- left click/space makes player jump                    [X] MOVEMENTS
-----------------------------------------------------------------------------
- obstacles(pipes)                                     OBJECT
    - data
        - position (x, y)                               [X]
        - size
            - weidth                                    [X]
            - top height                                [X]
            - gap height                                [X]
        - velocityX (move left)                         [X]
        - texture                                       [ ]
    - setup
        - load the obstacle image                       [X]
    - update
        - move left each frame                          [X]
        - respawn when off-screen                       [X]
        - randomize top pipe height                     [ ]
    - render
        - draw top pipe                                 [X]
        - draw bottom pipe                              [X]
    - cleanup
        - destroy obstacle textures                     [ ]
----------------------------------------------------------------------------- 
- player has collision with obstacle                    [ ] COLLISION
- when player collides with obstacle -> lose            [ ] RULE
- game Sound                                            [ ]
- game Menu                                             [ ]
*/

#include <ctime>
#include <cstdlib>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include "include/constants.h"
#include "include/Pipe.h"


int main()
{
    // seeding random
    srand((unsigned int)time(nullptr));

    // initialize SDL
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("INIT Failed: %s", SDL_GetError());
        return 1;
    }

    // create window 
    SDL_Window * window = SDL_CreateWindow("Flappy bird", win_W, win_H, 0);
    if(!window)
    {
        SDL_Log("Window Failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // creating renderer
    SDL_Renderer * renderer = SDL_CreateRenderer(window, nullptr);
     if(!renderer)
    {
        SDL_Log("RENDERER Failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // VSync
    SDL_SetRenderVSync(renderer, 1);
    
    // load image
    SDL_Texture * texture = IMG_LoadTexture(renderer, "assets/plane.png");               
    if (texture == nullptr) // error handeling
    {
        SDL_Log("Failed to load player image: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // player p and s define 
    SDL_FRect dst;
    // position
    dst.x = 150;
    dst.y = 150;
    // size
    dst.w = 88;
    dst.h = 73;
    // player volacity
    float velocityY = 0.0f;

    // obstacles setup
    Pipe pipe((float)win_W, 80.0f, 170.0f, -200.0f);

    // game running flag
    SDL_Event event;
    bool isRun = true;

     // jump request
    bool jumpRequested = false;

    // game loop
    while(isRun)
    {
        // event loop
        while(SDL_PollEvent(&event))
        {
            if(event.type == SDL_EVENT_QUIT)
            {
                isRun = false;
            }

            // space requests jump
            if(event.type == SDL_EVENT_KEY_DOWN)
            {
                if(event.key.key == SDLK_SPACE)  jumpRequested = true;
            }

            // left click requests jump
            if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
            {
                if(event.button.button == SDL_BUTTON_LEFT)  jumpRequested = true;
            }
        }

        if (jumpRequested)
        {
            velocityY = -350.0f;
            jumpRequested = false;
        }
        
        // velocityY
        velocityY += gravity * delta;

        // updating y => JUMP
        dst.y += velocityY * delta;
       
        // cleaning the screen
        SDL_SetRenderDrawColor(renderer, 200, 250, 255, 255);
        SDL_RenderClear(renderer);

        pipe.update(delta, (float)win_W, (float)win_H);

        pipe.render(renderer, (float)win_H);

        // draw the player textur
        SDL_RenderTexture(renderer, texture, nullptr, &dst);

        SDL_RenderPresent(renderer);
    }
    // cleanup
    SDL_DestroyTexture(texture); // destroy player texture
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}