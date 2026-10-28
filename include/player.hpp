#pragma once
#include <SDL3/SDL.h>
#include "RAII_SDL.hpp"

class Player {
public:
    explicit Player(SDL_Renderer* renderer);

    void jump();
    void update(float dt);
    void render(SDL_Renderer* renderer) const;
    void reset();

    // Visual bounds (sprite) and a smaller, fairer collision box
    const SDL_FRect& bounds() const { return m_bounds; }
    SDL_FRect hitbox() const;

private:
    SDL_FRect m_bounds{};
    float m_velocity = 0.0f;

    float m_animTimer = 0.0f;
    int   m_currentFrame = 0;

    raii::Texture m_texFrame1;
    raii::Texture m_texFrame2;
};
