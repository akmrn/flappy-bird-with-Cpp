#include "Player.hpp"
#include "constants.hpp"
#include <SDL3_image/SDL_image.h>
#include <stdexcept>
#include <string>

Player::Player(SDL_Renderer* renderer) {
    // Initializing position and dimensions
    m_bounds = { 150.0f, 150.0f, 88.0f, 73.0f };
    m_velocity = 0.0f;

    // Loading the texture for frame 1 and storing it in raii::Texture
    SDL_Texture* rawTex1 = IMG_LoadTexture(renderer, "assets/Bird-1.png");
    if (!rawTex1) {
        throw std::runtime_error(std::string(" Error loading BirdUp.png: ") + SDL_GetError());
    }
    m_texFrame1.reset(rawTex1);

    // Loading texture for frame 2
    SDL_Texture* rawTex2 = IMG_LoadTexture(renderer, "assets/Bird-2.png");
    if (!rawTex2) {
        throw std::runtime_error(std::string(" Error loading BirdDown.png: ") + SDL_GetError());
    }
    m_texFrame2.reset(rawTex2);
}

void Player::jump() {
    m_velocity = -350.0f; // Jumping
}

void Player::update(float dt) {
    // Applying gravitational physics
    m_velocity += gravity * dt;
    m_bounds.y += m_velocity * dt;

    // Wing-flapping animation (update every 0.15 seconds)
    m_animTimer += dt;
    if (m_animTimer >= 0.15f) {
        m_animTimer = 0.0f;
        m_currentFrame = (m_currentFrame == 0) ? 1 : 0;
    }
}

void Player::render(SDL_Renderer* renderer) {
    // Select the active texture based on the frame
    SDL_Texture* activeTex = (m_currentFrame == 0) ? m_texFrame1.get() : m_texFrame2.get();
    SDL_RenderTexture(renderer, activeTex, nullptr, &m_bounds);
}

void Player::reset() {
    m_bounds.x = 150.0f;
    m_bounds.y = 150.0f;
    m_velocity = 0.0f;
    m_animTimer = 0.0f;
    m_currentFrame = 0;
}