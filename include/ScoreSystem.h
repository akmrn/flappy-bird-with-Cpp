#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>

class ScoreSystem
{
    public:
        ScoreSystem();
        ~ScoreSystem();

        // Initialize the system, load the font, and create the initial texture
        bool init(SDL_Renderer* renderer, const char * fontPath, float fontSize);

        // Clear font and texture resources
        void cleanup();

        // Increase score and rebuild score text texture
        void addPoint(SDL_Renderer* renderer);

        // Reset score (for restart) and rebuild texture
        void reset(SDL_Renderer* renderer);

        // Draw the score on the screen
        void render(SDL_Renderer* render) const;

       // getting score
       int getScore() const { return m_score; }
       
    private:
        // Create a texture from the current score value
        bool updateTexture(SDL_Renderer* renderer);

        int m_score;
        TTF_Font* m_font;
        SDL_Texture* m_texture;
        SDL_FRect m_rect;
        float m_fontSize;
};