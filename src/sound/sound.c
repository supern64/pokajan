#include "sound.h"
#include <raylib.h>

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

static Sound SFX[10];
static Sound SFXCoinAlias[10];
static int CurrentCoinSoundSlot = 0;

static Sound CharacterVoices[4][3];
static int CharacterVoicesSlot[4] = { -1, -1, -1, -1 };

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

void SoundSetBGMVolume(float volume) {
    SetAudioStreamVolume(BGMAudioStream, volume);
}

void SoundStopBGM(void) {
    StopAudioStream(BGMAudioStream);
}

void SoundUnloadBGM(void) {
    UnloadAudioStream(BGMAudioStream);
    UnloadWaveSamples(BGMSamples);
    UnloadWave(BGMWave);
}

void SoundLoadSFX(void) {
    SFX[CARD_DRAWN] = LoadSound("assets/sfx/draw.wav");
    SFX[CARD_DISCARDED] = LoadSound("assets/sfx/select_discard.wav");
    SFX[DISCARD_AVAILABLE] = LoadSound("assets/sfx/discard_usable.wav");
    SFX[DISCARD_USED] = LoadSound("assets/sfx/discard_declare.wav");
    SFX[DECLARE_1] = LoadSound("assets/sfx/declare.wav");
    SFX[DECLARE_2] = LoadSound("assets/sfx/declare_big.wav");
    SFX[COIN] = LoadSound("assets/sfx/coin.wav");
    SFX[OUT_OF_COINS] = LoadSound("assets/sfx/end_drained.wav");
    SFX[OUT_OF_CARDS] = LoadSound("assets/sfx/end_nocards.wav");
    SFX[HIGHLIGHT_WINNER] = LoadSound("assets/sfx/end_winner.wav");

    // create sound aliases for coin
    for (int i = 0; i < 10; i++) {
        SFXCoinAlias[i] = LoadSoundAlias(SFX[COIN]);
    }
}

void SoundPlaySFX(SoundEffect sfx) {
    PlaySound(SFX[sfx]);
}

void SoundPlaySFXCoin(int count) {
    for (int i = 0; i < count; i++) {
        PlaySound(SFXCoinAlias[CurrentCoinSoundSlot++]);
        if (CurrentCoinSoundSlot > 9) CurrentCoinSoundSlot = 0;
    }
}

void SoundUnloadSFX(void) {
    for (int i = 0; i < 10; i++) {
        UnloadSoundAlias(SFXCoinAlias[i]);
    }
    for (int i = 0; i < 10; i++) {
        UnloadSound(SFX[i]);
    }
}

void SoundLoadCharacterVoiceIntoSlot(int id, int slot) {
    if (CharacterVoicesSlot[slot] != -1) return;
    CharacterVoices[slot][POKAJAN_1] = LoadSound(TextFormat("assets/vo/cv-sys_chr-%05d_mg-combo-card-glad-01.wav", id));
    CharacterVoices[slot][POKAJAN_2] = LoadSound(TextFormat("assets/vo/cv-sys_chr-%05d_mg-combo-card-glad-02.wav", id));
    CharacterVoices[slot][SELECTED] = LoadSound(TextFormat("assets/vo/cv-sys_chr-%05d_mg-combo-card-start-01.wav", id));
    CharacterVoicesSlot[slot] = id;
}

void SoundPlayCharacterVoiceFromSlot(int slot, CharacterVoice voice) {
    PlaySound(CharacterVoices[slot][voice]);
}

void SoundUnloadCharacterVoiceSlot(int slot) {
    if (CharacterVoicesSlot[slot] == -1) return;
    for (int j = 0; j < 3; j++) {
        UnloadSound(CharacterVoices[slot][j]);
    }
    CharacterVoicesSlot[slot] = -1;
}

void SoundUnloadCharacterVoices(void) {
    for (int i = 0; i < 4; i++) {
        if (CharacterVoicesSlot[i] == -1) continue;
        for (int j = 0; j < 3; j++) {
            UnloadSound(CharacterVoices[i][j]);
        }
        CharacterVoicesSlot[i] = -1;
    }
}