#ifndef CGFS_SDL_FPS_COUNTER_HPP
#define CGFS_SDL_FPS_COUNTER_HPP

#include <SDL3/SDL.h>
#include <optional>
#include "cgfs/types.hpp"

namespace cgfs {

/** @brief A small helper for measuring the frame rate using SDL. */
class SdlFpsCounter final {
private:

    /** @brief The interval between two measurements (in nanoseconds). */
    u64 _interval;

    /** @brief The time at which the current measurement window started. */
    u64 _start = SDL_GetTicksNS();

    /** @brief The number of frames in the current measurement window. */
    u64 _frames = 0;

public:

    /**
     * @brief Constructs an FPS counter.
     *
     * @param[in] intervalSeconds The interval between two measurements (in
     *                            seconds).
     */
    explicit SdlFpsCounter(f64 intervalSeconds = 0.5) noexcept
        : _interval(static_cast<u64>(intervalSeconds * SDL_NS_PER_SECOND)) { }

    /**
     * @brief Records a frame.
     *
     * @return The average frames per second since the last measurement, or
     * `std::nullopt` if the measurement interval has not yet elapsed.
     */
    std::optional<f64> tick() noexcept {
        _frames++;
        u64 now = SDL_GetTicksNS();
        u64 elapsed = now - _start;
        if (elapsed < _interval) {
            return std::nullopt;
        }
        f64 fps = static_cast<f64>(_frames) * SDL_NS_PER_SECOND
            / static_cast<f64>(elapsed);
        _frames = 0;
        _start = now;
        return fps;
    }

    /** @brief Resets this counter so the next measurement starts from now. */
    void reset() noexcept {
        _frames = 0;
        _start = SDL_GetTicksNS();
    }

};

}

#endif