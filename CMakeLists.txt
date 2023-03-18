cmake_minimum_required(VERSION 3.8)
project(VulkanGravity LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

# Add all files/folders under src folder automatically to the project
file(GLOB_RECURSE PROJ_SRC src/*.cpp)

add_executable(VulkanGravity ${PROJ_SRC})

target_link_libraries(VulkanGravity -lOpenCL -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi)