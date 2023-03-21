cmake_minimum_required(VERSION 3.8)
project(VulkanGravity LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

# Add all files/folders under src folder automatically to the project
file(GLOB_RECURSE PROJ_SRC src/*.cpp)

include_directories(libraries/glfw/include/)

add_subdirectory(libraries/glfw/)
add_subdirectory(libraries/glm/)

add_executable(VulkanGravity ${PROJ_SRC})

add_custom_command(TARGET VulkanGravity PRE_BUILD COMMAND ../compileShaders.sh)

target_link_libraries(VulkanGravity glm glfw -lvulkan)
#-lOpenCL -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi