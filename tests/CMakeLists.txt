set(target tests)

file(GLOB_RECURSE sources "*.cpp")

add_executable(${target} ${sources})

set_target_properties(PROPERTIES ${DEFAULT_PROJECT_OPTIONS})

target_include_directories(${target} PRIVATE ${INCLUDE_PATH})

target_compile_options(${target} PRIVATE ${DEFAULT_COMPILE_OPTIONS})

target_link_libraries(${target} 
    PRIVATE 
    ${PROJECT_NAME}::hw-eaton-lib
    Catch2::Catch2
    ${DEFAULT_LINKER_OPTIONS}
)
