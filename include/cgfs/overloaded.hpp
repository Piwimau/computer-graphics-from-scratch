#ifndef CGFS_OVERLOADED_HPP
#define CGFS_OVERLOADED_HPP

namespace cgfs {

/**
 * @brief A small helper to allow overloading of lambdas.
 *
 * @tparam ...Ts The types for overloading.
 */
template<class... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
};

/** @brief A deduction guide for the overload helper. */
template<class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

}

#endif