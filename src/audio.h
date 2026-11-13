#ifndef AUDIO_H
#define AUDIO_H

#include "raylib.h"
#include "types.h"

// Every sound the game plays
struct Sounds {
    bool isEnabled;
    Sound footstep;
    Sound torchCrackle;
    Sound lowTorch;
    Sound pickup;
    Sound doorUnlock;
    Sound trapSnap;
    Sound win;
    Sound torchOut;
    Sound click;
    Sound turnChime;
};

// Loads every sound. Call it once after the audio device is open.
Sounds loadSounds();

// Plays a sound, unless sound is switched off.
void playSound(const Sounds& sounds, SoundId id);

// Stops a sound if it is playing.
void stopSound(const Sounds& sounds, SoundId id);

// Returns true while a sound is playing.
bool isSoundPlaying(const Sounds& sounds, SoundId id);

// Switches all sounds on or off.
void setSoundEnabled(Sounds& sounds, bool isEnabled);

// Frees every sound.
void unloadSounds(Sounds& sounds);

#endif  // AUDIO_H
