#include "Pipe.hpp"
#include "Skin.hpp"
#include "constants.hpp"
#include <SDL3/SDL.h>
#include <algorithm>
#include <random>

namespace {
    // ---- Tuning ----
    // Pieces shorter than refHeight keep the base width; taller ones grow up to
    // (1 + maxWidthGrow) times the base width at refHeight * 2.
    constexpr float refHeight    = 200.0f;
    constexpr float maxWidthGrow = 0.6f;    // increase for even wider tall pipes
    // Extra height drawn below the bottom pipe so its base hides inside the ground
    constexpr float groundSink   = 22.0f;
    // Fraction of the base width removed from EACH side for collision only.
    // 0.0 = full image width, 0.2 = 20% narrower on both sides.
    constexpr float collisionInset = 0.01f;

    // One shared, seeded generator for all pipes
    std::mt19937& rng() {
        static std::mt19937 gen{std::random_device{}()};
        return gen;
    }

    // Rendered width for a piece of the given height (render only, not collision)
    float widenForHeight(float baseW, float pieceH) {
        const float t = std::clamp(pieceH / refHeight - 1.0f, 0.0f, 1.0f);
        return baseW * (1.0f + maxWidthGrow * t);
    }

    // Returns rect widened for its height, keeping the same horizontal center
    SDL_FRect widenedRect(const SDL_FRect& rect) {
        SDL_FRect out = rect;
        out.w = widenForHeight(rect.w, rect.h);
        out.x = rect.x - (out.w - rect.w) * 0.5f;
        return out;
    }

    // Draws one pipe piece into rect. The cap sits at the gap side of the rect;
    // the body stretches over the rest. If flipped, the piece is mirrored vertically
    // (used for the top pipe, whose cap is at the bottom of its rect).
    void drawPipePiece(SDL_Renderer* renderer, SDL_Texture* tex,
                       const SDL_FRect& rect, bool flipped) {
        if (rect.h <= 0.0f) return;

        float texW = 0.0f, texH = 0.0f;
        SDL_GetTextureSize(tex, &texW, &texH);
        if (texW <= 0.0f || texH <= 0.0f) return;

        const float capSrcH  = std::min(skin::pipeCapSourceHeight, texH);
        const float scale    = rect.w / texW;
        const float capDstH  = std::min(capSrcH * scale, rect.h);
        const float bodyDstH = rect.h - capDstH;

        const SDL_FRect capSrc{0.0f, 0.0f, texW, capSrcH};
        const SDL_FRect bodySrc{0.0f, capSrcH, texW, texH - capSrcH};

        const SDL_FlipMode flip = flipped ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE;

        // Bottom pipe: cap on top, body below. Top pipe (flipped): body on top, cap below.
        const float capY  = flipped ? rect.y + bodyDstH : rect.y;
        const float bodyY = flipped ? rect.y : rect.y + capDstH;

        const SDL_FRect capDst{rect.x, capY, rect.w, capDstH};
        SDL_RenderTextureRotated(renderer, tex, &capSrc, &capDst, 0.0, nullptr, flip);

        if (bodyDstH > 0.0f && bodySrc.h > 0.0f) {
            const SDL_FRect bodyDst{rect.x, bodyY, rect.w, bodyDstH};
            SDL_RenderTextureRotated(renderer, tex, &bodySrc, &bodyDst, 0.0, nullptr, flip);
        }
    }
}

Pipe::Pipe(float x) : m_x(x) {
    randomizeGap();
}

void Pipe::randomizeGap() {
    // Keep the gap away from the ceiling and the ground
    const float minY = cfg::pipeMargin;
    const float maxY = cfg::playHeight - cfg::pipeGap - cfg::pipeMargin;
    std::uniform_real_distribution<float> dist(minY, std::max(minY, maxY));
    m_gapY = dist(rng());
}

void Pipe::update(float dt, float speed) {
    m_x -= speed * dt;
}

void Pipe::reset(float x) {
    m_x = x;
    m_passed = false;
    randomizeGap();
}

float Pipe::right() const { return m_x + cfg::pipeWidth; }

bool Pipe::isOffscreen() const {
    // Account for the widened visual so pipes don't pop out while still visible
    const float extra = cfg::pipeWidth * maxWidthGrow * 0.5f;
    return right() + extra < 0.0f;
}

SDL_FRect Pipe::topRect() const {
    const float inset = cfg::pipeWidth * collisionInset;
    return SDL_FRect{m_x + inset, 0.0f, cfg::pipeWidth - 2.0f * inset, m_gapY};
}

SDL_FRect Pipe::bottomRect() const {
    const float inset = cfg::pipeWidth * collisionInset;
    const float y = m_gapY + cfg::pipeGap;
    return SDL_FRect{m_x + inset, y, cfg::pipeWidth - 2.0f * inset, cfg::playHeight - y};
}

void Pipe::render(SDL_Renderer* renderer, SDL_Texture* texture) const {
    // Visual rects use the full base width, not the collision rects
    SDL_FRect top{m_x, 0.0f, cfg::pipeWidth, m_gapY};
    SDL_FRect bottom{m_x, m_gapY + cfg::pipeGap, cfg::pipeWidth,
                     cfg::playHeight - (m_gapY + cfg::pipeGap)};

    // Visual only: extend the bottom pipe into the ground
    bottom.h += groundSink;

    // Visual only: tall pieces are drawn wider than their base width
    const SDL_FRect drawTop    = widenedRect(top);
    const SDL_FRect drawBottom = widenedRect(bottom);

    if (texture) {
        drawPipePiece(renderer, texture, drawTop, true);
        drawPipePiece(renderer, texture, drawBottom, false);
        return;
    }

    // Fallback: plain green rectangles
    SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255);
    SDL_RenderFillRect(renderer, &drawTop);
    SDL_RenderFillRect(renderer, &drawBottom);
}
