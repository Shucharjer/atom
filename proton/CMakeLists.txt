project(
    proton
    LANGUAGES CXX
)

# CMake

list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules)

# options

option(BUILD_PROTON_TESTING OFF)
option(BUILD_PROTON_BENCHMARK OFF)

# project source

file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(proton STATIC ${SRC_FILES})
add_library(atom::proton ALIAS proton)
target_include_directories(proton PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(proton PUBLIC neutron)
find_package(stdexec CONFIG REQUIRED)
target_link_libraries(proton PUBLIC STDEXEC::stdexec)

# options support

if (BUILD_PROTON_TESTING)
  enable_testing()

  add_executable(example_application ${CMAKE_CURRENT_SOURCE_DIR}/example/src/example_application.cpp)
  target_include_directories(example_application PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/example/include)
  target_link_libraries(example_application PRIVATE proton)

  set(test_dir ${CMAKE_CURRENT_SOURCE_DIR}/test)
  set(test_output_dir ${CMAKE_CURRENT_BINARY_DIR}/test)
  exec_dir(${test_dir} "" ${test_output_dir} true proton)
endif()

if(BUILD_PROTON_BENCHMARK)
  if(NOT ATOM_ENABLE_BENCHMARK)
    message(FATAL_ERROR "Set 'PROTON_BUILD_BENCHMARK', but do not enable 'ATOM_ENABLE_BENCHMARK'")
  endif()

  set(benchmark_dir ${CMAKE_CURRENT_SOURCE_DIR}/benchmark)
  set(benchmark_output_dir ${CMAKE_CURRENT_BINARY_DIR}/benchmark)
  exec_dir(${benchmark_dir} "bm-" ${benchmark_output_dir} false proton benchmark::benchmark)
endif()