#include "scoreSystem.h"

ScoreSystem::ScoreSystem() :
    m_score(0),
    m_highScore(0),
    m_saveFilePath("highscore.txt"),
    m_font(nullptr),
    m_texture(nullptr),
    m_rect{20.0f, 20.0f, 0.0f, 0.0f},
    m_fontSize(28.0f)
    {
    }
    ScoreSystem::~ScoreSystem()
    {
        cleanup();
    }

    void ScoreSystem::loadHighScore()
    {
        std::ifstream file(m_saveFilePath);
        if(file.is_open())
        {
            if(file >> m_highScore)
            {
                SDL_Log("ScoreSystem: High score loaded successfully: %d", m_highScore);
            } else
            {
                m_highScore = 0; // if empty
            }
            file.close();
        } else
        {
            m_highScore = 0;
            SDL_Log("ScoreSystem: No save file found. starting with high score at 0.");
        }
    }

    void ScoreSystem::saveHighScore()
    {
        std::ofstream file(m_saveFilePath);
        if (file.is_open())
        {
            file << m_highScore;
            file.close();
            SDL_Log("ScoreSystem: High Score saved successfully: %d", m_highScore);
        }
        else
        {
            SDL_Log("ScoreSystem: Failed to open file for writing high score.");
        }
    }

    bool ScoreSystem::init(SDL_Renderer* renderer, const char* fontPath, float fontSize, const std::string& savePath)
    {
        m_saveFilePath = savePath;

        m_fontSize = fontSize;

        m_font = TTF_OpenFont(fontPath, m_fontSize);
        if(!m_font)
        {
            SDL_Log("scoreSystem: Failed to load font '%s': %s", fontPath, SDL_GetError());
            return false;
        }

        loadHighScore();

        m_score = 0;
        return updateTexture(renderer);
    }

    void ScoreSystem::cleanup()
    {
        if(m_texture)
        {
            SDL_DestroyTexture(m_texture);
            m_texture = nullptr;
        }
        if(m_font)
        {
            TTF_CloseFont(m_font);
            m_font = nullptr;
        }
    }

    void ScoreSystem::addPoint(SDL_Renderer* renderer)
    {
        m_score ++;

        SDL_Log("Score update: %d", m_score);

        if(m_score > m_highScore)
        {
            m_highScore = m_score;
            saveHighScore();
        }

        updateTexture(renderer);
    }

    void ScoreSystem::reset(SDL_Renderer* renderer)
    {
        m_score = 0;
        updateTexture(renderer);
    }

    void ScoreSystem::render(SDL_Renderer* renderer) const{
        if(m_texture)
        {
            SDL_RenderTexture(renderer, m_texture, nullptr, &m_rect);
        }
    }

    bool ScoreSystem::updateTexture(SDL_Renderer* renderer)
    {
        // Adding a space to beautify the score text
        std::string scoreText = "Score: " + std::to_string(m_score) + "  Best: " + std::to_string(m_highScore);
        SDL_Color black = {0, 0, 0, 255};

        // Create a new surface
        SDL_Surface* surface = TTF_RenderText_Blended(m_font, scoreText.c_str(), 0, black);
        if(!surface)
        {
            SDL_Log("ScoreSystem: Failed to render text surface: %s", SDL_GetError());
            return false;
        }

        // Create a new texture temporarily
        SDL_Texture* newTexture = SDL_CreateTextureFromSurface(renderer, surface);
        if(!newTexture)
        {
            SDL_Log("ScoreSystem: Failed to create texture from surface: %s", SDL_GetError());
            SDL_DestroySurface(surface);
            return false;
        }

        // Now that we are sure the new texture has been successfully created, we will delete the old one.
        if(m_texture)
        {
            SDL_DestroyTexture(m_texture);
        }
        m_texture = newTexture;

        m_rect.x = 20.0f;
        m_rect.y = 20.0f;
        m_rect.w = (float)surface->w;
        m_rect.h = (float)surface->h;

        SDL_DestroySurface(surface);
        return true;
    }
