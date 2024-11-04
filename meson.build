project('gtest', 'cpp', default_options: ['cpp_std=c++23'])

add_project_arguments('-fmax-errors=1', language: 'cpp')

gtest_includes = include_directories('src')

gtest_sources = [
    'src/g_test_framework.cpp',
]

gtest_lib = static_library('gtest', gtest_sources, include_directories: gtest_includes)

gtest_dep = declare_dependency(link_with: gtest_lib, include_directories: gtest_includes)
