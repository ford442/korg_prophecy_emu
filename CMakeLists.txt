cmake_minimum_required(VERSION 3.15)
project(ProphecyWASM VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    src/main.cpp
    src/ProphecyDSP.cpp
    src/AudioEngine.cpp
    src/Voice.cpp
)

set(HEADERS
    include/DSPUtils.h
    include/BiquadFilter.h
    src/ProphecyDSP.h
    src/AudioEngine.h
    src/Voice.h
    src/FormantBank.h
)

# Emscripten-specific configuration
if(EMSCRIPTEN)
    message(STATUS "Configuring for Emscripten/WebAssembly")
    
    # Set SDL2 usage globally - this is crucial for Emscripten ports
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -sUSE_SDL=2")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -sUSE_SDL=2")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sUSE_SDL=2")
    
    add_executable(prophecy ${SOURCES} ${HEADERS})
    
    target_include_directories(prophecy PRIVATE include src)
    
    target_compile_definitions(prophecy PRIVATE SDL_DISABLE_IMMINTRIN_H)
    
    # Additional Emscripten link options
    target_link_options(prophecy PRIVATE
        "-sWASM=1"
        "-sEXPORT_NAME=ProphecyModule"
        "-sINITIAL_MEMORY=64MB"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sEXPORTED_FUNCTIONS=['_malloc','_free','_initAudio','_shutdownAudio','_midiNoteOn','_midiNoteOff','_setFormant','_setWaveform','_setCutoff','_setResonance','_allNotesOff']"
        "-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap','UTF8ToString']"
        "-sENVIRONMENT=web"
        "-O3"
        "-flto"
        "--closure=1"
    )
    
    # Set output to web directory
    set_target_properties(prophecy PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/web
    )
    
    # Copy HTML/JS assets
    file(COPY web/ DESTINATION ${CMAKE_BINARY_DIR}/web)
    
else()
    # Native build for testing (Linux/macOS)
    message(STATUS "Configuring for native build")
    
    find_package(SDL2 REQUIRED)
    
    add_executable(prophecy_native ${SOURCES} ${HEADERS})
    target_include_directories(prophecy_native PRIVATE include src ${SDL2_INCLUDE_DIRS})
    target_link_libraries(prophecy_native ${SDL2_LIBRARIES})
    
    enable_testing()
    add_subdirectory(tests)
endif()
