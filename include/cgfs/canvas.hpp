#ifndef CGFS_CANVAS_HPP
#define CGFS_CANVAS_HPP

#include <algorithm>
#include <cassert>
#include <span>
#include <vector>
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
class Canvas final {
private:

    /** @brief The pixels of this canvas. */
    std::vector<Pixel> _pixels;

    /** @brief The width of this canvas. */
    isize _width;

    /** @brief The height of this canvas. */
    isize _height;

    /**
     * @brief Initializes a new canvas with the specified properties.
     *
     * @warning The behavior is undefined if `pixels` does not have `width *
     * height` elements, or if `width` or `height` is negative.
     *
     * @param[in] pixels The pixels of the canvas.
     * @param[in] width  The width of the canvas.
     * @param[in] height The height of the canvas.
     */
    constexpr Canvas(
        std::vector<Pixel> pixels,
        isize width,
        isize height
    ) noexcept
        : _pixels(std::move(pixels)), _width(width), _height(height) {
        assert(_width >= 0);
        assert(_height >= 0);
        assert(std::ssize(_pixels) == width * height);
    }

public:

    /**
     * @brief Creates an empty canvas with a specified width and height.
     *
     * @note The pixels are zero-initialized (i.e., they are black by default).
     *
     * @warning The behavior is undefined if `width` or `height` is negative.
     *
     * @param[in] width  The width of the canvas.
     * @param[in] height The height of the canvas.
     * @return An empty canvas with the specified width and height.
     */
    static constexpr Canvas empty(isize width, isize height) {
        assert(width >= 0);
        assert(height >= 0);
        return Canvas(
            std::vector<Pixel>(width * height, { 0, 0, 0 }),
            width,
            height
        );
    }

    /**
     * @brief Returns a span of the pixels of this canvas.
     *
     * @return A span of the pixels of this canvas.
     */
    constexpr std::span<const Pixel> pixels() const noexcept {
        return std::span<const Pixel>(_pixels);
    }

    /**
     * @brief Returns the width of this canvas.
     *
     * @return The width of this canvas.
     */
    constexpr isize width() const noexcept {
        return _width;
    }

    /**
     * @brief Returns the height of this canvas.
     *
     * @return The height of this canvas.
     */
    constexpr isize height() const noexcept {
        return _height;
    }

    /**
     * @brief Sets a pixel at a specified position.
     *
     * @warning The behavior is undefined if `x` or `y` is out of bounds.
     *
     * @param[in] x     The x-coordinate of the pixel.
     * @param[in] y     The y-coordinate of the pixel.
     * @param[in] pixel The pixel to set.
     */
    constexpr void put_pixel(isize x, isize y, Pixel pixel) noexcept {
        assert((x >= 0) && (x < _width));
        assert((y >= 0) && (y < _height));
        _pixels[y * _width + x] = pixel;
    }

};

}

#endif