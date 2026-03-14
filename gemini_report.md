# Engineering a WebAssembly Emulation of the Korg Prophecy: From MOSS Architecture to AudioWorklet Implementation

The pursuit of emulating legacy digital synthesizers within the ecosystem of the modern web browser represents one of the most rigorous intersections of reverse engineering, digital signal processing (DSP), and systems architecture. The Korg Prophecy, released in 1995, stands as a distinct milestone in the history of electronic instruments, marking a paradigm shift away from the sample-based ROMplers that dominated the early 1990s and toward mathematically intensive algorithmic synthesis. Built upon the foundation of Korg's highly ambitious, though initially unreleased, Open Architecture Synthesis System (OASYS), the Prophecy introduced the Multi-Oscillator Synthesis System (MOSS). This system successfully integrated virtual analog modeling, Variable Phase Modulation (VPM), and, crucially, physical modeling of acoustic instruments within a single, highly cohesive digital environment.

Creating a cycle-accurate, or even mathematically faithful, high-level emulation of the Prophecy in a web environment via WebAssembly (WASM) requires navigating severe technical constraints. The original hardware relied on a dual-processor topology—a general-purpose microcontroller paired with a proprietary, undocumented DSP application-specific integrated circuit (ASIC)—to calculate complex acoustic waveguide models. To recreate this on the web, developers must bypass the inherent limitations of the JavaScript runtime, leveraging the Web Audio API's AudioWorklet interface, SharedArrayBuffer concurrency, and the high-performance memory safety of languages like Rust or C++. This report provides an exhaustive, authoritative blueprint for constructing a browser-based Korg Prophecy emulator, encompassing the mathematical deconstruction of the MOSS engine, reverse engineering strategies, and optimal WebAssembly deployment patterns.

## Historical Context and Hardware Topography of the Prophecy

To understand the scope of the emulation challenge, one must first analyze the physical and computational topography of the original hardware. In the mid-1990s, the synthesizer market was heavily oriented toward PCM (Pulse Code Modulation) playback engines, notably Korg's own M1 and its successors. While PCM synthesis excelled at imitating the static timbres of acoustic instruments, it severely lacked the dynamic expressiveness, non-linear harmonic evolution, and player-instrument interaction characteristic of true acoustic performance and analog circuitry.

Korg's response was the Prophecy, a monophonic, monotimbral performance synthesizer originally priced at £1000 or $1595. Positioned as a lead instrument, it featured a 37-note keyboard equipped with velocity and aftertouch sensitivity. The hardware interface was specifically engineered to exploit the dynamic parameters of the physical modeling engine. It included a traditional pitch wheel, a modulation wheel, and a unique, continuous multi-axis controller affectionately nicknamed the "log"—a horizontal cylinder that combined a rotational modulation action with a pressure-sensitive ribbon. Furthermore, the instrument featured a bank of five programmable Performance Editor (PE) knobs. Each of these infinite-rotary encoders could be mapped to control up to four discrete synthesis parameters simultaneously, permitting profound and immediate real-time manipulation of the sound engine's mathematical state.

Beneath the bespoke molded enclosure, the Prophecy was driven by a dual-chip architecture. The primary central processing unit (CPU) is documented as a UPD-70433GD paired with a Hitachi H8/3003 microcontroller. This general-purpose processor was tasked with handling the operating system, parsing incoming MIDI data, reading physical potentiometer values, updating the 40x2 character LCD, and managing patch memory.

However, the H8 lacked the floating-point or specialized multiply-accumulate (MAC) architecture required to process complex audio algorithms in real time. The actual synthesis was offloaded to a proprietary, custom-designed DSP chip. This DSP executed the intensive audio-rate calculations for the physical waveguides, filters, and effects, outputting the final digital audio stream to a TDA1305T Digital-to-Analog Converter (DAC). The bifurcation of control logic and audio generation in the hardware necessitates a similar multithreaded software architecture when emulated within a web browser.

## Architectural Deconstruction of the MOSS Engine

