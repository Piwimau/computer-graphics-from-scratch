#include <algorithm>
#include <cassert>
#include "cgfs/canvas.hpp"

namespace cgfs {

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
static std::unique_ptr<Color[]> make_pixels(isize width, isize height) {
    assert(width >= 0);
    assert(height >= 0);
    auto pixels = std::make_unique_for_overwrite<Color[]>(width * height);
    std::ranges::fill_n(pixels.get(), width * height, BLACK);
    return pixels;
}

Canvas::Canvas(isize width, isize height)
    : _pixels(make_pixels(width, height)),
      _width(width),
      _height(height) { }

std::span<const Color> Canvas::pixels() const noexcept {
    return std::span<const Color>(_pixels.get(), _width * _height);
}

isize Canvas::width() const noexcept {
    return _width;
}

isize Canvas::height() const noexcept {
    return _height;
}

void Canvas::draw_pixel(Vec2<isize> pos, Color color) noexcept {
    assert((pos.x >= -_width / 2) && (pos.x <= _width / 2 - 1));
    assert((pos.y >= -_height / 2) && (pos.y <= _height / 2 - 1));
    pos = { .x = _width / 2 + pos.x, .y = _height / 2 - 1 - pos.y };
    _pixels[pos.y * _width + pos.x] = color;
}

}