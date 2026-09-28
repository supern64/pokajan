#ifndef SOUND_H
#define SOUND_H

typedef enum {
    SFX_CARD_DRAWN,
    SFX_CARD_DISCARDED,
    SFX_DISCARD_AVAILABLE,
    SFX_DISCARD_USED,
    SFX_DECLARE_1,
    SFX_DECLARE_2,
    SFX_COIN,
    SFX_OUT_OF_COINS,
    SFX_OUT_OF_CARDS,
    SFX_HIGHLIGHT_WINNER
} SoundEffect;

typedef enum {
    CV_POKAJAN_1,
    CV_POKAJAN_2,
    CV_SELECTED
} CharacterVoice;

void SoundLoadBGM(void);
void SoundPlayBGM(void);
void SoundSetBGMVolume(float volume);
void SoundStopBGM(void);
void SoundUnloadBGM(void);

void SoundLoadSFX(void);
void SoundPlaySFX(SoundEffect sfx);
void SoundPlaySFXCoin(int count);
void SoundUnloadSFX(void);
void SoundLoadCharacterVoiceIntoSlot(int id, int slot);
void SoundEnsureCharacterVoiceLoaded(void);
void SoundPlayCharacterVoiceFromSlot(int slot, CharacterVoice voice);
void SoundUnloadCharacterVoiceSlot(int slot);
void SoundUnloadCharacterVoices(void);

#endif