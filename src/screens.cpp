#include "screens.h"

// Changes between the title and main menu screens when their keys are pressed.
Screen updateScreen(Screen screen, GameState&, UiState&, float) {
    if (screen == TITLE && IsKeyPressed(KEY_ENTER)) {
        return MAIN_MENU;
    }
    if (screen == MAIN_MENU && IsKeyPressed(KEY_ESCAPE)) {
        return TITLE;
    }
    return screen;
}

// Draws the title or main menu screen.
void drawScreen(Screen screen, const GameState&, const UiState&, const Textures&) {
    if (screen == TITLE) {
        DrawText("TITLE", 40, 40, 40, RAYWHITE);
    } else if (screen == MAIN_MENU) {
        DrawText("MAIN MENU", 40, 40, 40, RAYWHITE);
    }
}

bool drawButton(const char*, int, int, int, int) { return false; }

void drawTextInput(const std::string&, int, int, int, bool, const std::string&) {}
