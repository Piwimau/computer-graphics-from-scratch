#ifndef CGFS_SDL_WINDOW_HPP
#define CGFS_SDL_WINDOW_HPP

#include <SDL3/SDL.h>
#include <cassert>
#include <memory>
#include <span>
#include "cgfs/film.hpp"
#include "cgfs/types.hpp"
#include "sdl-deleter.hpp"
#include "sdl-error.hpp"

namespace cgfs {

/** @brief A small wrapper around an SDL window. */
class SdlWindow final {
private:

    /** @brief The underlying window. */
    std::unique_ptr<SDL_Window, SdlDeleter> _window;

    /** @brief The renderer used to draw to the window. */
    std::unique_ptr<SDL_Renderer, SdlDeleter> _renderer;

    /** @brief The texture to draw to the window. */
    std::unique_ptr<SDL_Texture, SdlDeleter> _texture;

public:

    /**
     * @brief Constructs an SDL window with the specified properties.
     *
     * @param[in] title  The title of the window.
     * @param[in] width  The width of the window.
     * @param[in] height The height of the window.
     * @param[in] vsync  Whether to enable VSync.
     * @param[in] flags  Optional window flags (see `SDL_CreateWindow()`).
     *
     * @throws `SdlError` Thrown when an SDL operation fails.
     *
     * @warning The behavior is undefined if `title` is `nullptr`, or if `width`
     * or `height` is negative.
     */
    SdlWindow(
        const char* title,
        int width,
        int height,
        bool vsync = true,
        u64 flags = 0
    ) {
        assert(title != nullptr);
        assert(width >= 0);
        assert(height >= 0);
        SDL_Window* window;
        SDL_Renderer* renderer;
        sdl_check(
            SDL_CreateWindowAndRenderer(
                title,
                width,
                height,
                flags,
                &window,
                &renderer
            )
        );
        _window.reset(window);
        _renderer.reset(renderer);
        sdl_check(
            SDL_SetRenderVSync(
                _renderer.get(),
                vsync ? 1 : SDL_RENDERER_VSYNC_DISABLED
            )
        );
        sdl_check(SDL_SetWindowRelativeMouseMode(_window.get(), true));
        SDL_Texture* texture = SDL_CreateTexture(
            _renderer.get(),
            SDL_PIXELFORMAT_RGB24,
            SDL_TEXTUREACCESS_STREAMING,
            width,
            height
        );
        sdl_check(texture != nullptr);
        _texture.reset(texture);
    }

    /**
     * @brief Fills this window with an image.
     *
     * @param[in] film The film providing the image.
     *
     * @warning The behavior is undefined if the dimensions of `film` do not
     * match the dimensions of this window.
     */
    void fill(const Film& film) {
        int width, height;
        sdl_check(SDL_GetWindowSize(_window.get(), &width, &height));
        void* ptr;
        int pitch;
        sdl_check(SDL_LockTexture(_texture.get(), nullptr, &ptr, &pitch));
        std::span<Pixel> pixels(static_cast<Pixel*>(ptr), width * height);
        film.develop(pixels);
        SDL_UnlockTexture(_texture.get());
        sdl_check(SDL_RenderClear(_renderer.get()));
        sdl_check(
            SDL_RenderTexture(
                _renderer.get(),
                _texture.get(),
                nullptr,
                nullptr
            )
        );
        sdl_check(SDL_RenderPresent(_renderer.get()));
    }

    /**
     * @brief Sets the title of this window.
     *
     * @param[in] title The new title.
     *
     * @throws `SdlError` Thrown when an SDL operation fails.
     */
    void set_title(const char* title) {
        sdl_check(SDL_SetWindowTitle(_window.get(), title));
    }

};

}

#endif