#ifndef CGFS_SDL_DELTA_TIMER_HPP
#define CGFS_SDL_DELTA_TIMER_HPP

#include <SDL3/SDL.h>
#include "cgfs/types.hpp"

namespace cgfs {

/** @brief A small helper for measuring the delta time using SDL. */
class SdlDeltaTimer final {
private:

    /** @brief The performance counter frequency (in ticks per second). */
    const f64 FREQUENCY = static_cast<f64>(SDL_GetPerformanceFrequency());

    /** @brief The last recorded performance counter. */
    u64 _last = SDL_GetPerformanceCounter();

public:

    /**
     * @brief Returns the time since the last tick (in seconds).
     *
     * @return The time since the last tick (in seconds).
     */
    f32 tick() {
        u64 now = SDL_GetPerformanceCounter();
        f64 deltaTime = static_cast<f64>(now - _last) / FREQUENCY;
        _last = now;
        return static_cast<f32>(deltaTime);
    }

    /** @brief Resets this timer so the next tick measures from now. */
    void reset() {
        _last = SDL_GetPerformanceCounter();
    }

};

}

#endif