The Multi-Oscillator Synthesis System (MOSS) is not a monolithic algorithm, but rather a highly modular, reconfigurable digital signal path. To emulate the Prophecy, the software architecture must exactly mirror the MOSS routing matrix, which follows a complex subtractive and waveshaping pipeline. A standard MOSS patch, or "Program," utilizes a combination of oscillators, wave shapers, mixers, filters, amplifiers, and effects.

The signal generation stage is determined by the "Oscillator Set." The Prophecy provides 12 distinct sets that configure how the DSP resources are allocated between Oscillator 1 and Oscillator 2.

| Oscillator Set | Oscillator 1 Configuration | Oscillator 2 Configuration | DSP Allocation Note |
|---------------|---------------------------|---------------------------|---------------------|
| Set 1 - 4 | Standard Analog / VPM / Mod | Standard Analog / VPM / Mod | Dual oscillator configurations with standard computational loads. |
| Set 5 - 7 | Comb Filter | VPM / Mod / Comb Filter | Comb filters require extensive delay line memory allocation. |
| Set 8 - 9 | Modulation (Sync/Ring/Cross) | Standard Analog / VPM | Inter-oscillator modulation topologies. |
| Set 10 | Brass Model | Disabled | High DSP load limits polyphony to a single physical model. |
| Set 11 | Reed Model | Disabled | High DSP load limits polyphony to a single physical model. |
| Set 12 | Plucked String Model | Disabled | High DSP load limits polyphony to a single physical model. |

As demonstrated in the architectural layout, Sets 1 through 9 permit the simultaneous use of two oscillators, covering analog, comb filtering, and Variable Phase Modulation (VPM) models. Sets 10 through 12 disable Oscillator 2 entirely, dedicating the maximum available DSP clock cycles to calculating the highly intensive, non-linear feedback loops required for the Brass, Reed, and Plucked String physical models.

Once the primary waveforms are generated, the signals from Oscillator 1 and Oscillator 2 are routed into independent Wave Shape sections. These blocks apply non-linear mathematical transfer functions to the waveforms, fundamentally mutating their harmonic spectra prior to filtration. Following the dual wave shapers, the signals from the main oscillators are combined in a central Mixer section, which also introduces signals from a dedicated Sub-Oscillator, a White Noise Generator, and an Amplifier Feedback loop.

The mixed composite signal is then passed to the Filter section, which comprises two independent, multi-mode resonant filters. These filters can operate in Low Pass (LPF), High Pass (HPF), Band Pass (BPF), or Band Reject (BRF) configurations, and can be routed in series or parallel. Following the filters, the signal proceeds through the Amplifier section and into the digital effects chain. The Prophecy's effects processor provides seven algorithms: distortion, wah, dual-band parametric equalization, chorus/flanger, and delay or reverb.

A highly accurate software emulation must respect the internal sample rate of the original DSP. While modern digital audio workstations (DAWs) default to 44.1 kHz or 96 kHz, analysis of subsequent Korg hardware utilizing MOSS technology, such as the EXB-MOSS board and the Kronos workstation, indicates an internal fixed processing rate of 48 kHz. Replicating this specific rate within the WASM module, and utilizing high-quality decimation/interpolation to interface with the browser's audio context, is essential for preserving the precise aliasing artifacts, filter cutoff behaviors, and high-frequency responses of the original hardware.

## Control Rate and Modulation Matrix

The audio path calculations represent only half of the computational burden. The Prophecy possesses an incredibly dense modulation matrix. The system features four independent Low-Frequency Oscillators (LFOs), each capable of generating 30 different waveform shapes. Furthermore, the architecture includes six multi-stage Envelope Generators (EGs) assigned to pitch, amplitude, filters, and general-purpose modulation tasks.

In a digital synthesizer, it is computationally wasteful to calculate envelope and LFO states at the full audio sample rate (e.g., 48,000 times per second). Instead, these control signals are typically calculated at a lower "control rate" (e.g., 1000 times per second), and the resulting values are linearly interpolated across the audio buffer block. The WASM emulation must establish a robust block-processing architecture that accurately replicates the Prophecy's control-rate smoothing to prevent zipper noise during rapid parameter sweeps from the Performance Editor knobs.

## Mathematical Analysis of MOSS Synthesis Algorithms

