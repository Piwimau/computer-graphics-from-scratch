#include <cstdio>
#include <format>
#include <memory>
#include <print>
#include <stdexcept>
#include "cgfs/ppm.hpp"
#include "cgfs/types.hpp"

namespace cgfs {

/** @brief Represents a custom deleter for `std::FILE` handles. */
struct FileDeleter {

    /**
     * @brief Deletes a `std::FILE` handle.
     *
     * @warning The behavior is undefined if `file` is a `nullptr` or if it has
     * already been closed.
     *
     * @param[in, out] file The `std::FILE` handle to delete.
     */
    void operator()(std::FILE* file) const noexcept {
        std::fclose(file);
    }

};

void save_ppm(const Canvas& canvas, const std::string& path) {
    std::unique_ptr<std::FILE, FileDeleter> file(
        std::fopen(path.c_str(), "wb")
    );
    if (file == nullptr) {
        throw std::runtime_error(std::format("Failed to open '{}'.", path));
    }
    std::println(file.get(), "P6");
    std::println(file.get(), "{} {}", canvas.width(), canvas.height());
    std::println(file.get(), "255");
    usize n = std::fwrite(
        canvas.pixels().data(),
        sizeof(Pixel),
        canvas.pixels().size(),
        file.get()
    );
    if (n != canvas.pixels().size()) {
        throw std::runtime_error(std::format("Failed to write to '{}'.", path));
    }
}

}