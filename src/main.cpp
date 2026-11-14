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

// Starts the tutorial maze with the level 1 look, loading those textures when they
// changed. Returns nothing.
static void startTutorialGame(GameState& game, const UiState& ui, Textures& textures,
                              int& loadedLevelId) {
    if (loadedLevelId != 1) {
        unloadTextures(textures);
        textures = loadTextures(*getLevel(1));
        loadUiTextures(textures);
        loadedLevelId = 1;
    }
    startTutorial(game, ui.playerName, ui.characters[0]);
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
    game.turnNumber = 1;
    game.turnSecondsLeft = getTurnSeconds(1, game.playerCount, game.maze.size);
}

// Plays the sounds the screens asked for this frame, the turn chime when the pass screen
// opens, keeps the ambience looping (menu or game volume) and plays the low-fuel heartbeat.
// Everything follows the Sound setting and the volume sliders. Returns nothing.
static void playFrameSounds(Sounds& sounds, UiState& ui, const GameState& game, Screen screen,
                            Screen previousScreen, float deltaTime) {
    setSoundEnabled(sounds, ui.settings.isSoundOn);
    setEffectsVolume(sounds, static_cast<float>(ui.settings.effectsVolume) / MAX_VOLUME);
    bool isInGame = screen == PLAYING || screen == PAUSED || screen == PASS_SCREEN;
    int musicVolume = isInGame ? ui.settings.gameMusicVolume : ui.settings.menuMusicVolume;
    updateAmbience(sounds, static_cast<float>(musicVolume) / MAX_VOLUME);
    if (screen == PASS_SCREEN && previousScreen != PASS_SCREEN) {
        ui.soundRequests[SOUND_TURN_CHIME] = true;
    }
    for (int i = 0; i < SOUND_COUNT; i++) {
        if (ui.soundRequests[i]) {
            playSound(sounds, static_cast<SoundId>(i));
            ui.soundRequests[i] = false;
        }
    }

    // Below 30% fuel a heartbeat starts. danger goes from 0 (30% fuel) to 1 (empty), and
    // the beat gets faster (60 to 130 a minute) and louder with it.
    float danger = 0.0f;
    if (screen == PLAYING && game.players != nullptr && game.winner == NO_WINNER) {
        const Player& player = game.players[game.currentPlayer];
        float fuelRatio = player.maxFuel > 0.0f ? player.fuel / player.maxFuel : 0.0f;
        if (!player.isOut && fuelRatio < 0.3f) {
            danger = 1.0f - fuelRatio / 0.3f;
        }
    }
    if (danger <= 0.0f) {
        ui.heartbeatSeconds = 0.0f;
        return;
    }
    ui.heartbeatSeconds -= deltaTime;
    if (ui.heartbeatSeconds <= 0.0f) {
        playSoundAtLoudness(sounds, SOUND_HEARTBEAT, 0.4f + 0.4f * danger);
        ui.heartbeatSeconds = 60.0f / (60.0f + 70.0f * danger);
    }
}

// Returns where the 1280 x 720 game picture goes in the window: as big as fits, centred,
// with black bars on the sides that are left over.
static Rectangle getViewRectangle() {
    float windowWidth = static_cast<float>(GetScreenWidth());
    float windowHeight = static_cast<float>(GetScreenHeight());
    float scaleX = windowWidth / GAME_WIDTH;
    float scaleY = windowHeight / GAME_HEIGHT;
    float scale = scaleX < scaleY ? scaleX : scaleY;
    float width = GAME_WIDTH * scale;
    float height = GAME_HEIGHT * scale;
    return Rectangle{(windowWidth - width) / 2.0f, (windowHeight - height) / 2.0f, width, height};
}

// Opens the game window and runs the screen state machine.
int main() {
    InitWindow(GAME_WIDTH, GAME_HEIGHT, "Three Seas");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);
    InitAudioDevice();
    Sounds sounds = loadSounds();

    GameState game = createEmptyGame();
    UiState ui = {};
    ui.settings = loadSettings();
    if (ui.settings.isFullscreen) {
        ToggleBorderlessWindowed();
    }
    // Every frame is drawn on this 1280 x 720 canvas and then scaled to the window
    RenderTexture2D canvas = LoadRenderTexture(GAME_WIDTH, GAME_HEIGHT);
    SetTextureFilter(canvas.texture, TEXTURE_FILTER_BILINEAR);
    srand(static_cast<unsigned int>(time(nullptr)));
    int loadedLevelId = 1;
    Textures textures = loadTextures(*getLevel(loadedLevelId));
    loadUiTextures(textures);
    Level arenaLevel = {};  // the multiplayer arena, game.level points to it
    Screen screen = TITLE;

    while (!WindowShouldClose() && !ui.isExitRequested) {
        // F11 switches fullscreen, like the switch in Settings
        if (IsKeyPressed(KEY_F11)) {
            ui.settings.isFullscreen = !ui.settings.isFullscreen;
            saveSettings(ui.settings);
        }
        if (ui.settings.isFullscreen != IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE)) {
            ToggleBorderlessWindowed();
        }
        // The mouse is turned into game coordinates, so buttons work at any window size
        Rectangle view = getViewRectangle();
        SetMouseOffset(static_cast<int>(-view.x), static_cast<int>(-view.y));
        SetMouseScale(GAME_WIDTH / view.width, GAME_HEIGHT / view.height);

        float deltaTime = GetFrameTime();
        Screen previousScreen = screen;
        Screen nextScreen = updateScreen(screen, game, ui, deltaTime);
        if (nextScreen != screen) {
            // Pausing and resuming must not fade the maze out and back in.
            bool isPauseSwitch = (screen == PLAYING && nextScreen == PAUSED) ||
                                 (screen == PAUSED && nextScreen == PLAYING);
            ui.screenSeconds = isPauseSwitch ? 1.0f : 0.0f;
        } else {
            ui.screenSeconds += deltaTime;
        }
        screen = nextScreen;
        if (screen == PLAYING && game.players == nullptr) {
            if (ui.isMultiplayerGame) {
                startMultiplayerGame(game, ui, arenaLevel, textures, loadedLevelId);
                screen = PASS_SCREEN;  // the first player takes the laptop
                ui.screenSeconds = 0.0f;
            } else if (ui.isTutorial) {
                startTutorialGame(game, ui, textures, loadedLevelId);
            } else {
                startCampaignLevel(game, ui, textures, loadedLevelId);
            }
        }

        if (ui.isExitRequested) {
            break;
        }
        playFrameSounds(sounds, ui, game, screen, previousScreen, deltaTime);

        BeginTextureMode(canvas);
        ClearBackground(BLACK);
        drawScreen(screen, game, ui, textures);
        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);
        // A render texture is upside down, so the source height is negative
        Rectangle source = {0.0f, 0.0f, static_cast<float>(GAME_WIDTH),
                            -static_cast<float>(GAME_HEIGHT)};
        DrawTexturePro(canvas.texture, source, getViewRectangle(), Vector2{0.0f, 0.0f}, 0.0f,
                       WHITE);
        EndDrawing();
    }

    destroyGame(game);
    UnloadRenderTexture(canvas);
    unloadTextures(textures);
    unloadSounds(sounds);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
