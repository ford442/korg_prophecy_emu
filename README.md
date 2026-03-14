# korg_prophecy_emu

A WebAssembly (WASM) / C++ emulation of the **Korg Prophecy Solo Synthesizer** — recreating its characteristic formant synthesis, VPM (Variable Phase Modulation), and physical modelling engine in real-time, playable in a web browser.

## Goals

- Faithfully emulate the Korg Prophecy's sound engine, including:
  - **Formant synthesis** via parallel resonant filters on a carrier wave
  - **VPM oscillators** (Variable Phase Modulation — Korg's FM variant)
  - **Physical modelling** algorithms (comb filters, waveguides)
  - **ADSR envelope generator** and LFOs
  - **Low-pass / high-pass filter** with resonance
- Target **real-time audio** in browsers via WebAssembly + Web Audio API
- Future integration with [Bespoke Synth](https://www.bespokesynth.com/) as a loadable DSP module

## Tech Stack

| Layer | Technology |
|---|---|
| DSP core | C++17 |
| Build to WASM | [Emscripten](https://emscripten.org/) |
| Browser audio | Web Audio API (AudioWorklet) |
| Desktop build | CMake + native toolchain |
| Unit tests | [Catch2](https://github.com/catchorg/Catch2) |
| Scripting / tooling | Node.js / npm |

## Repository Structure

```
korg_prophecy_emu/
├── CMakeLists.txt          # CMake build (Emscripten + native)
├── build.sh                # Convenience script: compile to WASM
├── index.html              # Browser demo using Web Audio API
├── package.json            # npm scripts (build, test)
├── LICENSE                 # GPL-3.0
├── include/
│   └── DSPUtils.h          # Shared constants (sample rate, buffer size, etc.)
├── src/
│   ├── main.cpp            # WASM entry point; exports initAudio / processAudio
│   ├── ProphecyDSP.h       # Core DSP class declaration
│   ├── ProphecyDSP.cpp     # Formant oscillators, filters, envelope generator
│   ├── AudioEngine.h       # Web Audio API wrapper declaration
│   └── AudioEngine.cpp     # AudioWorklet / SDL2 bridge
├── tests/
│   └── test_dsp.cpp        # Catch2 unit tests for DSP functions
├── docs/
│   ├── parameter_guide/    # Korg Prophecy parameter reference notes
│   └── design_notes/       # Architecture and algorithm design notes
└── build/                  # Emscripten output (prophecy.js, prophecy.wasm) — gitignored
```

## Reference Materials

- Korg Prophecy parameter guide and service manual:
  [bhamadicharef/Korg-Prophecy-Solo-Synthesizer](https://github.com/bhamadicharef/Korg-Prophecy-Solo-Synthesizer)
- Emscripten documentation: https://emscripten.org/docs/
- Web Audio API spec: https://webaudio.github.io/web-audio-api/

## Prerequisites

| Tool | Version |
|---|---|
| Emscripten | ≥ 3.1 |
| CMake | ≥ 3.15 |
| Node.js | ≥ 18 |
| C++ compiler (native) | GCC 11+ / Clang 14+ |

## Build

### WASM (browser target)

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

Output files are placed in `build/`: `prophecy.js` and `prophecy.wasm`.

### Native desktop build

```bash
mkdir -p build_native && cd build_native
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Running tests

```bash
cd build_native
ctest --output-on-failure
```

Or via npm:

```bash
npm run test
```

## Running the Browser Demo

After building the WASM module, serve `index.html` with a local HTTP server (required for `SharedArrayBuffer` / WASM):

```bash
npm run serve
# Then open http://localhost:8080
```

## Future Work

- Load actual Prophecy ROM waveform data for authentic oscillator waves
- MIDI input via Web MIDI API
- AudioWorklet processor for glitch-free real-time audio
- Bespoke Synth module integration (C++ plugin interface)
- Full parameter automation (all Prophecy CC mappings)

## License

GPL-3.0 — see [LICENSE](LICENSE).  
GPL-3.0 is chosen for compatibility with [Bespoke Synth](https://github.com/BespokeSynth/BespokeSynth) (GPL-3.0).

