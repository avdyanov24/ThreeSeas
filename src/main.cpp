#include "raylib.h"
#include "game.h"
#include "screens.h"

// Opens the game window and runs the screen state machine.
int main() {
    InitWindow(1280, 720, "Crossroads");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    GameState game = createEmptyGame();
    UiState ui = {};
    Textures textures = {};
    loadUiTextures(textures);
    Screen screen = TITLE;

    while (!WindowShouldClose() && !ui.isExitRequested) {
        float deltaTime = GetFrameTime();
        screen = updateScreen(screen, game, ui, deltaTime);

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
