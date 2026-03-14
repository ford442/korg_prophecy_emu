cmake_minimum_required(VERSION 3.15)

# Find Catch2 or download it
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(test_dsp test_dsp.cpp)
target_include_directories(test_dsp PRIVATE ../include ../src)
target_link_libraries(test_dsp PRIVATE Catch2::Catch2WithMain)

# Register with CTest
include(CTest)
include(Catch)
catch_discover_tests(test_dsp)
