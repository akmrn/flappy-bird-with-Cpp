#pragma once
#include <SDL3/SDL.h>

// Horizontally scrolling background (parallax layer).
// The texture is scaled to the world height and tiled along X.
class Background {
public:
    // 'speedFactor' is the fraction of the pipe speed (e.g. 0.3 = slower than ground)
    explicit Background(SDL_Texture* tex, float speedFactor = 0.3f);

    void update(float dt, float pipeSpeed);
    void render(SDL_Renderer* r) const;
    void reset();

private:
    SDL_Texture* m_tex;   // non-owning
    float m_speedFactor;
    float m_offset = 0.0f; // scrolled distance in [0, tileWidth)
};
