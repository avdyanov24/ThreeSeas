#include "render.h"

#include <cmath>
#include <cstdlib>
#include <string>

#include "game.h"
#include "maze.h"

#ifndef ART_DIR
#define ART_DIR "art"
#endif

struct LevelFolder {
    const char* folder;
    const char* prefix;
    const char* liquid;
};

const LevelFolder LEVEL_FOLDERS[LEVEL_COUNT] = {
    {"01_pliska", "pliska", nullptr},
    {"02_varbitsa_pass", "varbitsa", nullptr},
    {"03_preslav", "preslav", nullptr},
    {"04_tarnovo", "tarnovo", nullptr},
    {"05_cherven", "cherven", "water"},
    {"06_klokotnitsa", "klokotnitsa", "water"},
    {"07_koprivshtitsa", "koprivshtitsa", nullptr},
    {"08_plovdiv", "plovdiv", "water"},
    {"09_shipka", "shipka", "ice"},
};

const char* const PLISKA_DECALS[DECAL_VARIANTS] = {"ash",    "bones",  "grass", "puddle",
                                                   "rubble", "shards", "spear", "wheel"};

const char* const HERO_FOLDERS[CHARACTER_COUNT] = {"asparuh", "krum",      "boris",
                                                   "simeon",  "ivan_asen", "levski"};

// Returns a number as two digits, for example 3 becomes "03"
static std::string twoDigits(int number) {
    std::string digits = std::to_string(number);
    if (digits.length() < 2) {
        digits = "0" + digits;
    }
    return digits;
}

// Loads one PNG with nearest-neighbour filtering. A missing file gives an empty texture.
static Texture2D loadPixelTexture(const std::string& path) {
    Texture2D texture = LoadTexture(path.c_str());
    if (texture.id != 0) {
        SetTextureFilter(texture, TEXTURE_FILTER_POINT);
    }
    return texture;
}

// Loads the numbered files base_01.png, base_02.png... into the array
static void loadNumbered(Texture2D textures[], int count, const std::string& base) {
    for (int i = 0; i < count; i++) {
        textures[i] = loadPixelTexture(base + "_" + twoDigits(i + 1) + ".png");
    }
}

// Loads every sprite and the portrait of one hero
static void loadHero(HeroTextures& hero, const std::string& name) {
    std::string folder = std::string(ART_DIR) + "/05_characters/" + name;
    std::string sprites = folder + "/sprites/" + name;
    loadNumbered(hero.walkDown, HERO_FRAMES, sprites + "_walk_down");
    loadNumbered(hero.walkUp, HERO_FRAMES, sprites + "_walk_up");
    loadNumbered(hero.walkSide, HERO_FRAMES, sprites + "_walk_side");
    loadNumbered(hero.idle, HERO_FRAMES, sprites + "_idle");
    loadNumbered(hero.stuck, HERO_FRAMES, sprites + "_stuck");
    loadNumbered(hero.win, HERO_FRAMES, sprites + "_win");
    hero.portrait = loadPixelTexture(folder + "/portrait/" + name + "_portrait.png");
}

// Frees one texture if it was loaded and leaves it empty
static void unloadOne(Texture2D& texture) {
    if (texture.id != 0) {
        UnloadTexture(texture);
    }
    texture = Texture2D{};
}

// Frees every texture of an array
static void unloadMany(Texture2D textures[], int count) {
    for (int i = 0; i < count; i++) {
        unloadOne(textures[i]);
    }
}

