#ifndef CGFS_PATHTRACER_HPP
#define CGFS_PATHTRACER_HPP

#include "cgfs/api.hpp"
#include "cgfs/film.hpp"
#include "cgfs/scene.hpp"
#include "cgfs/thread-pool.hpp"

namespace cgfs {

/**
 * @brief Renders a scene to a film using pathtracing.
 *
 * @param[in]      scene      The scene to render.
 * @param[in, out] threadPool A thread pool for accelerating the computation.
 * @param[out]     film       The film to render the scene to.
 */
CGFS_API void pathtrace(const Scene& scene, ThreadPool& threadPool, Film& film);

}

#endif