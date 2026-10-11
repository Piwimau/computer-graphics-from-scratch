#ifndef CGFS_BOUNCE_HPP
#define CGFS_BOUNCE_HPP

#include <optional>
#include "cgfs/color.hpp"
#include "cgfs/types.hpp"
#include "cgfs/vec3.hpp"

namespace cgfs {

/** @brief A continuation of a path at a surface. */
struct Bounce {

    /** @brief The outgoing direction. */
    Vec3<f32> direction;

    /** @brief The weight of the bounce. */
    Color weight;

    /**
     * @brief The solid-angle PDF with which the direction was sampled, or
     * `std::nullopt` for delta bounces (which cannot be MIS-weighted).
     */
    std::optional<f32> pdf;

    /** @brief Whether the bounce was a transmission. */
    bool transmitted;

};

}

#endif