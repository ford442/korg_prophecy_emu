/**
 * Biquad Filter implementation for formant synthesis
 * 
 * Design follows standard DSP biquad topology:
 * y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]
 * 
 * Used in parallel configuration for Prophecy-style formant filtering
 */

#pragma once
#include "DSPUtils.h"

namespace ProphecyDSP {

class BiquadFilter {
public:
    enum class Type {
        LowPass,
        BandPass,  // Used for formants
        HighPass,
        Notch
    };

    BiquadFilter() = default;
    
    void setSampleRate(float sampleRate) { sr = sampleRate; }
    
    // Configure as resonant bandpass for formant filtering
    // Per Prophecy specs: high Q (resonance) for vocal character
    void setBandPass(float frequency, float bandwidth) {
        float omega = TWO_PI * frequency / sr;
        float alpha = sinf(omega) * sinh(logf(2.0f) / 2.0f * bandwidth * omega / sinf(omega));
        
        float b0_coeff = alpha;
        float b1_coeff = 0.0f;
        float b2_coeff = -alpha;
        float a0_coeff = 1.0f + alpha;
        float a1_coeff = -2.0f * cosf(omega);
        float a2_coeff = 1.0f - alpha;
        
        // Normalize
        this->b0 = b0_coeff / a0_coeff;
        this->b1 = b1_coeff / a0_coeff;
        this->b2 = b2_coeff / a0_coeff;
        this->a1 = a1_coeff / a0_coeff;
        this->a2 = a2_coeff / a0_coeff;
    }
    
    // Process single sample
    inline float process(float input) {
        float output = b0 * input + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
        x2 = x1;
        x1 = input;
        y2 = y1;
        y1 = output;
        return output;
    }
    
    void reset() {
        x1 = x2 = y1 = y2 = 0.0f;
    }

private:
    float sr = SAMPLE_RATE;
    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
    float a1 = 0.0f, a2 = 0.0f;
    float x1 = 0.0f, x2 = 0.0f;
    float y1 = 0.0f, y2 = 0.0f;
};

} // namespace ProphecyDSP
