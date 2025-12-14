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
add_executable(world_base ${CMAKE_CURRENT_SOURCE_DIR}/test/src/world_base.cpp)
target_include_directories(world_base PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/test/include)
target_link_libraries(world_base PRIVATE proton)
add_executable(commands ${CMAKE_CURRENT_SOURCE_DIR}/test/src/commands.cpp)
target_include_directories(commands PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/test/include)
target_link_libraries(commands PRIVATE proton)
add_executable(archetype ${CMAKE_CURRENT_SOURCE_DIR}/test/src/archetype.cpp)
target_include_directories(archetype PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/test/include)
target_link_libraries(archetype PRIVATE proton)
add_executable(query ${CMAKE_CURRENT_SOURCE_DIR}/test/src/query.cpp)
target_include_directories(query PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/test/include)
target_link_libraries(query PRIVATE proton)
endif()