// Loads every texture the game draws for this level
Textures loadTextures(const Level& level) {
    Textures textures{};
    if (level.id < 1 || level.id > LEVEL_COUNT) {
        return textures;
    }
    const LevelFolder& entry = LEVEL_FOLDERS[level.id - 1];
    std::string art = ART_DIR;
    std::string tiles = art + "/03_levels/" + entry.folder + "/tiles/" + entry.prefix;
    std::string props = art + "/03_levels/" + entry.folder + "/props/" + entry.prefix;

    loadNumbered(textures.floors, FLOOR_VARIANTS, tiles + "_floor");
    if (level.id == 1) {
        for (int i = 0; i < DECAL_VARIANTS; i++) {
            textures.decals[i] = loadPixelTexture(tiles + "_decal_" + PLISKA_DECALS[i] + ".png");
        }
    } else {
        loadNumbered(textures.decals, DECAL_VARIANTS, tiles + "_decal");
    }
    loadNumbered(textures.wallFront, WALL_FRONT_VARIANTS, tiles + "_wall_front");
    textures.wallHorizontal = loadPixelTexture(tiles + "_wall_horizontal.png");
    textures.wallVertical = loadPixelTexture(tiles + "_wall_vertical.png");
    textures.wallCornerTopLeft = loadPixelTexture(tiles + "_wall_corner_tl.png");
    textures.wallCornerTopRight = loadPixelTexture(tiles + "_wall_corner_tr.png");
    textures.wallCornerBottomLeft = loadPixelTexture(tiles + "_wall_corner_bl.png");
    textures.wallCornerBottomRight = loadPixelTexture(tiles + "_wall_corner_br.png");
    textures.wallCross = loadPixelTexture(tiles + "_wall_cross.png");
    textures.wallTJunction = loadPixelTexture(tiles + "_wall_t_junction.png");
    textures.wallTorch = loadPixelTexture(tiles + "_wall_torch.png");
    if (entry.liquid != nullptr) {
        std::string liquid = tiles + "_" + entry.liquid;
        loadNumbered(textures.liquid, LIQUID_FRAMES, liquid);
        textures.liquidEdge = loadPixelTexture(liquid + "_edge.png");
    }

    textures.doorHorizontalClosed = loadPixelTexture(props + "_door_h_closed.png");
    textures.doorVerticalClosed = loadPixelTexture(props + "_door_v_closed.png");
    loadNumbered(textures.doorHorizontalOpen, DOOR_FRAMES, props + "_door_h_open");
    loadNumbered(textures.doorVerticalOpen, DOOR_FRAMES, props + "_door_v_open");
    textures.exitTile = loadPixelTexture(props + "_exit.png");
    loadNumbered(textures.exitPulse, EXIT_FRAMES, props + "_exit_pulse");
    textures.startPad = loadPixelTexture(props + "_start.png");
    textures.trapOpen = loadPixelTexture(props + "_trap_open.png");

    textures.oil = loadPixelTexture(art + "/04_items/item_oil.png");
    textures.mapItem = loadPixelTexture(art + "/04_items/item_map.png");
    textures.key = loadPixelTexture(art + "/04_items/item_key.png");
    textures.trapItem = loadPixelTexture(art + "/04_items/item_trap.png");

    Image glowImage = GenImageGradientRadial(128, 128, 0.0f, Color{255, 170, 70, 110},
                                             Color{0, 0, 0, 0});
    textures.glow = LoadTextureFromImage(glowImage);
    UnloadImage(glowImage);
    SetTextureFilter(textures.glow, TEXTURE_FILTER_BILINEAR);

    for (int i = 0; i < CHARACTER_COUNT; i++) {
        loadHero(textures.heroes[i], HERO_FOLDERS[i]);
    }
    return textures;
}

// Frees every texture
void unloadTextures(Textures& textures) {
    unloadMany(textures.floors, FLOOR_VARIANTS);
    unloadMany(textures.decals, DECAL_VARIANTS);
    unloadMany(textures.wallFront, WALL_FRONT_VARIANTS);
    unloadOne(textures.wallHorizontal);
    unloadOne(textures.wallVertical);
    unloadOne(textures.wallCornerTopLeft);
    unloadOne(textures.wallCornerTopRight);
    unloadOne(textures.wallCornerBottomLeft);
    unloadOne(textures.wallCornerBottomRight);
    unloadOne(textures.wallCross);
    unloadOne(textures.wallTJunction);
    unloadOne(textures.wallTorch);
    unloadMany(textures.liquid, LIQUID_FRAMES);
    unloadOne(textures.liquidEdge);
    unloadOne(textures.doorHorizontalClosed);
    unloadOne(textures.doorVerticalClosed);
    unloadMany(textures.doorHorizontalOpen, DOOR_FRAMES);
    unloadMany(textures.doorVerticalOpen, DOOR_FRAMES);
    unloadOne(textures.exitTile);
    unloadMany(textures.exitPulse, EXIT_FRAMES);
    unloadOne(textures.startPad);
    unloadOne(textures.trapOpen);
    unloadOne(textures.oil);
    unloadOne(textures.mapItem);
    unloadOne(textures.key);
    unloadOne(textures.trapItem);
    unloadOne(textures.glow);
    for (int i = 0; i < CHARACTER_COUNT; i++) {
        HeroTextures& hero = textures.heroes[i];
        unloadMany(hero.walkDown, HERO_FRAMES);
        unloadMany(hero.walkUp, HERO_FRAMES);
        unloadMany(hero.walkSide, HERO_FRAMES);
        unloadMany(hero.idle, HERO_FRAMES);
        unloadMany(hero.stuck, HERO_FRAMES);
        unloadMany(hero.win, HERO_FRAMES);
        unloadOne(hero.portrait);
    }
}

