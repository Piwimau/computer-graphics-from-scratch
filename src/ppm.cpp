#include <cstdio>
#include <format>
#include <memory>
#include <print>
#include <stdexcept>
#include "cgfs/ppm.hpp"
#include "func-deleter.hpp"

namespace cgfs {

/** @brief Represents a custom deleter for `std::FILE` handles. */
using FileDeleter = FuncDeleter<&std::fclose>;

void save_ppm(const Canvas& canvas, const std::string& path) {
    std::unique_ptr<std::FILE, FileDeleter> file(
        std::fopen(path.c_str(), "wb")
    );
    if (file == nullptr) {
        throw std::runtime_error(std::format("Failed to open '{}'.", path));
    }
    std::println(file.get(), "P6");
    std::println(file.get(), "{} {}", canvas.width, canvas.height);
    std::println(file.get(), "255");
    usize n = std::fwrite(
        canvas.pixels.get(),
        sizeof(Pixel),
        canvas.width * canvas.height,
        file.get()
    );
    if (static_cast<isize>(n) != canvas.width * canvas.height) {
        throw std::runtime_error(std::format("Failed to write to '{}'.", path));
    }
}

}