The primary challenge of a WebAssembly emulation lies in the exact algorithmic reconstruction of the seven oscillator models provided by the MOSS engine. A successful High-Level Emulation (HLE) requires implementing the mathematical and physical formulas that underpin these sound generation techniques.

### Standard Analog and Modulation Oscillators

The Standard Oscillator model simulates the voltage-controlled oscillators (VCOs) of vintage analog equipment, generating sawtooth, pulse, triangle, and sine waveforms. A naive digital implementation of a sawtooth wave (e.g., a simple modulo counter) will generate severe, inharmonic aliasing when its mathematical harmonics exceed the Nyquist frequency. Therefore, the WASM engine must implement band-limited waveform generation. Modern DSP techniques such as MinBLEP (Minimum Bandlimited Step) or PolyBLEP (Polynomial Bandlimited Step) are highly efficient methods for generating alias-free analog shapes by smoothing the discontinuities of the waveform. The emulation must also support Pulse Width Modulation (PWM), allowing an LFO or EG to dynamically shift the duty cycle of the pulse wave.

The Modulation Oscillator sets expand this paradigm by allowing the oscillators to mathematically interact. The emulation must support:

- **Ring Modulation**: A simple four-quadrant multiplication of the carrier and modulator signals, resulting in sum and difference frequencies that produce metallic, bell-like timbres.
- **Cross Modulation (Exponential FM)**: The instantaneous output amplitude of Oscillator 2 directly modulates the frequency of Oscillator 1.
- **Hard Sync**: Oscillator 1's phase accumulator is forcibly reset to zero every time Oscillator 2 completes a cycle. In a digital environment, forcing a sudden phase reset produces extreme aliasing; thus, the WASM implementation must use sub-sample interpolation (like PolyBLEP) to calculate the exact fractional time between samples where the sync event occurred, mitigating digital artifacts.

### Variable Phase Modulation (VPM)

Variable Phase Modulation is Korg's proprietary adaptation of Frequency Modulation (FM) synthesis, heavily utilized to bypass Yamaha's historical patents on the technology. While traditional FM (such as the Yamaha DX7) relies on true phase modulation between up to six pure sine wave operators, the Prophecy's VPM engine streamlines this concept into an aggressive two-operator architecture (Carrier and Modulator) heavily augmented by wave shaping.

In the VPM mathematical model, the carrier's fundamental pitch is established by the primary oscillator frequency. The modulator's frequency is not absolute, but is rather defined as an integer multiple (harmonic index) of the carrier frequency. If $C(t)$ represents the carrier and $M(t)$ represents the modulator, the basic VPM output can be expressed mathematically as:

$$y(t) = A_c \cdot \Phi_c(\omega_c t + I \cdot M(t))$$

Where $A_c$ is the amplitude of the carrier, $\omega_c$ is the angular frequency of the carrier, $I$ represents the modulation index (intensity), and $\Phi_c$ represents the transfer function of the carrier waveform.

What fundamentally distinguishes VPM from standard FM is that $\Phi_c$ is not restricted to a sine wave. The user can select a sawtooth, triangle, or square wave as the carrier, which is then phase-modulated by the modulator. Furthermore, the VPM oscillator incorporates a self-feedback parameter ($FB$), where the output of the carrier is fed back into its own phase calculation, introducing chaotic, noise-like overtones as the feedback coefficient approaches unity. The complex, harmonically dense signal generated by this interaction is immediately routed into the dedicated wave shaping circuit, which applies a high-frequency emphasis function to further brighten the sound.

### The Comb Filter Oscillator and Noise Extraction

The Comb Filter model is engineered to extract pitched, harmonic resonance from broadband white noise, forming a rudimentary precursor to Karplus-Strong string synthesis. The architecture consists of a primary exciter (either a burst of noise or a continuous oscillator waveform) that is injected into a cyclical delay line featuring a feedback loop.

Within this feedback loop resides a low-pass filter (LPF). As the noise signal continuously cycles through the delay line, the LPF progressively attenuates the high-frequency energy on each iteration, simulating the natural acoustic decay of a plucked string or struck metallic tube. The length of the delay line, denoted as $L$ (in samples), strictly defines the fundamental frequency $f_0$ of the resulting pitch, calculated as $L = f_s / f_0$, where $f_s$ is the system sample rate.

