# Required libraries
find_package(Boost 1.71.0 REQUIRED COMPONENTS system program_options)
find_package(nlohmann_json 3.7.3 REQUIRED)

# Optional library for building and running tests
if(BUILD_TESTS)
    find_package(Catch2 2.0.0 REQUIRED)
endif()

