#include "raylib.h"

#include <cstdlib>
#include <ctime>

#include "game.h"
#include "levels.h"
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

// Opens the game window and runs the screen state machine.
int main() {
    InitWindow(1280, 720, "Crossroads");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    GameState game = createEmptyGame();
    UiState ui = {};
    srand(static_cast<unsigned int>(time(nullptr)));
    int loadedLevelId = 1;
    Textures textures = loadTextures(*getLevel(loadedLevelId));
    loadUiTextures(textures);
    Screen screen = TITLE;

    while (!WindowShouldClose() && !ui.isExitRequested) {
        float deltaTime = GetFrameTime();
        Screen previousScreen = screen;
        screen = updateScreen(screen, game, ui, deltaTime);
        // Coming back from the pause screen continues the same maze
        if (screen == PLAYING && previousScreen != PLAYING && previousScreen != PAUSED) {
            startCampaignLevel(game, ui, textures, loadedLevelId);
        }

        if (ui.isExitRequested) {
            break;
        }

        BeginDrawing();
        ClearBackground(BLACK);
        drawScreen(screen, game, ui, textures);
        EndDrawing();
    }

    destroyGame(game);
    unloadTextures(textures);
    CloseWindow();
    return 0;
}
