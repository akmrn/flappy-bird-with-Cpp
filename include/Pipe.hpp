#pragma once

#include <SDL3/SDL.h>

// One pair of pipes (top + bottom) with a gap between them.
class Pipe {
public:
    explicit Pipe(float x);

    // Move left by speed * dt
    void update(float dt, float speed);

    // Draw with an image; falls back to rectangles if texture is nullptr
    void render(SDL_Renderer* renderer, SDL_Texture* texture) const;

    // Move to a new x position, pick a new gap and clear the "passed" flag
    void reset(float x);

    // Collision rectangles (base width, not the widened visual)
    SDL_FRect topRect() const;
    SDL_FRect bottomRect() const;

    float x() const { return m_x; }
    float right() const;
    bool isOffscreen() const;
    bool isPassed() const { return m_passed; }
    void setPassed(bool passed) { m_passed = passed; }

private:
    void randomizeGap();

    float m_x = 0.0f;
    float m_gapY = 0.0f;   // top edge of the gap
    bool m_passed = false;
};
