/**
 * Emscripten WASM Entry Point
 * 
 * Exports C API for JavaScript integration.
 * Follows Emscripten Web Audio best practices.
 */

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "AudioEngine.h"
#include <memory>

using namespace ProphecyDSP;

static std::unique_ptr<AudioEngine> g_engine;

// C API for JS interop
extern "C" {

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
bool initAudio() {
    g_engine = std::make_unique<AudioEngine>();
    return g_engine->initialize();
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
void shutdownAudio() {
    g_engine.reset();
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
void midiNoteOn(int note, int velocity) {
    if (g_engine) g_engine->handleMidiNoteOn(note, velocity);
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
void midiNoteOff(int note) {
    if (g_engine) g_engine->handleMidiNoteOff(note);
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
void setFormant(char vowel) {
    if (g_engine) g_engine->getDSP().setFormantVowel(vowel);
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
void setWaveform(int wf) {
    if (g_engine) g_engine->getDSP().setWaveform(static_cast<Voice::Waveform>(wf));
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
void setCutoff(float freq) {
    if (g_engine) g_engine->getDSP().setCutoff(freq);
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
void setResonance(float res) {
    if (g_engine) g_engine->getDSP().setResonance(res);
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
void allNotesOff() {
    if (g_engine) g_engine->getDSP().allNotesOff();
}

} // extern "C"
