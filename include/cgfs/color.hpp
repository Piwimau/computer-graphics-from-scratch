#ifndef CGFS_COLOR_HPP
#define CGFS_COLOR_HPP

#include "cgfs/types.hpp"

namespace cgfs {

/** @brief Represents a color in the RGB888 color format. */
struct Color {

    /** @brief The red component of this color. */
    u8 r;

    /** @brief The green component of this color. */
    u8 g;

    /** @brief The blue component of this color. */
    u8 b;

};

/** @brief The color black. */
static constexpr Color BLACK = { 0, 0, 0 };

}

#endif