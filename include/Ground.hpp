#pragma once

#include <SDL3/SDL.h>

// Scrolling ground strip. Tiles the texture horizontally and wraps the offset
// so the movement looks endless.
class Ground {
public:
    // texture is borrowed (owned by TextureCache) and may be nullptr
    explicit Ground(SDL_Texture* texture) : m_texture(texture) {}

    // Scroll left by speed * dt
    void update(float dt, float speed);

    void render(SDL_Renderer* renderer) const;

    void reset() { m_offset = 0.0f; }

private:
    SDL_Texture* m_texture = nullptr; // non-owning
    float m_offset = 0.0f;            // distance scrolled inside one tile
};
