#pragma once

// All tunable game values live here (no mutable globals).
namespace cfg {
    // Fixed logical world size; SDL scales it to the real window (letterbox).
    inline constexpr int   worldW = 600;
    inline constexpr int   worldH = 900;
    inline constexpr float groundHeight = 60.0f;
    inline constexpr float playHeight = static_cast<float>(worldH) - groundHeight;

    // Physics
    inline constexpr float gravity   = 900.0f;
    inline constexpr float jumpSpeed = -350.0f;

    // Player
    inline constexpr float birdStartX = 150.0f;
    inline constexpr float birdStartY = 150.0f;
    inline constexpr float birdW = 88.0f;
    inline constexpr float birdH = 73.0f;
    inline constexpr float hitboxInset = 0.15f; // fraction removed from each side

    // Pipes
    inline constexpr int   pipeCount   = 6;
    inline constexpr float pipeWidth   = 80.0f;
    inline constexpr float pipeGap     = 240.0f;
    inline constexpr float pipeSpacing = 380.0f;
    inline constexpr float pipeEdgePadding = 50.0f;
    inline constexpr float pipeBaseSpeed = 200.0f;
    inline constexpr float pipeMaxSpeed  = 380.0f;
    inline constexpr float pipeSpeedPerPoint = 4.0f;

    // Timing
    inline constexpr float fixedStep = 1.0f / 120.0f;
    inline constexpr float maxFrameTime = 0.1f;
    inline constexpr float restartCooldown = 0.5f;

    // Minimum distance between the pipe gap and the ceiling/ground
    constexpr float pipeMargin = 80.0f;
}
