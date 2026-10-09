#ifndef CGFS_OVERLOAD_HPP
#define CGFS_OVERLOAD_HPP

namespace cgfs {

/**
 * @brief A small helper to allow overloading using lambdas.
 *
 * @tparam ...Ts The types to overload on.
 */
template<class... Ts>
struct Overload : Ts... {
    using Ts::operator()...;
};

/** @brief A deduction guide for the overload helper. */
template<class... Ts>
Overload(Ts...) -> Overload<Ts...>;

}

#endif