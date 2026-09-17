#include <cstdlib>
#include "cgfs/ppm.hpp"
#include "cgfs/raytracer.hpp"
#include "cgfs/util.hpp"

int main() {
    cgfs::Canvas canvas = cgfs::Canvas::empty(1920, 1080);
    cgfs::Scene scene = {
        .objects = {
            cgfs::Sphere {
                .center = { -2.0, 0.0, -4.0 },
                .radius = 1.0,
                .material = cgfs::Material::create(
                    { 0.0, 1.0, 0.0 },
                    0.25,
                    0.25
                )
            },
            cgfs::Sphere {
                .center = { 0.0, -1.0, -3.0 },
                .radius = 1.0,
                .material = cgfs::Material::create(
                    { 1.0, 0.0, 0.0 },
                    0.1,
                    0.75
                )
            },
            cgfs::Sphere {
                .center = { 2.0, 0.0, -4.0 },
                .radius = 1.0,
                .material = cgfs::Material::create(
                    { 0.0, 0.0, 1.0 },
                    0.35,
                    0.25
                )
            },
            cgfs::Sphere {
                .center = { 0.0, -5001.0, 0.0 },
                .radius = 5000.0,
                .material = cgfs::Material::create(
                    { 1.0, 1.0, 0.0 },
                    0.25,
                    0.25
                )
            }
        },
        .pointLights = {
            {
                .color = { 1.0, 1.0, 1.0 },
                .intensity = 1.0,
                .position = { 2.0, 1.0, 0.0 },
                .radius = 1.0,
                .kc = 1.0,
                .kl = 0.09,
                .kq = 0.032
            }
        },
        .spotLights = { },
        .directionalLights = {
            {
                .color = { 1.0, 1.0, 1.0 },
                .intensity = 1.0,
                .direction = { -4.0, -1.0, -1.0 },
                .radius = 1.0
            }
        },
        .viewport = cgfs::Viewport::create(
            cgfs::radians(60.0),
            static_cast<cgfs::f64>(canvas.width())
                / static_cast<cgfs::f64>(canvas.height())
        ),
        .camera = cgfs::Camera::look({ 0.0, 0.0, 0.0 }, { 0.0, 0.0, -1.0 })
    };
    cgfs::raytrace(scene, canvas, 16);
    cgfs::save_ppm(canvas, "output.ppm");
    return EXIT_SUCCESS;
}