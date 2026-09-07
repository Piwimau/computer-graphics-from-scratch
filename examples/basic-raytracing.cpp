#include <cstdlib>
#include <vector>
#include "cgfs/camera.hpp"
#include "cgfs/canvas.hpp"
#include "cgfs/color.hpp"
#include "cgfs/ppm.hpp"
#include "cgfs/raytracer.hpp"
#include "cgfs/sphere.hpp"
#include "cgfs/types.hpp"
#include "cgfs/vec.hpp"
#include "cgfs/viewport.hpp"

int main() {
    std::vector<cgfs::Sphere> spheres = {
        { .center = { 0.0, -1.0, 3.0 }, .radius = 1.0, .color = { 255, 0, 0 } },
        { .center = { -2.0, 0.0, 4.0 }, .radius = 1.0, .color = { 0, 255, 0 } },
        { .center = { 2.0, 0.0, 4.0 }, .radius = 1.0, .color = { 0, 0, 255 } }
    };
    cgfs::Camera camera = {
        .position = { 0.0, 0.0, 0.0 },
        .orientation = { 0.0, 0.0, 1.0 }
    };
    cgfs::Viewport viewport = { .width = 1.0, .height = 1.0, .distance = 1.0 };
    cgfs::Canvas canvas(512, 512);
    cgfs::raytrace(spheres, camera, viewport, canvas);
    cgfs::save_ppm(canvas, "output.ppm");
    return EXIT_SUCCESS;
}