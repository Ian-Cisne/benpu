
find_package(Boost COMPONENTS unit_test_framework REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(
  test_args 
  core/utils/test_args.cc
)

target_link_libraries(
  test_args Boost::unit_test_framework)

target_link_libraries(test_args benpu_lib)

add_test(NAME test_args COMMAND test_args)