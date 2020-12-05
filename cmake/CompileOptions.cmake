set(DEFAULT_PROJECT_OPTIONS
    CXX_STANDARD              20
    CXX_STANDARD_REQUIRED     ON
    LINKER_LANGUAGE           "CXX"
    CXX_VISIBILITY_PRESET     "default"
)

# Compile definitions
set(DEFAULT_COMPILE_OPTIONS
        -Wall
        -Wextra
        -Wunused

        -Wreorder
        -Wignored-qualifiers
        -Wmissing-braces
        -Wreturn-type
        -Wswitch
        -Wuninitialized
        -Wmissing-field-initializers

        -std=c++20 # TODO: should work without this line
)

# Linker options
set(DEFAULT_LINKER_OPTIONS -pthread)

