
function(translate CPPFRONT_EXECUTABLE CPP2_SRC_DIR CPP_OUTPUT_DIR)
  # gather sources
  file(GLOB_RECURSE CPP2_FILES LIST_DIRECTORIES false RELATIVE ${CPP2_SRC_DIR} ${CPP2_SRC_DIR}/*.h2 ${CPP2_SRC_DIR}/*.cpp2)

  # translate
  set(GENERATED_SOURCES "")
  foreach(CPP2_FILE ${CPP2_FILES})
    if(CPP2_FILE MATCHES ".*\\.cpp2$")
      string(REGEX REPLACE "\\.cpp2$" ".cpp" CPP_FILE ${CPP2_FILE})
    elseif(CPP2_FILE MATCHES ".*\\.h2$")
      string(REGEX REPLACE "\\.h2$" ".hpp" CPP_FILE ${CPP2_FILE})
    else()
      # jump
    endif()

    set(input_file ${CPP2_SRC_DIR}/${CPP2_FILE})
    set(output_file ${CPP_OUTPUT_DIR}/${CPP_FILE})

    get_filename_component(output_dir "${output_file}" DIRECTORY)
    file(MAKE_DIRECTORY ${output_dir})

    set(CPPFRONT_COMMAND
      ${CPPFRONT_EXECUTABLE}
      ${input_file}
      -o ${output_file}
      -cl)
    add_custom_command(
      OUTPUT ${output_file}
      COMMAND ${CPPFRONT_COMMAND}
      DEPENDS ${input_file}
      COMMENT "Cpp2 -> Cpp: ${CPP_FILE}"
    )
    list(APPEND GENERATED_SOURCES ${output_file})
  endforeach()

  set(TRANSLATED_SOURCES ${GENERATED_SOURCES} PARENT_SCOPE)
endfunction()

function(target_translate_sources CPPFRONT_EXECUTABLE target visibility)
  set(dirs ${ARGN})
  foreach(dir ${dirs})
    file(RELATIVE_PATH RELATIVE_SUBDIR ${CMAKE_CURRENT_SOURCE_DIR} ${dir})
    set(CPP_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated/${RELATIVE_SUBDIR})
    translate(${CPPFRONT_EXECUTABLE} ${dir} ${CPP_OUTPUT_DIR})
    target_sources(${target} ${visibility} ${TRANSLATED_SOURCES})
  endforeach()
endfunction()

function(target_translate_include_directories CPPFRONT_EXECUTABLE target visibility)
  set(dirs ${ARGN})
  foreach(dir ${dirs})
    file(RELATIVE_PATH RELATIVE_SUBDIR ${CMAKE_CURRENT_SOURCE_DIR} ${dir})
    set(CPP_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated/${RELATIVE_SUBDIR})
    translate(${CPPFRONT_EXECUTABLE} ${dir} ${CPP_OUTPUT_DIR})
    target_sources(${target} ${visibility} ${TRANSLATED_SOURCES})
    target_include_directories(${target} ${visibility} ${CPP_OUTPUT_DIR})
  endforeach()
endfunction()

function(translate_include_directories CPPFRONT_EXECUTABLE CPPFRONT_INCLUDE_DIR target visibility)
  set(dirs ${ARGN})
  foreach(dir ${dirs})
    file(RELATIVE_PATH RELATIVE_SUBDIR ${CMAKE_CURRENT_SOURCE_DIR} ${dir})
    set(CPP_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated/${RELATIVE_SUBDIR})
    translate(${CPPFRONT_EXECUTABLE} ${dir} ${CPP_OUTPUT_DIR})
    target_sources(${target} ${visibility} ${TRANSLATED_SOURCES})
    target_include_directories(${target} ${visibility} ${CPP_OUTPUT_DIR})
  endforeach()
  target_include_directories(${target} ${visibility} ${CPPFRONT_INCLUDE_DIR})
endfunction()
