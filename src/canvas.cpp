#include <algorithm>
#include <cassert>
#include "cgfs/canvas.hpp"

namespace cgfs {

/** @brief The default color of pixels. */
static constexpr Rgb BLACK = { 0, 0, 0 };

/**
 * @brief Creates an array of pixels with a specified width and height.
 *
 * @note The pixels are zero-initialized (i.e., they are black by default).
 *
 * @warning The behavior is undefined if `width` or `height` is negative.
 *
 * @param[in] width  The width of the array.
 * @param[in] height The height of the array.
 * @return The array of pixels.
 */
static std::unique_ptr<Rgb[]> make_pixels(isize width, isize height) {
    assert(width >= 0);
    assert(height >= 0);
    auto pixels = std::make_unique_for_overwrite<Rgb[]>(width * height);
    std::ranges::fill_n(pixels.get(), width * height, BLACK);
    return pixels;
}

Canvas::Canvas(isize width, isize height)
    : _pixels(make_pixels(width, height)),
      _width(width),
      _height(height) { }

std::span<const Rgb> Canvas::pixels() const noexcept {
    return std::span<const Rgb>(_pixels.get(), _width * _height);
}

isize Canvas::width() const noexcept {
    return _width;
}

isize Canvas::height() const noexcept {
    return _height;
}

isize Canvas::min_x() const noexcept {
    return -_width / 2;
}

isize Canvas::max_x() const noexcept {
    return _width / 2 - 1;
}

isize Canvas::min_y() const noexcept {
    return -_height / 2;
}

isize Canvas::max_y() const noexcept {
    return _height / 2 - 1;
}

void Canvas::draw_pixel(Vec2<isize> pos, Rgb color) noexcept {
    assert((pos.x >= min_x()) && (pos.x <= max_x()));
    assert((pos.y >= min_y()) && (pos.y <= max_y()));
    pos = { .x = pos.x - min_x(), .y = max_y() - pos.y };
    _pixels[pos.y * _width + pos.x] = color;
}

}