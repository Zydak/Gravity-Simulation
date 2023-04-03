cmake_minimum_required(VERSION 3.8)
project(VulkanGravity LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE PROJ_SRC src/*.cpp)

include_directories(/)
include_directories(libraries/)
include_directories(libraries/glfw/include/)

add_subdirectory(libraries/glfw/)
add_subdirectory(libraries/glm/)

if (DEFINED VULKAN_SDK_PATH)
    set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include") # 1.1 Make sure this include path is correct
    set(Vulkan_LIBRARIES "${VULKAN_SDK_PATH}/Lib") # 1.2 Make sure lib path is correct
    set(Vulkan_FOUND "True")
else()
    find_package(Vulkan REQUIRED) # throws error if could not find Vulkan
    message(STATUS "Found Vulkan: $ENV{VULKAN_SDK}")
endif()
if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Could not find Vulkan library!")
else()
    message(STATUS "Using vulkan lib at: ${Vulkan_LIBRARIES}")
endif()

add_executable(VulkanGravity ${PROJ_SRC})

add_custom_command(TARGET VulkanGravity PRE_BUILD COMMAND ../compileShadersAuto.sh)

target_link_libraries(VulkanGravity glm glfw ${Vulkan_LIBRARIES})
#target_link_libraries(VulkanGravity -lOpenCL -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi)