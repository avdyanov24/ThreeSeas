#include "render.h"

#include <string>

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

void drawMaze(const GameState&, const Player&, const Textures&) {}

void drawTorchLight(const Player&, float) {}

void drawPlayer(const Player&, const Textures&) {}

void drawHud(const GameState&, const Player&, const Textures&) {}
