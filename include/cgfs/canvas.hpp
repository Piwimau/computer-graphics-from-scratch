#ifndef CGFS_CANVAS_HPP
#define CGFS_CANVAS_HPP

#include <algorithm>
#include <memory>
#include "cgfs/types.hpp"

namespace cgfs {

/** @brief Represents a single pixel on a canvas. */
struct Pixel {

    /** @brief The red component of this pixel. */
    u8 r;

    /** @brief The green component of this pixel. */
    u8 g;

    /** @brief The blue component of this pixel. */
    u8 b;

};

/** @brief Represents a canvas that can be drawn to. */
struct Canvas {

    /** @brief The pixels of this canvas. */
    std::unique_ptr<Pixel[]> pixels;

    /** @brief The width of this canvas. */
    isize width;

    /** @brief The height of this canvas. */
    isize height;

    /**
     * @brief Creates an empty canvas with a specified width and height.
     *
     * @note The pixels of the canvas are initialized to zero (i.e., the are
     * black by default).
     *
     * @param[in] width  The width of the canvas.
     * @param[in] height The height of the canvas.
     * @return An empty canvas with the specified width and height.
     */
    static constexpr Canvas empty(isize width, isize height) {
        auto pixels = std::make_unique_for_overwrite<Pixel[]>(width * height);
        std::ranges::fill_n(pixels.get(), width * height, Pixel { 0, 0, 0 });
        return { std::move(pixels), width, height };
    }

};

}

#endif