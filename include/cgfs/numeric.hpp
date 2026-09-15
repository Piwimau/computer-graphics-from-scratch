#ifndef CGFS_NUMERIC_HPP
#define CGFS_NUMERIC_HPP

#include <concepts>

namespace cgfs {

/**
 * @brief Represents a numeric type (either integral or floating-point).
 *
 * @tparam T The underlying numeric type.
 */
template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

}

#endif