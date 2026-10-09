examples = {
    'basic-pathtracing': 'basic-pathtracing.cpp',
    'metalness-roughness': 'metalness-roughness.cpp',
    'transparency': 'transparency.cpp',
}

cpp = meson.get_compiler('cpp')
deps = []

if cpp.get_id() == 'gcc'
    # Required for some new C++23 features like `std::println()`.
    deps += cpp.find_library('stdc++exp')
endif

deps += cpp.find_library('m', required: false)
deps += dependency('sdl3')
deps += cgfs_dep

foreach name, src : examples
    executable(name, src, dependencies: deps)
endforeach