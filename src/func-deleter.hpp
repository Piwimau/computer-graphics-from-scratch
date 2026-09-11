#ifndef CGFS_FUNC_DELETER_HPP
#define CGFS_FUNC_DELETER_HPP

/**
 * @brief Represents a generic deleter that calls a specified function with an
 * object when deleting it.
 *
 * @tparam Func The function to call with an object when deleting it.
 */
template<auto Func>
struct FuncDeleter {

    /**
     * @brief Calls the specified function with an object to delete it.
     *
     * @param[in, out] ptr The object to delete.
     */
    template<typename T>
    void operator()(T* ptr) const noexcept {
        Func(ptr);
    }

};

#endif