#ifndef CGFS_PRNG_HPP
#define CGFS_PRNG_HPP

#include <array>
#include <bit>
#include "cgfs/types.hpp"

namespace cgfs {

/** @brief A pseudorandom number generator. */
class Prng final {
private:

    /** @brief The internal state. */
    std::array<u32, 4> _state;

    /**
     * @brief Constructs an initial state based on a specified seed.
     *
     * @param[in] seed The seed for the initialization.
     * @return An initial state for the pseudorandom number generator.
     */
    static constexpr std::array<u32, 4> make_state(u64 seed) noexcept {
        auto splitmix64 = [](u64& state) {
            state += 0x9E3779B97F4A7C15ULL;
            u64 z = state;
            z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
            z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
            return z ^ (z >> 31);
        };
        u64 a = splitmix64(seed);
        u64 b = splitmix64(seed);
        return {
            static_cast<u32>(a),
            static_cast<u32>(a >> 32),
            static_cast<u32>(b),
            static_cast<u32>(b >> 32)
        };
    }

public:

    /**
     * @brief Constructs a pseudorandom number generator with a specified seed.
     *
     * @param[in] seed The seed for the initialization.
     */
    constexpr explicit Prng(u64 seed) noexcept : _state(make_state(seed)) { }

    /**
     * @brief Generates a pseudorandom `u32`.
     *
     * @return A pseudorandom `u32`.
     */
    constexpr u32 next_u32() noexcept {
        u32 result = std::rotl(_state[1] * 5, 7) * 9;
        u32 t = _state[1] << 9;
        _state[2] ^= _state[0];
        _state[3] ^= _state[1];
        _state[1] ^= _state[2];
        _state[0] ^= _state[3];
        _state[2] ^= t;
        _state[3] = std::rotl(_state[3], 11);
        return result;
    }

    /**
     * @brief Generates a pseudorandom `f32` in the range `[0.0F, 1.0F)`.
     *
     * @return A pseudorandom `f32` in the range `[0.0F, 1.0F)`.
     */
    constexpr f32 next_f32() noexcept {
        return static_cast<f32>(next_u32() >> 8) * (1.0F / 16777216.0F);
    }

};

}

#endif