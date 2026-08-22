#include "sound.h"
#include <raylib.h>
#include <stdio.h>

#define BGM_LOOP_START 9.75
#define BGM_LOOP_END 113.25


typedef struct {
    float *samples;         // interleaved PCM data
    unsigned int channels;
    unsigned int cursor;    // current frame index (not sample index)
    unsigned int loopStart; // in frames
    unsigned int loopEnd;   // in frames
} LoopData;

static Wave BGMWave;
static float *BGMSamples;
static LoopData BGMLoopData;
static AudioStream BGMAudioStream;

static void GiveMeADamnFrame(void *bufferData, unsigned int frames) {
    float *out = (float *)bufferData;
    for (unsigned int i = 0; i < frames; i++) {
        for (unsigned int c = 0; c < BGMLoopData.channels; c++) {
            out[i*BGMLoopData.channels + c] = BGMLoopData.samples[BGMLoopData.cursor*BGMLoopData.channels + c];
        }
        BGMLoopData.cursor++;
        if (BGMLoopData.cursor >= BGMLoopData.loopEnd) BGMLoopData.cursor = BGMLoopData.loopStart;
    }
}

void SoundLoadBGM(void) {
    BGMWave = LoadWave("assets/bgm.wav");
    BGMSamples = LoadWaveSamples(BGMWave);

    BGMLoopData = (LoopData){
        .samples = BGMSamples,
        .channels = 2,
        .cursor = 0,
        .loopStart = 429975,
        .loopEnd = 4994325
    };

    BGMAudioStream = LoadAudioStream(44100, 32, 2);
    SetAudioStreamCallback(BGMAudioStream, GiveMeADamnFrame);
}

void SoundPlayBGM(void) {
    PlayAudioStream(BGMAudioStream);
}

void SoundUnloadBGM(void) {
    UnloadAudioStream(BGMAudioStream);
    UnloadWaveSamples(BGMSamples);
    UnloadWave(BGMWave);
}