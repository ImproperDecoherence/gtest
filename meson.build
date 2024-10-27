project('gtest', 'cpp', default_options: ['cpp_std=c++23'])

add_project_arguments('-std=c++23', '-fmax-errors=1', language: 'cpp')

if get_option('buildtype') == 'debug'
    add_project_arguments('-g', language: 'cpp')
endif

gtest_includes = include_directories('src')

gtest_sources = [
    'src/testframework.cpp',    
]

gtest_lib = static_library('gtest', gtest_sources, include_directories: gtest_includes)

gtest_dep = declare_dependency(link_with: gtest_lib, include_directories: gtest_includes)

