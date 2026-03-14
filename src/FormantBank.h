/**
 * Formant Filter Bank
 * 
 * Implements the Prophecy's "Vocal" model parallel filter structure.
 * 4 resonant bandpass filters process a common carrier oscillator.
 * 
 * Reference: Korg Prophecy Parameter Guide, "Vocal" model section.
 */

#pragma once
#include "BiquadFilter.h"
#include <array>

namespace ProphecyDSP {

class FormantBank {
public:
    FormantBank() {
        for (auto& filter : filters) {
            filter.setSampleRate(SAMPLE_RATE);
        }
        setVowel('a');
    }
    
    // Set formant frequencies based on vowel phoneme
    // Frequencies derived from Prophecy service manual specifications
    void setVowel(char vowel) {
        const FormantSet* set = nullptr;
        switch (vowel) {
            case 'a': case 'A': set = &FORMANT_A; break;
            case 'e': case 'E': set = &FORMANT_E; break;
            case 'i': case 'I': set = &FORMANT_I; break;
            case 'o': case 'O': set = &FORMANT_O; break;
            case 'u': case 'U': set = &FORMANT_U; break;
            default: set = &FORMANT_A;
        }
        
        for (int i = 0; i < 4; ++i) {
            filters[i].setBandPass(set->frequencies[i], set->bandwidths[i]);
        }
        currentVowel = vowel;
    }
    
    // Process sample through parallel formant filters
    // Returns summed output (weighted sum per Prophecy specs)
    inline float process(float input) {
        float sum = 0.0f;
        // Weight first two formants higher (vocal energy concentration)
        sum += filters[0].process(input) * 1.0f;  // F1
        sum += filters[1].process(input) * 0.9f;  // F2
        sum += filters[2].process(input) * 0.6f;  // F3
        sum += filters[3].process(input) * 0.4f;  // F4
        return sum * 0.5f;  // Scale to prevent clipping
    }
    
    void reset() {
        for (auto& f : filters) f.reset();
    }
    
    char getCurrentVowel() const { return currentVowel; }

private:
    std::array<BiquadFilter, 4> filters;
    char currentVowel = 'a';
};

} // namespace ProphecyDSP
