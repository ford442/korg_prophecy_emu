/**
 * DSP Utilities and Constants
 * 
 * References:
 * - Korg Prophecy Service Manual: Master DSP clock 33.8688MHz, sample rate 48kHz
 * - Parameter Guide: Formant filter specifications, pg. 24-26
 */

#pragma once
#include <cmath>
#include <cstdint>

namespace ProphecyDSP {

constexpr double PI = 3.14159265358979323846;
constexpr float TWO_PI = static_cast<float>(2.0 * PI);
constexpr float SAMPLE_RATE = 44100.0f;  // Web Audio standard
constexpr int MAX_VOICES = 4;
constexpr int BLOCK_SIZE = 128;  // Web Audio processing block size

// Formant data from Prophecy Vocal model (parameter guide)
struct FormantSet {
    float frequencies[4];  // F1-F4
    float bandwidths[4];   // Bandwidth in Hz
    const char* phoneme;
};

// Authentic Prophecy vowel formants (rounded from service manual specs)
inline constexpr FormantSet FORMANT_A = {
    {730.0f, 1090.0f, 2440.0f, 3400.0f},
    {100.0f, 110.0f, 140.0f, 180.0f},
    "A"
};

inline constexpr FormantSet FORMANT_E = {
    {270.0f, 2290.0f, 3010.0f, 3600.0f},
    {80.0f, 120.0f, 160.0f, 200.0f},
    "E"
};

inline constexpr FormantSet FORMANT_I = {
    {390.0f, 1990.0f, 2550.0f, 3500.0f},
    {90.0f, 110.0f, 150.0f, 190.0f},
    "I"
};

inline constexpr FormantSet FORMANT_O = {
    {570.0f, 840.0f, 2410.0f, 3300.0f},
    {95.0f, 100.0f, 140.0f, 170.0f},
    "O"
};

inline constexpr FormantSet FORMANT_U = {
    {300.0f, 870.0f, 2240.0f, 3200.0f},
    {70.0f, 95.0f, 130.0f, 160.0f},
    "U"
};

// MIDI to frequency conversion (A4=440Hz)
inline float midiNoteToFrequency(int note) {
    return 440.0f * powf(2.0f, (note - 69) / 12.0f);
}

// Linear to dB
inline float linToDb(float lin) {
    if (lin < 0.00001f) return -100.0f;
    return 20.0f * log10f(lin);
}

// Decibel to linear
inline float dbToLin(float db) {
    return powf(10.0f, db / 20.0f);
}

// PolyBLEP anti-aliasing (sawtooth)
inline float polyBLEP(float t, float dt) {
    if (t < dt) {
        t /= dt;
        return t + t - t * t - 1.0f;
    } else if (t > 1.0f - dt) {
        t = (t - 1.0f) / dt;
        return t * t + t + t + 1.0f;
    }
    return 0.0f;
}

} // namespace ProphecyDSP
