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

const int FLOOR_WEIGHTS[LEVEL_COUNT][FLOOR_VARIANTS] = {
    {22, 22, 22, 8, 4, 4, 8, 10},  {20, 14, 14, 6, 10, 10, 8, 18}, {32, 22, 3, 16, 6, 2, 3, 16},
    {22, 22, 22, 10, 8, 8, 2, 6},  {20, 20, 20, 10, 8, 8, 10, 4}, {24, 24, 20, 10, 4, 6, 6, 6},
    {20, 20, 20, 20, 10, 4, 6, 0}, {20, 20, 20, 12, 4, 8, 10, 6}, {24, 24, 20, 10, 8, 6, 4, 4},
};

const int GLOW_DECOR_MASKS[LEVEL_COUNT] = {1, 0, 32, 8, 4, 4, 8, 136, 36};

const Color FLOOR_DARKEN = {192, 192, 192, 255};
const unsigned int DECAL_CHANCE = 7;
const int WALL_TOP_ROWS = 12;
const int WALL_FACE_START = 6;
const int WALL_FACE_ROWS = 20;
const int WALL_FACE_WEIGHTS[WALL_FACE_VARIANTS] = {40, 40, 10, 10};

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

// Reads the darkest and the lightest colour of a wall top picture, used for wall outlines
static void loadEdgeColors(Textures& textures, const std::string& path) {
    textures.edgeDark = Color{20, 14, 12, 255};
    textures.edgeLight = Color{230, 220, 200, 255};
    Image image = LoadImage(path.c_str());
    if (image.data == nullptr) {
        return;
    }
    int darkest = 1000;
    int lightest = -1;
    for (int y = 0; y < image.height; y++) {
        for (int x = 0; x < image.width; x++) {
            Color pixel = GetImageColor(image, x, y);
            int brightness = pixel.r + pixel.g + pixel.b;
            if (brightness < darkest) {
                darkest = brightness;
                textures.edgeDark = pixel;
            }
            if (brightness > lightest) {
                lightest = brightness;
                textures.edgeLight = pixel;
            }
        }
    }
    UnloadImage(image);
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

    loadNumbered(textures.floors, FLOOR_VARIANTS, tiles + "_kitfloor");
    for (int i = 0; i < FLOOR_VARIANTS; i++) {
        textures.floorWeights[i] = FLOOR_WEIGHTS[level.id - 1][i];
    }
    loadNumbered(textures.wallTops, WALL_TOP_VARIANTS, tiles + "_walltop");
    loadNumbered(textures.wallFaces, WALL_FACE_VARIANTS, tiles + "_wallface");
    loadNumbered(textures.wallDecor, WALL_DECOR_VARIANTS, tiles + "_walldecor");
    textures.glowDecorMask = GLOW_DECOR_MASKS[level.id - 1];
    loadEdgeColors(textures, tiles + "_walltop_01.png");
    if (entry.liquid != nullptr) {
        std::string liquid = tiles + "_" + entry.liquid;
        loadNumbered(textures.liquid, LIQUID_FRAMES, liquid);
        textures.liquidEdge = loadPixelTexture(liquid + "_edge.png");
    }

    std::string kit = tiles + "_prop";
    loadNumbered(textures.doorFaces, DOOR_FRAMES, kit + "_doorface");
    loadNumbered(textures.doorTops, DOOR_FRAMES, kit + "_doortop");
    loadNumbered(textures.decals, DECAL_VARIANTS, kit + "_decal");
    textures.exitTile = loadPixelTexture(props + "_exit.png");
    loadNumbered(textures.exitPulse, EXIT_FRAMES, props + "_exit_pulse");
    textures.startPad = loadPixelTexture(kit + "_start.png");
    textures.trapOpen = loadPixelTexture(kit + "_trap.png");

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

// Loads the title and main-menu art with nearest-neighbour filtering.
void loadUiTextures(Textures& textures) {
    std::string art = ART_DIR;
    textures.titleScreen =
        loadPixelTexture(art + "/07_screens/title/title_screen.png");
    textures.menuBackground = loadPixelTexture(art + "/01_ui/backgrounds/menu_background.png");
    textures.menuLogo = loadPixelTexture(art + "/01_ui/logo/three_seas_logo.png");
    textures.buttonNormal =
        loadPixelTexture(art + "/01_ui/buttons/ui_button_normal.png");
    textures.buttonHover =
        loadPixelTexture(art + "/01_ui/buttons/ui_button_hover.png");

    const char* const eraMapFiles[ERA_MAP_COUNT] = {
        "era1_first_state/map_era1_first_state", "era2_second_state/map_era2_second_state",
        "era3_road_to_liberation/map_era3_road_to_liberation"};
    for (int i = 0; i < ERA_MAP_COUNT; i++) {
        textures.eraMaps[i] = loadPixelTexture(art + "/02_era_maps/" + eraMapFiles[i] + ".png");
    }
    loadNumbered(textures.pinOpen, PIN_FRAMES, art + "/02_era_maps/pins/pin_open");
    loadNumbered(textures.pinLocked, PIN_FRAMES, art + "/02_era_maps/pins/pin_locked");
    loadNumbered(textures.pinFinished, PIN_FRAMES, art + "/02_era_maps/pins/pin_finished");
    textures.passScreen = loadPixelTexture(art + "/07_screens/pass_screen/pass_screen.png");
    textures.multiplayerResult =
        loadPixelTexture(art + "/07_screens/multiplayer_result/multiplayer_result.png");
    textures.resultWin = loadPixelTexture(art + "/07_screens/result_win/result_win.png");
    textures.resultLose = loadPixelTexture(art + "/07_screens/result_lose/result_lose.png");
    textures.starFilled = loadPixelTexture(art + "/01_ui/icons/icon_star_filled.png");
    textures.starEmpty = loadPixelTexture(art + "/01_ui/icons/icon_star_empty.png");

    std::string hud = art + "/01_ui/hud/";
    textures.hudFlame = loadPixelTexture(hud + "hud_flame.png");
    textures.hudTimer = loadPixelTexture(hud + "hud_timer.png");
    textures.hudSlot = loadPixelTexture(hud + "hud_slot.png");
    textures.hudMap = loadPixelTexture(hud + "hud_map.png");
    textures.hudKey = loadPixelTexture(hud + "hud_key.png");
    textures.hudTrap = loadPixelTexture(hud + "hud_trap.png");
    textures.hudPortraitFrame = loadPixelTexture(hud + "hud_portrait_frame.png");
    textures.hudTorchbarFull = loadPixelTexture(hud + "hud_torchbar_full.png");
    textures.hudTorchbarEmpty = loadPixelTexture(hud + "hud_torchbar_empty.png");
    textures.hudTorchbarLow = loadPixelTexture(hud + "hud_torchbar_low.png");
    textures.iconCrown = loadPixelTexture(art + "/01_ui/icons/icon_crown.png");

    const char* const introFiles[LEVEL_COUNT] = {
        "01_pliska/intro_background/pliska_intro",
        "02_varbitsa_pass/intro_background/varbitsa_intro",
        "03_preslav/intro_background/preslav_intro",
        "04_tarnovo/intro_background/tarnovo_intro",
        "05_cherven/intro_background/cherven_intro",
        "06_klokotnitsa/intro_background/klokotnitsa_intro",
        "07_koprivshtitsa/intro_background/koprivshtitsa_intro",
        "08_plovdiv/intro_background/plovdiv_intro",
        "09_shipka/intro_background/shipka_intro"};
    for (int i = 0; i < LEVEL_COUNT; i++) {
        textures.introBackgrounds[i] =
            loadPixelTexture(art + "/03_levels/" + introFiles[i] + ".png");
    }

    const char* const howToFiles[HOWTO_ICON_COUNT] = {
        "torch",   "torch_bar", "oil",         "map",   "door",  "water",
        "trap",    "stars",     "exit",        "clock", "key_one", "key_two",
        "key_enter", "pass_laptop", "cold",    "shield", "trophy"};
    for (int i = 0; i < HOWTO_ICON_COUNT; i++) {
        textures.howTo[i] =
            loadPixelTexture(art + "/07_screens/how_to_play/howto_" + howToFiles[i] + ".png");
    }
}

// Frees every texture
void unloadTextures(Textures& textures) {
    unloadOne(textures.titleScreen);
    unloadOne(textures.menuBackground);
    unloadOne(textures.menuLogo);
    unloadOne(textures.buttonNormal);
    unloadOne(textures.buttonHover);
    unloadMany(textures.eraMaps, ERA_MAP_COUNT);
    unloadOne(textures.passScreen);
    unloadOne(textures.multiplayerResult);
    unloadOne(textures.resultWin);
    unloadOne(textures.resultLose);
    unloadMany(textures.pinOpen, PIN_FRAMES);
    unloadMany(textures.pinLocked, PIN_FRAMES);
    unloadMany(textures.pinFinished, PIN_FRAMES);
    unloadOne(textures.starFilled);
    unloadOne(textures.starEmpty);
    unloadMany(textures.howTo, HOWTO_ICON_COUNT);
    unloadOne(textures.hudFlame);
    unloadOne(textures.hudTimer);
    unloadOne(textures.hudSlot);
    unloadOne(textures.hudMap);
    unloadOne(textures.hudKey);
    unloadOne(textures.hudTrap);
    unloadOne(textures.hudPortraitFrame);
    unloadOne(textures.hudTorchbarFull);
    unloadOne(textures.hudTorchbarEmpty);
    unloadOne(textures.hudTorchbarLow);
    unloadOne(textures.iconCrown);
    unloadMany(textures.introBackgrounds, LEVEL_COUNT);
    unloadMany(textures.floors, FLOOR_VARIANTS);
    unloadMany(textures.wallTops, WALL_TOP_VARIANTS);
    unloadMany(textures.wallFaces, WALL_FACE_VARIANTS);
    unloadMany(textures.wallDecor, WALL_DECOR_VARIANTS);
    unloadMany(textures.liquid, LIQUID_FRAMES);
    unloadOne(textures.liquidEdge);
    unloadMany(textures.doorFaces, DOOR_FRAMES);
    unloadMany(textures.doorTops, DOOR_FRAMES);
    unloadMany(textures.decals, DECAL_VARIANTS);
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
    float centerX = (viewer.visualCol + 1.5f) * TILE_SIZE;
    float centerY = (viewer.visualRow + 1.5f) * TILE_SIZE;
    return Vector2{getCameraAxis(centerX, worldSize, static_cast<float>(GetScreenWidth())),
                   getCameraAxis(centerY, worldSize,
                                 static_cast<float>(GetScreenHeight() - HUD_HEIGHT)) - HUD_HEIGHT};
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

// Multiplies a colour by the light tint of a cell
static Color shade(Color color, Color tint) {
    return Color{static_cast<unsigned char>(color.r * tint.r / 255),
                 static_cast<unsigned char>(color.g * tint.g / 255),
                 static_cast<unsigned char>(color.b * tint.b / 255), color.a};
}

// Returns an index from weights that add up to 100, for a roll from 0 to 99
static int pickWeighted(const int weights[], int count, unsigned int roll) {
    int total = 0;
    for (int i = 0; i < count; i++) {
        total += weights[i];
        if (static_cast<int>(roll) < total) {
            return i;
        }
    }
    return 0;
}

// Draws the lowest rowCount rows of a texture from firstRow, scaled up, with its
// top-left corner at (x, y)
static void drawTileRows(const Texture2D& texture, int firstRow, int rowCount, int x, int y,
                         Color tint) {
    Rectangle source = {0.0f, static_cast<float>(firstRow), static_cast<float>(texture.width),
                        static_cast<float>(rowCount)};
    Rectangle destination = {static_cast<float>(x), static_cast<float>(y),
                             static_cast<float>(TILE_SIZE),
                             static_cast<float>(rowCount * TILE_SCALE)};
    DrawTexturePro(texture, source, destination, Vector2{0.0f, 0.0f}, 0.0f, tint);
}

// Returns the decoration (0 to 7) on a wall front, or -1 for a plain wall. Only walls with
// wall on both sides carry one, so a decoration always sits in the middle of a wall.
static int getWallDecor(const Maze& maze, int row, int col) {
    bool isMiddle = isWallForTiling(maze, row, col - 1) && isWallForTiling(maze, row, col + 1);
    if (!isMiddle || hashCell(row, col, 5) % 100 >= 14) {
        return -1;
    }
    return static_cast<int>(hashCell(row, col, 6) % WALL_DECOR_VARIANTS);
}

// Draws a dark outline with a light line inside it along every open side of a wall
static void drawWallEdges(const Textures& textures, int x, int y, bool north, bool east,
                          bool west, Color tint) {
    Color dark = shade(textures.edgeDark, tint);
    Color light = shade(textures.edgeLight, tint);
    light.a = 110;
    if (north) {
        DrawRectangle(x, y, TILE_SIZE, 2, dark);
        DrawRectangle(x, y + 2, TILE_SIZE, 2, light);
    }
    if (east) {
        DrawRectangle(x + TILE_SIZE - 2, y, 2, TILE_SIZE, dark);
        DrawRectangle(x + TILE_SIZE - 4, y, 2, TILE_SIZE, light);
    }
    if (west) {
        DrawRectangle(x, y, 2, TILE_SIZE, dark);
        DrawRectangle(x + 2, y, 2, TILE_SIZE, light);
    }
}

// Draws a wall front: a strip of top surface above a front face, a shadow under the lip,
// and outlines along the open sides
static void drawWallFront(const Textures& textures, const Texture2D& top, const Texture2D& face,
                          int x, int y, bool north, bool east, bool west, Color tint) {
    int faceY = y + WALL_TOP_ROWS * TILE_SCALE;
    drawTileRows(top, 0, WALL_TOP_ROWS, x, y, tint);
    drawTileRows(face, WALL_FACE_START, WALL_FACE_ROWS, x, faceY, tint);
    Color dark = shade(textures.edgeDark, tint);
    DrawRectangle(x, faceY - 2, TILE_SIZE, 2, dark);
    DrawRectangleGradientV(x, faceY, TILE_SIZE, 10, Color{0, 0, 0, 110}, Color{0, 0, 0, 0});
    DrawRectangle(x, y + TILE_SIZE - 2, TILE_SIZE, 2, dark);
    drawWallEdges(textures, x, y, north, east, west, tint);
}

// Draws a wall cell. A wall with a wall to its south shows only its flat top. A wall with
// open floor to its south shows a strip of top above its front face, with a shadow under
// the lip, and sometimes a decoration on the face.
static void drawWall(const Maze& maze, const Textures& textures, int row, int col, int x, int y,
                     Color tint) {
    bool north = isWallForTiling(maze, row - 1, col);
    bool east = isWallForTiling(maze, row, col + 1);
    bool south = isWallForTiling(maze, row + 1, col);
    bool west = isWallForTiling(maze, row, col - 1);
    const Texture2D& top = textures.wallTops[hashCell(row, col, 4) % WALL_TOP_VARIANTS];
    if (south) {
        drawTile(top, x, y, 0.0f, tint);
        drawWallEdges(textures, x, y, !north, !east, !west, tint);
        return;
    }
    int decor = getWallDecor(maze, row, col);
    unsigned int faceRoll = hashCell(row, col, 7) % 100;
    int faceIndex = pickWeighted(WALL_FACE_WEIGHTS, WALL_FACE_VARIANTS, faceRoll);
    const Texture2D& face = decor >= 0 ? textures.wallDecor[decor] : textures.wallFaces[faceIndex];
    drawWallFront(textures, top, face, x, y, !north, !east, !west, tint);
}

// Lets a lit torch or lantern on a wall front glow
static void drawWallGlow(const Maze& maze, const Textures& textures, int row, int col, int x,
                         int y) {
    if (isWallForTiling(maze, row + 1, col)) {
        return;
    }
    int decor = getWallDecor(maze, row, col);
    if (decor < 0 || ((textures.glowDecorMask >> decor) & 1) == 0) {
        return;
    }
    float phase = static_cast<float>(hashCell(row, col, 8) % 100);
    float flicker = 1.0f + 0.06f * std::sin(static_cast<float>(GetTime()) * 9.0f + phase);
    float radius = 1.7f * TILE_SIZE * flicker;
    Rectangle source = {0.0f, 0.0f, static_cast<float>(textures.glow.width),
                        static_cast<float>(textures.glow.height)};
    float centerX = x + TILE_SIZE / 2.0f;
    float centerY = y + TILE_SIZE * 0.55f;
    Rectangle destination = {centerX - radius, centerY - radius, radius * 2.0f, radius * 2.0f};
    BeginBlendMode(BLEND_ADDITIVE);
    DrawTexturePro(textures.glow, source, destination, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
    EndBlendMode();
}

// Draws a floor cell: a weighted pick of this level's floors, with a soft shadow along
// every wall next to it
static void drawFloor(const Maze& maze, const Textures& textures, int row, int col, int x,
                      int y, Color tint) {
    int variant = pickWeighted(textures.floorWeights, FLOOR_VARIANTS, hashCell(row, col, 1) % 100);
    drawTile(textures.floors[variant], x, y, 0.0f, shade(tint, FLOOR_DARKEN));
    if (hashCell(row, col, 9) % 100 < DECAL_CHANCE) {
        drawTile(textures.decals[hashCell(row, col, 10) % DECAL_VARIANTS], x, y, 0.0f, tint);
    }
    if (isWallForTiling(maze, row - 1, col)) {
        DrawRectangleGradientV(x, y, TILE_SIZE, 18, Color{0, 0, 0, 120}, Color{0, 0, 0, 0});
    }
    if (isWallForTiling(maze, row, col - 1)) {
        DrawRectangleGradientH(x, y, 10, TILE_SIZE, Color{0, 0, 0, 70}, Color{0, 0, 0, 0});
    }
    if (isWallForTiling(maze, row, col + 1)) {
        DrawRectangleGradientH(x + TILE_SIZE - 10, y, 10, TILE_SIZE, Color{0, 0, 0, 0},
                               Color{0, 0, 0, 70});
    }
}

// Draws a door: set in a front face in a wall that runs east to west, or seen from above in
// a wall that runs north to south. isOpen draws the open frame, for a viewer who can pass.
static void drawDoor(const Maze& maze, const Textures& textures, int row, int col, int x, int y,
                     Color tint, bool isOpen) {
    int frame = isOpen ? DOOR_FRAMES - 1 : 0;
    bool runsEastWest = isWallForTiling(maze, row, col - 1) && isWallForTiling(maze, row, col + 1);
    if (runsEastWest) {
        const Texture2D& top = textures.wallTops[hashCell(row, col, 4) % WALL_TOP_VARIANTS];
        drawWallFront(textures, top, textures.doorFaces[frame], x, y, false, false, false, tint);
    } else {
        drawTile(textures.doorTops[frame], x, y, 0.0f, tint);
        drawWallEdges(textures, x, y, false, true, true, tint);
    }
}

// True if the cell is inside the maze and is water
static bool isWaterAt(const Maze& maze, int row, int col) {
    return isInside(maze, row, col) && maze.cells[row][col].tile == WATER;
}

// Draws a dark line with a light foam line inside it along every side of a water cell
// that does not touch more water
static void drawShore(const Maze& maze, int row, int col, int x, int y, Color tint) {
    Color dark = shade(Color{8, 22, 34, 230}, tint);
    Color foam = shade(Color{200, 230, 240, 150}, tint);
    if (!isWaterAt(maze, row - 1, col)) {
        DrawRectangle(x, y, TILE_SIZE, 2, dark);
        DrawRectangle(x, y + 2, TILE_SIZE, 2, foam);
    }
    if (!isWaterAt(maze, row + 1, col)) {
        DrawRectangle(x, y + TILE_SIZE - 2, TILE_SIZE, 2, dark);
        DrawRectangle(x, y + TILE_SIZE - 4, TILE_SIZE, 2, foam);
    }
    if (!isWaterAt(maze, row, col - 1)) {
        DrawRectangle(x, y, 2, TILE_SIZE, dark);
        DrawRectangle(x + 2, y, 2, TILE_SIZE, foam);
    }
    if (!isWaterAt(maze, row, col + 1)) {
        DrawRectangle(x + TILE_SIZE - 2, y, 2, TILE_SIZE, dark);
        DrawRectangle(x + TILE_SIZE - 4, y, 2, TILE_SIZE, foam);
    }
}

// Draws the ground of one cell: floor, water, door, exit or wall
static void drawCellBase(const Maze& maze, const Textures& textures, int row, int col, int x,
                         int y, Color tint, bool canOpenDoors) {
    bool isBorder = !isInside(maze, row, col);
    Tile tile = isBorder ? WALL : maze.cells[row][col].tile;
    if (tile == WALL) {
        drawWall(maze, textures, row, col, x, y, tint);
    } else if (tile == DOOR) {
        drawDoor(maze, textures, row, col, x, y, tint, canOpenDoors);
    } else if (tile == WATER && textures.liquid[0].id != 0) {
        int frame = static_cast<int>(GetTime() * 3.0) % LIQUID_FRAMES;
        drawTile(textures.liquid[frame], x, y, 0.0f, tint);
        drawShore(maze, row, col, x, y, tint);
    } else if (tile == EXIT) {
        int frame = static_cast<int>(GetTime() * 6.0) % EXIT_FRAMES;
        drawTile(textures.exitPulse[frame], x, y, 0.0f, tint);
    } else if (row == 0 && col == 0) {
        drawFloor(maze, textures, row, col, x, y, tint);
        drawTile(textures.startPad, x, y, 0.0f, tint);
    } else {
        drawFloor(maze, textures, row, col, x, y, tint);
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
    // A viewer with the key (or Ivan Asen II) sees every door open
    bool canOpenDoors = viewer.hasKey || viewer.character == IVAN_ASEN;
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
            drawCellBase(maze, textures, row, col, x, y, tint, canOpenDoors);
            if (view == LIT_CELL) {
                drawWallGlow(maze, textures, row, col, x, y);
            }
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
    float centerX = (viewer.visualCol + 1.5f) * TILE_SIZE - camera.x;
    float centerY = (viewer.visualRow + 1.5f) * TILE_SIZE - camera.y;
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

// Draws the player's sprite on their cell, with the camera centred on the player.
void drawPlayer(const GameState& game, const Player& player, const Textures& textures) {
    drawPlayerSeenBy(game, player, player, textures);
}

// Draws a hero at the same camera position and scale as the maze, as the viewer sees it.
void drawPlayerSeenBy(const GameState& game, const Player& viewer, const Player& player,
                      const Textures& textures) {
    if (player.character < ASPARUH || player.character > LEVSKI) {
        return;
    }
    const HeroTextures& hero = textures.heroes[player.character];
    // The hero walks while the drawn position is still sliding to the real cell
    float rowGap = player.visualRow - static_cast<float>(player.row);
    float colGap = player.visualCol - static_cast<float>(player.col);
    bool isMoving = (rowGap > 0.02f || rowGap < -0.02f || colGap > 0.02f || colGap < -0.02f);

    const Texture2D* frames = hero.idle;
    double framesPerSecond = 5.0;
    bool isMirrored = false;
    if (player.stuckSeconds > 0.0f) {
        frames = hero.stuck;
    } else if (isMoving) {
        framesPerSecond = 12.0;
        if (player.facing == UP) {
            frames = hero.walkUp;
        } else if (player.facing == DOWN) {
            frames = hero.walkDown;
        } else {
            frames = hero.walkSide;
            isMirrored = player.facing == LEFT;
        }
    }
    const Texture2D& sprite = frames[static_cast<int>(GetTime() * framesPerSecond) % HERO_FRAMES];
    if (sprite.id == 0) {
        return;
    }
    Vector2 camera = getCameraOrigin(game.maze, viewer);
    float width = static_cast<float>(sprite.width * TILE_SCALE);
    float height = static_cast<float>(sprite.height * TILE_SCALE);
    // A negative source width mirrors the picture, so the side sprite can face left
    Rectangle source = {0.0f, 0.0f, static_cast<float>(isMirrored ? -sprite.width : sprite.width),
                        static_cast<float>(sprite.height)};
    Rectangle destination = {(player.visualCol + 1.5f) * TILE_SIZE - camera.x - width / 2.0f,
                             (player.visualRow + 2.0f) * TILE_SIZE - camera.y - height,
                             width, height};
    DrawTexturePro(sprite, source, destination, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
}

// Draws a picture stretched to the rectangle, with a tint. A missing picture draws nothing.
static void drawPicture(const Texture2D& texture, float x, float y, float width, float height,
                        Color tint) {
    if (texture.id == 0) {
        return;
    }
    Rectangle source = {0.0f, 0.0f, static_cast<float>(texture.width),
                        static_cast<float>(texture.height)};
    Rectangle destination = {x, y, width, height};
    DrawTexturePro(texture, source, destination, Vector2{0.0f, 0.0f}, 0.0f, tint);
}

// Draws text with a dark shadow so it stays readable on any background.
static void drawShadowText(const char* text, int x, int y, int fontSize, Color color) {
    DrawText(text, x + 2, y + 2, fontSize, Color{0, 0, 0, 200});
    DrawText(text, x, y, fontSize, color);
}

// Draws text with a thin black outline, for text on a bright background such as the lit bar.
static void drawOutlinedText(const char* text, int x, int y, int fontSize, Color color) {
    const Color outline = {0, 0, 0, 255};
    DrawText(text, x - 1, y, fontSize, outline);
    DrawText(text, x + 1, y, fontSize, outline);
    DrawText(text, x, y - 1, fontSize, outline);
    DrawText(text, x, y + 1, fontSize, outline);
    DrawText(text, x, y, fontSize, color);
}

// Draws the torch bar at 2x: the empty bar, then the lit part of the full bar cut to the fuel
// ratio (0 to 1). The lit part uses the red art when the torch is low.
static void drawTorchBar(const Textures& textures, int x, int y, float fuelRatio) {
    const float scale = 2.0f;
    const float barWidth = 231.0f;
    const float barHeight = 32.0f;
    drawPicture(textures.hudTorchbarEmpty, static_cast<float>(x), static_cast<float>(y),
                barWidth * scale, barHeight * scale, WHITE);
    if (fuelRatio <= 0.0f || textures.hudTorchbarFull.id == 0) {
        return;
    }
    // the lit part starts after the flame cap (12 px) and stops 9 px before the right end
    float litWidth = 12.0f + (barWidth - 21.0f) * fuelRatio;
    Rectangle source = {0.0f, 0.0f, litWidth, barHeight};
    Rectangle destination = {static_cast<float>(x), static_cast<float>(y), litWidth * scale,
                             barHeight * scale};
    // Below 30% the bar switches to the red low-fuel art
    const Texture2D* litBar = &textures.hudTorchbarFull;
    Color tint = WHITE;
    if (fuelRatio < 0.3f) {
        if (textures.hudTorchbarLow.id != 0) {
            litBar = &textures.hudTorchbarLow;
        } else {
            tint = Color{255, 120, 100, 255};
        }
    }
    DrawTexturePro(*litBar, source, destination, Vector2{0.0f, 0.0f}, 0.0f, tint);
}

// Draws an inventory slot with a picture and an optional count (-1 draws no count). A slot
// that is not owned (grayed) is drawn dim.
static void drawSlot(const Textures& textures, const Texture2D& icon, int x, int count,
                     bool isOwned) {
    drawPicture(textures.hudSlot, static_cast<float>(x), 16.0f, 56.0f, 56.0f, WHITE);
    Color tint = isOwned ? WHITE : Color{255, 255, 255, 70};
    drawPicture(icon, static_cast<float>(x + 10), 26.0f, 36.0f, 36.0f, tint);
    if (count >= 0) {
        drawShadowText(TextFormat("%d", count), x + 40, 54, 18, RAYWHITE);
    }
}

// Draws the torch bar, the timer (campaign) or the name, turn and time left (multiplayer),
// the inventory slots, the place and year, and the hero portrait in its frame.
void drawHud(const GameState& game, const Player& viewer, const Textures& textures) {
    float fuelRatio = 0.0f;
    if (viewer.maxFuel > 0.0f) {
        fuelRatio = viewer.fuel / viewer.maxFuel;
    }
    if (fuelRatio < 0.0f) {
        fuelRatio = 0.0f;
    } else if (fuelRatio > 1.0f) {
        fuelRatio = 1.0f;
    }
    int screenWidth = GetScreenWidth();
    DrawRectangle(0, 0, screenWidth, HUD_HEIGHT, Color{18, 14, 20, 255});
    DrawRectangle(0, HUD_HEIGHT - 3, screenWidth, 3, Color{110, 78, 40, 255});

    // The torch bar has its own flame, so there is no second one next to it
    drawTorchBar(textures, 8, 12, fuelRatio);
    const char* fuelText = TextFormat("%d / %d", static_cast<int>(viewer.fuel + 0.5f),
                                      static_cast<int>(viewer.maxFuel + 0.5f));
    drawOutlinedText(fuelText, 8 + (462 - MeasureText(fuelText, 20)) / 2, 33, 20, RAYWHITE);

    drawPicture(textures.hudTimer, 482.0f, 24.0f, 40.0f, 40.0f, WHITE);
    const char* timeText = "00:00";
    Color timeColor = RAYWHITE;
    if (game.isMultiplayer) {
        float secondsLeft = game.turnSecondsLeft > 0.0f ? game.turnSecondsLeft : 0.0f;
        timeText = TextFormat("%.1f s", secondsLeft);
        if (secondsLeft < 3.0f) {
            timeColor = Color{255, 96, 80, 255};
        }
    } else {
        int seconds = game.timeSeconds > 0.0f ? static_cast<int>(game.timeSeconds) : 0;
        timeText = TextFormat("%02d:%02d", seconds / 60, seconds % 60);
    }
    drawShadowText(timeText, 530, 32, 26, timeColor);

    drawSlot(textures, textures.hudMap, 620, viewer.maps, viewer.maps > 0);
    drawSlot(textures, textures.hudKey, 688, -1, viewer.hasKey);
    if (game.isMultiplayer) {
        drawSlot(textures, textures.hudTrap, 756, viewer.traps, viewer.traps > 0);
    }

    if (game.isMultiplayer) {
        DrawText(viewer.name.c_str(), 836, 12, 24, GOLD);
        DrawText(TextFormat("Turn %d", game.turnNumber), 836, 40, 20, RAYWHITE);
        DrawText("Hold arrows  1: map  2: trap  Enter: done", 836, 66, 16, LIGHTGRAY);
    } else {
        if (game.level != nullptr) {
            // Cherven has no known year (0), so only its name is shown
            const char* place = game.level->place.c_str();
            if (game.level->year != 0) {
                place = TextFormat("%s - %d", place, game.level->year);
            }
            DrawText(place, 836, 12, 24, GOLD);
        }
        DrawText("Arrows: move   M: map   Esc: pause", 836, 66, 16, LIGHTGRAY);
    }

    float frameX = static_cast<float>(screenWidth - 84);
    drawPicture(textures.hudPortraitFrame, frameX, 10.0f, 70.0f, 65.0f, WHITE);
    if (viewer.character >= ASPARUH && viewer.character <= LEVSKI) {
        drawPicture(textures.heroes[viewer.character].portrait, frameX + 10.0f, 18.0f, 50.0f,
                    50.0f, WHITE);
    }
}
