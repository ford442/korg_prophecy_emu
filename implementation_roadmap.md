# Implementation Roadmap — Korg Prophecy WASM Emulator

> Synthesized from: `AGENTS.md`, `docs/DESIGN.md`, `gemini_report.md`, `README.md`, and current source state.
> Last updated: 2026-03-14

---

## Current Status

The foundation is in place: a working 4-voice polyphonic **Vocal/Formant** model compiles to WASM and runs in-browser with Web MIDI support.

| Component | State |
|---|---|
| Biquad bandpass filter | Complete |
| PolyBLEP oscillator (saw, square, triangle, pulse) | Complete |
| 4-band formant bank (A/E/I/O/U) | Complete |
| ADSR envelope | Complete |
| 4-voice round-robin polyphony + voice stealing | Complete |
| SDL2 audio callback / Emscripten bridge | Complete |
| Web MIDI + browser keyboard UI | Complete |
| Catch2 unit tests (filter, voice, tuning) | Complete |

---

## Phase 1 — Stability & Authenticity (Short Term)

Goal: Make the existing Vocal model production-quality and audibly accurate.

### 1.1 Audio Quality
- [ ] Raise internal sample rate from 44100 to **48000 Hz** (Prophecy hardware rate per `gemini_report.md §2.3`)
- [ ] Add **oversampling** (2x) for the oscillator stage to reduce aliasing at high pitches
- [ ] Tune formant **Q factors** per Prophecy service manual (currently uncalibrated)
- [ ] Add **formant interpolation** — smooth morphing between vowels instead of hard switch
- [ ] Fix soft-clipping in `ProphecyDSP.cpp` — current `tanh` approximation introduces DC offset; replace with proper waveshaper

### 1.2 Envelope
- [ ] Implement **non-linear ADSR curves** (exponential attack/decay, matching hardware behavior)
- [ ] Add **velocity sensitivity** to envelope amplitude (currently velocity is unused)
- [ ] Add **key tracking** for filter cutoff

### 1.3 Portamento / Glide
- [ ] Verify portamento frequency interpolation is pitch-linear (not frequency-linear)
- [ ] Expose portamento time as a MIDI CC (suggest CC5)

### 1.4 Testing
- [ ] Add Catch2 tests for ADSR curve shapes
- [ ] Add test for formant vowel interpolation output
- [ ] Add regression test for sample rate constant

---

## Phase 2 — Synthesis Model Expansion (Medium Term)

Goal: Implement the full MOSS oscillator set beyond Formant/Vocal.

### 2.1 Standard Analog Oscillators
- [ ] **PWM (Pulse Width Modulation)** — LFO-driven pulse width; CC-mappable
- [ ] **Noise generator** — white noise source for comb/filter models
- [ ] **Sub-oscillator** — square wave one octave below fundamental
- [ ] **Oscillator detune** — slight pitch spread for chorus/unison effect

### 2.2 VPM (Variable Phase Modulation)
Reference: `gemini_report.md §3.4`
- [ ] Implement **carrier + modulator** oscillator pair
- [ ] Add **feedback path** on modulator (distinguishes VPM from standard FM)
- [ ] Implement 6 VPM algorithms (operator routing variants)
- [ ] Add **modulation depth** envelope (EG3 in Prophecy architecture)
- [ ] Expose VPM parameters to JS API: `setVPMDepth`, `setVPMRatio`, `setVPMFeedback`

### 2.3 Oscillator Interactions
- [ ] **Ring Modulation** — multiply carrier × modulator signals
- [ ] **Cross Modulation (XMod)** — exponential FM (non-linear frequency modulation)
- [ ] **Hard Sync** — reset slave oscillator phase on master cycle; implement with PolyBLEP correction at sync point

### 2.4 Comb Filter Model
Reference: `gemini_report.md §3.5`
- [ ] Implement delay-line comb filter
- [ ] Add **noise injection** at comb input
- [ ] Add **feedback coefficient** control (resonance/decay)
- [ ] Tune delay time to pitch (note → comb frequency)

### 2.5 Physical Models (Waveguide)
Reference: `gemini_report.md §3.6-3.8`; consider STK (Synthesis ToolKit) as reference

**Brass:**
- [ ] Lip-reed nonlinear junction
- [ ] Cylindrical bore waveguide
- [ ] Breath pressure modulation (mapped to mod wheel / CC2)
- [ ] Bell radiation filter

**Reed (Clarinet/Saxophone approximation):**
- [ ] Pressure-dependent reed aperture (13 reed parameters per `gemini_report.md`)
- [ ] Cylindrical vs conical bore selection
- [ ] Reed stiffness and damping parameters

**Plucked String (Karplus-Strong extended):**
- [ ] Delay line with low-pass loop filter
- [ ] Inharmonicity filter (dispersive waveguide)
- [ ] Attack noise injection (plectrum model)
- [ ] Attack level control

---

## Phase 3 — Effects & Modulation (Medium Term)

Goal: Replicate Prophecy's post-filter effects chain.

### 3.1 Effects Chain
Reference: `gemini_report.md §2.2` (signal path: oscillators → wave shapers → mixer → filters → amplifier → effects)

