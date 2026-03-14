# Prophecy Formant Synthesis Design Notes

## Signal Flow Architecture

Based on Korg Prophecy "Vocal" model (Parameter Guide pg. 24-26):

```
[Multi-Osc] -> [Pre-Filter] -> [Formant Bank] -> [VCA] -> [Output]
                  (4 BP filters)    (ADSR)
[LFO/Mod]         [Formant Shift]
```

## Implementation Details

### Formant Filters
- 4 parallel resonant bandpass filters (Biquad)
- Frequencies: F1-F4 per vowel table from service manual
- Bandwidths: 60-200Hz (Q = fc/BW ranges 5-15)

### Carrier Oscillator
- Anti-aliased via PolyBLEP algorithm
- Sawtooth recommended (rich harmonics excite formants effectively)
- Alternative: Square with PWM (Prophecy "Saw/PWM" mode)

### Voice Allocation
- 4-voice polyphonic (extension of original mono Prophecy)
- Round-robin with steal
- Portamento available per-voice

## Authenticity Notes

ROM-based waveforms from original Prophecy require hardware ROM dumps.
This implementation uses band-limited synthesis mathematically equivalent 
to the analog modeling in the original DSP (33.8688MHz Motorola 56303).

## Future Bespoke Synth Integration

Module I/O design:
- Input 0: Pitch CV (1V/octave)
- Input 1: Formant mod
- Input 2: Gate
- Output 0: Audio L
- Output 1: Audio R