The user possesses absolute control over the feedback gain coefficient. At a feedback setting of zero, the output is pure, unpitched noise. As the feedback level approaches 99%, the resonance of the delay line intensifies, producing a distinct, ringing tone. To emulate this efficiently in WebAssembly, developers must implement a highly optimized, lock-free circular buffer for the delay line, combined with a first-order recursive LPF calculation executed within the innermost loop of the audio thread. The mathematical decay of the comb filter can be analyzed using an adaptation of Schroeder's reverberation equation, $T = t / (-20 \log_{10}|g|)$, where $T$ is the decay time, $t$ is the delay length, and $g$ is the feedback gain.

### Digital Waveguide Physical Modeling: Brass, Reed, and Pluck

The most computationally demanding aspect of the MOSS engine—and the primary reason Sets 10 through 12 restrict the synth to a single oscillator—is the implementation of acoustic physical modeling. The Prophecy was one of the first commercial instruments to leverage Digital Waveguide Synthesis, a methodology heavily researched by Julius O. Smith III at Stanford University's CCRMA.

Waveguide synthesis eschews traditional oscillators in favor of modeling the physical propagation of acoustic pressure waves through a medium (such as an air column or a string) using bidirectional delay lines. This relies on the discrete-time realization of d'Alembert's solution to the one-dimensional wave equation:

$$y(m,n) = y^{+}(m-n) + y^{-}(m+n)$$

Here, the acoustic pressure $y$ at a physical position $m$ and discrete time $n$ is calculated as the superposition of a right-traveling wave $y^+$ and a left-traveling wave $y^-$. In DSP architecture, this translates to dual delay lines closed by reflection filters.

**The Brass Model**: This algorithm mathematically simulates lip-reed instruments such as trumpets and trombones. The model is driven by a "Breath Pressure" Envelope Generator, which dictates the force of the virtual air column applied to the mouthpiece. This air pressure excites a non-linear "Lip Character" transfer function, simulating the mass, tension, and oscillatory behavior of the human lips acting as a pressure-controlled valve. The resulting acoustic wave propagates through the delay-line bore and reflects at the "Brass Bell." The bell applies an equalization curve and a reflection coefficient that simulates the acoustic radiation properties of open air or a physical mute inserted into the instrument.

**The Reed Model**: Utilized for synthesizing saxophones, oboes, and flutes, this model replaces the lip simulation with a highly complex "Reed" transfer function. The pressure differential between the player's simulated mouth (Breath Pressure) and the internal bore of the instrument determines the physical aperture of the reed. The mathematical relationship between air pressure and airflow through a vibrating reed is fiercely non-linear; in a WASM emulation, this is most efficiently executed using a pre-calculated lookup table combined with polynomial interpolation. The Prophecy firmware contains 13 distinct reed parameters, allowing it to morph seamlessly between the bore characteristics of a Soprano Saxophone, a Bassoon, or a synthesized "Monster".

**The Plucked String Model**: This is a highly advanced iteration of the Karplus-Strong algorithm. A short burst of noise acts as the plectrum, exciting a feedback delay line. The Prophecy allows deep manipulation of the excitation parameters, including "Attack Level" (the physical force of the pluck) and a "Noise Filter" (Cutoff and Resonance) which dictates the material density of the plectrum. A critical parameter is "String Position," which alters the comb-filtering effect based on where the string is struck relative to the bridge. Furthermore, an "Inharmonicity" parameter is introduced to simulate the stiffness of solid metallic strings. In DSP terms, this is achieved by inserting cascade all-pass filters into the waveguide loop, causing higher-frequency overtones to travel slightly faster than the fundamental, pushing them sharp and creating metallic dissonance.

### Non-Linear Wave Shaping

A defining characteristic of the Prophecy's aggressive timbre is the Wave Shape section, situated immediately post-oscillator and pre-mixer. The emulation must meticulously replicate two mathematical tables: "CLIP" and "RESO".

