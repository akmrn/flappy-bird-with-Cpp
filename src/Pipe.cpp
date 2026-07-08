#include "Pipe.h"
#include <algorithm> // std::clamp
#include <cstdlib>   // rand, RAND_MAX

static float randomRange(float minVal, float maxVal)
{
    float t = (float)rand() / (float) RAND_MAX; // [0,1]
    return minVal + t * (maxVal - minVal);
}

Pipe::Pipe(float startX, float width, float gapHeight, float speedX, float windowH, float minEdgePadding):
    m_x(startX),
    m_width(width),
    m_gapHeight(gapHeight),
    m_speedX(speedX),
    m_minEdgePadding(minEdgePadding)
    {
        m_passed = false;
        randomizeTopHeight(windowH);
    }

    void Pipe::reset(float startX, float windowH)
    {
        m_x = startX;
        m_passed = false;
        randomizeTopHeight(windowH);
    }

    void Pipe::update(float deltaSeconds, float windowW, float windowH)
    {
        m_x += m_speedX * deltaSeconds;

        if(m_x + m_width < 0.0f)
        {
            respawn(windowW, windowH);
        }
    }

    void Pipe::render(SDL_Renderer* renderer, float windowH) const
    {
        SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255);

        SDL_FRect top = topRect();
        SDL_FRect bot = bottomRect(windowH);

        SDL_RenderFillRect(renderer, &top);
        SDL_RenderFillRect(renderer, &bot);
    }

    SDL_FRect Pipe::topRect() const
    {
        return SDL_FRect { m_x, 0.0f, m_width, m_topHeight };
    }

    SDL_FRect Pipe::bottomRect(float windowH) const
    {
        float bottomY = m_topHeight + m_gapHeight;
        float bottomH = windowH - bottomY;
        return SDL_FRect{ m_x, bottomY, m_width, bottomH };
    }

    void Pipe::respawn(float windowW, float windowH)
    {
        m_x = windowW;
        randomizeTopHeight(windowH);
        m_passed = false;
    }

    void Pipe::randomizeTopHeight(float windowH)
    {
        float minTop = m_minEdgePadding;
        float maxTop = windowH - m_gapHeight - m_minEdgePadding;

        if(maxTop < minTop)
        {
            maxTop = minTop;
        }

        m_topHeight = randomRange(minTop, maxTop);
        m_topHeight = std::clamp(m_topHeight, minTop, maxTop);
    }