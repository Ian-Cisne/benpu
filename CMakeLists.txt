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

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOURCES 
  src/main.cc
  src/video_manager.cc
  src/vulkan_functions.cc)

configure_file(src/configuration.h.in "${PROJECT_SOURCE_DIR}/src/configuration.h")


add_executable(benpu ${SOURCES}) 

target_link_libraries(benpu Boost::log Boost::log_setup Boost::date_time)
target_link_libraries(benpu ${Vulkan_LIBRARIES})
