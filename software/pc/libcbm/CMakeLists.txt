cmake_minimum_required (VERSION 3.1)

project (libcbm)

set(CMAKE_INSTALL_PREFIX "/usr/")
include(GNUInstallDirs)

file(GLOB SRC
    "*.cpp"
    "*.c"
    )

add_library(libcbm SHARED ${SRC})
target_compile_options(libcbm PRIVATE "-std=gnu++20" "-g")
set_target_properties(libcbm PROPERTIES PREFIX "")

# Install directives
install(TARGETS libcbm DESTINATION lib)
install(FILES libcbm.h DESTINATION include)
