#include "scoreSystem.h"

ScoreSystem::ScoreSystem() 
    : m_score(0), m_font(nullptr), m_texture(nullptr), m_rect{20.0f, 20.0f, 0.0f, 0.0f}, m_fontSize(28.0f)
    {
    }
    ScoreSystem::~ScoreSystem()
    {
        cleanup();
    }

    bool ScoreSystem::init(SDL_Renderer* renderer, const char* fontPath, float fontSize)
    {
        m_fontSize = fontSize;

        m_font = TTF_OpenFont(fontPath, m_fontSize);
        if(!m_font)
        {
            SDL_Log("scoreSystem: Failed to load font '%s': %s", fontPath, SDL_GetError());
            return false;
        }

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
        updateTexture(renderer);
        SDL_Log("Score update: %d", m_score);
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
        // free the memory
        if(m_texture)
        {
            SDL_DestroyTexture(m_texture);
            m_texture = nullptr;
        }

        std::string scoreText = "Score" + std::to_string(m_score);
        SDL_Color black = {0, 0, 0, 0};

        SDL_Surface* surface = TTF_RenderText_Blended(m_font, scoreText.c_str(), 0, black);
        if(!surface)
        {
            SDL_Log("ScoreSystem: Failed to render text surface: %s", SDL_GetError());
            return false;
        }

        m_texture = SDL_CreateTextureFromSurface(renderer, surface);
        if(!m_texture)
        {
            SDL_Log("ScoreSystem: Failled to create texture from surface: %s", SDL_GetError());
            SDL_DestroySurface(surface);
            return false;
        }

        m_rect.x = 20.0f;
        m_rect.y = 20.0f;
        m_rect.w = (float)surface->w;
        m_rect.h = (float)surface->h;

        SDL_DestroySurface(surface);
        return true;
    }
