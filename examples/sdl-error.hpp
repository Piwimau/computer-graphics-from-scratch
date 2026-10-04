#ifndef CGFS_SDL_ERROR_HPP
#define CGFS_SDL_ERROR_HPP

#include <SDL3/SDL.h>
#include <stdexcept>

namespace cgfs {

/** @brief An error thrown when an SDL operation fails. */
class SdlError : public std::runtime_error {

    using std::runtime_error::runtime_error;

};

/**
 * @brief Throws an `SdlError` if a specified condition is `false`.
 *
 * @param[in] condition The condition to check.
 *
 * @throws `SdlError` Thrown when `condition` is false.
 */
inline void sdl_check(bool condition) {
    if (!condition) {
        throw SdlError(SDL_GetError());
    }
}

}

#endif