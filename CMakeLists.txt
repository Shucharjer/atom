cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 20 CACHE STRING "The C++ standard to use")

project(
  atom
  DESCRIPTION
    "A headers-only util library in C++20, including reflection, structures, and some magic."
  LANGUAGES CXX)

# options

option(ATOM_BUILD_DOCS "" OFF)
option(ATOM_ENABLE_BENCHMARK "Enable benchmark" OFF)
option(ATOM_ENABLE_AVX512 "Enable AVX512 instructions" OFF)
option(ATOM_ENABLE_AVX10_1 "Enable AVX10.1. Enable AVX512 before enable this" OFF)
option(ATOM_ENABLE_AVX512VL "Enable AVX512VL" OFF)
option(ATOM_ENABLE_AVX512BW "Enable AVX512BW" OFF)

# ATOM_ENABLE_AVX (simd)

if (MSVC)
  add_compile_options(/arch:SSE2 /arch:SSE4.2 /arch:AVX /arch:AVX2)
  if (ATOM_ENABLE_AVX512)
    add_compile_options(/arch:AVX512)
    if (ATOM_ENABLE_AVX10_1)
      add_compile_options(/arch:AVX10.1)
    endif()
  endif()
else()
  add_compile_options(-msse -msse4.1 -msse4.2 -mavx -mavx2 -mfma)
  if (ATOM_ENABLE_AVX512)
    add_compile_options(-mavx512f)
    if (ATOM_ENABLE_AVX10_1)
      add_compile_options("-mavx10.1")
    endif()
    if (ATOM_ENABLE_AVX512VL)
      add_compile_options(-mavx512vl)
    endif()
    if (ATOM_ENABLE_AVX512BW)
      add_compile_options(-mavx512bw)
    endif()
  endif()
endif()

include(GNUInstallDirs)

add_subdirectory(neutron)
add_subdirectory(proton)
add_subdirectory(electron)

if(ATOM_BUILD_DOCS)
  find_package(Doxygen REQUIRED dot)
  if(NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "Required to build documents but doxygen is not found!")
  endif()
  include(FetchContent)

  FetchContent_Declare(
    doxygen-awesome-css
    GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css
    GIT_TAG main
    GIT_SHALLOW 1)
  FetchContent_MakeAvailable(doxygen-awesome-css)

  FetchContent_GetProperties(doxygen-awesome-css)

  if(NOT doxygen-awesome-css_POPULATED)
    FetchContent_Populate(doxygen-awesome-css)
    set(doxygen-awesome-css_INCLUDE_DIR ${doxygen-awesome-css_SOURCE_DIR})
  endif()
endif()

if(ATOM_ENABLE_BENCHMARK)
  include(FetchContent)
  # set(FETCHCONTENT_FULLY_DISCONNECTED ON)
  FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG main
    GIT_SHALLOW 1)
  set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON)
  FetchContent_MakeAvailable(googlebenchmark)
endif()

if(ATOM_BUILD_EXAMPLE)
  add_subdirectory(example)
endif()