enum CellView { UNKNOWN_CELL, SEEN_CELL, LIT_CELL };

const int PLAIN_FLOORS[4] = {0, 1, 2, 8};
const int MEDIUM_FLOORS[2] = {3, 9};
const int RARE_FLOORS[4] = {4, 5, 6, 7};
const int PLAIN_FRONTS[3] = {0, 1, 2};
const int RARE_FRONTS[5] = {3, 4, 5, 6, 7};
const Color SEEN_TINT = {76, 80, 108, 255};

// Returns a repeatable pseudo-random number for a cell, so a cell always looks the same
static unsigned int hashCell(int row, int col, int salt) {
    unsigned int hash = static_cast<unsigned int>(row) * 73856093u;
    hash ^= static_cast<unsigned int>(col) * 19349663u;
    hash ^= static_cast<unsigned int>(salt) * 83492791u;
    hash ^= hash >> 13;
    hash *= 1274126177u;
    hash ^= hash >> 16;
    return hash;
}

// Returns one axis of the camera origin: the viewer centred, stopped at the maze border,
// or the whole maze centred when it is smaller than the window
static float getCameraAxis(float center, float worldSize, float screenSize) {
    if (worldSize <= screenSize) {
        return std::floor((worldSize - screenSize) / 2.0f);
    }
    float origin = center - screenSize / 2.0f;
    if (origin < 0.0f) {
        origin = 0.0f;
    }
    if (origin > worldSize - screenSize) {
        origin = worldSize - screenSize;
    }
    return std::floor(origin);
}

// Returns the world position shown in the top-left corner of the screen
Vector2 getCameraOrigin(const Maze& maze, const Player& viewer) {
    float worldSize = static_cast<float>((maze.size + 2) * TILE_SIZE);
    float centerX = (viewer.col + 1.5f) * TILE_SIZE;
    float centerY = (viewer.row + 1.5f) * TILE_SIZE;
    return Vector2{getCameraAxis(centerX, worldSize, static_cast<float>(GetScreenWidth())),
                   getCameraAxis(centerY, worldSize, static_cast<float>(GetScreenHeight()))};
}

// Draws a texture at TILE_SCALE with its top-left at (x, y), turned around its centre
static void drawTile(const Texture2D& texture, int x, int y, float rotation, Color tint) {
    Rectangle source = {0.0f, 0.0f, static_cast<float>(texture.width),
                        static_cast<float>(texture.height)};
    Rectangle destination = {x + TILE_SIZE / 2.0f, y + TILE_SIZE / 2.0f,
                             static_cast<float>(TILE_SIZE), static_cast<float>(TILE_SIZE)};
    Vector2 origin = {TILE_SIZE / 2.0f, TILE_SIZE / 2.0f};
    DrawTexturePro(texture, source, destination, origin, rotation, tint);
}

// True if the cell counts as a wall when choosing wall pieces: walls, doors, and the
// border ring around the maze. Everything beyond the ring counts as open.
static bool isWallForTiling(const Maze& maze, int row, int col) {
    if (row < -1 || row > maze.size || col < -1 || col > maze.size) {
        return false;
    }
    if (!isInside(maze, row, col)) {
        return true;
    }
    Tile tile = maze.cells[row][col].tile;
    return tile == WALL || tile == DOOR;
}

// Draws a wall cell. A wall with open floor to its south shows its front face; any
// other wall shows the top of the wall: a straight piece, a corner, a T or a cross.
static void drawWall(const Maze& maze, const Textures& textures, int row, int col, int x, int y,
                     Color tint) {
    bool north = isWallForTiling(maze, row - 1, col);
    bool east = isWallForTiling(maze, row, col + 1);
    bool south = isWallForTiling(maze, row + 1, col);
    bool west = isWallForTiling(maze, row, col - 1);
    if (!south) {
        unsigned int hash = hashCell(row, col, 3) % 100;
        int variant = PLAIN_FRONTS[(hash / 7) % 3];
        if (hash < 14) {
            variant = RARE_FRONTS[(hash / 3) % 5];
        }
        drawTile(textures.wallFront[variant], x, y, 0.0f, tint);
    } else if (north && east && west) {
        drawTile(textures.wallCross, x, y, 0.0f, tint);
    } else if (east && west) {
        drawTile(textures.wallTJunction, x, y, 0.0f, tint);
    } else if (north && east) {
        drawTile(textures.wallTJunction, x, y, -90.0f, tint);
    } else if (north && west) {
        drawTile(textures.wallTJunction, x, y, 90.0f, tint);
    } else if (east) {
        drawTile(textures.wallCornerTopLeft, x, y, 0.0f, tint);
    } else if (west) {
        drawTile(textures.wallCornerTopRight, x, y, 0.0f, tint);
    } else {
        drawTile(textures.wallVertical, x, y, 0.0f, tint);
    }
}

