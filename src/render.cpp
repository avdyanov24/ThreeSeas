#include "render.h"

Textures loadTextures() { return Textures{}; }

void unloadTextures(Textures&) {}

void drawMaze(const GameState&, const Player&, const Textures&) {}

void drawTorchLight(const Player&, float) {}

void drawPlayer(const Player&, const Textures&) {}

void drawHud(const GameState&, const Player&, const Textures&) {}
