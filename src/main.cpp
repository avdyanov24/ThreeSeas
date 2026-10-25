#include "raylib.h"

#include <cstdlib>
#include <ctime>

#include "game.h"
#include "levels.h"
#include "screens.h"

// Opens the game window and runs the screen state machine.
int main() {
    InitWindow(1280, 720, "Crossroads");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    GameState game = createEmptyGame();
    UiState ui = {};
    srand(static_cast<unsigned int>(time(nullptr)));
    Textures textures = loadTextures(*getLevel(1));
    loadUiTextures(textures);
    Screen screen = TITLE;

    while (!WindowShouldClose() && !ui.isExitRequested) {
        float deltaTime = GetFrameTime();
        Screen previousScreen = screen;
        screen = updateScreen(screen, game, ui, deltaTime);
        if (screen == PLAYING && previousScreen != PLAYING) {
            startLevel(game, *getLevel(1), MIN_CAMPAIGN_SIZE, false);
            std::string names[1] = {ui.playerName};
            CharacterId characters[1] = {KRUM};
            createPlayers(game, 1, names, characters);
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
