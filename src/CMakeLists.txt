set(target monitor) # TODO: monitor?

set(include_path "${CMAKE_SOURCE_DIR}/include/")
set(source_path  ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE headers ${include_path}/*.h)
file(GLOB_RECURSE sources ${source_path}/*.cpp)

add_library(${target} ${headers} ${sources})
add_library(${PROJECT_NAME}::${target} ALIAS ${target})

set_target_properties(PROPERTIES ${DEFAULT_PROJECT_OPTIONS})

target_include_directories(${target} PRIVATE ${INCLUDE_PATH})

target_compile_options(${target} PRIVATE ${DEFAULT_COMPILE_OPTIONS})

target_link_libraries(${target} PRIVATE ${DEFAULT_LINKER_OPTIONS})
