#pragma once
#include <SDL3/SDL.h>

class Pipe
{
    public:
        Pipe(
            float startX,
            float width,
            float gapHeight,
            float speedX,
            float windowH,
            float minEdgePadding = 50.0f);
        
        // New function to reset the pipe without having to create a new object
        void reset(float startX, float windowH);

        // Every frame is called
        void update(float deltaSeconds, float windowW, float windowH);
        
        // drawing pipes
        void render(SDL_Renderer* renderer, float windowH) const;
        
        // pipes rect
        SDL_FRect topRect() const;
        SDL_FRect bottomRect(float windowH) const;

        float x() const { return m_x; }
        float width() const { return m_width; }
        float gapHeight() const { return m_gapHeight; }
        float topHeight() const { return m_topHeight; }
        bool m_passed = false;

    private:
        void respawn(float windowW, float windowH);
        void randomizeTopHeight(float windowH);

    private:
        float m_x = 0.0f;
        float m_width = 0.0f;
        float m_topHeight = 0.0f;
        float m_gapHeight = 0.0f;
        float m_speedX = 0.0f;
        
        // Minimum distance from top/bottom edge to prevent pipes from becoming impossible to pass through
        float m_minEdgePadding = 50.0f;
    
};