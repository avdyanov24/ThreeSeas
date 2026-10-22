#ifndef RENDER_H
#define RENDER_H

#include "raylib.h"
#include "types.h"

const int TILE_SOURCE_SIZE = 32;  // tiles in art/ are 32x32
const int TILE_SCALE = 2;         // drawn at 2x, so 64 px on screen

// Every texture the drawing code needs
struct Textures {
    Texture2D floor;
    Texture2D wall;
    Texture2D door;
    Texture2D exit;
    Texture2D water;
    Texture2D trap;
    Texture2D oil;
    Texture2D mapItem;
    Texture2D key;
    Texture2D heroes[CHARACTER_COUNT];
    Texture2D portraits[CHARACTER_COUNT];
};

// Loads every texture the game draws. Call it once after the window is open.
Textures loadTextures();

// Frees every texture. Call it before the window closes.
void unloadTextures(Textures& textures);

// Draws the maze as the viewer sees it. The camera follows the viewer.
// Lit cells are drawn in full colour, seen cells dim (items hidden), unknown cells
// black. Tiles are drawn at TILE_SCALE with nearest-neighbour scaling.
void drawMaze(const GameState& game, const Player& viewer, const Textures& textures);

// Draws the dark layer with the soft, flickering circle of light around the viewer.
// The flicker gets stronger below 30% fuel.
void drawTorchLight(const Player& viewer, float timeSeconds);

// Draws the player's sprite on their cell.
void drawPlayer(const Player& player, const Textures& textures);

// Draws the torch bar, the timer (campaign) or the name, turn and steps left
// (multiplayer), the inventory, the place and year, and the hero portrait.
void drawHud(const GameState& game, const Player& viewer, const Textures& textures);

#endif  // RENDER_H
