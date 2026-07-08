#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <fstream>
#include <sstream>

class ScoreSystem
{
    public:
        ScoreSystem();
        ~ScoreSystem();

        // Initialize the system, load the font, and create the initial texture
        bool init(SDL_Renderer* renderer, const char * fontPath, float fontSize, const std::string& savePath = "highscore.txt");

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

       int getHighScore() const { return m_highScore; }
       
    private:
        int m_score;
        int m_highScore;
        std::string m_saveFilePath;

        TTF_Font* m_font;
        SDL_Texture* m_texture;
        SDL_FRect m_rect;
        float m_fontSize;

        void loadHighScore();
        void saveHighScore();

        // Create a texture from the current score value
        bool updateTexture(SDL_Renderer* renderer);
};