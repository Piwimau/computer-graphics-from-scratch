project(
    'cgfs',
    'cpp',
    default_options: [
        'cpp_std=c++23',
        'buildtype=debug',
        'b_ndebug=if-release',
        'warning_level=3',
    ],
    meson_version: '>=1.1',
    version: '0.1.0',
)

cpp = meson.get_compiler('cpp')
deps = []

if cpp.get_id() == 'gcc'
    add_project_arguments(
        '-Wundef',
        '-Wshadow=local',
        '-Wconversion',
        '-Wdouble-promotion',
        '-Wcast-align',
        '-Wcast-qual',
        '-Wlogical-op',
        '-Wpointer-arith',
        '-Wformat=2',
        '-Wnrvo',
        '-Wswitch-default',
        '-Wswitch-enum',
        '-Wuseless-cast',
        '-Wmissing-declarations',
        '-Wmissing-include-dirs',
        '-Wduplicated-cond',
        '-Wduplicated-branches',
        '-Wzero-as-null-pointer-constant',
        '-Wunused-macros',
        '-Wenum-conversion',
        '-Wflex-array-member-not-at-end',
        '-Wredundant-decls',
        language: 'cpp',
    )
    if get_option('native')
        add_project_arguments('-march=native', '-mtune=native', language: 'cpp')
    endif
    # Required for some new C++23 features like `std::println()`.
    deps += cpp.find_library('stdc++exp')
endif

deps += cpp.find_library('m', required: false)

incs = include_directories('include')
srcs = files('src/pathtracer.cpp')

if get_option('default_library') == 'static'
    lib_cpp_args = ['-DCGFS_STATIC']
    dep_cpp_args = ['-DCGFS_STATIC']
else
    lib_cpp_args = ['-DCGFS_BUILD_SHARED']
    dep_cpp_args = ['-DCGFS_SHARED']
endif

cgfs_lib = library(
    'cgfs',
    srcs,
    include_directories: incs,
    dependencies: deps,
    gnu_symbol_visibility: 'hidden',
    cpp_args: lib_cpp_args,
    install: true,
)

install_subdir('include/cgfs', install_dir: 'include')

cgfs_dep = declare_dependency(
    include_directories: incs,
    link_with: cgfs_lib,
    compile_args: dep_cpp_args,
)

meson.override_dependency('cgfs', cgfs_dep)

pkg = import('pkgconfig')
pkg.generate(
    cgfs_lib,
    description: 'A small computer graphics library.',
    url: 'https://github.com/Piwimau/computer-graphics-from-scratch',
    extra_cflags: dep_cpp_args,
)

if get_option('examples')
    subdir('examples')
endif