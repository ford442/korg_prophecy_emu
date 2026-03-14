/**
 * Synthesizer Voice
 * 
 * Single voice architecture following Prophecy signal flow:
 * Oscillator -> Formant Bank -> VCA (envelope)
 * 
 * Supports monophonic/portamento behavior typical of Prophecy solo synth character.
 */

#pragma once
#include "FormantBank.h"
#include <atomic>

namespace ProphecyDSP {

struct ADSR {
    float attack = 0.01f;   // seconds
    float decay = 0.3f;     // seconds
    float sustain = 0.7f;   // level 0-1
    float release = 0.5f;   // seconds
    
    float attackRate;
    float decayRate;
    float releaseRate;
    float level = 0.0f;
    enum State { Idle, Attack, Decay, Sustain, Release } state = Idle;
    
    void calculateRates() {
        attackRate = (attack > 0.0f) ? (1.0f / (attack * SAMPLE_RATE)) : 1.0f;
        decayRate = (decay > 0.0f) ? ((1.0f - sustain) / (decay * SAMPLE_RATE)) : 1.0f;
        releaseRate = (release > 0.0f) ? (sustain / (release * SAMPLE_RATE)) : 1.0f;
    }
    
    inline float process() {
        switch (state) {
            case Attack:
                level += attackRate;
                if (level >= 1.0f) { level = 1.0f; state = Decay; }
                break;
            case Decay:
                level -= decayRate;
                if (level <= sustain) { level = sustain; state = Sustain; }
                break;
            case Sustain:
                break;
            case Release:
                level -= releaseRate;
                if (level <= 0.0f) { level = 0.0f; state = Idle; }
                break;
            case Idle:
                level = 0.0f;
                break;
        }
        return level;
    }
    
    void noteOn() { state = Attack; }
    void noteOff() { if (state != Idle) state = Release; }
    void reset() { state = Idle; level = 0.0f; }
};

class Voice {
public:
    enum Waveform { Saw, Square, Triangle, Pulse };
    
    Voice() {
        formantBank.setVowel('a');
        adsr.calculateRates();
    }
    
    void setNote(float note, float velocity) {
        targetFrequency = midiNoteToFrequency(static_cast<int>(note));
        this->velocity = velocity;
        
        // Simple portamento: glide from current freq if active
        if (!active) {
            currentFrequency = targetFrequency;
        }
        
        active = true;
        adsr.noteOn();
    }
    
    void noteOff() {
        adsr.noteOff();
    }
    
    void setWaveform(Waveform wf) { waveform = wf; }
    void setVowel(char vowel) { formantBank.setVowel(vowel); }
    void setFormantShift(float shift) { formantShift = shift; }
    
    inline float process() {
        if (!active && adsr.state == ADSR::Idle) return 0.0f;
        
        // Portamento/glissando
        if (currentFrequency != targetFrequency) {
            float diff = targetFrequency - currentFrequency;
            currentFrequency += diff * portamentoRate;
        }
        
        // Oscillator phase increment
        float phaseInc = currentFrequency / SAMPLE_RATE;
        phase += phaseInc;
        if (phase >= 1.0f) phase -= 1.0f;
        
        // Generate carrier wave (rich harmonics for formant filtering)
        float osc = generateWaveform(phase, phaseInc);
        
        // Apply formant filtering
        float filtered = formantBank.process(osc);
        
        // Apply envelope and velocity
        float amp = adsr.process() * velocity * 0.5f;
        
        // Check if voice can be freed
        if (adsr.state == ADSR::Idle && active) {
            active = false;
        }
        
        return filtered * amp;
    }
    
    bool isActive() const { return active || adsr.state != ADSR::Idle; }
    void reset() { active = false; phase = 0.0f; adsr.reset(); formantBank.reset(); }
    float getCurrentFrequency() const { return currentFrequency; }

private:
    FormantBank formantBank;
    ADSR adsr;
    
    float phase = 0.0f;
    float currentFrequency = 440.0f;
    float targetFrequency = 440.0f;
    float velocity = 0.0f;
    float portamentoRate = 0.05f;  // Portamento time coefficient
    float formantShift = 0.0f;     // +/- semitone shift for formants
    
    std::atomic<bool> active{false};
    Waveform waveform = Saw;
    
    inline float generateWaveform(float ph, float dt) {
        switch (waveform) {
            case Saw: {
                float saw = 2.0f * ph - 1.0f;
                saw -= polyBLEP(ph, dt);  // Anti-aliased
                return saw;
            }
            case Square: {
                float sq = (ph < 0.5f) ? 1.0f : -1.0f;
                sq += polyBLEP(ph, dt);        // Rising edge
                sq -= polyBLEP(fmodf(ph + 0.5f, 1.0f), dt);  // Falling edge
                return sq;
            }
            case Triangle:
                return 4.0f * fabsf(ph - 0.5f) - 1.0f;
            case Pulse:
                return (ph < 0.25f) ? 1.0f : -1.0f;  // Narrow pulse
            default:
                return 0.0f;
        }
    }
};

} // namespace ProphecyDSP