**CLIP (Hard Clipping)**: The CLIP table applies a mathematical saturation function. Upper and lower thresholds limit the amplitude of the signal. If the input sample exceeds these limits, it is abruptly truncated to the threshold value. This violent alteration of the waveform introduces heavy odd-order harmonics and aggressive distortion, fundamentally changing a pure sine wave into a square-like shape.

**RESO (Resonant Wavefolding)**: Inherited from Korg's earlier 01/W workstation, the RESO table operates as a wavefolder. Rather than clipping the signal when it exceeds a threshold, the RESO table passes the input waveform through a sine wave transfer function. A "Shape" parameter controls the frequency (number of cycles) of this transfer function. As the amplitude of the input signal increases, it pushes further along the x-axis of the sine function, causing the output signal to fold back upon itself. This creates deeply complex, formant-like vocal resonances without the use of standard subtractive filters. Recreating this requires mapping the exact offset, gain, and feedback multiplier matrices of the original hardware.

| Wave Shaper Type | Transfer Function | Harmonic Effect | Primary Use Case |
|-----------------|-------------------|-----------------|------------------|
| CLIP | $y = \max(L_{min}, \min(x, L_{max}))$ | Introduces odd-order harmonics; squares off waveforms. | Hard distortion, aggressive bass, electric guitar overdrive. |
| RESO | $y = \sin(x \cdot \text{Shape} + \text{Offset})$ | Introduces dense, inharmonic sidebands via wavefolding. | Formant generation, vocal timbres, metallic textures. |

## Reverse Engineering Strategies: LLE vs. HLE

To build a mathematically faithful WASM emulation, developers must extract the logic from the physical hardware. The Prophecy firmware resides on internal EPROMs. By physically desoldering these chips (or removing them from sockets) and utilizing an EPROM reader, the binary data can be extracted. The community generally targets firmware version 20 (v20), the final update which resolved critical System Exclusive (SysEx) communication bugs present in earlier v1.13 iterations.

Once the ROM binaries are dumped, developers face a critical architectural decision between Low-Level Emulation (LLE) and High-Level Emulation (HLE).

### The Impossibility of Low-Level Web Emulation

Low-Level Emulation involves writing a virtual machine that perfectly interprets and executes the original machine code instructions of the hardware's processors, cycle-by-cycle. This is the approach taken by the highly successful DSP56300 project, which emulates the Motorola DSPs used in the Access Virus and Waldorf microQ synthesizers.

However, applying LLE to the Korg Prophecy is largely unfeasible. While the primary Hitachi H8/3003 microcontroller utilizes a well-documented instruction set that can be decompiled using tools like Ghidra or IDA Pro , the audio generation occurs on a custom, proprietary Korg ASIC DSP chip. The instruction set, memory map, and register behavior of this DSP are entirely undocumented outside of Korg's internal engineering archives. Without documentation, disassembling the DSP binary payload is an exercise in futility. Furthermore, even if the architecture were mapped, a cycle-accurate LLE virtual machine incurs massive CPU overhead, rendering it far too inefficient to execute within the strict constraints of a WebAssembly browser environment.

### The Necessity of High-Level Emulation (HLE)

Consequently, a WebAssembly emulation must utilize High-Level Emulation. HLE treats the DSP chip as a "black box." By analyzing the Prophecy's technical block diagrams, studying the service manual schematics , and passing diagnostic signals (impulses, sine sweeps) through the physical hardware to analyze the resulting phase and spectral behaviors on an oscilloscope, developers can construct a "white box" algorithmic twin.

The HLE approach involves writing modern, optimized code (in C++ or Rust) that mathematically replicates the results of the DSP algorithms without relying on the original machine code. For the complex waveguide models, developers can rely on established academic frameworks, such as the Synthesis ToolKit (STK) developed by Perry Cook and Gary Scavone, which provides extensive open-source C++ implementations of brass and reed physical models that closely mirror Korg's Sondius-XG implementations.

## Web Audio API and Concurrency Architecture

Deploying a complex, multi-algorithmic DSP engine into a web browser requires profound architectural planning to bypass the limitations of JavaScript. Standard JavaScript executes on a single main thread, relies on a Just-In-Time (JIT) compiler, and is subject to unpredictable Garbage Collection (GC) pauses. In the context of real-time audio, a GC pause lasting merely five milliseconds will result in a buffer underrun, manifesting as unacceptable audible clicks, pops, and dropouts.

