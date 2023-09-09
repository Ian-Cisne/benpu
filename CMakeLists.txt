cmake_minimum_required(VERSION 3.10)

project(benpu VERSION 0.0.1)

find_package(Boost COMPONENTS log log_setup REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})


find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOURCES 
  src/main.cc)

configure_file(src/configuration.h.in "${PROJECT_SOURCE_DIR}/src/configuration.h")


add_executable(benpu ${SOURCES}) 

target_link_libraries(benpu Boost::log Boost::log_setup)
target_link_libraries(benpu ${Vulkan_LIBRARIES})
