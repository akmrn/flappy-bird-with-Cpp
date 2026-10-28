#pragma once

#include <SDL3/SDL.h>

class Player {
public:
    // Textures are owned by TextureCache; Player only borrows them
    Player(SDL_Texture* frame1, SDL_Texture* frame2);

    void jump();
    void update(float dt);
    void render(SDL_Renderer* renderer) const;
    void reset();

    // Visible sprite rectangle
    const SDL_FRect& bounds() const { return m_bounds; }
    // Smaller collision rectangle (more forgiving than the sprite)
    SDL_FRect hitbox() const;

private:
    SDL_Texture* m_texFrame1 = nullptr; // non-owning
    SDL_Texture* m_texFrame2 = nullptr; // non-owning

    SDL_FRect m_bounds{};
    float m_velocity = 0.0f;
    float m_animTimer = 0.0f;
    int m_currentFrame = 0;
};
