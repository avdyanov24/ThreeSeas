#include "screens.h"

Screen updateScreen(Screen, GameState&, UiState&, float) { return TITLE; }

void drawScreen(Screen, const GameState&, const UiState&, const Textures&) {}

bool drawButton(const char*, int, int, int, int) { return false; }

void drawTextInput(const std::string&, int, int, int, bool, const std::string&) {}
