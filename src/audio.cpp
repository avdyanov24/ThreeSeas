#include "audio.h"

#ifndef SOUND_DIR
#define SOUND_DIR "assets/sounds/"
#endif

// Loads every sound from SOUND_DIR and returns them enabled.
Sounds loadSounds() {
    Sounds sounds = {};
    sounds.isEnabled = true;
    sounds.footstep = LoadSound(SOUND_DIR "footstep.wav");
    sounds.torchCrackle = LoadSound(SOUND_DIR "torch_crackle.wav");
    sounds.lowTorch = LoadSound(SOUND_DIR "torch_low.wav");
    sounds.pickup = LoadSound(SOUND_DIR "pickup.wav");
    sounds.doorUnlock = LoadSound(SOUND_DIR "door_unlock.wav");
    sounds.trapSnap = LoadSound(SOUND_DIR "trap_snap.wav");
    sounds.win = LoadSound(SOUND_DIR "win.wav");
    sounds.torchOut = LoadSound(SOUND_DIR "torch_out.wav");
    sounds.click = LoadSound(SOUND_DIR "ui_click.wav");
    sounds.turnChime = LoadSound(SOUND_DIR "turn_chime.wav");
    return sounds;
}

// Copies the sound for id into sound. Returns false if id is not a sound.
static bool getSound(const Sounds& sounds, SoundId id, Sound& sound) {
    switch (id) {
        case SOUND_FOOTSTEP:
            sound = sounds.footstep;
            return true;
        case SOUND_TORCH_CRACKLE:
            sound = sounds.torchCrackle;
            return true;
        case SOUND_LOW_TORCH:
            sound = sounds.lowTorch;
            return true;
        case SOUND_PICKUP:
            sound = sounds.pickup;
            return true;
        case SOUND_DOOR_UNLOCK:
            sound = sounds.doorUnlock;
            return true;
        case SOUND_TRAP_SNAP:
            sound = sounds.trapSnap;
            return true;
        case SOUND_WIN:
            sound = sounds.win;
            return true;
        case SOUND_TORCH_OUT:
            sound = sounds.torchOut;
            return true;
        case SOUND_CLICK:
            sound = sounds.click;
            return true;
        case SOUND_TURN_CHIME:
            sound = sounds.turnChime;
            return true;
        default:
            return false;
    }
}

// Plays the sound for id unless sounds are disabled or id is invalid.
void playSound(const Sounds& sounds, SoundId id) {
    Sound sound = {};
    if (sounds.isEnabled && getSound(sounds, id, sound)) {
        PlaySound(sound);
    }
}

// Stops the sound for id. Does nothing if id is invalid.
void stopSound(const Sounds& sounds, SoundId id) {
    Sound sound = {};
    if (getSound(sounds, id, sound)) {
        StopSound(sound);
    }
}

// Returns true while the sound for id is playing, false for an invalid id.
bool isSoundPlaying(const Sounds& sounds, SoundId id) {
    Sound sound = {};
    return getSound(sounds, id, sound) && IsSoundPlaying(sound);
}

// Sets whether playSound is allowed to play sounds.
void setSoundEnabled(Sounds& sounds, bool isEnabled) {
    sounds.isEnabled = isEnabled;
}

// Unloads every sound and clears the Sounds value.
void unloadSounds(Sounds& sounds) {
    UnloadSound(sounds.footstep);
    UnloadSound(sounds.torchCrackle);
    UnloadSound(sounds.lowTorch);
    UnloadSound(sounds.pickup);
    UnloadSound(sounds.doorUnlock);
    UnloadSound(sounds.trapSnap);
    UnloadSound(sounds.win);
    UnloadSound(sounds.torchOut);
    UnloadSound(sounds.click);
    UnloadSound(sounds.turnChime);
    sounds = {};
}
