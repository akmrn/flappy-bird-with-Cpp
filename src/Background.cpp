#include "Background.hpp"
#include "constants.hpp"
#include <cmath>

Background::Background(SDL_Texture* tex, float speedFactor)
    : m_tex(tex), m_speedFactor(speedFactor) {}

void Background::reset() { m_offset = 0.0f; }

void Background::update(float dt, float pipeSpeed)
{
    if (!m_tex) return;

    float texW = 0.0f, texH = 0.0f;
    SDL_GetTextureSize(m_tex, &texW, &texH);
    if (texW <= 0.0f || texH <= 0.0f) return;

    // Width of one tile after scaling the image to the world height
    const float tileW = texW * (static_cast<float>(cfg::worldH) / texH);

    m_offset = std::fmod(m_offset + pipeSpeed * m_speedFactor * dt, tileW);
}

void Background::render(SDL_Renderer* r) const
{
    const float worldW = static_cast<float>(cfg::worldW);
    const float worldH = static_cast<float>(cfg::worldH);

    if (!m_tex) {
        // Fallback: solid sky color
        SDL_SetRenderDrawColor(r, 200, 250, 255, 255);
        SDL_FRect sky{0.0f, 0.0f, worldW, worldH};
        SDL_RenderFillRect(r, &sky);
        return;
    }

    float texW = 0.0f, texH = 0.0f;
    SDL_GetTextureSize(m_tex, &texW, &texH);
    if (texW <= 0.0f || texH <= 0.0f) return;

    const float tileW = texW * (worldH / texH);

    // Floored start position and 1px overlap avoid seams between tiles
    float x = std::floor(-m_offset);
    while (x < worldW) {
        SDL_FRect dst{x, 0.0f, tileW + 1.0f, worldH};
        SDL_RenderTexture(r, m_tex, nullptr, &dst);
        x += tileW;
    }
}
