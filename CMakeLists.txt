cmake_minimum_required(VERSION 3.8)
project(VulkanGravity LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE PROJ_SRC src/*.cpp libraries/tinyobjloader/tiny_obj_loader.h)

include_directories(/)
include_directories(libraries/)
include_directories(libraries/glfw/include/)

add_subdirectory(libraries/glfw/)
add_subdirectory(libraries/glm/)

add_executable(VulkanGravity ${PROJ_SRC})

add_custom_command(TARGET VulkanGravity PRE_BUILD COMMAND ../compileShaders.sh)

target_link_libraries(VulkanGravity glm glfw -lvulkan)
#target_link_libraries(VulkanGravity -lOpenCL -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi)