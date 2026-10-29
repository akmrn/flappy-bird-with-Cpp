#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include "RAII_SDL.hpp"

// Shows "score / best" inside the top-left panel.
// The panel texture is owned by TextureCache; this class keeps a non-owning pointer.
class ScoreSystem {
public:
    // box may be nullptr; then a plain rectangle is drawn instead.
    ScoreSystem(SDL_Renderer* renderer, raii::Font font,
                const std::string& savePath, SDL_Texture* box = nullptr);

    ~ScoreSystem() = default;

    ScoreSystem(const ScoreSystem&) = delete;
    ScoreSystem& operator=(const ScoreSystem&) = delete;

    void addPoint();   // increments score; updates best score for display only
    void reset();      // resets current score; does not touch the saved best
    void commit();     // writes best score to disk (call once, at game over)
    void render(SDL_Renderer* renderer) const;

    int score() const { return m_score; }
    int highScore() const { return m_highScore; }

private:
    void loadHighScore();
    void saveHighScore() const;
    void rebuildText();

    SDL_Renderer* m_renderer = nullptr; // non-owning
    SDL_Texture* m_box = nullptr;       // non-owning, owned by TextureCache
    float m_boxH = 0.0f;                // height derived from the panel aspect ratio

    raii::Font m_font;
    std::string m_savePath;

    int m_score = 0;
    int m_highScore = 0;

    raii::Texture m_text;  // white text, drawn in black for the shadow
    float m_textW = 0.0f;
    float m_textH = 0.0f;
};
