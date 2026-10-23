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
    Screen screen = TITLE;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        screen = updateScreen(screen, game, ui, deltaTime);

        BeginDrawing();
        ClearBackground(BLACK);
        drawScreen(screen, game, ui, textures);
        EndDrawing();
    }

    destroyGame(game);
    CloseWindow();
    return 0;
}
