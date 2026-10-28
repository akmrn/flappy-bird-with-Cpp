#pragma once

#include <SDL3/SDL.h>

// A pair of pipes (top + bottom) with a gap between them.
// Pure logic and drawing; textures are borrowed and never owned here.
class Pipe
{
public:
    // Creates a pipe at startX with a random gap position
    explicit Pipe(float startX);

    // Moves the pipe back to a new X and picks a new random gap position
    void reset(float startX);

    // Scrolls the pipe left by speed * dt
    void update(float dt, float speed);

    // Draws both pipes. 'tex' may be nullptr (green rectangles are used then)
    void render(SDL_Renderer* renderer, SDL_Texture* tex) const;

    // Collision / layout rectangles
    SDL_FRect topRect() const;
    SDL_FRect bottomRect() const;

    float x() const { return m_x; }
    float right() const;
    bool isOffscreen() const;

    bool isPassed() const { return m_passed; }
    void setPassed(bool passed) { m_passed = passed; }

private:
    void randomizeTopHeight();

    float m_x = 0.0f;
    float m_topHeight = 0.0f;
    bool m_passed = false;
};
