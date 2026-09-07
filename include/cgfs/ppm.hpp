#ifndef CGFS_PPM_HPP
#define CGFS_PPM_HPP

#include <string>
#include "cgfs/canvas.hpp"

namespace cgfs {

/**
 * @brief Saves a canvas to a Portable Pixmap (PPM) file with a specified path.
 *
 * @param[in] canvas The canvas to save.
 * @param[in] path   The path of the PPM file.
 * @throws `std::exception` Thrown if any error occurs while saving the canvas
 *                          to the PPM file.
 */
void save_ppm(const Canvas& canvas, const std::string& path);

}

#endif