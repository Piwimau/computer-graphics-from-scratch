#ifndef CGFS_SPACE_HPP
#define CGFS_SPACE_HPP

namespace cgfs {

/** @brief A coordinate space. */
enum class Space {

    /** @brief Indicates that coordinates are in view space. */
    VIEW,

    /** @brief Indicates that coordinates are in world space. */
    WORLD

};

}

#endif