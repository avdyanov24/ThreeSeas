#include "raylib.h"

#include <cstdlib>
#include <ctime>

#include "audio.h"
#include "game.h"
#include "levels.h"
#include "save.h"
#include "screens.h"
#include "validation.h"

// Starts the level picked in the menus: loads its textures when the level changed,
// builds a new random maze and creates the player. Returns nothing.
static void startCampaignLevel(GameState& game, const UiState& ui, Textures& textures,
                               int& loadedLevelId) {
    int levelId = ui.selectedLevel;
    if (levelId < 1 || levelId > LEVEL_COUNT) {
        levelId = 1;
    }
    const Level& level = *getLevel(levelId);

    int size = MIN_CAMPAIGN_SIZE;
    if (isValidSize(ui.chosenSize, false)) {
        size = ui.chosenSize;
    }

    if (levelId != loadedLevelId) {
        unloadTextures(textures);
        textures = loadTextures(level);
        loadUiTextures(textures);
        loadedLevelId = levelId;
    }

    startLevel(game, level, size, false);
    std::string names[1] = {ui.playerName};
    CharacterId characters[1] = {ui.characters[0]};
    createPlayers(game, 1, names, characters);
}

// Starts a multiplayer game on the arena picked in the setup: builds the arena as a level
// (the look comes from a campaign level, the water and cold from the arena), loads its
// textures when they changed, builds a new maze and creates the players. The first player
// gets the first turn time. Returns nothing.
static void startMultiplayerGame(GameState& game, const UiState& ui, Level& arenaLevel,
                                 Textures& textures, int& loadedLevelId) {
    // The campaign level whose pictures each arena borrows: Pliska, Cherven (water), Shipka (ice)
    const int arenaLookLevels[ARENA_COUNT] = {1, 5, 9};
    const Arena* arena = getArena(ui.arenaId);
    if (arena == nullptr) {
        arena = getArena(1);
    }
    int lookLevelId = arenaLookLevels[arena->id - 1];

    arenaLevel = *getLevel(lookLevelId);
    arenaLevel.place = arena->name;
    arenaLevel.hasWater = arena->hasWater;
    arenaLevel.isCold = arena->isCold;
    arenaLevel.oilPer100 = 3.0f;
    arenaLevel.trapsPer100 = 0.0f;  // the players place the traps
    arenaLevel.doors = ui.chosenSize / 10;

    if (lookLevelId != loadedLevelId) {
        unloadTextures(textures);
        textures = loadTextures(arenaLevel);
        loadUiTextures(textures);
        loadedLevelId = lookLevelId;
    }

    startLevel(game, arenaLevel, ui.chosenSize, true);
    std::string names[MAX_PLAYERS];
    CharacterId characters[MAX_PLAYERS];
    for (int i = 0; i < MAX_PLAYERS; i++) {
        names[i] = ui.playerNames[i];
        characters[i] = ui.characters[i];
    }
    createPlayers(game, ui.playerCount, names, characters);
    game.stepsLeft = 3;
    game.turnNumber = 1;
    game.turnSecondsLeft = getTurnSeconds(1, game.playerCount, game.maze.size);
}

// Plays the sounds the screens asked for this frame, the turn chime when the pass screen
// opens, and keeps the torch crackle looping while a torch burns in the game. Everything
// follows the Sound setting. Returns nothing.
static void playFrameSounds(Sounds& sounds, UiState& ui, const GameState& game, Screen screen,
                            Screen previousScreen) {
    setSoundEnabled(sounds, ui.settings.isSoundOn);
    if (screen == PASS_SCREEN && previousScreen != PASS_SCREEN) {
        ui.soundRequests[SOUND_TURN_CHIME] = true;
    }
    for (int i = 0; i < SOUND_COUNT; i++) {
        if (ui.soundRequests[i]) {
            playSound(sounds, static_cast<SoundId>(i));
            ui.soundRequests[i] = false;
        }
    }

    bool isTorchBurning = ui.settings.isSoundOn && screen == PLAYING &&
                          game.players != nullptr && !game.players[game.currentPlayer].isOut;
    bool isCrackling = isSoundPlaying(sounds, SOUND_TORCH_CRACKLE);
    if (isTorchBurning && !isCrackling) {
        // The crackle is short, so it starts again whenever it ends
        playSound(sounds, SOUND_TORCH_CRACKLE);
    } else if (!isTorchBurning && isCrackling) {
        stopSound(sounds, SOUND_TORCH_CRACKLE);
    }
}

// Opens the game window and runs the screen state machine.
int main() {
    InitWindow(1280, 720, "Crossroads");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);
    InitAudioDevice();
    Sounds sounds = loadSounds();

    GameState game = createEmptyGame();
    UiState ui = {};
    ui.settings = loadSettings();
    srand(static_cast<unsigned int>(time(nullptr)));
    int loadedLevelId = 1;
    Textures textures = loadTextures(*getLevel(loadedLevelId));
    loadUiTextures(textures);
    Level arenaLevel = {};  // the multiplayer arena, game.level points to it
    Screen screen = TITLE;

    while (!WindowShouldClose() && !ui.isExitRequested) {
        float deltaTime = GetFrameTime();
        Screen previousScreen = screen;
        Screen nextScreen = updateScreen(screen, game, ui, deltaTime);
        if (nextScreen != screen) {
            ui.screenSeconds = 0.0f;
        } else {
            ui.screenSeconds += deltaTime;
        }
        screen = nextScreen;
        if (screen == PLAYING && game.players == nullptr) {
            if (ui.isMultiplayerGame) {
                startMultiplayerGame(game, ui, arenaLevel, textures, loadedLevelId);
                screen = PASS_SCREEN;  // the first player takes the laptop
                ui.screenSeconds = 0.0f;
            } else {
                startCampaignLevel(game, ui, textures, loadedLevelId);
            }
        }

        if (ui.isExitRequested) {
            break;
        }
        playFrameSounds(sounds, ui, game, screen, previousScreen);

        BeginDrawing();
        ClearBackground(BLACK);
        drawScreen(screen, game, ui, textures);
        EndDrawing();
    }

    destroyGame(game);
    unloadTextures(textures);
    unloadSounds(sounds);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
