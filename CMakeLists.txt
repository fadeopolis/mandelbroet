
cmake_minimum_required(VERSION 3.11)
project(mandelbröt)

set(CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED)

find_package(OpenMP)
if (OPENMP_FOUND)
  set(CMAKE_CXX_FLAGS        "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

add_executable(mandelbroet
  mandelbröt.cpp
  common.hpp common.cpp
  Colour.hpp Colour.cpp
  Renderer.hpp Renderer.cpp
  Input.hpp Input.cpp
  Performance_Counter.hpp Performance_Counter.cpp
  Fractal.hpp Fractal.cpp
)
target_link_libraries(mandelbroet SDL2)
target_compile_options(mandelbroet PRIVATE -ffast-math -march=native -mtune=native)
