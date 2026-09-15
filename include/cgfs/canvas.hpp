#ifndef CGFS_CANVAS_HPP
#define CGFS_CANVAS_HPP

#include <memory>
#include <span>
#include "cgfs/types.hpp"
#include "cgfs/vec2.hpp"

namespace cgfs {

/** @brief Represents a color in the RGB888 color format. */
struct Rgb {

    /** @brief The red component of the color. */
    u8 r;

    /** @brief The green component of the color. */
    u8 g;

    /** @brief The blue component of the color. */
    u8 b;

};

/** @brief Represents a two-dimensional canvas that can be drawn to. */
class Canvas final {
private:

    /** @brief The pixels of this canvas. */
    std::unique_ptr<Rgb[]> _pixels;

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
     * @brief Returns the pixels of this canvas in row-major order.
     *
     * @return The pixels of this canvas in row-major order.
     */
    std::span<const Rgb> pixels() const noexcept;

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
     * @brief Draws a colored pixel at a specified position.
     *
     * @note The origin of this canvas is placed in the top-left corner, with
     * the positive x-axis pointing to the right and the positive y-axis
     * pointing downwards. The coordinates of the pixels range from `(0, 0)` in
     * the top-left corner to `(width() - 1, height() - 1)` in the bottom-right
     * corner.
     *
     * @warning The behavior is undefined if `pos` is outside the bounds of this
     * canvas.
     *
     * @param[in] pos   The position of the pixel.
     * @param[in] color The color of the pixel.
     */
    void draw_pixel(Vec2<isize> pos, Rgb color) noexcept;

};

}

#endif