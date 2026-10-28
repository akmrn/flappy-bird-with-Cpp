#pragma once
#include <SDL3/SDL.h>

class Pipe {
public:
    explicit Pipe(float startX);

    // Reuse the pipe at a new position with a new random gap
    void reset(float startX);
    void update(float deltaSeconds, float speed);
    void render(SDL_Renderer* renderer) const;

    SDL_FRect topRect() const;
    SDL_FRect bottomRect() const;

    float x() const { return m_x; }
    float right() const;
    bool isOffscreen() const { return right() < 0.0f; }

    bool isPassed() const { return m_passed; }
    void setPassed(bool passed) { m_passed = passed; }

private:
    void randomizeGap();

    float m_x = 0.0f;
    float m_topHeight = 0.0f;
    bool  m_passed = false;
};
