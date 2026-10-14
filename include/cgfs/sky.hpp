#ifndef CGFS_SKY_HPP
#define CGFS_SKY_HPP

#include "cgfs/color.hpp"
#include "cgfs/light.hpp"

namespace cgfs {

/** @brief A sky. */
struct Sky {

    /** @brief The radiance at the horizon. */
    Color horizon;

    /** @brief The radiance at the zenith. */
    Color zenith;

    /**
     * @brief The sun light.
     *
     * @note The sun light must be a directional light.
     */
    Light sunLight;

    /**
     * @brief Returns a default daytime sky.
     *
     * @return A default daytime sky.
     */
    static constexpr Sky day() noexcept {
        return {
            .horizon = { 0.97F, 0.98F, 0.98F },
            .zenith = { 0.1F, 0.25F, 0.6F },
            .sunLight = Light::directional()
                .direction({ -1.5F, -2.5F, -1.5F })
                .angular_radius(cgfs::radians(3.0F))
                .build()
        };
    }

    /**
     * @brief Returns a default sunset sky.
     *
     * @return A default sunset sky.
     */
    static constexpr Sky sunset() noexcept {
        return {
            .horizon = { 0.95F, 0.35F, 0.12F },
            .zenith = { 0.08F, 0.18F, 0.45F },
            .sunLight = Light::directional()
                .direction({ 0.5F, -0.18F, 1.0F })
                .color({ 1.0F, 0.5F, 0.2F })
                .angular_radius(cgfs::radians(3.0F))
                .build()
        };
    }

};

}

#endif