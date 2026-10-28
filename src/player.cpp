#include "player.hpp"   // lower-case: matches the real file name
#include "Assets.hpp"
#include "constants.hpp"
#include <algorithm>

Player::Player(SDL_Renderer* renderer)
    : m_texFrame1(assets::loadTexture(renderer, "Bird-1.png")),
      m_texFrame2(assets::loadTexture(renderer, "Bird-2.png")) {
    reset();
}

void Player::jump() { m_velocity = cfg::jumpSpeed; }

void Player::update(float dt) {
    m_velocity += cfg::gravity * dt;
    m_bounds.y += m_velocity * dt;

    // Do not let the bird leave through the ceiling
    if (m_bounds.y < 0.0f) {
        m_bounds.y = 0.0f;
        m_velocity = std::max(m_velocity, 0.0f);
    }

    m_animTimer += dt;
    if (m_animTimer >= 0.15f) {
        m_animTimer = 0.0f;
        m_currentFrame = 1 - m_currentFrame;
    }
}

void Player::render(SDL_Renderer* renderer) const {
    SDL_Texture* tex = (m_currentFrame == 0) ? m_texFrame1.get() : m_texFrame2.get();

    // Tilt according to vertical speed: nose up when rising, down when falling
    const double angle = std::clamp(m_velocity * 0.1, -25.0, 60.0);
    SDL_RenderTextureRotated(renderer, tex, nullptr, &m_bounds, angle, nullptr, SDL_FLIP_NONE);
}

void Player::reset() {
    m_bounds = {cfg::birdStartX, cfg::birdStartY, cfg::birdW, cfg::birdH};
    m_velocity = 0.0f;
    m_animTimer = 0.0f;
    m_currentFrame = 0;
}

SDL_FRect Player::hitbox() const {
    const float dx = m_bounds.w * cfg::hitboxInset;
    const float dy = m_bounds.h * cfg::hitboxInset;
    return SDL_FRect{m_bounds.x + dx, m_bounds.y + dy,
                     m_bounds.w - 2.0f * dx, m_bounds.h - 2.0f * dy};
}
