set(target monitor) # TODO: monitor?

set(include_path "${CMAKE_SOURCE_DIR}/include/")
set(source_path  ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE headers_and_sources ${include_path}/*.h ${source_path}/*.cpp)

add_library(${target} ${headers_and_sources})
add_library(${PROJECT_NAME}::${target} ALIAS ${target})