### The AudioWorklet Paradigm

To guarantee audio fidelity, the Web Audio API introduced the AudioWorklet interface. This API completely removes audio processing from the main JavaScript event loop, executing custom DSP code within a dedicated, high-priority background audio rendering thread.

The AudioWorkletProcessor functions on a strict, immutable render quantum of 128 sample frames. Operating at a standard sample rate of 48,000 Hz, the processor has precisely 2.66 milliseconds ($128 / 48000$) to complete all synthesis calculations for a single block. If the DSP logic—encompassing the MOSS physical models, multi-mode filters, and envelope generation—exceeds this 2.66ms time budget, the audio engine drops the frame, resulting in severe degradation.

### SharedArrayBuffer and Lock-Free Ring Buffers

A synthesizer must respond instantaneously to asynchronous user inputs: MIDI note data, pitch bend, control changes from the Log wheel, and UI parameter modifications. However, the Web MIDI API and the Document Object Model (DOM) reside strictly on the main JavaScript thread, completely isolated from the AudioWorklet thread.

The native method for inter-thread communication, postMessage(), relies on structured cloning and the browser's asynchronous event loop. This introduces unacceptable latency and unpredictable overhead, making it wholly unsuitable for real-time MIDI parsing.

The definitive architectural solution is the deployment of a SharedArrayBuffer (SAB). A SAB provisions a block of linear binary memory that can be accessed concurrently by both the main thread and the WASM/AudioWorklet thread without any data serialization or copying overhead.

To utilize SAB safely, the architecture must implement a Lock-Free Ring Buffer (a Single-Producer, Single-Consumer FIFO queue).

- **The Main Thread (Producer)**: When a MIDI event or UI change occurs, the main thread writes an opcode (e.g., 0x01 for Note On) and the payload (e.g., Velocity and Pitch) directly into the SAB ring buffer. It then updates an atomic 'write' pointer.
- **The AudioWorklet (Consumer)**: At the exact beginning of every 128-frame render quantum, the WASM DSP engine checks the atomic 'read' pointer. If new data is present, it pops the opcodes off the queue, updates the internal MOSS synthesizer state (e.g., resetting the Brass Breath Pressure EG), and then proceeds to calculate the audio block.

Crucially, because the AudioWorklet is a hard real-time thread, it must never be blocked by a mutex, lock, or blocking wait state. The use of JavaScript Atomics operations ensures that memory barriers are respected and that partial read/write data tearing does not occur during concurrent access. It must be noted that enabling SharedArrayBuffer requires the web server to emit strict cross-origin isolation HTTP headers (Cross-Origin-Embedder-Policy: require-corp and Cross-Origin-Opener-Policy: same-origin) to mitigate transient execution CPU vulnerabilities like Spectre.

| Inter-Thread Communication Method | Latency | Real-Time Safe? | Use Case |
|----------------------------------|---------|-----------------|----------|
| postMessage() | High / Unpredictable | No | Loading large presets, non-critical UI updates. |
| SharedArrayBuffer (Ring Buffer) | Near-Zero | Yes | MIDI Note On/Off, CC sweeping, Audio parameter modulation. |
| WASM Linear Memory | Zero | Yes | Direct audio sample array reading/writing by the Worklet. |

## Language Selection for WebAssembly DSP: C++ vs. Rust vs. TypeScript

To meet the 2.66ms execution budget, the MOSS algorithms must be written in a high-performance systems language and compiled directly to WebAssembly (WASM). WASM provides a portable, low-level binary instruction format with a flat linear memory model, executing at near-native speeds completely devoid of garbage collection. The choice of language—TypeScript, C++, or Rust—determines the workflow, security, and ultimate efficiency of the emulator.

### TypeScript / AssemblyScript

AssemblyScript allows developers to write code in a strict, statically-typed variant of TypeScript, which is then compiled directly to WebAssembly. This path is highly attractive as it minimizes the context switch for front-end web developers, allowing the UI and the DSP to be written in the same syntax.

