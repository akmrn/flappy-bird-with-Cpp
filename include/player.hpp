#pragma once
#include <SDL3/SDL.h>
#include "RAII_SDL.hpp"

class Player 
{
    public:
        Player(SDL_Renderer* renderer);

        void jump();
        void update(float dt);
        void render(SDL_Renderer* renderer);
        void reset();

        // To check for collisions with pipes and the ground
        const SDL_FRect& getBounds() const { return m_bounds; }

    private:
        SDL_FRect m_bounds;
        float m_velocity = 0.0f;

        // Variables for the wing-flapping animation
        float m_animTimer = 0.0f;
        int m_currentFrame = 0;

        // Two flying textures with automatic RAII management
        raii::Texture m_texFrame1;
        raii::Texture m_texFrame2;
};   