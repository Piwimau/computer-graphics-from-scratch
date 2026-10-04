#ifndef CGFS_SDL_DELETER_HPP
#define CGFS_SDL_DELETER_HPP

#include <SDL3/SDL.h>

namespace cgfs {

/** @brief A custom deleter for SDL resources. */
struct SdlDeleter {

    /**
     * @brief Deletes an SDL window.
     *
     * @param[in] window The SDL window to delete.
     */
    void operator()(SDL_Window* window) const noexcept {
        SDL_DestroyWindow(window);
    }

    /**
     * @brief Deletes an SDL renderer.
     *
     * @param[in] renderer The SDL renderer to delete.
     */
    void operator()(SDL_Renderer* renderer) const noexcept {
        SDL_DestroyRenderer(renderer);
    }

    /**
     * @brief Deletes an SDL texture.
     *
     * @param[in] texture The SDL texture to delete.
     */
    void operator()(SDL_Texture* texture) const noexcept {
        SDL_DestroyTexture(texture);
    }

};

}

#endif