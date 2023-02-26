# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

#file(GLOB ESPIDF_PYTHON_ENV LIST_DIRECTORIES true "~/.espressif/python_env/idf*_py*_env")
#LIST(APPEND CMAKE_PROGRAM_PATH "$ENV{IDF_PATH}/components/esptool_py/esptool" "$ENV{IDF_PATH}/components/espcoredump" "$ENV{IDF_PATH}/components/partition_table" "$ENV{IDF_PATH}/components/app_update" ${ESPIDF_PYTHON_ENV})

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(CBModem)
