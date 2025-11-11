
function(translate CPPFRONT_EXECUTABLE CPP2_SRC_DIR CPP_OUTPUT_DIR)
  file(GLOB_RECURSE CPP2_FILES LIST_DIRECTORIES false RELATIVE ${CPP2_SRC_DIR} ${CPP2_SRC_DIR}/*.h2 ${CPP2_SRC_DIR}/*.cpp2)
  set(GENERATED_SOURCES)
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

    message("cmd: ${CPPFRONT_EXECUTABLE} ${input_file} -o ${output_file} -cl")
    add_custom_command(
      OUTPUT ${output_file}
      COMMAND "${CPPFRONT_EXECUTABLE} ${input_file} -o ${output_file} -cl"
      COMMENT "Cpp2 -> Cpp: ${CPP_FILE}"
    )
    list(APPEND GENERATED_SOURCES ${output_file})
  endforeach()
  set(TRANSLATED_SOURCES ${GENERATED_SOURCES} PARENT_SCOPE)
endfunction()