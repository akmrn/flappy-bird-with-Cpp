#include "Pipe.hpp"
#include "constants.hpp"
#include <random>

namespace {
    // Properly seeded engine instead of unseeded rand()
    std::mt19937& rng() {
        static std::mt19937 engine{std::random_device{}()};
        return engine;
    }
}

Pipe::Pipe(float startX) { reset(startX); }

void Pipe::reset(float startX) {
    m_x = startX;
    m_passed = false;
    randomizeGap();
}

void Pipe::update(float deltaSeconds, float speed) {
    m_x -= speed * deltaSeconds;
}

float Pipe::right() const { return m_x + cfg::pipeWidth; }

void Pipe::render(SDL_Renderer* renderer) const {
    SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255);
    const SDL_FRect top = topRect();
    const SDL_FRect bottom = bottomRect();
    SDL_RenderFillRect(renderer, &top);
    SDL_RenderFillRect(renderer, &bottom);
}

SDL_FRect Pipe::topRect() const {
    return SDL_FRect{m_x, 0.0f, cfg::pipeWidth, m_topHeight};
}

SDL_FRect Pipe::bottomRect() const {
    const float y = m_topHeight + cfg::pipeGap;
    return SDL_FRect{m_x, y, cfg::pipeWidth, cfg::playHeight - y};
}

void Pipe::randomizeGap() {
    const float minTop = cfg::pipeEdgePadding;
    const float maxTop = cfg::playHeight - cfg::pipeGap - cfg::pipeEdgePadding;
    std::uniform_real_distribution<float> dist(minTop, maxTop);
    m_topHeight = dist(rng());
}
