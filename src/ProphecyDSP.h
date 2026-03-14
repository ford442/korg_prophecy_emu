/**
 * Main DSP Controller
 * 
 * Manages polyphonic voice allocation and global parameters.
 * Designed as modular unit compatible with Bespoke Synth architecture.
 */

#pragma once
#include "Voice.h"
#include <array>
#include <algorithm>

namespace ProphecyDSP {

class ProphecyDSP {
public:
    ProphecyDSP();
    
    // Audio processing
    void process(float* outputL, float* outputR, int frames);
    
    // MIDI control
    void noteOn(int note, float velocity);
    void noteOff(int note);
    void allNotesOff();
    
    // Parameter control
    void setFormantVowel(char vowel);
    void setWaveform(Voice::Waveform wf);
    void setCutoff(float freq);      // Global cutoff if needed
    void setResonance(float res);    // Formant Q factor
    void setPortamento(float time);  // Glide time in seconds
    
    // Module I/O interface (for future Bespoke integration)
    void setInput(int channel, float* buffer) { inputs[channel] = buffer; }
    float* getOutput(int channel) { return outputs[channel]; }
    
private:
    std::array<Voice, MAX_VOICES> voices;
    float* inputs[4] = {nullptr};   // Modular inputs
    float* outputs[2] = {nullptr};  // L/R outputs
    
    Voice::Waveform currentWaveform = Voice::Saw;
    char currentVowel = 'a';
    float portamentoTime = 0.0f;
    
    // Simple round-robin voice allocation (Prophecy is typically mono, 
    // but polyphony useful for web testing)
    int findFreeVoice();
    int lastVoice = 0;
};

} // namespace ProphecyDSP
