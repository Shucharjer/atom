project(
    proton
    LANGUAGES CXX
)

file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(proton STATIC ${SRC_FILES})
target_include_directories(proton PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(proton PUBLIC neutron)
find_package(stdexec CONFIG REQUIRED)
target_link_libraries(proton PUBLIC STDEXEC::stdexec)

option(PROTON_BUILD_TEST OFF)

if (PROTON_BUILD_TEST)
add_executable(example_application ${CMAKE_CURRENT_SOURCE_DIR}/test/src/example_application.cpp)
target_include_directories(example_application PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/test/include)
target_link_libraries(example_application PRIVATE proton)
endif()