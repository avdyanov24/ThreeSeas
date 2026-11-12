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

// Plays the sound for id unless sounds are disabled or id is invalid.
void playSound(const Sounds& sounds, SoundId id) {
    if (!sounds.isEnabled) {
        return;
    }

    Sound sound = {};
    switch (id) {
        case SOUND_FOOTSTEP:
            sound = sounds.footstep;
            break;
        case SOUND_TORCH_CRACKLE:
            sound = sounds.torchCrackle;
            break;
        case SOUND_LOW_TORCH:
            sound = sounds.lowTorch;
            break;
        case SOUND_PICKUP:
            sound = sounds.pickup;
            break;
        case SOUND_DOOR_UNLOCK:
            sound = sounds.doorUnlock;
            break;
        case SOUND_TRAP_SNAP:
            sound = sounds.trapSnap;
            break;
        case SOUND_WIN:
            sound = sounds.win;
            break;
        case SOUND_TORCH_OUT:
            sound = sounds.torchOut;
            break;
        case SOUND_CLICK:
            sound = sounds.click;
            break;
        case SOUND_TURN_CHIME:
            sound = sounds.turnChime;
            break;
        default:
            return;
    }

    PlaySound(sound);
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
