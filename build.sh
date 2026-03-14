#!/bin/bash
set -e

echo "🔧 Building Korg Prophecy WASM Emulator..."

# Resolve the repo's wasm_renderer directory (always relative to this script,
# regardless of where the script is invoked from).
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
echo "Source directory: $SCRIPT_DIR"

# Source Emscripten from wherever emsdk lives
CANDIDATES=(
    "/content/build_space/emsdk/emsdk_env.sh"
    "${REPO_ROOT:-}/emsdk/emsdk_env.sh"
    "$HOME/emsdk/emsdk_env.sh"
    "/usr/local/emsdk/emsdk_env.sh"
)
for f in "${CANDIDATES[@]}"; do
    if [ -f "$f" ]; then
        # shellcheck disable=SC1090
        source "$f"
        break
    fi
done


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