// Draws a floor cell: mostly plain stone or earth, now and then a decoration
static void drawFloor(const Textures& textures, int row, int col, int x, int y, Color tint) {
    unsigned int hash = hashCell(row, col, 1) % 100;
    unsigned int pick = hashCell(row, col, 2);
    if (hash < 6) {
        drawTile(textures.decals[pick % DECAL_VARIANTS], x, y, 0.0f, tint);
    } else if (hash < 70) {
        drawTile(textures.floors[PLAIN_FLOORS[pick % 4]], x, y, 0.0f, tint);
    } else if (hash < 92) {
        drawTile(textures.floors[MEDIUM_FLOORS[pick % 2]], x, y, 0.0f, tint);
    } else {
        drawTile(textures.floors[RARE_FLOORS[pick % 4]], x, y, 0.0f, tint);
    }
}

// Draws a door: horizontal in a wall that runs east to west, vertical otherwise
static void drawDoor(const Maze& maze, const Textures& textures, int row, int col, int x, int y,
                     Color tint) {
    bool runsEastWest = isWallForTiling(maze, row, col - 1) && isWallForTiling(maze, row, col + 1);
    if (runsEastWest) {
        drawTile(textures.doorHorizontalClosed, x, y, 0.0f, tint);
    } else {
        drawTile(textures.doorVerticalClosed, x, y, 0.0f, tint);
    }
}

// Draws the ground of one cell: floor, water, door, exit or wall
static void drawCellBase(const Maze& maze, const Textures& textures, int row, int col, int x,
                         int y, Color tint) {
    bool isBorder = !isInside(maze, row, col);
    Tile tile = isBorder ? WALL : maze.cells[row][col].tile;
    if (tile == WALL) {
        drawWall(maze, textures, row, col, x, y, tint);
    } else if (tile == DOOR) {
        drawDoor(maze, textures, row, col, x, y, tint);
    } else if (tile == WATER && textures.liquid[0].id != 0) {
        int frame = static_cast<int>(GetTime() * 3.0) % LIQUID_FRAMES;
        drawTile(textures.liquid[frame], x, y, 0.0f, tint);
    } else if (tile == EXIT) {
        int frame = static_cast<int>(GetTime() * 6.0) % EXIT_FRAMES;
        drawTile(textures.exitPulse[frame], x, y, 0.0f, tint);
    } else if (row == 0 && col == 0) {
        drawTile(textures.startPad, x, y, 0.0f, tint);
    } else {
        drawFloor(textures, row, col, x, y, tint);
    }
}

// Returns the index of the viewer in game.players, or -1 if it is not one of them
static int getPlayerIndex(const GameState& game, const Player& viewer) {
    for (int i = 0; i < game.playerCount; i++) {
        if (&game.players[i] == &viewer) {
            return i;
        }
    }
    return -1;
}

// True if the viewer can see the trap on this cell: Krum sees every trap, anyone sees
// a trap right next to them, and a player always sees their own traps
static bool canSeeTrap(const GameState& game, const Player& viewer, int row, int col) {
    int owner = game.maze.cells[row][col].trapOwner;
    if (owner == NO_TRAP) {
        return false;
    }
    bool isNextToViewer = std::abs(row - viewer.row) <= 1 && std::abs(col - viewer.col) <= 1;
    return viewer.character == KRUM || isNextToViewer || owner == getPlayerIndex(game, viewer);
}

// Draws the item and the trap lying on a lit cell
static void drawCellObjects(const GameState& game, const Player& viewer, const Textures& textures,
                            int row, int col, int x, int y, Color tint) {
    const Cell& cell = game.maze.cells[row][col];
    if (cell.item == OIL) {
        drawTile(textures.oil, x, y, 0.0f, tint);
    } else if (cell.item == MAP_ITEM) {
        drawTile(textures.mapItem, x, y, 0.0f, tint);
    } else if (cell.item == KEY) {
        drawTile(textures.key, x, y, 0.0f, tint);
    } else if (cell.item == TRAP_ITEM) {
        drawTile(textures.trapItem, x, y, 0.0f, tint);
    }
    if (canSeeTrap(game, viewer, row, col)) {
        drawTile(textures.trapOpen, x, y, 0.0f, tint);
    }
}

