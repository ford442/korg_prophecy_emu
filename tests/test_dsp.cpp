#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "../src/Voice.h"
#include "../include/BiquadFilter.h"

using namespace Catch;
using namespace ProphecyDSP;

TEST_CASE("Biquad Bandpass Filter", "[dsp]") {
    BiquadFilter filter;
    filter.setSampleRate(44100.0f);
    filter.setBandPass(1000.0f, 100.0f);  // Center 1kHz, 100Hz BW
    
    // Filter should pass frequencies near center
    float input = 1.0f;
    float output = 0.0f;
    
    // Warm up filter
    for (int i = 0; i < 1000; ++i) {
        output = filter.process(input);
    }
    
    // Output should be non-zero for resonant BP
    REQUIRE(output != 0.0f);
}

TEST_CASE("Voice Formant Processing", "[dsp]") {
    Voice voice;
    voice.setNote(60, 127);  // Middle C
    voice.setVowel('a');
    
    // Process a few samples
    float sample = 0.0f;
    for (int i = 0; i < 100; ++i) {
        sample = voice.process();
    }
    
    // Should produce audio (not silent when note is on)
    // Note: ADSR attack might start at 0, so check after attack phase
    bool hasSignal = false;
    for (int i = 0; i < 10000; ++i) {
        if (std::abs(voice.process()) > 0.001f) {
            hasSignal = true;
            break;
        }
    }
    
    REQUIRE(hasSignal);
}

TEST_CASE("Formant Frequencies", "[tuning]") {
    // Verify formant frequencies match Prophecy specs
    REQUIRE(FORMANT_A.frequencies[0] == Approx(730.0f));
    REQUIRE(FORMANT_E.frequencies[1] == Approx(2290.0f));
}
