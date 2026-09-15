#include <cstdlib>
#include "cgfs/ppm.hpp"
#include "cgfs/raytracer.hpp"
#include "cgfs/util.hpp"

int main() {
    cgfs::Canvas canvas(1280, 720);
    cgfs::Scene scene = {
        .objects = {
            cgfs::Sphere {
                .center = { -1.5, -0.6, -2.2 },
                .radius = 0.4,
                .material = {
                    .ambient = { 255, 255, 255 },
                    .diffuse = { 255, 255, 255 },
                    .specular = { 255, 255, 255 },
                    .shininess = 100.0,
                    .reflectivity = 1.0
                }
            },
            cgfs::Sphere {
                .center = { -2.0, 0.0, -4.0 },
                .radius = 1.0,
                .material = {
                    .ambient = { 0, 255, 0 },
                    .diffuse = { 0, 255, 0 },
                    .specular = { 0, 255, 0 },
                    .shininess = 10.0,
                    .reflectivity = 0.4
                }
            },
            cgfs::Sphere {
                .center = { 0.0, -1.0, -3.0 },
                .radius = 1.0,
                .material = {
                    .ambient = { 255, 0, 0 },
                    .diffuse = { 255, 0, 0 },
                    .specular = { 255, 0, 0 },
                    .shininess = 500.0,
                    .reflectivity = 0.2
                }
            },
            cgfs::Sphere {
                .center = { 2.0, 1.0, -15.0 },
                .radius = 3.0,
                .material = {
                    .ambient = { 255, 255, 255 },
                    .diffuse = { 255, 255, 255 },
                    .specular = { 255, 255, 255 },
                    .shininess = 100.0,
                    .reflectivity = 1.0
                }
            },
            cgfs::Sphere {
                .center = { 2.0, 0.0, -4.0 },
                .radius = 1.0,
                .material = {
                    .ambient = { 0, 0, 255 },
                    .diffuse = { 0, 0, 255 },
                    .specular = { 0, 0, 255 },
                    .shininess = 500.0,
                    .reflectivity = 0.3
                }
            },
            cgfs::Sphere {
                .center = { 0.0, -5001.0, 0.0 },
                .radius = 5000.0,
                .material = {
                    .ambient = { 255, 255, 0 },
                    .diffuse = { 255, 255, 0 },
                    .specular = { 255, 255, 0 },
                    .shininess = 1000.0,
                    .reflectivity = 0.5
                }
            }
        },
        .ambientLight = { .intensity = 0.2 },
        .pointLights = { { .intensity = 0.6, .pos = { 2.0, 1.0, 0.0 } } },
        .directionalLights = {
            { .intensity = 0.2, .dir = { 1.0, 4.0, -4.0 } }
        },
        .viewport = cgfs::Viewport::with(
            cgfs::radians(60.0),
            static_cast<cgfs::f64>(canvas.width())
                / static_cast<cgfs::f64>(canvas.height())
        ),
        .camera = cgfs::Camera::look({ 0.0, 0.0, 0.0 }, { 0.0, 0.0, -1.0 })
    };
    cgfs::raytrace(scene, canvas);
    cgfs::save_ppm(canvas, "output.ppm");
    return EXIT_SUCCESS;
}