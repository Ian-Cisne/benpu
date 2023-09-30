cmake_minimum_required(VERSION 3.27)

project(benpu VERSION 0.0.1)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message("Setting build type to 'RelWithDebInfo' as none was specified.")
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
  
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo")
endif()

find_package(Boost COMPONENTS log log_setup date_time REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

if(WIN32)
  set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
elseif(UNIX)
  set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_XLIB_KHR)
endif()

add_subdirectory(lib/volk)
include_directories(lib/volk)

find_package(glm REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

find_package(glfw3 REQUIRED)
include_directories(${GLFW_INCLUDE_DIRS})

find_package(nlohmann_json 3.2.0 REQUIRED)
include_directories(${nlohmann_json_INCLUDE_DIRS})

file(GLOB SHADERS "src/shaders/*.vert" "src/shaders/*.frag")
set(SHADER_BUILD_PATH "${CMAKE_BINARY_DIR}/shaders")
file(MAKE_DIRECTORY ${SHADER_BUILD_PATH})
foreach(SHADER_FILE ${SHADERS})
    get_filename_component(SHADER_NAME ${SHADER_FILE} NAME)
    set(SPIRV_FILE "${SHADER_BUILD_PATH}/${SHADER_NAME}.spv")
    message(${SPIRV_FILE})
    
    add_custom_command(
        OUTPUT ${SPIRV_FILE}
        COMMAND glslc ${SHADER_FILE} -o ${SPIRV_FILE}
        DEPENDS ${SHADER_FILE}
        COMMENT "Compiling ${SHADER_NAME} to SPIR-V"
    )
    
    list(APPEND SPIRV_FILES ${SPIRV_FILE})
endforeach()

add_custom_target(shaders DEPENDS ${SPIRV_FILES})

include_directories(lib/vkfw)
include_directories(src)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOURCES_FILES
  core/configuration_manager.cc
  core/utils/args.cc
  core/utils/system.cc
  render/vulkan/renderer.cc
  render/vulkan/command_buffer.cc
  render/vulkan/swapchain.cc
  render/vulkan/window.cc
)

foreach(SOURCE IN LISTS SOURCES_FILES)
  list(APPEND SOURCES "${PROJECT_SOURCE_DIR}/src/${SOURCE}")
endforeach()

set(BENPU_VERSION_MINOR)

configure_file(configuration.h.in "${PROJECT_SOURCE_DIR}/src/core/utils/configuration.h")

add_library(benpu_lib ${SOURCES})
add_dependencies(benpu_lib shaders)

enable_testing()
add_subdirectory(test)

add_executable(benpu src/main.cc) 

target_link_libraries(benpu_lib Boost::log Boost::log_setup Boost::date_time)
target_link_libraries(benpu_lib Vulkan::Vulkan)
target_link_libraries(benpu_lib glfw)
target_link_libraries(benpu_lib volk)
target_link_libraries(benpu_lib nlohmann_json::nlohmann_json)

target_link_libraries(benpu benpu_lib)

