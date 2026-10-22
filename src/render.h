#ifndef RENDER_H
#define RENDER_H

#include "raylib.h"
#include "types.h"

const int TILE_SOURCE_SIZE = 32;
const int TILE_SCALE = 2;
const int TILE_SIZE = TILE_SOURCE_SIZE * TILE_SCALE;

const int FLOOR_VARIANTS = 10;
const int DECAL_VARIANTS = 8;
const int WALL_FRONT_VARIANTS = 8;
const int LIQUID_FRAMES = 3;
const int DOOR_FRAMES = 4;
const int EXIT_FRAMES = 4;
const int HERO_FRAMES = 4;

// Every sprite of one hero. Left-facing is the side sprite mirrored in code.
struct HeroTextures {
    Texture2D walkDown[HERO_FRAMES];
    Texture2D walkUp[HERO_FRAMES];
    Texture2D walkSide[HERO_FRAMES];
    Texture2D idle[HERO_FRAMES];
    Texture2D stuck[HERO_FRAMES];
    Texture2D win[HERO_FRAMES];
    Texture2D portrait;
};

// Every texture the drawing code needs for one level. The liquid is water on most
// levels and ice on Shipka; levels without it leave those textures empty.
struct Textures {
    Texture2D floors[FLOOR_VARIANTS];
    Texture2D decals[DECAL_VARIANTS];
    Texture2D wallFront[WALL_FRONT_VARIANTS];
    Texture2D wallHorizontal;
    Texture2D wallVertical;
    Texture2D wallCornerTopLeft;
    Texture2D wallCornerTopRight;
    Texture2D wallCornerBottomLeft;
    Texture2D wallCornerBottomRight;
    Texture2D wallCross;
    Texture2D wallTJunction;
    Texture2D wallTorch;
    Texture2D liquid[LIQUID_FRAMES];
    Texture2D liquidEdge;
    Texture2D doorHorizontalClosed;
    Texture2D doorVerticalClosed;
    Texture2D doorHorizontalOpen[DOOR_FRAMES];
    Texture2D doorVerticalOpen[DOOR_FRAMES];
    Texture2D exitTile;
    Texture2D exitPulse[EXIT_FRAMES];
    Texture2D startPad;
    Texture2D trapOpen;
    Texture2D oil;
    Texture2D mapItem;
    Texture2D key;
    Texture2D trapItem;
    HeroTextures heroes[CHARACTER_COUNT];
};

// Loads every texture the game draws for this level: its tileset and props, the
// items and all six heroes. Every texture uses nearest-neighbour filtering so the
// pixels stay sharp. Call it once after the window is open. A file that cannot be
// loaded leaves an empty texture (id 0) and raylib logs a warning.
Textures loadTextures(const Level& level);

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

#endif
