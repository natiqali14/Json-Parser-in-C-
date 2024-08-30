cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(MySimpleCppProject VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set the source directory
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")

include_directories(${CMAKE_SOURCE_DIR}/src)

# Collect all .cpp files in the src directory
file(GLOB_RECURSE SOURCES "${SRC_DIR}/*.cpp")

# Add the executable
add_executable(app ${SOURCES})

set_target_properties(app PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)