cmake_minimum_required(VERSION 3.12)
project(dag-object LANGUAGES CXX)

# Force C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Optional: disable compiler-specific extensions

# Option to build the example executable
option(BUILD_DAG_SAMPLE "Build the dag-object executable AKA sample" OFF)

# Add dag library (always)
add_subdirectory(dag)

# Conditionally add the executable
if(BUILD_DAG_SAMPLE)
    add_subdirectory(dag-object)
endif()