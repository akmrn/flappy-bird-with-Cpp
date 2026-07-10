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
- score system                                          [X]
- game Sound                                            [ ]
- game Menu                                             [ ]
*/

#include <ctime>
#include <cstdlib>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "constants.h"
#include "Pipe.h"
#include "init.h"
#include "ScoreSystem.h"

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

    GameApp app;

    if (!app.init("Flappy bird", win_W, win_H))
    {
        return 1;
    }

    // creating renderer
    SDL_Renderer* renderer = app.getRenderer();

    // VSync
    SDL_SetRenderVSync(renderer, 1);
    if (!SDL_SetRenderVSync(renderer, 1))
    {
        SDL_Log("Failed to enable VSync: %s", SDL_GetError());
    }

    //======================PLAYER=========================================
    // load player texture
    app.setPlayerFrame1(IMG_LoadTexture(renderer, "assets/bird-1.png"));
    app.setPlayerFrame2(IMG_LoadTexture(renderer, "assets/bird-2.png"));
    SDL_Texture* playerFrame1 = app.getPlayerFrame1();
    SDL_Texture* playerFrame2 = app.getPlayerFrame2();

    if (!playerFrame1 || !playerFrame2)
    {
        SDL_Log("Failed to load player image: %s", SDL_GetError());
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
    app.setGameOverFont(TTF_OpenFont("assets/Bitcount.ttf", 72.0f));
    app.setRestartFont(TTF_OpenFont("assets/Anton.ttf", 22.0f));

    TTF_Font* gameOverFont = app.getGameOverFont();
    TTF_Font* restartFont = app.getRestartFont();
    if(!gameOverFont || !restartFont)
    {
        SDL_Log("failed to load font! check path: %s", SDL_GetError());
            return 1;
    }

    SDL_FRect gameOverRect{};
    SDL_FRect restartTextRect{};

    SDL_Color whiteColor = {255, 255, 255, 255};

    // Create the GameOver text in advance (to avoid creating and destroying the texture every frame)
    if(gameOverFont)
    {
        SDL_Surface* surface = TTF_RenderText_Blended(gameOverFont, "GAME OVER", 0, whiteColor);
        if(!surface)
        {
            SDL_Log("Failed to render game over text: %s", SDL_GetError());
        } else
        {
            app.setGameOverTexture(SDL_CreateTextureFromSurface(renderer, surface));
            if(!app.getGameOverTexture())
                    {
                        SDL_Log("Failed to create game over texture: %s", SDL_GetError());
                    } else
                    {
                        //textBox
                        gameOverRect.w = (float)surface->w;
                        gameOverRect.h = (float)surface->h;

                        //Centering text
                        gameOverRect.x = ((float)win_W - gameOverRect.w) / 2.0f;
                        gameOverRect.y = 210.0f;
                    }
            SDL_DestroySurface(surface);
        }
    }

    if(restartFont)
    {
        SDL_Surface* surface = TTF_RenderText_Blended(restartFont, "Press Space or Left Click to Restart", 0, whiteColor);
        if(!surface)
        {
            SDL_Log("Failed to render restart text: %s", SDL_GetError());
        } else
        {
            app.setRestartTextTexture(SDL_CreateTextureFromSurface(renderer, surface));
            if(!app.getRestartTextTexture())
            {
                SDL_Log("Failed to create restart texture: %s", SDL_GetError());
            } else
            {
                //textBox
                restartTextRect.w = (float)surface->w;
                restartTextRect.h = (float)surface->h;

                //Centering text
                restartTextRect.x = ((float)win_W - restartTextRect.w) / 2.0f;
                restartTextRect.y = (gameOverRect.y + gameOverRect.h) + 20.0f;
            }

            SDL_DestroySurface(surface);
        }
    }
    //=====================================================================

    //======================OBSTACLES======================================
    // obstacles setup
    Pipe pipe((float)win_W, 80.0f, 200.0f, -200.0f, (float)win_H);
    //=====================================================================

    // init score system
    ScoreSystem& scoreSystem = app.getScoreSystem();
    if (!scoreSystem.init(renderer, "assets/Yuyu.ttf", 28.0f, "highscore.txt"))
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

            if(state == GameState::Playing)
            {
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
            } else // GameState::GameOver
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
            if (!pipe.isPassed() && (pipe.topRect().x + pipe.topRect().w) < dst.x)
            {
                pipe.setPassed(true);
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

            if (app.getGameOverTexture())
            {
                SDL_RenderTexture(renderer, app.getGameOverTexture(), nullptr, &gameOverRect);
            }

            if (app.getRestartTextTexture())
            {
                SDL_RenderTexture(renderer, app.getRestartTextTexture(), nullptr, &restartTextRect);
            }
        }

        SDL_RenderPresent(renderer);
    }

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
