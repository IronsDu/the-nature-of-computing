cmake_minimum_required (VERSION 3.14)

project(theory-computation)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")

include(FetchContent)

# Fetch Catch2
FetchContent_Declare(
  catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.7.0
)
FetchContent_MakeAvailable(catch2)

# Add sub directories
add_subdirectory(chapter3)
