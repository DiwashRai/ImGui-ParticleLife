
cmake_minimum_required(VERSION 3.0)
project(ImGui-ParticleLife)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-g -Wall -Wformat)

add_subdirectory(imgui)
add_subdirectory(src)

