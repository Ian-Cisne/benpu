cmake_minimum_required(VERSION 3.10)

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


include_directories(lib/vkfw)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOURCES 
  src/main.cc
  src/video_manager.cc
  src/vulkan_functions.cc
  src/window.cc)

configure_file(src/configuration.h.in "${PROJECT_SOURCE_DIR}/src/configuration.h")


add_executable(benpu ${SOURCES}) 

target_link_libraries(benpu Boost::log Boost::log_setup Boost::date_time)
target_link_libraries(benpu Vulkan::Vulkan)
target_link_libraries(benpu glfw)
target_link_libraries(benpu volk)

