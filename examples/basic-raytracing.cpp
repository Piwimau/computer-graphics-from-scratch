#include <cstdlib>
#include <vector>
#include "cgfs/camera.hpp"
#include "cgfs/canvas.hpp"
#include "cgfs/color.hpp"
#include "cgfs/light.hpp"
#include "cgfs/ppm.hpp"
#include "cgfs/raytracer.hpp"
#include "cgfs/sphere.hpp"
#include "cgfs/types.hpp"
#include "cgfs/vector.hpp"
#include "cgfs/viewport.hpp"

int main() {
    std::vector<cgfs::Sphere> spheres = {
        {
            .center = { 0.0, -1.0, 3.0 },
            .radius = 1.0,
            .color = { 255, 0, 0 },
            .shininess = 500.0,
            .reflectiveness = 0.2
        },
        {
            .center = { -2.0, 0.0, 4.0 },
            .radius = 1.0,
            .color = { 0, 255, 0 },
            .shininess = 10.0,
            .reflectiveness = 0.4
        },
        {
            .center = { 2.0, 0.0, 4.0 },
            .radius = 1.0,
            .color = { 0, 0, 255 },
            .shininess = 500.0,
            .reflectiveness = 0.3
        },
        {
            .center = { 0.0, -5001.0, 0.0 },
            .radius = 5000.0,
            .color = { 255, 255, 0 },
            .shininess = 1000.0,
            .reflectiveness = 0.5
        }
    };
    std::vector<cgfs::Light> lights = {
        cgfs::AmbientLight { .intensity = 0.2 },
        cgfs::PointLight { .intensity = 0.6, .position = { 2.0, 1.0, 0.0 } },
        cgfs::DirectionalLight {
            .intensity = 0.2,
            .direction = { 1.0, 4.0, 4.0 }
        }
    };
    cgfs::Camera camera = {
        .position = { 0.0, 0.0, 0.0 },
        .orientation = { 0.0, 0.0, 1.0 }
    };
    cgfs::Viewport viewport = { .width = 1.0, .height = 1.0, .distance = 1.0 };
    cgfs::Canvas canvas(1024, 1024);
    cgfs::raytrace(spheres, lights, camera, viewport, canvas);
    cgfs::save_ppm(canvas, "output.ppm");
    return EXIT_SUCCESS;
}