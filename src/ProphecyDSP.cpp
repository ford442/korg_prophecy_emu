#include "ProphecyDSP.h"

namespace ProphecyDSP {

ProphecyDSP::ProphecyDSP() {
    setWaveform(Voice::Saw);
    setFormantVowel('a');
}

void ProphecyDSP::process(float* outputL, float* outputR, int frames) {
    // Clear outputs
    for (int i = 0; i < frames; ++i) {
        outputL[i] = 0.0f;
        outputR[i] = 0.0f;
    }
    
    // Sum active voices
    for (auto& voice : voices) {
        if (voice.isActive()) {
            for (int i = 0; i < frames; ++i) {
                float sample = voice.process();
                outputL[i] += sample;
                outputR[i] += sample;  // Mono source, stereo out
            }
        }
    }
    
    // Soft clipping/limiting (Prophecy style output stage)
    for (int i = 0; i < frames; ++i) {
        auto softClip = [](float x) {
            if (x > 0.7f) return 0.7f + (x - 0.7f) / (1.0f + (x - 0.7f) * (x - 0.7f));
            if (x < -0.7f) return -0.7f + (x + 0.7f) / (1.0f + (x + 0.7f) * (x + 0.7f));
            return x;
        };
        outputL[i] = softClip(outputL[i]);
        outputR[i] = softClip(outputR[i]);
    }
}

int ProphecyDSP::findFreeVoice() {
    // Find inactive voice or steal oldest
    for (int i = 0; i < MAX_VOICES; ++i) {
        if (!voices[i].isActive()) return i;
    }
    // Steal last allocated (simple)
    lastVoice = (lastVoice + 1) % MAX_VOICES;
    voices[lastVoice].reset();
    return lastVoice;
}

void ProphecyDSP::noteOn(int note, float velocity) {
    int idx = findFreeVoice();
    voices[idx].setNote(static_cast<float>(note), velocity / 127.0f);
    lastVoice = idx;
}

void ProphecyDSP::noteOff(int note) {
    for (auto& voice : voices) {
        // Simple release matching by frequency approximation
        // (Production would use note ID tracking)
        float freq = midiNoteToFrequency(note);
        if (voice.isActive() && std::abs(voice.getCurrentFrequency() - freq) < 1.0f) {
            voice.noteOff();
        }
    }
}

void ProphecyDSP::allNotesOff() {
    for (auto& voice : voices) voice.noteOff();
}

void ProphecyDSP::setFormantVowel(char vowel) {
    currentVowel = vowel;
    for (auto& voice : voices) voice.setVowel(vowel);
}

void ProphecyDSP::setWaveform(Voice::Waveform wf) {
    currentWaveform = wf;
    for (auto& voice : voices) voice.setWaveform(wf);
}

void ProphecyDSP::setPortamento(float time) {
    portamentoTime = time;
    // Apply to all voices
    for (auto& voice : voices) {
        // Convert to rate coefficient (simplified)
        // voice.setPortamentoRate(1.0f / (time * SAMPLE_RATE + 1.0f));
    }
}

void ProphecyDSP::setCutoff(float freq) {
    // Global cutoff post-processing placeholder
}

void ProphecyDSP::setResonance(float res) {
    // Modulate formant Q values
}

} // namespace ProphecyDSP