- [ ] **Chorus** — short delay with LFO modulation, stereo spread
- [ ] **Delay** — tempo-syncable digital delay with feedback
- [ ] **Reverb** — Schroeder or FDN reverb (keep simple for WASM budget)

### 3.2 Wave Shaping
- [ ] **CLIP** — hard saturation (tanh/clamp with drive parameter)
- [ ] **RESO** — wavefolding (oscillator output fed back through folder)

### 3.3 Modulation Matrix
- [ ] LFO with waveform selection (sine, triangle, sample-and-hold)
- [ ] LFO → pitch, filter, formant, VPM depth
- [ ] Mod wheel (CC1) → LFO depth
- [ ] Pitch bend (CC bend) → semitone range (default ±2 semitones)
- [ ] Aftertouch → vibrato / filter depth

---

## Phase 4 — Web Audio Architecture Upgrade (Medium-Long Term)

Goal: Move DSP off the main thread; target latency <5ms.

Reference: `gemini_report.md §5`

### 4.1 AudioWorklet Migration
- [ ] Port SDL2 audio callback logic to an **AudioWorkletProcessor** (`prophecy-processor.js`)
- [ ] Compile WASM with `--shared-memory` and `PTHREAD_POOL_SIZE=2`
- [ ] Remove SDL2 dependency from WASM build (replace with direct `emscripten_set_main_loop` or WorkletProcessor render quantum)

### 4.2 Lock-Free Communication
- [ ] Implement **SharedArrayBuffer ring buffers** for MIDI events: main thread → worklet
- [ ] Use `Atomics.store` / `Atomics.load` for parameter updates (no mutex)
- [ ] Implement **double-buffer** for telemetry: worklet → UI (oscilloscope/spectrum display)

### 4.3 WASM Memory
- [ ] Profile current 64MB initial memory allocation; reduce if possible
- [ ] Disable `ALLOW_MEMORY_GROWTH` for predictable real-time behavior once memory usage is known
- [ ] Ensure all DSP data structures are allocated at startup (no runtime heap alloc in audio path)

---

## Phase 5 — UI & Accessibility (Long Term)

Goal: A professional browser UI matching the Prophecy's panel layout.

### 5.1 Controls
- [ ] Visual **oscilloscope** display (read from SharedArrayBuffer telemetry)
- [ ] **Spectrum analyzer** (FFT via Web Workers)
- [ ] Per-model parameter panels (show relevant knobs for active oscillator set)
- [ ] **Preset management** — save/load JSON presets via localStorage
- [ ] MIDI learn mode (click knob → move CC to assign)

### 5.2 Keyboard
- [ ] Extend from 2 octaves to 3+ octaves
- [ ] Octave shift buttons (± 1 octave)
- [ ] Velocity-sensitive mouse/touch (y-position on press → velocity)

### 5.3 Accessibility
- [ ] Keyboard navigation for all controls
- [ ] ARIA labels on synth controls

---

## Phase 6 — Bespoke Synth Integration (Long Term)

Reference: `docs/DESIGN.md` (Bespoke Synth section)

- [ ] Expose CV input hooks for pitch, gate, velocity, mod
- [ ] Add audio input path (process external audio through formant bank)
- [ ] Module metadata (name, author, version, I/O port descriptors)
- [ ] Optional: compile as native shared library for DAW plugin (CLAP/VST3 wrapper)

---

## Infrastructure & Maintenance

### Security
- [ ] Remove hardcoded credentials from `deploy.py` — use `os.environ` or SSH key auth
- [ ] Add `.env.example` and document deployment env vars in README

### CI/CD
- [ ] Add GitHub Actions workflow:
  - Native build + Catch2 tests on every PR
  - WASM build verification (check output artifact exists and is valid)
- [ ] Add `npm run lint` for JS files (eslint)

### Documentation
- [ ] Add JSDoc comments to `web/js/midi.js`
- [ ] Document all MIDI CC mappings in README
- [ ] Add architecture diagram showing WASM ↔ JS boundary
- [ ] Document VPM algorithm routing diagrams (once implemented)

---

## Priority Order (Suggested)

| Priority | Item | Rationale |
|---|---|---|
| P0 | Phase 1 audio quality fixes | Correctness before expansion |
| P0 | 48kHz sample rate | Hardware-accurate foundation |
| P1 | VPM oscillators | Most distinctive Prophecy feature |
| P1 | AudioWorklet migration | Required for low-latency stability |
| P2 | Hard sync + cross mod + ring mod | Completes analog oscillator set |
| P2 | Comb filter model | Natural step before waveguides |
| P3 | Physical models (brass, reed, string) | Complex, needs careful tuning |
| P3 | Effects chain | Noticeable quality improvement |
| P4 | UI overhaul | Polish |
| P4 | Bespoke Synth integration | Niche use case |

---

## Reference Documents

| File | Contents |
|---|---|
| `gemini_report.md` | Authoritative hardware analysis, algorithm specs, web audio architecture |
| `docs/DESIGN.md` | Signal flow, filter design, voice allocation |
| `AGENTS.md` | Development conventions, security notes, project goals |
| `CLAUDE.md` | Claude Code guide: build commands, conventions, formant tables |
| `include/DSPUtils.h` | Authentic formant frequency tables (source of truth) |
