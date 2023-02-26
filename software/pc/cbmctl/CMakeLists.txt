cmake_minimum_required (VERSION 3.1)

project (cbmctl)

add_executable(cbmctl cbmctl.cpp)
target_compile_options(cbmctl PRIVATE "-std=gnu++20" "-g")
