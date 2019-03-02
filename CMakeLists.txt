
cmake_minimum_required(VERSION 3.11)
project(mandelbröt)

set(CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED)

add_executable(mandelbroet mandelbröt.cpp)
target_link_libraries(mandelbroet SDL2)
target_compile_options(mandelbroet PRIVATE -ffast-math)