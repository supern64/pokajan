#ifndef SOUND_H
#define SOUND_H

typedef enum {
    CARD_DRAWN,
    CARD_DISCARDED,
    DISCARD_AVAILABLE,
    DISCARD_USED,
    DECLARE_1,
    DECLARE_2,
    COIN,
    OUT_OF_COINS,
    OUT_OF_CARDS,
    HIGHLIGHT_WINNER
} SoundEffect;

typedef enum {
    POKAJAN_1,
    POKAJAN_2,
    SELECTED
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
void SoundPlayCharacterVoiceFromSlot(int slot, CharacterVoice voice);
void SoundUnloadCharacterVoiceSlot(int slot);
void SoundUnloadCharacterVoices(void);

#endif