However, AssemblyScript is fundamentally disqualified for high-fidelity audio emulation. Because standard TypeScript relies on dynamic objects, AssemblyScript must bundle its own lightweight garbage collector into the WASM binary. Any form of garbage collection—even lightweight—introduces unpredictable, non-deterministic execution pauses. Furthermore, AssemblyScript lacks the extensive ecosystem of mature, optimized numerical DSP libraries necessary for solving complex waveguide equations.

### C++ and Emscripten

C++ has been the undisputed industry standard for audio DSP and native plugin (VST/AU) development for over two decades. Compiling a C++ codebase to WASM is managed through the Emscripten toolchain, which provides embind macros to facilitate function calls between C++ classes and the JavaScript runtime.

The primary advantage of C++ is legacy integration. A developer can leverage existing, highly optimized open-source libraries, such as the aforementioned Synthesis ToolKit (STK), to rapidly prototype the brass and reed models. Furthermore, a C++ core allows the codebase to be easily wrapped in the JUCE framework, permitting the emulation to be compiled not only for the browser via WASM, but also as a native desktop application.

The disadvantage of C++ lies in memory management and tooling. Emscripten produces relatively large binaries and verbose JavaScript glue code. Furthermore, C++ requires manual memory management (malloc and free); a single memory leak, dangling pointer, or buffer overrun in the delay line array will silently crash the entire WebAssembly module, bringing down the audio context.

### Rust and wasm-bindgen

Rust represents the modern vanguard of systems programming and has become the premier choice for WebAssembly engineering. Benchmarks conducted in 2024 and 2025 demonstrate that Rust WASM modules consistently achieve 8-10x speedups over highly optimized native JavaScript. Furthermore, Rust's aggressive LLVM compiler backend often generates tighter, smaller, and marginally faster WASM binaries than equivalent C++ code. When leveraging WASM SIMD (Single Instruction, Multiple Data) intrinsics, Rust can achieve 10-15x performance improvements in parallelizable array operations—an absolute necessity when processing large arrays of physical modeling delay lines.

The paramount advantage of Rust over C++ is its compiler. Rust's strict ownership model and borrow checker enforce absolute memory safety and thread safety at compile time, entirely without the use of a garbage collector. This guarantees that the AudioWorklet thread will never suffer from memory leaks, race conditions, or undefined behavior, resulting in an exceptionally stable audio engine.

Additionally, the Rust tooling ecosystem is vastly superior to Emscripten. The wasm-bindgen tool automates the generation of highly optimized JavaScript glue code, marshaling data between the JS UI and the Rust WASM memory heap with near-zero overhead. Rust also features a rapidly maturing ecosystem of native audio crates, such as FunDSP, providing foundational DSP primitives.

| Feature / Metric | AssemblyScript | C++ (Emscripten) | Rust (wasm-bindgen) |
|-----------------|----------------|------------------|---------------------|
| Execution Speed | Moderate | Very Fast | Very Fast (Often 9% faster Wasm) |
| Memory Management | Bundled Garbage Collector | Manual (malloc/free) | Compile-time Ownership (Safe) |
| Real-Time DSP Suitability | Poor (GC pauses) | Excellent | Excellent |
| Legacy Ecosystem | None | Massive (JUCE, STK) | Growing (FunDSP) |
| WASM Toolchain | Native | Heavy, verbose glue code | Seamless, modern, integrated |

### The Faust Transpilation Alternative

An alternative to writing raw C++ or Rust is the utilization of Faust (Functional Audio Stream). Faust is a functional, domain-specific programming language explicitly designed for real-time audio synthesis. A developer writes the mathematical logic of the synthesizer in Faust, and the Faust compiler automatically transpiles the code into highly optimized C++, Rust, or directly into WebAssembly.

Crucially for a Prophecy emulation, the Faust ecosystem includes physmodels.lib, a comprehensive, open-source physical modeling toolkit engineered by Romain Michon. This library contains pre-built, highly refined waveguide models for strings, bells, and brass instruments. Leveraging Faust could drastically accelerate the development of the MOSS physical models, automatically handling the WASM compilation and AudioWorklet integration.

## Strategic Implementation Pipeline

