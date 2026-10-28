#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <memory>
#include <stdexcept>
#include <string>

namespace raii
{
    // ─────────── Window ───────────
    struct WindowDeleter
    {
        void operator()(SDL_Window* p) const noexcept
        {
            if(p) SDL_DestroyWindow(p);
        }
    };
    using Window = std::unique_ptr<SDL_Window, WindowDeleter>;

    // ─────────── Renderer ───────────
    struct RendererDeleter {
        void operator()(SDL_Renderer* p) const noexcept {
            if (p) SDL_DestroyRenderer(p);
        }
    };
    using Renderer = std::unique_ptr<SDL_Renderer, RendererDeleter>;

    // ─────────── Texture ───────────
    struct TextureDeleter {
        void operator()(SDL_Texture* p) const noexcept {
            if (p) SDL_DestroyTexture(p);
        }
    };
    using Texture = std::unique_ptr<SDL_Texture, TextureDeleter>;

    // ─────────── Font ───────────
    struct FontDeleter {
        void operator()(TTF_Font* p) const noexcept {
            if (p) TTF_CloseFont(p);
        }
    };
    using Font = std::unique_ptr<TTF_Font, FontDeleter>;    

    // ─────────── Surface (Required for the text) ───────────
    struct SurfaceDeleter {
        void operator()(SDL_Surface* p) const noexcept {
            if (p) SDL_DestroySurface(p);
        }
    };
    using Surface = std::unique_ptr<SDL_Surface, SurfaceDeleter>;

    // SDL Subsystem Management (replaces manual SDL_Init and SDL_Quit)
    class SdlContext {
        public:
            SdlContext() {
                if (!SDL_Init(SDL_INIT_VIDEO)) {
                    // Fixed: the message used to say "TTF" by mistake
                    throw std::runtime_error(std::string("Error initializing SDL: ") + SDL_GetError());
                }
            }
            ~SdlContext() { SDL_Quit(); }
            SdlContext(const SdlContext&) = delete;
            SdlContext& operator=(const SdlContext&) = delete;
    };

    // TTF (Font) Subsystem Management
    class TtfContext {
        public:
            TtfContext() {
                    if (!TTF_Init()) {
                    throw std::runtime_error(std::string(" Error initializing TTF: ") + SDL_GetError());
                }
            }
            ~TtfContext() {
                TTF_Quit();
            }
            TtfContext(const TtfContext&) = delete;
            TtfContext& operator=(const TtfContext&) = delete;
    };  

} // namespace raii
