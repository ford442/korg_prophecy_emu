# Korg Prophecy WASM Emulator

A high-performance WebAssembly emulation of the Korg Prophecy solo synthesizer, focusing on the iconic Formant/Vocal synthesis model. Built with Emscripten and modern C++17.

## Features

- **Formant Synthesis Engine**: 4-band resonant filter bank modeling vocal tract acoustics
- **Multi-Waveform Carrier**: Anti-aliased saw, square, and triangle oscillators with PWM
- **Polyphonic Architecture**: 4-voice polyphony with voice allocation
- **Web MIDI Support**: Full hardware controller integration
- **Bespoke Synth Compatible**: Modular I/O design for future patching integration

## Technical Stack

- **DSP**: Custom C++17 formant filter bank, ADSR envelopes, PolyBLEP oscillators
- **Web**: Emscripten WASM target, Web Audio API via SDL2, Web MIDI API
- **Build**: CMake 3.15+, Emscripten 3.1.0+

## References

This implementation references the [Korg Prophecy Parameter Guide and Service Manual](https://github.com/bhamadicharef/Korg-Prophecy-Solo-Synthesizer) for authentic signal flow and parameter ranges.

### Authentic Formant Frequencies (Prophecy Vocal Model)

| Vowel | F1 (Hz) | F2 (Hz) | F3 (Hz) | F4 (Hz) | Bandwidth |
|-------|---------|---------|---------|---------|-----------|
| A     | 730     | 1090    | 2440    | 3400    | 80-150Hz  |
| E     | 270     | 2290    | 3010    | 3600    | 60-120Hz  |
| I     | 390     | 1990    | 2550    | 3500    | 70-130Hz  |
| O     | 570     | 840     | 2410    | 3300    | 80-140Hz  |
| U     | 300     | 870     | 2240    | 3200    | 60-110Hz  |

*Note: Authentic Prophecy waveforms require ROM dumps. This implementation uses mathematically equivalent band-limited synthesis.*

## Build Instructions

```bash
# Install Emscripten (https://emscripten.org/docs/getting_started/downloads.html)
git clone https://github.com/emscripten-core/emsdk.git

# Build WASM target
./build.sh

# Serve and test
cd build/web
python3 -m http.server 8000
```

## Architecture

The DSP follows the Prophecy's "Vocal" model signal flow:

```
Multi-oscillator carrier (sawtooth recommended for rich harmonics)
    ↓
Parallel formant filter bank (4 resonant bandpass filters)
    ↓
Amplitude modulation via ADSR envelope
    ↓
Mono/Poly voice allocation with portamento option
```

## License

GPL-3.0 - Compatible with Bespoke Synth modular environment.
