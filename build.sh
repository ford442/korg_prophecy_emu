#!/bin/bash
set -e

echo "🔧 Building Korg Prophecy WASM Emulator..."

# Check for Emscripten
if ! command -v emcmake &> /dev/null; then
    echo "❌ Emscripten not found. Please install and activate emsdk:"
    echo "   git clone https://github.com/emscripten-core/emsdk.git"
    echo "   cd emsdk && ./emsdk install latest && ./emsdk activate latest"
    exit 1
fi

# Create build directory
mkdir -p build
cd build

# Configure with Emscripten
echo "⚙️  Configuring CMake..."
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
echo "🏗️  Building..."
emmake make -j$(nproc)

echo "✅ Build complete! Output in build/web/"
echo "🚀 Run: cd build/web && python3 -m http.server 8000"
echo "🌐 Then open http://localhost:8000"
