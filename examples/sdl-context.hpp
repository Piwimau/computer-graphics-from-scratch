#ifndef CGFS_SDL_CONTEXT_HPP
#define CGFS_SDL_CONTEXT_HPP

#include <SDL3/SDL.h>
#include "sdl-error.hpp"

namespace cgfs {

/** @brief A small helper to manage an SDL context. */
struct SdlContext {

    /** 
     * @brief Constructs an SDL context.
     * 
     * @throws `SdlError` Thrown when an SDL operation fails.
     */
    SdlContext() {
        sdl_check(SDL_Init(SDL_INIT_VIDEO));
    }

    SdlContext(const SdlContext&) = delete;

    SdlContext& operator=(const SdlContext&) = delete;

    /** @brief Destructs this SDL context. */
    ~SdlContext() {
        SDL_Quit();
    }

};

}

#endif