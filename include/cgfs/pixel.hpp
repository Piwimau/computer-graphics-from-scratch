#ifndef CGFS_PIXEL_HPP
#define CGFS_PIXEL_HPP

#include "cgfs/types.hpp"

namespace cgfs {

/** @brief A pixel in the RGB color format. */
struct Pixel {

    /** @brief The red component. */
    u8 r;

    /** @brief The green component. */
    u8 g;

    /** @brief The blue component. */
    u8 b;

};

}

#endif