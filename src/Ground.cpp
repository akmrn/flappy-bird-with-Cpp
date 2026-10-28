#include "Ground.hpp"
#include "constants.hpp"
#include <cmath>

namespace {
    // Width of one fallback stripe, in world units
    constexpr float stripeWidth = 40.0f;
}

void Ground::update(float dt, float speed) {
    m_offset += speed * dt;

    // Keep the offset inside one tile so it never grows without bound
    float tileW = stripeWidth * 2.0f; // fallback pattern period
    if (m_texture) {
        float texW = 0.0f, texH = 0.0f;
        SDL_GetTextureSize(m_texture, &texW, &texH);
        if (texH > 0.0f) tileW = texW * (cfg::groundHeight / texH);
    }
    if (tileW > 0.0f) m_offset = std::fmod(m_offset, tileW);
}

void Ground::render(SDL_Renderer* renderer) const {
    const float worldW = static_cast<float>(cfg::worldW);

    if (m_texture) {
        float texW = 0.0f, texH = 0.0f;
        SDL_GetTextureSize(m_texture, &texW, &texH);
        if (texW <= 0.0f || texH <= 0.0f) return;

        // Scale the tile so its height matches the ground height
        const float tileW = texW * (cfg::groundHeight / texH);

        // Draw tiles from just left of the screen until the right edge is covered.
        // Positions are snapped to whole pixels and tiles overlap by 1px to avoid seams.
        for (float x = -m_offset; x < worldW; x += tileW) {
            const SDL_FRect dst{std::floor(x), cfg::playHeight, tileW + 1.0f, cfg::groundHeight};
            SDL_RenderTexture(renderer, m_texture, nullptr, &dst);
        }
        return;
    }

    // Fallback: sandy base with darker stripes that scroll
    SDL_SetRenderDrawColor(renderer, 222, 184, 135, 255);
    const SDL_FRect base{0.0f, cfg::playHeight, worldW, cfg::groundHeight};
    SDL_RenderFillRect(renderer, &base);

    SDL_SetRenderDrawColor(renderer, 196, 160, 110, 255);
    for (float x = -m_offset; x < worldW; x += stripeWidth * 2.0f) {
        const SDL_FRect stripe{std::floor(x), cfg::playHeight, stripeWidth, cfg::groundHeight};
        SDL_RenderFillRect(renderer, &stripe);
    }
}
