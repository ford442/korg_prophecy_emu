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
    
    add_executable(prophecy ${SOURCES} ${HEADERS})
    
    target_include_directories(prophecy PRIVATE include src)
    
    # Emscripten flags for audio and performance
    target_link_options(prophecy PRIVATE
        "-s WASM=1"
        "-s EXPORT_ES6=1"
        "-s MODULARIZE=1"
        "-s EXPORT_NAME=ProphecyModule"
        "-s USE_SDL=2"
        "-s INITIAL_MEMORY=64MB"
        "-s ALLOW_MEMORY_GROWTH=1"
        "-s EXPORTED_FUNCTIONS=['_malloc','_free','_initAudio','_processMidi','_setFormant','_setWaveform','_setCutoff','_setResonance']"
        "-s EXPORTED_RUNTIME_METHODS=['ccall','cwrap','UTF8ToString']"
        "-s ENVIRONMENT=web,worker"
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
