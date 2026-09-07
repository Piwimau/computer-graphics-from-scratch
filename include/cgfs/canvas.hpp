#ifndef CGFS_CANVAS_HPP
#define CGFS_CANVAS_HPP

#include <memory>
#include "cgfs/color.hpp"
#include "cgfs/types.hpp"
#include "cgfs/vec.hpp"

namespace cgfs {

/** @brief Represents a two-dimensional canvas that can be drawn to. */
class Canvas final {
private:

    /** @brief The pixels of this canvas. */
    std::unique_ptr<Color[]> _pixels;

    /** @brief The width of this canvas. */
    isize _width;

    /** @brief The height of this canvas. */
    isize _height;

public:

    /**
     * @brief Initializes a new canvas with a specified width and height.
     *
     * @warning The behavior is undefined if `width` or `height` is negative.
     *
     * @param[in] width  The width of the canvas.
     * @param[in] height The height of the canvas.
     */
    Canvas(isize width, isize height);

    /**
     * @brief Returns the width of this canvas.
     *
     * @return The width of this canvas.
     */
    isize width() const noexcept;

    /**
     * @brief Returns the height of this canvas.
     *
     * @return The height of this canvas.
     */
    isize height() const noexcept;

    /**
     * @brief Draws a pixel at a specified position with a specified color.
     *
     * @note The origin of this canvas is placed at its center, with the
     * positive x-axis pointing to the right and the positive y-axis pointing
     * upwards. The x-coordinates of the pixels range from `-width() / 2` to
     * `width() / 2 - 1` (inclusive), while the y-coordinates range from
     * `-height() / 2` to `height() / 2 - 1` (inclusive).
     *
     * @warning The behavior is undefined if `pos` is outside the bounds of this
     * canvas.
     *
     * @param[in] pos   The position of the pixel.
     * @param[in] color The color of the pixel.
     */
    void draw_pixel(Vec2<isize> pos, Color color) noexcept;

};

}

#endif