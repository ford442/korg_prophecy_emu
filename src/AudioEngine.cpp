#include "AudioEngine.h"
#include <cstring>
#include <algorithm>
#include <cmath>

namespace ProphecyDSP {

AudioEngine::AudioEngine() = default;
AudioEngine::~AudioEngine() { shutdown(); }

bool AudioEngine::initialize() {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        return false;
    }
    
    SDL_AudioSpec desired, obtained;
    SDL_zero(desired);
    desired.freq = static_cast<int>(SAMPLE_RATE);
    desired.format = AUDIO_F32;  // Float32 (Web Audio native)
    desired.channels = 2;        // Stereo
    desired.samples = BLOCK_SIZE;
    desired.callback = audioCallback;
    desired.userdata = this;
    
    deviceId = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, 0);
    if (deviceId == 0) {
        return false;
    }
    
    outputBuffer.resize(BLOCK_SIZE * 2);
    running = true;
    SDL_PauseAudioDevice(deviceId, 0);  // Start playback
    
    return true;
}

void AudioEngine::shutdown() {
    if (deviceId != 0) {
        SDL_CloseAudioDevice(deviceId);
        deviceId = 0;
    }
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    running = false;
}

void AudioEngine::handleMidiNoteOn(int note, int velocity) {
    // Thread-safe: SDL audio callback runs on different thread
    // In production, use ring buffer or atomic queue
    dsp.noteOn(note, static_cast<float>(velocity));
}

void AudioEngine::handleMidiNoteOff(int note) {
    dsp.noteOff(note);
}

void AudioEngine::handleControlChange(int cc, int value) {
    float norm = value / 127.0f;
    switch (cc) {
        case 1:   // Mod wheel - could affect formant shift
            break;
        case 7:   // Volume
            break;
        case 74:  // Filter cutoff
            dsp.setCutoff(200.0f + norm * 8000.0f);
            break;
        case 71:  // Resonance
            dsp.setResonance(0.5f + norm * 10.0f);
            break;
    }
}

void AudioEngine::audioCallback(void* userdata, Uint8* stream, int len) {
    auto* engine = static_cast<AudioEngine*>(userdata);
    int frames = len / (sizeof(float) * 2);  // Stereo F32
    engine->processAudio(reinterpret_cast<float*>(stream), frames);
}

void AudioEngine::processAudio(float* buffer, int frames) {
    // Deinterleave for processing if needed, but our DSP handles stereo pointers
    float* left = outputBuffer.data();
    float* right = outputBuffer.data() + BLOCK_SIZE;
    
    // Ensure we don't exceed block size (shouldn't happen with proper SDL setup)
    if (frames > BLOCK_SIZE) frames = BLOCK_SIZE;
    
    dsp.process(left, right, frames);
    
    // Interleave to stereo output buffer
    for (int i = 0; i < frames; ++i) {
        buffer[i * 2] = left[i];
        buffer[i * 2 + 1] = right[i];
    }
}

} // namespace ProphecyDSP
