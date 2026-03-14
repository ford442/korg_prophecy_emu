# CLAUDE.md — Korg Prophecy WASM Emulator

## Project Overview

A WebAssembly emulation of the Korg Prophecy Solo Synthesizer (1995), written in C++17 with Emscripten. The primary focus is High-Level Emulation (HLE) — algorithmic reconstruction of the MOSS synthesis engine, not cycle-accurate LLE (the Korg DSP ASIC is undocumented).

**Target:** Browser-based real-time audio via Web Audio API + Web MIDI.

---

## Tech Stack

| Layer | Technology |
|---|---|
| DSP Engine | C++17 |
| WASM Compilation | Emscripten 3.1+ / emcmake |
| Build System | CMake 3.15+ |
| Audio Backend | SDL2 (Emscripten port) |
| Web Frontend | Vanilla JS + Web MIDI API |
| Testing | Catch2 (native build) |

---

## Repository Layout

```
src/           # C++ DSP implementation
  main.cpp     # Emscripten entry point; all EMSCRIPTEN_KEEPALIVE exports
  ProphecyDSP.h/.cpp   # Polyphony manager (4-voice round-robin + voice stealing)
  AudioEngine.h/.cpp   # SDL2 audio callback bridge
  Voice.h/.cpp         # Single voice: osc → formant bank → ADSR
  FormantBank.h        # 4-band parallel biquad resonant filters
include/
  BiquadFilter.h       # Biquad bandpass implementation
  DSPUtils.h           # Constants, MIDI utils, PolyBLEP, formant tables
tests/
  test_dsp.cpp         # Catch2 unit tests (filter, voice, tuning)
web/
  index.html           # Browser UI (keyboard, controls)
  js/midi.js           # Web MIDI integration
  css/style.css        # Dark theme
docs/
  DESIGN.md            # Signal flow architecture
gemini_report.md       # Exhaustive Prophecy hardware/algorithm whitepaper
AGENTS.md              # Agent-facing project guide (read this too)
```

---

## Build Commands

### WASM Build (requires Emscripten)
```bash
./build.sh
# Output: build/prophecy.js + build/prophecy.wasm
```

### Native Build (for running tests)
```bash
mkdir build_native && cd build_native
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
ctest --output-on-failure
```

### Serve Locally
```bash
cd build && python3 -m http.server 8000
# then open http://localhost:8000
```

---

## Key Conventions

### C++ Style
- C++17; use `auto`, `constexpr`, structured bindings where clear
- No RTTI, no exceptions — WASM hostile
- No heap allocation in the audio callback path (real-time safe)
- Use `std::atomic<bool>` for voice active state (shared across threads)
- All audio processing at **44100 Hz**, block size **512 samples**
- Internal constants live in `DSPUtils.h`

### Emscripten Exports
All JS-callable functions in `main.cpp` must be:
- Declared `extern "C"` to avoid name mangling
- Tagged `EMSCRIPTEN_KEEPALIVE`
- Listed in `CMakeLists.txt` under `-sEXPORTED_FUNCTIONS`

### DSP
- **PolyBLEP** anti-aliasing on all oscillator waveforms (in `DSPUtils.h`)
- **Formant frequencies** are authentic from the Prophecy service manual — do not change without a reference
- Filter topology: parallel biquad bandpass (4 bands), weighted sum F1=1.0, F2=0.9, F3=0.6, F4=0.4
- Voice stealing: steal quietest active voice when all 4 are used

### Git
- Work on feature branches, never commit directly to `main`
- Keep commits focused; reference the synthesis model being implemented in the message
- Do not commit build artifacts (`build/`, `*.wasm`, `*.js` outputs)

---

## Critical Reference: Authentic Formant Frequencies (Hz)

| Vowel | F1  | F2   | F3   | F4   |
|-------|-----|------|------|------|
| A     | 730 | 1090 | 2440 | 3400 |
| E     | 270 | 2290 | 3010 | 3600 |
| I     | 390 | 1990 | 2550 | 3500 |
| O     | 570 |  840 | 2410 | 3300 |
| U     | 300 |  870 | 2240 | 3200 |

Source: Korg Prophecy service manual (via `DSPUtils.h:FORMANT_*` arrays).

---

## Synthesis Models (MOSS Engine — HLE Targets)

The Prophecy has 12 oscillator set types. Priority order for implementation:

1. **Vocal / Formant** — 4-band parallel formant filter (already implemented)
2. **Standard Analog** — sawtooth + square with PWM (partially implemented)
3. **VPM (Variable Phase Modulation)** — Korg's FM variant with feedback; see `gemini_report.md §3.4`
4. **Ring Modulation** — dual oscillator ring mod
5. **Cross Modulation** — exponential FM (non-linear)
6. **Hard Sync** — oscillator slave/master reset
7. **Comb Filter** — noise + comb, precursor to waveguide
8. **Brass (Waveguide)** — lip-reed physical model
9. **Reed (Waveguide)** — pressure-controlled aperture
10. **Plucked String (Karplus-Strong)** — with inharmonicity
11. **Resonator** — body resonance model
12. **Organ** — additive harmonic stacking

---

## Security Note

`deploy.py` contains a **hardcoded plaintext password**. Do not run it as-is; migrate to environment variables or SSH key auth before any production deployment. Never commit credentials.

---

## Key References

- `gemini_report.md` — Deep technical analysis of Prophecy hardware, MOSS DSP, synthesis algorithms, and web audio architecture. Read this before implementing any new synthesis model.
- `docs/DESIGN.md` — Signal flow and filter design notes.
- `AGENTS.md` — Additional context for AI assistants working on this project.
- Synthesis ToolKit (STK) — Reference for digital waveguide physical models.
