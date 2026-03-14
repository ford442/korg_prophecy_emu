# Korg Prophecy Emulation Project

## Project Overview

This is the **korg_prophecy_emu** project — a WebAssembly (WASM) / C++ emulation of the Korg Prophecy Solo Synthesizer. The project aims to recreate the synthesizer's characteristic formant synthesis, VPM (Variable Phase Modulation), and physical modelling engine for real-time playback in web browsers.

**Repository:** https://github.com/ford442/korg_prophecy_emu  
**License:** GPL-3.0 (chosen for compatibility with Bespoke Synth)  
**Language:** English (all documentation and code comments)

### Current State

This project is currently in the **planning/initialization phase**. The repository contains project documentation and utility scripts, but the core implementation files described in the README (C++ source code, build configuration, etc.) are not yet present in the working directory.

---

## Planned Technology Stack

| Layer | Technology |
|-------|------------|
| DSP core | C++17 |
| Build to WASM | [Emscripten](https://emscripten.org/) (≥ 3.1) |
| Browser audio | Web Audio API (AudioWorklet) |
| Desktop build | CMake (≥ 3.15) + native toolchain |
| Unit tests | [Catch2](https://github.com/catchorg/Catch2) |
| Scripting / tooling | Node.js (≥ 18) / npm |

### Compiler Requirements (Native)
- GCC 11+ or Clang 14+

---

## Project Structure

### Current Files (Existing)

```
korg_prophecy_emu/
├── README.md              # Main project documentation
├── deploy.py              # Python SFTP deployment script
├── git.sh                 # Git convenience script (add, commit, push)
└── AGENTS.md              # This file
```

### Planned Structure (Per README)

```
korg_prophecy_emu/
├── CMakeLists.txt         # CMake build (Emscripten + native)
├── build.sh               # Convenience script: compile to WASM
├── index.html             # Browser demo using Web Audio API
├── package.json           # npm scripts (build, test)
├── LICENSE                # GPL-3.0
├── include/
│   └── DSPUtils.h         # Shared constants (sample rate, buffer size, etc.)
├── src/
│   ├── main.cpp           # WASM entry point; exports initAudio / processAudio
│   ├── ProphecyDSP.h      # Core DSP class declaration
│   ├── ProphecyDSP.cpp    # Formant oscillators, filters, envelope generator
│   ├── AudioEngine.h      # Web Audio API wrapper declaration
│   └── AudioEngine.cpp    # AudioWorklet / SDL2 bridge
├── tests/
│   └── test_dsp.cpp       # Catch2 unit tests for DSP functions
├── docs/
│   ├── parameter_guide/   # Korg Prophecy parameter reference notes
│   └── design_notes/      # Architecture and algorithm design notes
└── build/                 # Emscripten output (prophecy.js, prophecy.wasm) — gitignored
```

---

## File Details

### deploy.py
- **Purpose:** Deploy built files to a remote server via SFTP
- **Language:** Python 3
- **Dependencies:** `paramiko`
- **Configuration:**
  - `HOSTNAME = "1ink.us"` — Target server
  - `PORT = 22` — SSH port
  - `USERNAME = "ford442"` — SFTP username
  - `LOCAL_DIRECTORY = "dist"` — Local build output to upload
  - `REMOTE_DIRECTORY = "test.1ink.us/prophecy"` — Target path on server
- **Usage:**
  ```bash
  python deploy.py
  ```
  - Requires `dist/` directory to exist (run `npm run build` first)
  - Hardcoded password (should be moved to environment variable for production)
  - Recursively uploads all files from `dist/` to the remote directory

### git.sh
- **Purpose:** Quick git commit and push
- **Usage:**
  ```bash
  ./git.sh
  ```
  - Adds all changes (`git add .`)
  - Commits with message "push fix"
  - Pushes to origin

---

## Planned Build Commands

### WASM (Browser Target)

```bash
# Activate Emscripten environment first:
source /path/to/emsdk/emsdk_env.sh

# Using the convenience script:
./build.sh

# Or manually via CMake:
mkdir -p build && cd build
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release
emmake make -j$(nproc)
```

Output files will be placed in `build/`: `prophecy.js` and `prophecy.wasm`.

### Native Desktop Build

```bash
mkdir -p build_native && cd build_native
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Running Tests

```bash
# Via CTest
cd build_native
ctest --output-on-failure

# Or via npm
npm run test
```

### Running the Browser Demo

After building the WASM module:

```bash
npm run serve
# Then open http://localhost:8080
```

Requires a local HTTP server (for `SharedArrayBuffer` / WASM support).

---

## Development Conventions

### Code Style Guidelines (Planned)

Based on the project structure documentation:

- **Language:** C++17
- **Naming:** 
  - Classes: `PascalCase` (e.g., `ProphecyDSP`, `AudioEngine`)
  - Files: `PascalCase` for class files (e.g., `ProphecyDSP.cpp`)
  - Headers: Matching `.h` extensions
- **Directory Organization:**
  - `include/` — Public headers
  - `src/` — Implementation files
  - `tests/` — Unit tests
  - `docs/` — Documentation

### Git Conventions

- **Commit signing:** Enabled (`gpgsign = true`)
- **Remote:** `origin` points to `https://github.com/ford442/korg_prophecy_emu`
- **Default branch:** `main`
- **Git LFS:** Enabled (for potentially storing binary assets like ROM data)

---

## Project Goals

### Audio Engine Features (Planned)

- **Formant synthesis** via parallel resonant filters on a carrier wave
- **VPM oscillators** (Variable Phase Modulation — Korg's FM variant)
- **Physical modelling** algorithms (comb filters, waveguides)
- **ADSR envelope generator** and LFOs
- **Low-pass / high-pass filter** with resonance

### Integration Targets

1. **Primary:** Real-time audio in browsers via WebAssembly + Web Audio API
2. **Secondary:** Bespoke Synth integration as a loadable DSP module

---

## Security Considerations

### Current Issues

1. **deploy.py contains hardcoded credentials:**
   - Password is visible in plaintext: `'GoogleBez12!'`
   - Should be migrated to environment variables or a separate config file
   - Should use SSH key authentication instead of password auth

2. **deploy.py uses legacy password authentication:**
   - Consider migrating to SSH key-based authentication for better security

---

## Reference Materials

- Korg Prophecy parameter guide and service manual:  
  [bhamadicharef/Korg-Prophecy-Solo-Synthesizer](https://github.com/bhamadicharef/Korg-Prophecy-Solo-Synthesizer)
- Emscripten documentation: https://emscripten.org/docs/
- Web Audio API spec: https://webaudio.github.io/web-audio-api/

---

## Future Work

Per the README:

- Load actual Prophecy ROM waveform data for authentic oscillator waves
- MIDI input via Web MIDI API
- AudioWorklet processor for glitch-free real-time audio
- Bespoke Synth module integration (C++ plugin interface)
- Full parameter automation (all Prophecy CC mappings)

---

## Notes for AI Agents

1. This is a project skeleton — the core implementation files do not yet exist
2. Before modifying build scripts or adding code, verify which planned files have been created
3. The deployment script (`deploy.py`) should NOT be modified to accept credentials as arguments without proper security review
4. When implementing the C++ DSP code, refer to the Korg Prophecy service manual (linked in reference materials) for authentic algorithm details
5. The project uses Git LFS — be aware when committing binary files