// Returns how the viewer sees a cell: lit by the torch, remembered, or never seen.
// Border cells take the memory of the nearest cell inside the maze.
static CellView getCellView(const Maze& maze, const Player& viewer, int row, int col) {
    if (isLit(viewer, row, col)) {
        return LIT_CELL;
    }
    if (viewer.seen == nullptr) {
        return UNKNOWN_CELL;
    }
    int innerRow = row < 0 ? 0 : (row >= maze.size ? maze.size - 1 : row);
    int innerCol = col < 0 ? 0 : (col >= maze.size ? maze.size - 1 : col);
    return viewer.seen[innerRow][innerCol] ? SEEN_CELL : UNKNOWN_CELL;
}

// Returns the warm colour of a lit cell: full brightness at the viewer, softly
// darker towards the edge of the torchlight
static Color getLitTint(const Player& viewer, int row, int col) {
    float rowDistance = static_cast<float>(row - viewer.row);
    float colDistance = static_cast<float>(col - viewer.col);
    float distance = std::sqrt(rowDistance * rowDistance + colDistance * colDistance);
    float ratio = distance / (getTorchRadius(viewer) + 0.5f);
    float brightness = 1.0f - 0.62f * ratio * ratio;
    return Color{static_cast<unsigned char>(255.0f * brightness),
                 static_cast<unsigned char>(232.0f * brightness),
                 static_cast<unsigned char>(200.0f * brightness), 255};
}

// Draws the maze as the viewer sees it
void drawMaze(const GameState& game, const Player& viewer, const Textures& textures) {
    const Maze& maze = game.maze;
    if (maze.cells == nullptr) {
        return;
    }
    Vector2 camera = getCameraOrigin(maze, viewer);
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    for (int row = -1; row <= maze.size; row++) {
        for (int col = -1; col <= maze.size; col++) {
            int x = (col + 1) * TILE_SIZE - static_cast<int>(camera.x);
            int y = (row + 1) * TILE_SIZE - static_cast<int>(camera.y);
            if (x <= -TILE_SIZE || x >= screenWidth || y <= -TILE_SIZE || y >= screenHeight) {
                continue;
            }
            CellView view = getCellView(maze, viewer, row, col);
            if (view == UNKNOWN_CELL) {
                continue;
            }
            Color tint = view == LIT_CELL ? getLitTint(viewer, row, col) : SEEN_TINT;
            drawCellBase(maze, textures, row, col, x, y, tint);
            if (view == LIT_CELL && isInside(maze, row, col)) {
                drawCellObjects(game, viewer, textures, row, col, x, y, tint);
            }
        }
    }
}

// Draws the glow of the torch around the viewer
void drawTorchLight(const GameState& game, const Player& viewer, const Textures& textures) {
    if (viewer.isOut) {
        return;
    }
    Vector2 camera = getCameraOrigin(game.maze, viewer);
    float centerX = (viewer.col + 1.5f) * TILE_SIZE - camera.x;
    float centerY = (viewer.row + 1.5f) * TILE_SIZE - camera.y;
    float fuelRatio = viewer.maxFuel > 0.0f ? viewer.fuel / viewer.maxFuel : 0.0f;
    float strength = fuelRatio < 0.3f ? 0.10f : 0.03f;
    float time = static_cast<float>(GetTime());
    float wobble = std::sin(time * 11.0f) + 0.6f * std::sin(time * 27.3f);
    float flicker = 1.0f + strength * wobble;
    float radius = (getTorchRadius(viewer) + 1.5f) * TILE_SIZE * flicker;
    Rectangle source = {0.0f, 0.0f, static_cast<float>(textures.glow.width),
                        static_cast<float>(textures.glow.height)};
    Rectangle destination = {centerX - radius, centerY - radius, radius * 2.0f, radius * 2.0f};
    BeginBlendMode(BLEND_ADDITIVE);
    DrawTexturePro(textures.glow, source, destination, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
    EndBlendMode();
}

void drawPlayer(const Player&, const Textures&) {}

void drawHud(const GameState&, const Player&, const Textures&) {}
