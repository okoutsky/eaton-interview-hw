set(target file_reading_device_tool)

set(source_path  ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE sources ${source_path}/*.cpp)

add_executable(${target} ${sources})

set_target_properties(PROPERTIES ${DEFAULT_PROJECT_OPTIONS})

target_include_directories(${target} PRIVATE ${INCLUDE_PATH})

target_compile_options(${target} PRIVATE ${DEFAULT_COMPILE_OPTIONS})

target_link_libraries(${target} PRIVATE ${DEFAULT_LINKER_OPTIONS} ${PROJECT_NAME}::hw-eaton-lib Boost::program_options)
