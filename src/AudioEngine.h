/**
 * Web Audio Interface
 * 
 * SDL2 Audio backend for Emscripten.
 * Handles real-time callback from browser AudioWorklet via SDL2 abstraction.
 */

#pragma once
#include "ProphecyDSP.h"
#include <SDL.h>
#include <vector>
#include <memory>

namespace ProphecyDSP {

class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();
    
    bool initialize();
    void shutdown();
    bool isRunning() const { return running; }
    
    // MIDI interface hooks
    void handleMidiNoteOn(int note, int velocity);
    void handleMidiNoteOff(int note);
    void handleControlChange(int cc, int value);
    
    // DSP parameter access
    ProphecyDSP& getDSP() { return dsp; }
    
private:
    static void audioCallback(void* userdata, Uint8* stream, int len);
    void processAudio(float* buffer, int frames);
    
    SDL_AudioDeviceID deviceId = 0;
    bool running = false;
    ProphecyDSP dsp;
    
    // Stereo interleaved buffer
    std::vector<float> outputBuffer;
};

} // namespace ProphecyDSP
