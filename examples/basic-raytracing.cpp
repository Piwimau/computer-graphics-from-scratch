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
            .center = { 0.0F, -1.0F, 3.0F },
            .radius = 1.0F,
            .color = { 255, 0, 0 },
            .shininess = 500.0F
        },
        {
            .center = { -2.0F, 0.0F, 4.0F },
            .radius = 1.0F,
            .color = { 0, 255, 0 },
            .shininess = 10.0F
        },
        {
            .center = { 2.0F, 0.0F, 4.0F },
            .radius = 1.0F,
            .color = { 0, 0, 255 },
            .shininess = 500.0F
        },
        {
            .center = { 0.0F, -5001.0F, 0.0F },
            .radius = 5000.0F,
            .color = { 255, 255, 0 },
            .shininess = 1000.0F
        }
    };
    std::vector<cgfs::Light> lights = {
        cgfs::AmbientLight { .intensity = 0.2F },
        cgfs::PointLight {
            .intensity = 0.6F,
            .position = { 2.0F, 1.0F, 0.0F }
        },
        cgfs::DirectionalLight {
            .intensity = 0.2F,
            .direction = { 1.0F, 4.0F, 4.0F }
        }
    };
    cgfs::Camera camera = {
        .position = { 0.0F, 0.0F, 0.0F },
        .orientation = { 0.0F, 0.0F, 1.0F }
    };
    cgfs::Viewport viewport = {
        .width = 1.0F,
        .height = 1.0F,
        .distance = 1.0F
    };
    cgfs::Canvas canvas(1024, 1024);
    cgfs::raytrace(spheres, lights, camera, viewport, canvas);
    cgfs::save_ppm(canvas, "output.ppm");
    return EXIT_SUCCESS;
}