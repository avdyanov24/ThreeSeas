#ifndef AUDIO_H
#define AUDIO_H

#include "raylib.h"
#include "types.h"

// Every sound the game plays, the looping ambience and the volumes
struct Sounds {
    bool isEnabled;
    float effectsVolume;  // 0 to 1, for every sound effect
    Music ambience;       // "Dungeon Pulse", loops in the menus and in the game
    Sound footstep;
    Sound heartbeat;
    Sound lowTorch;
    Sound pickup;
    Sound doorUnlock;
    Sound trapSnap;
    Sound win;
    Sound torchOut;
    Sound click;
    Sound turnChime;
};

// Loads every sound and the ambience. Call it once after the audio device is open.
Sounds loadSounds();

// Plays a sound at the effects volume, unless sound is switched off.
void playSound(const Sounds& sounds, SoundId id);

// Plays a sound at the effects volume times loudness (0 to 1), unless sound is off.
void playSoundAtLoudness(const Sounds& sounds, SoundId id, float loudness);

// Stops a sound if it is playing.
void stopSound(const Sounds& sounds, SoundId id);

// Returns true while a sound is playing.
bool isSoundPlaying(const Sounds& sounds, SoundId id);

// Switches all sounds and the ambience on or off.
void setSoundEnabled(Sounds& sounds, bool isEnabled);

// Sets how loud the sound effects are, from 0 to 1.
void setEffectsVolume(Sounds& sounds, float volume);

// Keeps the ambience playing at volume (0 to 1). Call it once every frame.
void updateAmbience(Sounds& sounds, float volume);

// Frees every sound and the ambience.
void unloadSounds(Sounds& sounds);

#endif  // AUDIO_H
