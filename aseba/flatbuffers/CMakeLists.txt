function(compile_flatbuffers_schema_to_cpp SRC_FBS HEADER)
  get_filename_component(SRC_FBS_ABS ${SRC_FBS} ABSOLUTE)
  string(REGEX REPLACE "\\.fbs$" "_generated.h" GEN_HEADER ${SRC_FBS})
  set(${HEADER} ${GEN_HEADER})
  add_custom_command(
    OUTPUT ${GEN_HEADER}
    COMMAND "$<TARGET_FILE:flatc>" --cpp --no-includes --gen-mutable --scoped-enums
            --gen-object-api -o "${CMAKE_CURRENT_BINARY_DIR}"
            --cpp-ptr-type std::unique_ptr
            "${SRC_FBS_ABS}"
    DEPENDS flatc ${SRC_FBS_ABS}
    )
    set(${HEADER} "${${HEADER}}" PARENT_SCOPE)
endfunction()

function(compile_flatbuffers_schema_to_js SRC_FBS HEADER)
  get_filename_component(SRC_FBS_ABS ${SRC_FBS} ABSOLUTE)
  string(REGEX REPLACE "\\.fbs$" "_generated.js" GEN_HEADER ${SRC_FBS})
  set(${HEADER} ${GEN_HEADER})
  add_custom_command(
    OUTPUT ${GEN_HEADER}
    COMMAND "$<TARGET_FILE:flatc>" --js --gen-mutable --es6-js-export
            --gen-object-api -o "${CMAKE_BINARY_DIR}/js"
            "${SRC_FBS_ABS}"
    DEPENDS flatc ${SRC_FBS_ABS}
    )
    set(${HEADER} "${${HEADER}}" PARENT_SCOPE)
endfunction()


compile_flatbuffers_schema_to_cpp(thymio.fbs FLATBUFFER_HEADER)
compile_flatbuffers_schema_to_js(thymio.fbs FLATBUFFER_JS)
add_library(thymio-flatbuffers
    thymio-flatbuffers.cpp
    ${FLATBUFFER_HEADER}
    ${FLATBUFFER_JS}
    thymio.fbs
)
target_link_libraries(thymio-flatbuffers PUBLIC flatbuffers)
target_include_directories(thymio-flatbuffers PUBLIC ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/third_party/flatbuffers/include)
