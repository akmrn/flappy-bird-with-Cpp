#include "Pipe.hpp"
#include "constants.hpp"
#include "Skin.hpp"
#include <algorithm>
#include <random>

namespace {
    // How many pixels the bottom pipe sinks below the ground line,
    // so its base looks planted in the ground (the ground is drawn on top)
    constexpr float groundSink = 22.0f;

    // Visual width growth for tall pipes:
    // at refHeight or shorter the width is unchanged,
    // at full play height the width grows by maxWidthGrow (0.35 = +35%)
    constexpr float refHeight = 250.0f;
    constexpr float maxWidthGrow = 0.35f;

    // Single seeded generator shared by all pipes
    float randomRange(float minVal, float maxVal)
    {
        static std::mt19937 engine{std::random_device{}()};
        std::uniform_real_distribution<float> dist(minVal, maxVal);
        return dist(engine);
    }

    // Returns a visual rect that is wider than 'rect' when the pipe is tall.
    // The widened rect stays centered on the original one.
    SDL_FRect widenForHeight(const SDL_FRect& rect)
    {
        const float range = cfg::playHeight - refHeight;
        float t = (range > 0.0f) ? (rect.h - refHeight) / range : 0.0f;
        t = std::clamp(t, 0.0f, 1.0f);

        const float newW = rect.w * (1.0f + maxWidthGrow * t);
        return SDL_FRect{rect.x - (newW - rect.w) * 0.5f, rect.y, newW, rect.h};
    }

    // Draws a column image into 'dst' using three slices:
    // [top decoration] [stretchable shaft] [bottom decoration].
    // Decorations keep their aspect ratio; only the shaft is stretched.
    // If 'flipped' is true the whole column is mirrored vertically.
    void drawColumn(SDL_Renderer* r, SDL_Texture* tex, const SDL_FRect& dst,
                    float topSrcH, float bottomSrcH, bool flipped)
    {
        float texW = 0.0f, texH = 0.0f;
        SDL_GetTextureSize(tex, &texW, &texH);
        if (texW <= 0.0f || texH <= 0.0f || dst.h <= 0.0f) return;

        // Uniform scale derived from the width, so decorations are not distorted
        const float scale = dst.w / texW;
        float topH = topSrcH * scale;
        float botH = bottomSrcH * scale;

        // If the pipe is shorter than both decorations, shrink them proportionally
        if (topH + botH > dst.h) {
            const float k = dst.h / (topH + botH);
            topH *= k;
            botH *= k;
        }
        const float midH = dst.h - topH - botH;
        const float midSrcH = texH - topSrcH - bottomSrcH;

        const SDL_FlipMode flip = flipped ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE;

        // In a flipped column the top decoration is drawn at the bottom of dst
        const float yTop = flipped ? dst.y + dst.h - topH : dst.y;
        const float yBot = flipped ? dst.y : dst.y + dst.h - botH;
        const float yMid = dst.y + (flipped ? botH : topH);

        const SDL_FRect srcTop{0.0f, 0.0f, texW, topSrcH};
        const SDL_FRect srcMid{0.0f, topSrcH, texW, midSrcH};
        const SDL_FRect srcBot{0.0f, texH - bottomSrcH, texW, bottomSrcH};

        const SDL_FRect dTop{dst.x, yTop, dst.w, topH};
        const SDL_FRect dMid{dst.x, yMid, dst.w, midH};
        const SDL_FRect dBot{dst.x, yBot, dst.w, botH};

        SDL_RenderTextureRotated(r, tex, &srcTop, &dTop, 0.0, nullptr, flip);
        if (midH > 0.0f && midSrcH > 0.0f) {
            SDL_RenderTextureRotated(r, tex, &srcMid, &dMid, 0.0, nullptr, flip);
        }
        SDL_RenderTextureRotated(r, tex, &srcBot, &dBot, 0.0, nullptr, flip);
    }
}

Pipe::Pipe(float startX) : m_x(startX)
{
    randomizeTopHeight();
}

void Pipe::reset(float startX)
{
    m_x = startX;
    m_passed = false;
    randomizeTopHeight();
}

void Pipe::update(float dt, float speed)
{
    m_x -= speed * dt;
}

float Pipe::right() const
{
    return m_x + cfg::pipeWidth;
}

bool Pipe::isOffscreen() const
{
    // Use the widest possible visual width so a pipe never pops out early
    return right() + cfg::pipeWidth * maxWidthGrow < 0.0f;
}

SDL_FRect Pipe::topRect() const
{
    // Starts at the ceiling and ends where the gap begins
    return SDL_FRect{m_x, 0.0f, cfg::pipeWidth, m_topHeight};
}

SDL_FRect Pipe::bottomRect() const
{
    // Starts after the gap and ends exactly at the ground line (collision box)
    const float y = m_topHeight + cfg::pipeGap;
    return SDL_FRect{m_x, y, cfg::pipeWidth, cfg::playHeight - y};
}

void Pipe::render(SDL_Renderer* renderer, SDL_Texture* tex) const
{
    const SDL_FRect top = topRect();
    const SDL_FRect bot = bottomRect();

    if (tex) {
        // Visual rect of the bottom pipe is extended into the ground
        SDL_FRect botVisual = bot;
        botVisual.h += groundSink;

        // Bottom pipe: capital faces the gap, base sits in the ground
        drawColumn(renderer, tex, widenForHeight(botVisual),
                   skin::pipeTopSrcH, skin::pipeBottomSrcH, false);
        // Top pipe: mirrored, capital faces the gap, base touches the ceiling
        drawColumn(renderer, tex, widenForHeight(top),
                   skin::pipeTopSrcH, skin::pipeBottomSrcH, true);
        return;
    }

    // Fallback: plain green rectangles
    SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255);
    SDL_RenderFillRect(renderer, &top);
    SDL_RenderFillRect(renderer, &bot);
}

void Pipe::randomizeTopHeight()
{
    const float minTop = cfg::pipeMargin;
    float maxTop = cfg::playHeight - cfg::pipeGap - cfg::pipeMargin;
    if (maxTop < minTop) maxTop = minTop;

    m_topHeight = randomRange(minTop, maxTop);
}
