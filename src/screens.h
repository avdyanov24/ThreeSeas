#ifndef SCREENS_H
#define SCREENS_H

#include "raylib.h"
#include "render.h"
#include "types.h"

// Reads input and runs the logic of the current screen. Returns the screen to show
// next (the same one if nothing changed). Never draws.
Screen updateScreen(Screen screen, GameState& game, UiState& ui, float deltaTime);

// Draws the current screen. Never reads input or changes the game.
void drawScreen(Screen screen, const GameState& game, const UiState& ui,
                const Textures& textures);

// Draws a button in the shared style and returns true if it was clicked.
bool drawButton(const char* label, int x, int y, int width, int height);

// Draws a text box with the typed text, and the error message in red under it
// when errorMessage is not empty.
void drawTextInput(const std::string& text, int x, int y, int width, bool hasFocus,
                   const std::string& errorMessage);

#endif  // SCREENS_H