Synthesizing the hardware analysis, DSP theory, and WebAssembly architecture, the optimal path for creating a browser-based Korg Prophecy emulation involves a hybrid stack: utilizing TypeScript for the frontend UI, Rust for the core MOSS DSP engine, and a SharedArrayBuffer for lock-free telemetry.

### Phase 1: Algorithmic Definition and Prototyping

Because the hardware DSP firmware is inaccessible, developers must initiate a High-Level Emulation process. The VPM, Analog, and Comb filter models should be mathematically defined and prototyped in Python or Python/Jupyter environments to verify the spectral outputs against recordings of the physical hardware. The complex Brass and Reed waveguide models should be defined utilizing the academic frameworks established by Julius O. Smith and the STK library, ensuring the non-linear transfer functions for lip and reed pressure accurately mimic the Korg implementation.

### Phase 2: Rust Engine Construction

The synthesizer engine is coded entirely in Rust `#![no_std]` (to ensure no heavy operating system dependencies are compiled). The engine must enforce zero-cost abstractions, strictly avoiding dynamic memory allocation (Box, Vec instantiation) within the audio loop. All memory for the delay lines, the CLIP and RESO waveshaper tables, and the EG states must be statically allocated during the WASM module's initialization phase.

The Rust code must expose a primary `process(input_buffer, output_buffer)` function that computes exactly 128 frames per invocation. Within this tight loop, the engine evaluates the state of the EGs and LFOs at the control rate, updates the phase accumulators of the active MOSS oscillator models, applies the appropriate waveshaping functions, mixes the signals, and solves the multi-mode resonant filter difference equations.

### Phase 3: Telemetry and Concurrency

To bridge the UI and the DSP, a Single-Producer, Single-Consumer (SPSC) ring buffer is written in Rust, backed by a SharedArrayBuffer passed from the JavaScript environment. wasm-bindgen is used to expose methods to JavaScript, allowing the frontend to push 32-bit opcodes (e.g., mapping to the physical Log wheel or PE knobs) directly into this shared memory space.

At the start of the 128-frame DSP loop, the Rust engine drains this lock-free queue, executing the parameter changes instantaneously before generating the next block of audio. This guarantees sample-accurate parameter modulation without violating the real-time constraints of the AudioWorklet.

### Phase 4: Web Audio Integration and UI

The frontend application is constructed using TypeScript, utilizing a framework like React or Vue to render the complex menu diving interfaces and graphical representations of the Prophecy's 12 Oscillator Sets. The Web MIDI API is implemented to capture input from external hardware controllers, seamlessly mapping physical mod wheels to the virtual Prophecy engine.

Upon initialization, the main thread creates an AudioContext, fetches the compiled .wasm binary, and registers the AudioWorklet script. The worklet script instantiates the WASM module, binds the SharedArrayBuffer ring buffer, and hooks the Rust `process` function directly into the AudioWorkletProcessor's render loop. The resulting audio stream is routed to the AudioContext.destination, outputting the emulated MOSS algorithms through the user's hardware DAC.

## Conclusion

The emulation of the Korg Prophecy's Multi-Oscillator Synthesis System is a formidable undertaking that requires bridging historical acoustic DSP theory with cutting-edge web architecture. Due to the undocumented nature of the original custom DSP ASIC, Low-Level Emulation is unfeasible, mandating a rigorous High-Level algorithmic reconstruction of Variable Phase Modulation, Comb Filtering, and intricate Digital Waveguide physical models.

To execute these algorithms within the strict 2.66-millisecond render quantum of the browser's AudioWorklet, developers must abandon the garbage-collected constraints of JavaScript and AssemblyScript. While C++ remains a viable legacy choice, Rust has definitively established itself as the superior language for this application. Its compiler-enforced memory safety guarantees robust, crash-free audio threads, while its aggressive LLVM optimizations and seamless wasm-bindgen integration provide the computational velocity required to solve complex waveguide arrays in real time. By linking a TypeScript UI to a Rust DSP engine via a lock-free SharedArrayBuffer, the expressive, mathematically modeled timbres of the 1995 Korg Prophecy can be resurrected and preserved with absolute fidelity within a zero-installation web environment.
