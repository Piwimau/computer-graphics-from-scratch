#ifndef CGFS_FILM_HPP
#define CGFS_FILM_HPP

#include <algorithm>
#include <cassert>
#include <span>
#include <utility>
#include <vector>
#include "cgfs/color.hpp"
#include "cgfs/types.hpp"
#include "cgfs/util.hpp"

namespace cgfs {

/** @brief The tone mapping operators supported by a film. */
enum class ToneMapping {

    /** @brief Apply no tone mapping operator (i.e., clamp everything). */
    NONE,

    /** @brief Apply the Reinhard tone mapping operator. */
    REINHARD,

    /** @brief Apply the ACES tone mapping operator. */
    ACES,

    /** @brief Apply the Khronos PBR Neutral tone mapping operator. */
    KHRONOS_PBR_NEUTRAL

};

/** @brief A pixel in the RGB color format. */
struct Pixel {

    /** @brief The red component. */
    u8 r;

    /** @brief The green component. */
    u8 g;

    /** @brief The blue component. */
    u8 b;

};

/** @brief A film for accumulating frames. */
class Film final {
private:

    /** @brief The color of an empty accumulation buffer. */
    static constexpr Color DEFAULT_COLOR = Color::splat(0.0F);

    /** @brief The accumulation buffer. */
    std::vector<Color> _buffer;

    /** @brief The width of this film. */
    isize _width;

    /** @brief The height of this film. */
    isize _height;

    /** @brief The number of accumulated frames. */
    isize _frames = 0;

    /** @brief The exposure multiplier applied before tone mapping. */
    f32 _exposure;

    /** @brief The tone mapping operator to apply. */
    ToneMapping _toneMapping;

    /**
     * @brief Constructs an empty accumulation buffer.
     *
     * @param[in] width  The width of the film.
     * @param[in] height The height of the film.
     * @return An empty accumulation buffer.
     *
     * @warning The behavior is undefined if `width` or `height` is negative.
     */
    static constexpr std::vector<Color> make_buffer(isize width, isize height) {
        assert(width >= 0);
        assert(height >= 0);
        return std::vector<Color>(width * height, DEFAULT_COLOR);
    }

public:

    /**
     * @brief Constructs a film with the specified parameters.
     *
     * @param[in] width       The width of the film.
     * @param[in] height      The height of the film.
     * @param[in] exposure    The exposure multiplier applied before tone
     *                        mapping.
     * @param[in] toneMapping The tone mapping operator to apply.
     *
     * @warning The behavior is undefined if `width`, `height`, or `exposure` is
     * negative.
     */
    constexpr Film(
        isize width,
        isize height,
        f32 exposure = 1.0F,
        ToneMapping toneMapping = ToneMapping::KHRONOS_PBR_NEUTRAL
    )
        : _buffer(make_buffer(width, height)),
          _width(width),
          _height(height),
          _exposure(exposure),
          _toneMapping(toneMapping) {
        assert(_exposure >= 0.0F);
    }

    /**
     * @brief Returns the width of this film.
     *
     * @return The width of this film.
     */
    constexpr isize width() const noexcept {
        return _width;
    }

    /**
     * @brief Returns the height of this film.
     *
     * @return The height of this film.
     */
    constexpr isize height() const noexcept {
        return _height;
    }

    /**
     * @brief Returns the number of accumulated frames.
     *
     * @return The number of accumulated frames.
     */
    constexpr isize frames() const noexcept {
        return _frames;
    }

    /**
     * @brief Adds a sample to this film.
     *
     * @param[in] x      The x-coordinate of the sample.
     * @param[in] y      The y-coordinate of the sample.
     * @param[in] sample The sample to add.
     *
     * @warning The behavior is undefined if `x` or `y` is out of bounds.
     */
    constexpr void add_sample(isize x, isize y, const Color& sample) noexcept {
        assert((x >= 0) && (x < _width));
        assert((y >= 0) && (y < _height));
        _buffer[y * _width + x] += sample;
    }

    /** @brief Marks the end of the current frame. */
    constexpr void end_frame() noexcept {
        _frames++;
    }

    /**
     * @brief Develops this film into an image.
     *
     * @param[out] image The destination image.
     *
     * @warning The behavior is undefined if the dimensions of `image` do not
     * match this film.
     */
    constexpr void develop(std::span<Pixel> image) const noexcept {
        assert(std::ssize(image) == std::ssize(_buffer));
        f32 scale = (_frames > 0) ? 1.0F / static_cast<f32>(_frames) : 1.0F;
        for (isize i = 0; i < std::ssize(_buffer); i++) {
            Color color = _buffer[i] * (scale * _exposure);
            switch (_toneMapping) {
                case ToneMapping::NONE:
                    color = color.clamp(0.0F, 1.0F);
                    break;
                case ToneMapping::REINHARD:
                    color = color.tone_map_reinhard();
                    break;
                case ToneMapping::ACES:
                    color = color.tone_map_aces();
                    break;
                case ToneMapping::KHRONOS_PBR_NEUTRAL:
                    color = color.tone_map_khronos_pbr_neutral();
                    break;
                default:
                    std::unreachable();
            }
            color = color.to_srgb();
            image[i] = {
                .r = static_cast<u8>(color.r * 255.0F + 0.5F),
                .g = static_cast<u8>(color.g * 255.0F + 0.5F),
                .b = static_cast<u8>(color.b * 255.0F + 0.5F)
            };
        }
    }

    /** @brief Clears this film. */
    constexpr void clear() noexcept {
        std::ranges::fill(_buffer, DEFAULT_COLOR);
        _frames = 0;
    }

};

}

#endif