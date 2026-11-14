#include "audio.h"

#ifndef SOUND_DIR
#define SOUND_DIR "assets/sounds/"
#endif

// Loads every sound from SOUND_DIR and returns them enabled at full effects volume.
Sounds loadSounds() {
    Sounds sounds = {};
    sounds.isEnabled = true;
    sounds.effectsVolume = 1.0f;
    sounds.ambience = LoadMusicStream(SOUND_DIR "dungeon_pulse.wav");
    sounds.ambience.looping = true;
    sounds.footstep = LoadSound(SOUND_DIR "footstep.wav");
    sounds.heartbeat = LoadSound(SOUND_DIR "heartbeat.wav");
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
        case SOUND_HEARTBEAT:
            sound = sounds.heartbeat;
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

// Plays the sound for id at full loudness.
void playSound(const Sounds& sounds, SoundId id) {
    playSoundAtLoudness(sounds, id, 1.0f);
}

// Plays the sound for id, scaled by the effects volume and loudness, unless sounds are
// disabled or id is invalid.
void playSoundAtLoudness(const Sounds& sounds, SoundId id, float loudness) {
    Sound sound = {};
    if (sounds.isEnabled && getSound(sounds, id, sound)) {
        SetSoundVolume(sound, sounds.effectsVolume * loudness);
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

// Sets whether sounds and the ambience are allowed to play.
void setSoundEnabled(Sounds& sounds, bool isEnabled) {
    sounds.isEnabled = isEnabled;
}

// Stores the effects volume, kept between 0 and 1.
void setEffectsVolume(Sounds& sounds, float volume) {
    if (volume < 0.0f) {
        volume = 0.0f;
    } else if (volume > 1.0f) {
        volume = 1.0f;
    }
    sounds.effectsVolume = volume;
}

// Starts the ambience if needed, sets its volume (0 when sounds are off) and feeds the
// stream so it keeps playing.
void updateAmbience(Sounds& sounds, float volume) {
    if (!IsMusicStreamPlaying(sounds.ambience)) {
        PlayMusicStream(sounds.ambience);
    }
    SetMusicVolume(sounds.ambience, sounds.isEnabled ? volume : 0.0f);
    UpdateMusicStream(sounds.ambience);
}

// Unloads every sound and the ambience and clears the Sounds value.
void unloadSounds(Sounds& sounds) {
    UnloadMusicStream(sounds.ambience);
    UnloadSound(sounds.footstep);
    UnloadSound(sounds.heartbeat);
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
