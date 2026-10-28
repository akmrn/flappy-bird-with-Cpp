#include "Skin.hpp"
#include <algorithm>

namespace {
    // Draws one pipe piece into rect. The cap sits at the gap side of the rect;
    // the body stretches over the rest. If flipped, the piece is mirrored vertically
    // (used for the top pipe, whose cap is at the bottom of its rect).
    void drawPipePiece(SDL_Renderer* renderer, SDL_Texture* tex,
                       const SDL_FRect& rect, bool flipped) {
        if (rect.h <= 0.0f) return;

        float texW = 0.0f, texH = 0.0f;
        SDL_GetTextureSize(tex, &texW, &texH);

        const float capSrcH = std::min(skin::pipeCapSourceHeight, texH);
        const float scale   = rect.w / texW;
        const float capDstH = std::min(capSrcH * scale, rect.h);
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

void Pipe::render(SDL_Renderer* renderer, SDL_Texture* texture) const {
    const SDL_FRect top = topRect();
    const SDL_FRect bottom = bottomRect();

    if (texture) {
        drawPipePiece(renderer, texture, top, true);
        drawPipePiece(renderer, texture, bottom, false);
        return;
    }

    // Fallback: plain rectangles
    SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255);
    SDL_RenderFillRect(renderer, &top);
    SDL_RenderFillRect(renderer, &bottom);
}
