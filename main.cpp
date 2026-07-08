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
        - animation system                              [X]
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
        - randomize top pipe height                     [X]
    - render
        - draw top pipe                                 [X]
        - draw bottom pipe                              [X]
    - cleanup
        - destroy obstacle textures                     [ ]
----------------------------------------------------------------------------- 
- player has collision with obstacle                    [X] COLLISION
- when player collides with obstacle -> lose            [X] 
- score system                                          [ ] 
- game Sound                                            [ ]
- game Menu                                             [ ]
*/

#include <ctime>
#include <cstdlib>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "include/constants.h"
#include "include/Pipe.h"
#include "include/init.h"
#include "scoreSystem.h"

enum class GameState
{
    Playing,
    GameOver
};

bool intersects(const SDL_FRect& a, const SDL_FRect& b)
{
    return (a.x < b.x + b.w &&
            a.x + a.w > b.x &&
            a.y < b.y + b.h &&
            a.y + a.h > b.y);
}

void restartGame(Pipe& pipe, SDL_FRect& dst, float& velocityY, ScoreSystem& scoreSystem, SDL_Renderer* renderer);

int main(int argc, char** argv)
{
    (void)argc; (void)argv;

    // seed RNG (once)
    srand((unsigned int)time(nullptr));

    init();
    
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
    
    //======================PLAYER=========================================
    // load player texture
    SDL_Texture * playerFrame1 = IMG_LoadTexture(renderer, "assets/bird-1.png"); 
    SDL_Texture * playerFrame2 = IMG_LoadTexture(renderer, "assets/bird-2.png");

    if (playerFrame1 == nullptr || playerFrame2 == nullptr) // error handeling
    {
        SDL_Log("Failed to load player image: %s", SDL_GetError());
        if (playerFrame1) SDL_DestroyTexture(playerFrame1);
        if (playerFrame2) SDL_DestroyTexture(playerFrame2);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // A variable that holds the current texture to render at each frame.
    SDL_Texture* activePlayerTexture = playerFrame1;

    // animation variables
    float animTimer = 0.0f;
    constexpr float animSpeed = 0.15f;
    int currentFrameIndex = 0;

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
    //=====================================================================

    //======================FONT===========================================
    // load font
    TTF_Font* gameOverFont = TTF_OpenFont("assets/Bitcount.ttf", 72.0f);
    TTF_Font* restartFont = TTF_OpenFont("assets/Anton.ttf", 22.0f);
    if(!gameOverFont || !restartFont)
    {
        SDL_Log("failed to load font! check path: %s", SDL_GetError());
    }
    SDL_Texture* gameOverTexture = nullptr;
    SDL_Texture* restartTextTexture = nullptr;

    SDL_FRect gameOverRect{};
    SDL_FRect restartTextRect{};

    SDL_Color whiteColor = {255, 255, 255, 255};

    // Create the GameOver text in advance (to avoid creating and destroying the texture every frame)
    if(gameOverFont)
    {
        SDL_Surface* surface = TTF_RenderText_Blended(gameOverFont, "GAME OVER", 0, whiteColor);
        if(surface)
        {
            gameOverTexture = SDL_CreateTextureFromSurface(renderer, surface);

            //textBox
            gameOverRect.w = (float)surface->w;
            gameOverRect.h = (float)surface->h;

            //Centering text
            gameOverRect.x = ((float)win_W - gameOverRect.w) / 2.0f;
            gameOverRect.y = 210.0f;

            SDL_DestroySurface(surface);
        }
    }

    if(restartFont)
    {
        SDL_Surface* surface = TTF_RenderText_Blended(restartFont, "Press Space or Left Click to Restart", 0, whiteColor);
        if(surface)
        {
            restartTextTexture = SDL_CreateTextureFromSurface(renderer, surface);

            //textBox
            restartTextRect.w = (float)surface->w;
            restartTextRect.h = (float)surface->h;

            //Centering text
            restartTextRect.x = ((float)win_W - restartTextRect.w) / 2.0f;
            restartTextRect.y = (gameOverRect.y + gameOverRect.h) + 20.0f;

            SDL_DestroySurface(surface);
        }
    }
    //=====================================================================

    //======================OBSTACLES======================================
    // obstacles setup
    Pipe pipe((float)win_W, 80.0f, 200.0f, -200.0f, (float)win_H);
    //=====================================================================

    // init score system 
    ScoreSystem scoreSystem;
    if(!scoreSystem.init(renderer, "assets/Yuyu.ttf", 28.0f, "highscore.txt"))
    {
        SDL_Log("ScoreSystem INIT Error: %s", SDL_GetError());
        return 1;
    }

    // game running flag
    SDL_Event event;
    bool isRun = true;
    bool jumpRequested = false; // jump request
    GameState state = GameState::Playing;
    bool restartRequested = false;
    
    // defining delta and scheduling variables
    Uint64 lastTime = SDL_GetTicks();
    float delta = 0.0f;

    // game loop
    while(isRun)
    {   
        // Calculate the actual delta time (convert milliseconds to seconds)
        Uint64 currentTime = SDL_GetTicks();
        delta = (float)(currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        // To prevent sudden physics jumps in case of system lag
        if(delta > 0.1f) delta = 0.1f;

        // event loop
        while(SDL_PollEvent(&event))
        {
            if(event.type == SDL_EVENT_QUIT)
            {
                isRun = false;
            }

            // space requests jump
            if(event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_SPACE)
            {
                jumpRequested = true;
            }

            // left click requests jump
            if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT)
            {
                jumpRequested = true;
            }

            if(state == GameState::GameOver)
            {
                if(event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_SPACE)
                    restartRequested = true;

                if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT)
                    restartRequested = true;
            }

        }

        bool hitTop = false, hitBottom = false, hitGround = false, hitCeil = false;
        if(state == GameState::Playing)
        {
            // jump action
            if (jumpRequested)
            {
                velocityY = -350.0f;
                jumpRequested = false;
            }

            // physics
            velocityY += gravity * delta;
            dst.y += velocityY * delta;
            
            // player animation system
            animTimer += delta;
            if(animTimer >= animSpeed)
            {
                animTimer = 0.0f;
                currentFrameIndex = (currentFrameIndex + 1) % 2; // switch between 0 and 1

                if(currentFrameIndex == 0) activePlayerTexture = playerFrame1;
                else activePlayerTexture = playerFrame2;
            }

            // update pipe
            pipe.update(delta, (float)win_W, (float)win_H);

            // score logic
            if (!pipe.m_passed && (pipe.topRect().x + pipe.topRect().w) < dst.x)
            {
                pipe.m_passed = true;
                scoreSystem.addPoint(renderer);
            }

            // collision
            SDL_FRect rectTop = pipe.topRect();
            SDL_FRect rectBottom = pipe.bottomRect((float)win_H);

            hitTop    = intersects(dst, rectTop);
            hitBottom = intersects(dst, rectBottom);
            hitGround = (dst.y + dst.h >= (float)win_H);
            hitCeil   = (dst.y <= 0.0f);

            if (hitTop || hitBottom || hitGround || hitCeil)
                state = GameState::GameOver;
        }

        if(restartRequested)
        {
            restartGame(pipe, dst, velocityY, scoreSystem, renderer);
            state = GameState::Playing;
            restartRequested = false;
            jumpRequested = false; // Be sure to reset the jump flag so it doesn't fly on restart

            // reset the animation
            animTimer = 0.0f;
            currentFrameIndex = 0;
            activePlayerTexture = playerFrame1;
        }

        // render
        SDL_SetRenderDrawColor(renderer, 200, 250, 255, 255);
        SDL_RenderClear(renderer);

        pipe.render(renderer, (float)win_H);

        // draw the player textur
        if(activePlayerTexture)
            SDL_RenderTexture(renderer, activePlayerTexture, nullptr, &dst);

        scoreSystem.render(renderer);

        // game over
        if (state == GameState::GameOver)
        {   
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); // A semi-transparent black background for better text readability
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
            SDL_FRect overlay{0.0f, 0.0f, (float)win_W, (float)win_H};
            SDL_RenderFillRect(renderer, &overlay);

            if (gameOverTexture)
            {
                SDL_RenderTexture(renderer, gameOverTexture, nullptr, &gameOverRect);
            }

            if (restartTextTexture)
            {
                SDL_RenderTexture(renderer, restartTextTexture, nullptr, &restartTextRect);
            }
        }
        
        SDL_RenderPresent(renderer);
    }

    // cleanup
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
    quit();
    return 0;
}

void restartGame(Pipe& pipe, SDL_FRect& dst, float& velocityY, ScoreSystem& scoreSystem, SDL_Renderer* renderer)
{
    dst.x = 150.0f;
    dst.y = 150.0f;
    velocityY = 0.0f;

    scoreSystem.reset(renderer); //reset score
    pipe.reset((float)win_W, (float)win_H);
}