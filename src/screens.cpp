#include "screens.h"

#include <cmath>

#include "game.h"
#include "levels.h"
#include "maze.h"
#include "save.h"
#include "validation.h"

namespace {

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const Color ERROR_RED = {255, 96, 80, 255};
const int BUTTON_NATIVE_WIDTH = 32;
const int BUTTON_NATIVE_HEIGHT = 9;
const int BUTTON_SCALE = 8;
const int BUTTON_WIDTH = BUTTON_NATIVE_WIDTH * BUTTON_SCALE;
const int BUTTON_HEIGHT = BUTTON_NATIVE_HEIGHT * BUTTON_SCALE;
const int BUTTON_SPACING = 76;
const int MENU_BUTTON_X = (SCREEN_WIDTH - BUTTON_WIDTH) / 2;
const int MENU_BUTTON_START_Y = 218;
const int BACK_BUTTON_X = (SCREEN_WIDTH - BUTTON_WIDTH) / 2;
const int BACK_BUTTON_Y = 590;
const int NAME_FIRST_ROW_Y = 200;  // name entry: first remembered player row
const int NAME_ROW_STEP = 48;
const int INTRO_BACK_X = 364;   // the level intro has Back and Start side by side
const int INTRO_START_X = 660;
const int MENU_ITEM_COUNT = 6;
const int LOGO_WIDTH = 366;
const int LOGO_HEIGHT = 144;
const char* const MENU_ITEMS[MENU_ITEM_COUNT] = {
    "Campaign", "Multiplayer", "Leaderboard", "How to Play", "Settings", "Exit"};
const int SETTINGS_ROW_COUNT = 5;  // Sound, Free Play and three volume sliders
const int SETTINGS_FIRST_ROW_Y = 220;
const int SETTINGS_ROW_STEP = 54;
const int SLIDER_X = 640;
const int SLIDER_WIDTH = 220;
const int PAUSE_ITEM_COUNT = 3;
const int PAUSE_BUTTON_Y = 300;
const char* const PAUSE_ITEMS[PAUSE_ITEM_COUNT] = {"Resume", "Restart", "Quit to map"};
const char* const PAUSE_ITEMS_MP[PAUSE_ITEM_COUNT] = {"Resume", "Restart", "Quit to menu"};
const int RESULT_ITEM_COUNT = 2;
const int RESULT_BUTTON_Y = 440;
const char* const RESULT_ITEMS[RESULT_ITEM_COUNT] = {"Play again", "Back to map"};
const int MP_RESULT_ITEM_COUNT = 2;
const int MP_RESULT_BUTTON_Y = 560;
const char* const MP_RESULT_ITEMS[MP_RESULT_ITEM_COUNT] = {"Rematch", "Menu"};
const int ERA_COUNT = 3;
const int PIN_RADIUS = 36;
const int PIN_SIZE = 72;  // the 36 px pin art at exactly 2x, so it stays sharp
const int EMBER_COUNT = 40;
const float SCREEN_FADE_SECONDS = 0.3f;
const float END_BEAT_SECONDS = 0.8f;  // pause on the maze before the result screen
const int HEADING_Y = 24;
const int HEADING_SIZE = 40;
const int BUTTON_CAP_WIDTH = 10;  // width of the left and right ends of the button art
// Where each maze pin stands on its era map: PIN_POSITIONS[era - 1][pin] = {x, y}
const int PIN_POSITIONS[ERA_COUNT][PINS_PER_ERA][2] = {
    {{870, 240}, {600, 420}, {700, 305}},
    {{650, 270}, {820, 240}, {610, 430}},
    {{560, 310}, {700, 400}, {830, 265}}};
const int HOWTO_ROWS = 9;
const char* const HOWTO_CAMPAIGN_TEXT[HOWTO_ROWS] = {
    "Your torch lights the maze around you.",
    "Fuel burns with time. At zero the torch dies.",
    "Oil refills 25% of your fuel.",
    "Press M to read a Map of the area around you.",
    "Pick up the key to open doors.",
    "Doors stay shut without the key.",
    "Water slows you down.",
    "Traps freeze you for 2 seconds.",
    "Finish fast for up to 3 stars."};
const char* const HOWTO_MULTIPLAYER_TEXT[HOWTO_ROWS] = {
    "Find the hidden exit before the others.",
    "Each turn has a time limit. Hold arrows to walk.",
    "Press 1 to use a Map.",
    "Press 2 to place a Trap under your feet.",
    "Press Enter to end your turn early.",
    "Pass the laptop between turns, no peeking.",
    "Cold arenas burn your torch twice as fast.",
    "Levski is immune to traps.",
    "First to the exit wins, or the last torch lit."};
const HowToIcon HOWTO_MULTIPLAYER_ICONS[HOWTO_ROWS] = {
    HOWTO_EXIT,      HOWTO_CLOCK,       HOWTO_KEY_ONE, HOWTO_KEY_TWO, HOWTO_KEY_ENTER,
    HOWTO_PASS_LAPTOP, HOWTO_COLD,      HOWTO_SHIELD,  HOWTO_TROPHY};
const int ERA_TAB_Y = 100;
// Three tabs fill the screen width, so a tab that slides out ends fully off-screen
const int ERA_TAB_WIDTH = BUTTON_NATIVE_WIDTH * 11;
const int ERA_TAB_SPACING = SCREEN_WIDTH / ERA_COUNT;
const float ERA_TAB_MIDDLE_SCALE = 1.1f;
const float ERA_TAB_SIDE_SCALE = 0.8f;
const float PIN_RING_FADE_SPEED = 4.0f;
const float ERA_SPIN_SECONDS = 0.55f;
const float PIN_FADE_SECONDS = 0.35f;
const char* const ERA_TAB_LABELS[ERA_COUNT] = {"Era I", "Era II", "Era III"};

// Tests whether the pointer is inside a button's screen rectangle.
bool isPointerOverButton(Vector2 pointer, int x, int y, int width, int height) {
    Rectangle bounds = {static_cast<float>(x), static_cast<float>(y),
                        static_cast<float>(width), static_cast<float>(height)};
    return CheckCollisionPointRec(pointer, bounds);
}

// Draws button art into bounds without stretching its pixels unevenly: the left and right
// ends keep their shape at the same scale as the height, only the plain middle gets wider.
static void drawButtonArt(const Texture2D& texture, Rectangle bounds, Color tint) {
    float textureWidth = static_cast<float>(texture.width);
    float textureHeight = static_cast<float>(texture.height);
    float cap = static_cast<float>(BUTTON_CAP_WIDTH);
    float scaledCap = cap * bounds.height / textureHeight;
    Rectangle leftSource = {0.0f, 0.0f, cap, textureHeight};
    Rectangle middleSource = {cap, 0.0f, textureWidth - 2.0f * cap, textureHeight};
    Rectangle rightSource = {textureWidth - cap, 0.0f, cap, textureHeight};
    Rectangle left = {bounds.x, bounds.y, scaledCap, bounds.height};
    Rectangle middle = {bounds.x + scaledCap, bounds.y, bounds.width - 2.0f * scaledCap,
                        bounds.height};
    Rectangle right = {bounds.x + bounds.width - scaledCap, bounds.y, scaledCap, bounds.height};
    DrawTexturePro(texture, leftSource, left, Vector2{0.0f, 0.0f}, 0.0f, tint);
    DrawTexturePro(texture, middleSource, middle, Vector2{0.0f, 0.0f}, 0.0f, tint);
    DrawTexturePro(texture, rightSource, right, Vector2{0.0f, 0.0f}, 0.0f, tint);
}

// Draws the shared pixel-art button style and returns the caller's click result.
bool drawButtonStyle(const char* label, int x, int y, int width, int height,
                     bool isHighlighted, bool isClicked, const Textures& textures) {
    const Texture2D& buttonTexture =
        isHighlighted ? textures.buttonHover : textures.buttonNormal;
    Rectangle destination = {static_cast<float>(x), static_cast<float>(y),
                             static_cast<float>(width), static_cast<float>(height)};

    // The highlighted button is lifted by 2 pixels, so it is clear what is selected
    int lift = isHighlighted ? 2 : 0;
    destination.y -= static_cast<float>(lift);

    if (buttonTexture.id != 0) {
        drawButtonArt(buttonTexture, destination, WHITE);
    } else {
        DrawRectangleRec(destination, isHighlighted ? Color{125, 63, 24, 255}
                                                   : Color{45, 35, 32, 255});
        DrawRectangleLinesEx(destination, 2.0f, GOLD);
    }

    int fontSize = 22;
    int textWidth = MeasureText(label, fontSize);
    DrawText(label, x + (width - textWidth) / 2, y - lift + (height - fontSize) / 2, fontSize,
             isHighlighted ? GOLD : RAYWHITE);
    return isClicked;
}

// Draws an art texture across the game window, with a solid fallback if it is missing.
void drawScreenBackground(const Texture2D& texture, Color fallbackColor) {
    if (texture.id != 0) {
        Rectangle source = {0.0f, 0.0f, static_cast<float>(texture.width),
                            static_cast<float>(texture.height)};
        Rectangle destination = {0.0f, 0.0f, static_cast<float>(SCREEN_WIDTH),
                                 static_cast<float>(SCREEN_HEIGHT)};
        DrawTexturePro(texture, source, destination, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
    } else {
        ClearBackground(fallbackColor);
    }
}

// Draws a dark panel with a gold double border behind a block of text. Returns nothing.
void drawPanel(int x, int y, int width, int height) {
    Rectangle outer = {static_cast<float>(x), static_cast<float>(y), static_cast<float>(width),
                       static_cast<float>(height)};
    DrawRectangleRec(outer, Color{14, 10, 14, 215});
    DrawRectangleLinesEx(outer, 3.0f, Color{110, 78, 40, 255});
    Rectangle inner = {outer.x + 6.0f, outer.y + 6.0f, outer.width - 12.0f,
                       outer.height - 12.0f};
    DrawRectangleLinesEx(inner, 1.0f, Color{200, 150, 60, 255});
}

// Draws text centred on centerX with a dark plate behind it, so it stays readable over
// art. alpha (0 to 1) fades the plate and the text. Returns nothing.
void drawFadedTextPlate(const char* text, int centerX, int y, int fontSize, Color color,
                        float alpha) {
    int width = MeasureText(text, fontSize);
    Rectangle plate = {static_cast<float>(centerX - width / 2 - 14), static_cast<float>(y - 6),
                       static_cast<float>(width + 28), static_cast<float>(fontSize + 12)};
    DrawRectangleRec(plate, Fade(Color{0, 0, 0, 175}, alpha));
    DrawRectangleLinesEx(plate, 2.0f, Fade(Color{110, 78, 40, 255}, alpha));
    DrawText(text, centerX - width / 2, y, fontSize, Fade(color, alpha));
}

// Draws text centred on centerX with a dark plate behind it. Returns nothing.
void drawTextPlate(const char* text, int centerX, int y, int fontSize, Color color) {
    drawFadedTextPlate(text, centerX, y, fontSize, color, 1.0f);
}

// Draws the heading of a full screen: gold text on a plate, centred at the top. Every
// screen uses it so all headings look the same.
static void drawHeading(const char* text) {
    drawTextPlate(text, SCREEN_WIDTH / 2, HEADING_Y, HEADING_SIZE, GOLD);
}

// Draws a square picture of the given size with its top-left corner at (x, y). alpha (0 to 1)
// fades it. Returns nothing.
void drawIcon(const Texture2D& texture, int x, int y, int size, float alpha) {
    if (texture.id == 0) {
        return;
    }
    Rectangle source = {0.0f, 0.0f, static_cast<float>(texture.width),
                        static_cast<float>(texture.height)};
    Rectangle destination = {static_cast<float>(x), static_cast<float>(y),
                             static_cast<float>(size), static_cast<float>(size)};
    DrawTexturePro(texture, source, destination, Vector2{0.0f, 0.0f}, 0.0f, Fade(WHITE, alpha));
}

// Returns the screen position of the centre of a pin (0 to 2) on the map of an era (1 to 3).
Vector2 getPinCenter(int era, int pin) {
    return Vector2{static_cast<float>(PIN_POSITIONS[era - 1][pin][0]),
                   static_cast<float>(PIN_POSITIONS[era - 1][pin][1])};
}

// Returns a repeatable pseudo-random number from 0 to 1 for an index, so every spark keeps
// its own place without remembering anything between frames.
float getHash(int index, int salt) {
    float value = sinf(static_cast<float>(index) * 12.9898f + static_cast<float>(salt) * 78.233f) *
                  43758.5453f;
    return value - floorf(value);
}

// Returns how far an animation is (0 to 1) when it starts after delay seconds and lasts
// duration seconds.
float getProgress(float seconds, float delay, float duration) {
    float progress = (seconds - delay) / duration;
    if (progress < 0.0f) {
        return 0.0f;
    }
    return progress > 1.0f ? 1.0f : progress;
}

// Returns an eased value (0 to 1) that overshoots a little before it settles, for pop-in
// animations. progress goes from 0 to 1.
float getPopScale(float progress) {
    float back = progress - 1.0f;
    return 1.0f + 2.70158f * back * back * back + 1.70158f * back * back;
}

// Draws small glowing sparks that float up the window, like embers from a torch. Every
// spark's place is worked out from the time alone, so nothing has to be remembered.
void drawEmbers() {
    float time = static_cast<float>(GetTime());
    float travel = static_cast<float>(SCREEN_HEIGHT + 40);
    for (int i = 0; i < EMBER_COUNT; i++) {
        float speed = 20.0f + 40.0f * getHash(i, 1);
        float rise = fmodf(time * speed + getHash(i, 2) * travel, travel);
        float x = getHash(i, 3) * static_cast<float>(SCREEN_WIDTH) +
                  14.0f * sinf(time * 0.8f + static_cast<float>(i));
        float y = static_cast<float>(SCREEN_HEIGHT + 20) - rise;
        float flicker = 0.6f + 0.4f * sinf(time * 6.0f + static_cast<float>(i) * 1.7f);
        int size = 2 + i % 3;
        DrawRectangle(static_cast<int>(x), static_cast<int>(y), size, size,
                      Fade(Color{255, 175, 70, 255}, (1.0f - rise / travel) * flicker));
    }
}

// Darkens the whole window so text is easy to read, then draws the floating embers.
void drawDarkLayer() {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{0, 0, 0, 165});
    drawEmbers();
}

// Draws an art texture like drawScreenBackground, but zoomed in a little and drifting slowly,
// so menu pictures never stand still. Use it only where nothing is drawn at a fixed place
// on the picture.
void drawMovingBackground(const Texture2D& texture, Color fallbackColor) {
    if (texture.id == 0) {
        ClearBackground(fallbackColor);
        return;
    }
    float time = static_cast<float>(GetTime());
    float zoom = 0.93f + 0.02f * sinf(time * 0.35f);
    float width = static_cast<float>(texture.width) * zoom;
    float height = static_cast<float>(texture.height) * zoom;
    float x = (static_cast<float>(texture.width) - width) * (0.5f + 0.5f * sinf(time * 0.21f));
    float y = (static_cast<float>(texture.height) - height) * (0.5f + 0.5f * cosf(time * 0.17f));
    Rectangle source = {x, y, width, height};
    Rectangle destination = {0.0f, 0.0f, static_cast<float>(SCREEN_WIDTH),
                             static_cast<float>(SCREEN_HEIGHT)};
    DrawTexturePro(texture, source, destination, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
}

// Asks main to play a sound at the end of this frame. Returns nothing.
void requestSound(UiState& ui, SoundId id) {
    ui.soundRequests[id] = true;
}

// Compares a player before and after one frame of play and asks for the matching sounds:
// footstep for a new cell (and the lock for a door cell), pickup for a new item or more
// fuel, the trap snap, the low torch warning when the fuel drops below 30%, and torch out.
void requestPlayerSounds(const GameState& game, int playerIndex, const Player& before,
                         UiState& ui) {
    const Player& after = game.players[playerIndex];
    if (after.row != before.row || after.col != before.col) {
        requestSound(ui, SOUND_FOOTSTEP);
        if (game.maze.cells[after.row][after.col].tile == DOOR) {
            requestSound(ui, SOUND_DOOR_UNLOCK);
        }
    }
    if (after.fuel > before.fuel || after.maps > before.maps || after.traps > before.traps ||
        (after.hasKey && !before.hasKey)) {
        requestSound(ui, SOUND_PICKUP);
    }
    if ((after.stuckSeconds > 0.0f && before.stuckSeconds <= 0.0f) ||
        after.turnsToSkip > before.turnsToSkip) {
        requestSound(ui, SOUND_TRAP_SNAP);
    }
    if (after.maxFuel > 0.0f && !after.isOut && before.fuel >= 0.3f * before.maxFuel &&
        after.fuel < 0.3f * after.maxFuel) {
        requestSound(ui, SOUND_LOW_TORCH);
    }
    if (after.isOut && !before.isOut) {
        requestSound(ui, SOUND_TORCH_OUT);
    }
}

// Returns the next menu index, wrapping around in the requested direction.
int moveSelection(int selectedItem, int direction, int itemCount) {
    if (selectedItem < 0) {
        return direction > 0 ? 0 : itemCount - 1;
    }
    return (selectedItem + direction + itemCount) % itemCount;
}

// Prepares the campaign map with the first pin selected and a valid era.
// Returns CAMPAIGN_MAP.
Screen openCampaign(UiState& ui) {
    ui.selectedItem = 0;
    if (ui.selectedEra < 1 || ui.selectedEra > ERA_COUNT) {
        ui.selectedEra = 1;
    }
    return CAMPAIGN_MAP;
}

// Returns the level id (1 to 9) of a pin (0 to 2) in an era (1 to 3).
int getLevelIdOfPin(int era, int pin) {
    return (era - 1) * PINS_PER_ERA + pin + 1;
}

// Returns where an era's tab sits: -1 left (Era I), 0 middle (Era II), 1 right (Era III).
// The tabs always stay in this order.
int getEraSlot(int era) {
    return era - 2;
}

// Returns the screen rectangle of an era tab at a slot. selection goes from 0 (not
// selected) to 1 (selected); the selected tab is the biggest.
Rectangle getEraTabRect(int slot, float selection) {
    float scale =
        ERA_TAB_SIDE_SCALE + (ERA_TAB_MIDDLE_SCALE - ERA_TAB_SIDE_SCALE) * selection;
    float width = ERA_TAB_WIDTH * scale;
    float height = BUTTON_HEIGHT * scale;
    float centerX = SCREEN_WIDTH / 2.0f + slot * ERA_TAB_SPACING;
    float centerY = ERA_TAB_Y + BUTTON_HEIGHT / 2.0f;
    return Rectangle{centerX - width / 2.0f, centerY - height / 2.0f, width, height};
}

// Draws one era tab at a slot. Tabs that are not selected are darker, as if they stand
// in the shadow of the selected one.
void drawEraTab(const char* label, int slot, float selection, const Textures& textures) {
    Rectangle bounds = getEraTabRect(slot, selection);
    bool isSelected = selection > 0.5f;
    unsigned char light = static_cast<unsigned char>(115.0f + 140.0f * selection);
    Color shade = {light, light, light, 255};

    const Texture2D& texture = isSelected ? textures.buttonHover : textures.buttonNormal;
    if (texture.id != 0) {
        drawButtonArt(texture, bounds, shade);
    } else {
        Color fill = isSelected ? Color{125, 63, 24, 255} : Color{45, 35, 32, 255};
        DrawRectangleRec(bounds, ColorTint(fill, shade));
        DrawRectangleLinesEx(bounds, 2.0f, ColorTint(GOLD, shade));
    }

    int fontSize = static_cast<int>(bounds.height * 0.35f);
    int textWidth = MeasureText(label, fontSize);
    int textX = static_cast<int>(bounds.x + (bounds.width - textWidth) / 2.0f);
    int textY = static_cast<int>(bounds.y + (bounds.height - fontSize) / 2.0f);
    DrawText(label, textX, textY, fontSize, ColorTint(isSelected ? GOLD : RAYWHITE, shade));
}

// Returns how far an animation of this length is, from 0 to 1, slowing down at the end.
float getEasedProgress(float seconds, float duration) {
    float progress = seconds / duration;
    if (progress >= 1.0f) {
        return 1.0f;
    }
    float remaining = 1.0f - progress;
    return 1.0f - remaining * remaining * remaining;
}

// Notices when the selected era changes: remembers in ui the era shown before the switch
// and the time of the switch. Call it on every frame the campaign map is shown.
void updateEraSwitch(UiState& ui) {
    if (ui.selectedEra != ui.shownEra) {
        ui.previousEra = ui.shownEra == 0 ? ui.selectedEra : ui.shownEra;
        ui.shownEra = ui.selectedEra;
        ui.eraSwitchTime = GetTime();
    }
}

// Returns the seconds since the selected era last switched.
float getTimeSinceEraSwitch(const UiState& ui) {
    return static_cast<float>(GetTime() - ui.eraSwitchTime);
}

// Moves each pin's selection in ui a little towards selected or not selected, so its ring
// fades and its size grows smoothly. Call it on every frame the campaign map is shown.
void updatePinRings(UiState& ui) {
    float step = GetFrameTime() * PIN_RING_FADE_SPEED;
    for (int pin = 0; pin < PINS_PER_ERA; pin++) {
        float& ringAlpha = ui.pinRingAlpha[pin];
        if (ui.selectedItem == pin) {
            ringAlpha = ringAlpha + step > 1.0f ? 1.0f : ringAlpha + step;
        } else {
            ringAlpha = ringAlpha - step < 0.0f ? 0.0f : ringAlpha - step;
        }
    }
}

// Returns true if the player has a saved result for this level.
bool hasResult(const UiState& ui, int levelId) {
    Result best = {};
    return getBestResult(ui.playerName, levelId, best);
}

// Returns true if the player may open this level from the map.
bool isPinOpen(const UiState& ui, int levelId) {
    return isLevelUnlocked(levelId, levelId > 1 && hasResult(ui, levelId - 1),
                           ui.settings.isFreePlay);
}

// Returns the highest era (1 to 3) the player has entered, found from the last open
// level. Returns 1 if no level is open.
int getHighestEraEntered(const UiState& ui) {
    for (int levelId = LEVEL_COUNT; levelId >= 1; levelId--) {
        if (isPinOpen(ui, levelId)) {
            return (levelId - 1) / PINS_PER_ERA + 1;
        }
    }
    return 1;
}

// Returns the hero suggested for a level: Krum in era 1, Ivan Asen II in era 2 and
// Levski in era 3.
CharacterId getSuggestedHero(int levelId) {
    int era = (levelId - 1) / PINS_PER_ERA + 1;
    if (era == 1) {
        return KRUM;
    }
    if (era == 2) {
        return IVAN_ASEN;
    }
    return LEVSKI;
}

// Returns the next unlocked hero after the current one, stepping by direction (+1 or
// -1) and wrapping around. Returns the current hero if no other one is unlocked.
CharacterId getNextUnlockedHero(const UiState& ui, int direction) {
    int highestEra = getHighestEraEntered(ui);
    int id = ui.characters[0];
    for (int i = 0; i < CHARACTER_COUNT; i++) {
        id = (id + direction + CHARACTER_COUNT) % CHARACTER_COUNT;
        CharacterId hero = static_cast<CharacterId>(id);
        if (isCharacterUnlocked(hero, highestEra, ui.settings.isFreePlay)) {
            return hero;
        }
    }
    return ui.characters[0];
}

// Returns the next unlocked hero after the hero of one multiplayer player, stepping by
// direction (+1 or -1) and wrapping around. Returns that player's hero if no other one
// is unlocked.
CharacterId getNextUnlockedHeroFor(const UiState& ui, int player, int direction) {
    int highestEra = getHighestEraEntered(ui);
    int id = ui.characters[player];
    for (int i = 0; i < CHARACTER_COUNT; i++) {
        id = (id + direction + CHARACTER_COUNT) % CHARACTER_COUNT;
        CharacterId hero = static_cast<CharacterId>(id);
        if (isCharacterUnlocked(hero, highestEra, ui.settings.isFreePlay)) {
            return hero;
        }
    }
    return ui.characters[player];
}

// Returns how many eras the player has finished in order (0 to 3). An era counts when
// its last (hard) maze has a result; counting stops at the first unfinished era.
int getErasFinished(const UiState& ui) {
    int count = 0;
    for (int era = 1; era <= ERA_COUNT; era++) {
        if (!hasResult(ui, PINS_PER_ERA * era)) {
            break;
        }
        count++;
    }
    return count;
}

// Returns true if this multiplayer arena may be chosen.
bool isArenaOpen(const UiState& ui, int arenaId) {
    return isArenaUnlocked(arenaId, getErasFinished(ui), ui.settings.isFreePlay);
}

// Prepares the multiplayer setup. The first visit fills in the default players, arena
// and size; later visits keep the last values. Returns MP_SETUP.
Screen openMultiplayerSetup(UiState& ui) {
    if (ui.playerCount < 2) {
        const char* const defaultNames[MAX_PLAYERS] = {"Player1", "Player2", "Player3",
                                                       "Player4"};
        const CharacterId defaultHeroes[MAX_PLAYERS] = {ASPARUH, KRUM, BORIS, SIMEON};
        ui.playerCount = 2;
        for (int i = 0; i < MAX_PLAYERS; i++) {
            ui.playerNames[i] = defaultNames[i];
            ui.characters[i] = defaultHeroes[i];
        }
        ui.arenaId = 1;
        ui.typedText = "30";
        // The names and heroes of the last multiplayer game come back
        std::string lastNames[MAX_PLAYERS];
        CharacterId lastHeroes[MAX_PLAYERS];
        int lastCount = loadLastSetup(lastNames, lastHeroes, MAX_PLAYERS);
        if (lastCount >= 2) {
            ui.playerCount = lastCount;
            for (int i = 0; i < lastCount; i++) {
                ui.playerNames[i] = lastNames[i];
                ui.characters[i] = lastHeroes[i];
            }
        }
    }
    ui.selectedItem = 0;
    ui.errorMessage = "";
    return MP_SETUP;
}

// Checks the whole multiplayer setup. Returns "" when everything is valid, otherwise
// the message for the first problem found.
std::string getSetupError(const UiState& ui) {
    if (!isArenaOpen(ui, ui.arenaId)) {
        return "This arena is locked.";
    }
    if (!isValidPlayerCount(ui.playerCount)) {
        return "Choose 2 to 4 players.";
    }
    int size = 0;
    if (!parseWholeNumber(ui.typedText, size) || !isValidSize(size, true)) {
        return "Enter a whole number from 20 to 50.";
    }
    for (int i = 0; i < ui.playerCount; i++) {
        if (!isValidName(ui.playerNames[i])) {
            return "Player " + std::to_string(i + 1) +
                   ": use 1-12 letters or numbers, no spaces.";
        }
    }
    for (int i = 0; i < ui.playerCount; i++) {
        for (int j = 0; j < i; j++) {
            if (ui.characters[j] == ui.characters[i]) {
                return "Already taken by " + ui.playerNames[j] + ".";
            }
        }
    }
    return "";
}

// Handles the multiplayer setup form: Up/Down choose a row, Left/Right change the
// arena, the player count or a hero, and typing fills the size or a name. Returns
// PLAYING when Enter finds no error, MAIN_MENU for Esc or Back, otherwise MP_SETUP.
Screen updateMultiplayerSetup(UiState& ui) {
    Vector2 pointer = GetMousePosition();
    Vector2 mouseDelta = GetMouseDelta();
    bool isMouseClick = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f || isMouseClick) {
        ui.isMouseActive = true;
    }
    bool isOverBack = isPointerOverButton(pointer, BACK_BUTTON_X, BACK_BUTTON_Y,
                                          BUTTON_WIDTH, BUTTON_HEIGHT);
    if (IsKeyPressed(KEY_ESCAPE) || (isMouseClick && isOverBack)) {
        requestSound(ui, SOUND_CLICK);
        ui.selectedItem = 0;
        return MAIN_MENU;
    }

    int rowCount = 3 + ui.playerCount;
    if (IsKeyPressed(KEY_UP)) {
        requestSound(ui, SOUND_CLICK);
        ui.isMouseActive = false;
        ui.selectedItem = moveSelection(ui.selectedItem, -1, rowCount);
    }
    if (IsKeyPressed(KEY_DOWN)) {
        requestSound(ui, SOUND_CLICK);
        ui.isMouseActive = false;
        ui.selectedItem = moveSelection(ui.selectedItem, 1, rowCount);
    }

    int direction = 0;
    if (IsKeyPressed(KEY_LEFT)) {
        direction = -1;
    } else if (IsKeyPressed(KEY_RIGHT)) {
        direction = 1;
    }
    if (direction != 0) {
        requestSound(ui, SOUND_CLICK);
        if (ui.selectedItem == 0) {
            ui.arenaId = (ui.arenaId - 1 + direction + ARENA_COUNT) % ARENA_COUNT + 1;
        } else if (ui.selectedItem == 1) {
            int count = ui.playerCount + direction;
            if (count >= 2 && count <= MAX_PLAYERS) {
                ui.playerCount = count;
            }
            if (ui.selectedItem > 2 + ui.playerCount) {
                ui.selectedItem = 2 + ui.playerCount;
            }
        } else if (ui.selectedItem >= 3) {
            int player = ui.selectedItem - 3;
            ui.characters[player] = getNextUnlockedHeroFor(ui, player, direction);
        }
    }

    int key = GetCharPressed();
    while (key > 0) {
        if (ui.selectedItem == 2) {
            if (key >= '0' && key <= '9' && static_cast<int>(ui.typedText.size()) < 2) {
                ui.typedText += static_cast<char>(key);
            }
        } else if (ui.selectedItem >= 3) {
            std::string& name = ui.playerNames[ui.selectedItem - 3];
            if (key >= 32 && key <= 126 && static_cast<int>(name.size()) < MAX_NAME_LENGTH) {
                name += static_cast<char>(key);
            }
        }
        ui.errorMessage = "";
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
        if (ui.selectedItem == 2 && !ui.typedText.empty()) {
            ui.typedText.pop_back();
        } else if (ui.selectedItem >= 3 && !ui.playerNames[ui.selectedItem - 3].empty()) {
            ui.playerNames[ui.selectedItem - 3].pop_back();
        }
        ui.errorMessage = "";
    }

    if (IsKeyPressed(KEY_ENTER)) {
        requestSound(ui, SOUND_CLICK);
        ui.errorMessage = getSetupError(ui);
        if (ui.errorMessage.empty()) {
            int size = 0;
            parseWholeNumber(ui.typedText, size);
            ui.chosenSize = size;
            ui.isMultiplayerGame = true;
            saveLastSetup(ui.playerNames, ui.characters, ui.playerCount);
            return PLAYING;
        }
    }
    return MP_SETUP;
}

// Draws text split at spaces into lines no wider than maxWidth, each line under the
// previous one. Returns nothing.
void drawWrappedText(const std::string& text, int x, int y, int maxWidth, int fontSize,
                     Color color) {
    std::string line = "";
    std::string word = "";
    int lineY = y;
    int length = static_cast<int>(text.size());
    // i == length is one step past the end, so the last word is handled too
    for (int i = 0; i <= length; i++) {
        if (i < length && text[i] != ' ') {
            word += text[i];
        } else {
            std::string longerLine = line.empty() ? word : line + " " + word;
            if (!line.empty() && MeasureText(longerLine.c_str(), fontSize) > maxWidth) {
                DrawText(line.c_str(), x, lineY, fontSize, color);
                lineY += fontSize + 8;
                line = word;
            } else {
                line = longerLine;
            }
            word = "";
        }
    }
    if (!line.empty()) {
        DrawText(line.c_str(), x, lineY, fontSize, color);
    }
}

// Prepares the level intro for ui.selectedLevel with the suggested hero and the
// default maze size. Returns LEVEL_INTRO.
Screen openLevelIntro(UiState& ui) {
    ui.characters[0] = getSuggestedHero(ui.selectedLevel);
    ui.typedText = "20";
    ui.errorMessage = "";
    ui.selectedItem = -1;
    return LEVEL_INTRO;
}

// Opens the level of the selected pin if it is unlocked. Returns LEVEL_INTRO for an
// open pin, otherwise CAMPAIGN_MAP (a locked pin does nothing).
Screen openSelectedPin(UiState& ui) {
    if (ui.selectedItem < 0 || ui.selectedItem >= PINS_PER_ERA) {
        return CAMPAIGN_MAP;
    }
    int levelId = getLevelIdOfPin(ui.selectedEra, ui.selectedItem);
    if (!isPinOpen(ui, levelId)) {
        return CAMPAIGN_MAP;
    }
    ui.selectedLevel = levelId;
    return openLevelIntro(ui);
}

// Handles the hero choice, the maze size input and the way back on the level intro.
// Returns PLAYING for a valid size, CAMPAIGN_MAP for Esc or Back, otherwise LEVEL_INTRO.
Screen updateLevelIntro(UiState& ui) {
    Vector2 pointer = GetMousePosition();
    Vector2 mouseDelta = GetMouseDelta();
    bool isMouseClick = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f || isMouseClick) {
        ui.isMouseActive = true;
    }
    // Back is item 0 and Start is item 1, side by side under the panel
    if (ui.isMouseActive) {
        ui.selectedItem = -1;
        if (isPointerOverButton(pointer, INTRO_BACK_X, BACK_BUTTON_Y, BUTTON_WIDTH,
                                BUTTON_HEIGHT)) {
            ui.selectedItem = 0;
        } else if (isPointerOverButton(pointer, INTRO_START_X, BACK_BUTTON_Y, BUTTON_WIDTH,
                                       BUTTON_HEIGHT)) {
            ui.selectedItem = 1;
        }
    }

    int key = GetCharPressed();
    while (key > 0) {
        if (key >= '0' && key <= '9' && static_cast<int>(ui.typedText.size()) < 2) {
            ui.typedText += static_cast<char>(key);
        }
        ui.errorMessage = "";
        key = GetCharPressed();
    }

    if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) &&
        !ui.typedText.empty()) {
        ui.typedText.pop_back();
        ui.errorMessage = "";
    }

    if (IsKeyPressed(KEY_LEFT)) {
        requestSound(ui, SOUND_CLICK);
        ui.characters[0] = getNextUnlockedHero(ui, -1);
    }
    if (IsKeyPressed(KEY_RIGHT)) {
        requestSound(ui, SOUND_CLICK);
        ui.characters[0] = getNextUnlockedHero(ui, 1);
    }

    if (IsKeyPressed(KEY_ENTER) || (isMouseClick && ui.selectedItem == 1)) {
        requestSound(ui, SOUND_CLICK);
        int size = 0;
        if (parseWholeNumber(ui.typedText, size) && isValidSize(size, false)) {
            ui.chosenSize = size;
            ui.errorMessage = "";
            ui.isMultiplayerGame = false;
            return PLAYING;
        }
        ui.errorMessage = "Use a size from 10 to 50.";
        return LEVEL_INTRO;
    }

    if (IsKeyPressed(KEY_ESCAPE) || (isMouseClick && ui.selectedItem == 0)) {
        requestSound(ui, SOUND_CLICK);
        ui.selectedItem = 0;
        return CAMPAIGN_MAP;
    }
    return LEVEL_INTRO;
}

// Reads the save files for the leaderboard: the fastest result of every level and the
// players with the most multiplayer wins. Returns LEADERBOARD.
Screen openLeaderboard(UiState& ui) {
    ui.selectedItem = 0;
    Result results[MAX_RECORDS];
    int resultCount = loadResults(results, MAX_RECORDS);
    for (int i = 0; i < LEVEL_COUNT; i++) {
        ui.hasLevelBest[i] = false;
    }
    for (int i = 0; i < resultCount; i++) {
        int index = results[i].levelId - 1;
        if (index < 0 || index >= LEVEL_COUNT) {
            continue;
        }
        if (!ui.hasLevelBest[index] || results[i].bestSeconds < ui.levelBest[index].bestSeconds) {
            ui.levelBest[index] = results[i];
            ui.hasLevelBest[index] = true;
        }
    }

    WinRecord wins[MAX_RECORDS];
    int winCount = loadWins(wins, MAX_RECORDS);
    ui.topWinCount = 0;
    while (ui.topWinCount < LEADERBOARD_WINS) {
        int bestIndex = -1;
        for (int i = 0; i < winCount; i++) {
            if (wins[i].wins > 0 && (bestIndex == -1 || wins[i].wins > wins[bestIndex].wins)) {
                bestIndex = i;
            }
        }
        if (bestIndex == -1) {
            break;
        }
        ui.topWins[ui.topWinCount] = wins[bestIndex];
        ui.topWinCount++;
        wins[bestIndex].wins = 0;
    }
    return LEADERBOARD;
}

// Prepares the settings screen with the first button selected. Returns SETTINGS.
Screen openSettings(UiState& ui) {
    ui.selectedItem = 0;
    return SETTINGS;
}

// Sets up the destination screen for a selected main-menu entry.
Screen selectMenuItem(int selectedItem, UiState& ui) {
    ui.selectedItem = 0;
    switch (selectedItem) {
        case 0:
            return openCampaign(ui);
        case 1:
            return openMultiplayerSetup(ui);
        case 2:
            return openLeaderboard(ui);
        case 3:
            return HOW_TO_PLAY;
        case 4:
            return openSettings(ui);
        case 5:
            ui.isExitRequested = true;
            return MAIN_MENU;
        default:
            return MAIN_MENU;
    }
}

// Updates the main-menu selection and handles one input action for this frame.
Screen updateMainMenu(UiState& ui) {
    Vector2 pointer = GetMousePosition();
    Vector2 mouseDelta = GetMouseDelta();
    bool isMouseClick = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f || isMouseClick) {
        ui.isMouseActive = true;
    }

    if (ui.isMouseActive) {
        ui.selectedItem = -1;
        for (int i = 0; i < MENU_ITEM_COUNT; i++) {
            int buttonY = MENU_BUTTON_START_Y + i * BUTTON_SPACING;
            if (isPointerOverButton(pointer, MENU_BUTTON_X, buttonY, BUTTON_WIDTH,
                                    BUTTON_HEIGHT)) {
                ui.selectedItem = i;
                break;
            }
        }
    }

    if (IsKeyPressed(KEY_UP)) {
        requestSound(ui, SOUND_CLICK);
        ui.isMouseActive = false;
        ui.selectedItem = moveSelection(ui.selectedItem, -1, MENU_ITEM_COUNT);
        return MAIN_MENU;
    }
    if (IsKeyPressed(KEY_DOWN)) {
        requestSound(ui, SOUND_CLICK);
        ui.isMouseActive = false;
        ui.selectedItem = moveSelection(ui.selectedItem, 1, MENU_ITEM_COUNT);
        return MAIN_MENU;
    }
    if (IsKeyPressed(KEY_ENTER)) {
        requestSound(ui, SOUND_CLICK);
        ui.isMouseActive = false;
        if (ui.selectedItem < 0) {
            ui.selectedItem = 0;
        }
        return selectMenuItem(ui.selectedItem, ui);
    }
    if (isMouseClick && ui.selectedItem >= 0) {
        requestSound(ui, SOUND_CLICK);
        return selectMenuItem(ui.selectedItem, ui);
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        ui.isMouseActive = false;
    }
    return MAIN_MENU;
}

// Handles pins, era tabs and the way back on the campaign map. Returns LEVEL_INTRO
// for an opened pin, MAIN_MENU for Esc or Back, otherwise CAMPAIGN_MAP.
Screen updateCampaignMap(UiState& ui) {
    Vector2 pointer = GetMousePosition();
    Vector2 mouseDelta = GetMouseDelta();
    bool isMouseClick = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f || isMouseClick) {
        ui.isMouseActive = true;
    }

    if (ui.isMouseActive) {
        ui.selectedItem = -1;
        for (int i = 0; i < PINS_PER_ERA; i++) {
            Vector2 center = getPinCenter(ui.selectedEra, i);
            if (CheckCollisionPointCircle(pointer, center, static_cast<float>(PIN_RADIUS))) {
                ui.selectedItem = i;
                break;
            }
        }
    }

    if (isMouseClick) {
        for (int era = 1; era <= ERA_COUNT; era++) {
            float selection = era == ui.selectedEra ? 1.0f : 0.0f;
            if (CheckCollisionPointRec(pointer, getEraTabRect(getEraSlot(era), selection))) {
                requestSound(ui, SOUND_CLICK);
                ui.selectedEra = era;
                ui.selectedItem = 0;
                return CAMPAIGN_MAP;
            }
        }
        if (isPointerOverButton(pointer, BACK_BUTTON_X, BACK_BUTTON_Y, BUTTON_WIDTH,
                                BUTTON_HEIGHT)) {
            requestSound(ui, SOUND_CLICK);
            ui.selectedItem = 0;
            return MAIN_MENU;
        }
        Screen next = openSelectedPin(ui);
        if (next != CAMPAIGN_MAP) {
            requestSound(ui, SOUND_CLICK);
        }
        return next;
    }

    if (IsKeyPressed(KEY_LEFT)) {
        requestSound(ui, SOUND_CLICK);
        ui.isMouseActive = false;
        ui.selectedItem = moveSelection(ui.selectedItem, -1, PINS_PER_ERA);
        return CAMPAIGN_MAP;
    }
    if (IsKeyPressed(KEY_RIGHT)) {
        requestSound(ui, SOUND_CLICK);
        ui.isMouseActive = false;
        ui.selectedItem = moveSelection(ui.selectedItem, 1, PINS_PER_ERA);
        return CAMPAIGN_MAP;
    }
    // The eras go round in a circle: after Era III comes Era I again
    if (IsKeyPressed(KEY_UP)) {
        requestSound(ui, SOUND_CLICK);
        ui.isMouseActive = false;
        ui.selectedEra = ui.selectedEra % ERA_COUNT + 1;
        ui.selectedItem = 0;
        return CAMPAIGN_MAP;
    }
    if (IsKeyPressed(KEY_DOWN)) {
        requestSound(ui, SOUND_CLICK);
        ui.isMouseActive = false;
        ui.selectedEra = (ui.selectedEra + ERA_COUNT - 2) % ERA_COUNT + 1;
        ui.selectedItem = 0;
        return CAMPAIGN_MAP;
    }
    if (IsKeyPressed(KEY_ENTER)) {
        requestSound(ui, SOUND_CLICK);
        ui.isMouseActive = false;
        return openSelectedPin(ui);
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        requestSound(ui, SOUND_CLICK);
        ui.selectedItem = 0;
        return MAIN_MENU;
    }
    return CAMPAIGN_MAP;
}

// Handles the Back button and Escape on campaign and pending feature screens.
Screen updateBackScreen(Screen screen, UiState& ui) {
    Vector2 pointer = GetMousePosition();
    Vector2 mouseDelta = GetMouseDelta();
    bool isMouseClick = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f || isMouseClick) {
        ui.isMouseActive = true;
    }

    if (ui.isMouseActive) {
        ui.selectedItem =
            isPointerOverButton(pointer, BACK_BUTTON_X, BACK_BUTTON_Y, BUTTON_WIDTH,
                                BUTTON_HEIGHT)
                ? 0
                : -1;
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        requestSound(ui, SOUND_CLICK);
        ui.selectedItem = 0;
        return MAIN_MENU;
    }
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN)) {
        requestSound(ui, SOUND_CLICK);
        ui.isMouseActive = false;
        ui.selectedItem = 0;
        return screen;
    }
    if (IsKeyPressed(KEY_ENTER)) {
        requestSound(ui, SOUND_CLICK);
        ui.isMouseActive = false;
        return MAIN_MENU;
    }
    if (isMouseClick && ui.selectedItem == 0) {
        requestSound(ui, SOUND_CLICK);
        return MAIN_MENU;
    }
    return screen;
}

// Opens the name entry screen with the remembered players. The last player is chosen,
// or the new-name box when nobody is remembered yet. Returns NAME_ENTRY.
Screen openNameEntry(UiState& ui) {
    requestSound(ui, SOUND_CLICK);
    ui.savedPlayerCount = loadPlayers(ui.savedPlayers, MAX_SAVED_PLAYERS);
    ui.typedText = "";
    ui.errorMessage = "";
    ui.selectedItem = 0;
    return NAME_ENTRY;
}

// Returns the top of a row on the name entry screen: the remembered players first, then
// the new-name box (row savedPlayerCount).
static int getNameRowY(int row) {
    return NAME_FIRST_ROW_Y + row * NAME_ROW_STEP;
}

// Continues as name: remembers it as the last player and opens the main menu.
static Screen chooseName(UiState& ui, const std::string& name) {
    rememberPlayer(name);
    ui.playerName = name;
    ui.errorMessage = "";
    ui.selectedItem = 0;
    return MAIN_MENU;
}

// Handles the name entry screen. Up/Down choose a remembered player or the new-name box,
// typing goes into the box, Enter (or a click) continues, Delete forgets the chosen
// remembered player. Returns MAIN_MENU for a chosen or valid new name, TITLE for Esc or
// Back, otherwise NAME_ENTRY.
Screen updateNameEntry(UiState& ui) {
    const int newRow = ui.savedPlayerCount;
    const int backItem = ui.savedPlayerCount + 1;
    Vector2 pointer = GetMousePosition();
    Vector2 mouseDelta = GetMouseDelta();
    bool isMouseClick = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f || isMouseClick) {
        for (int row = 0; row <= newRow; row++) {
            int rowY = getNameRowY(row);
            if (pointer.x >= 420.0f && pointer.x <= 860.0f && pointer.y >= rowY - 6 &&
                pointer.y <= rowY + 40) {
                ui.selectedItem = row;
            }
        }
        if (isPointerOverButton(pointer, BACK_BUTTON_X, BACK_BUTTON_Y, BUTTON_WIDTH,
                                BUTTON_HEIGHT)) {
            ui.selectedItem = backItem;
        } else if (ui.selectedItem == backItem) {
            ui.selectedItem = newRow;
        }
    }

    // Typing always goes into the new-name box
    int key = GetCharPressed();
    while (key > 0) {
        ui.selectedItem = newRow;
        if (key >= 32 && key <= 126 &&
            static_cast<int>(ui.typedText.size()) < MAX_NAME_LENGTH) {
            ui.typedText += static_cast<char>(key);
        }
        ui.errorMessage = "";
        key = GetCharPressed();
    }
    if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) &&
        ui.selectedItem == newRow && !ui.typedText.empty()) {
        ui.typedText.pop_back();
        ui.errorMessage = "";
    }

    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN)) {
        requestSound(ui, SOUND_CLICK);
        int direction = IsKeyPressed(KEY_UP) ? -1 : 1;
        ui.selectedItem = moveSelection(ui.selectedItem, direction, newRow + 1);
    }

    if (IsKeyPressed(KEY_DELETE) && ui.selectedItem >= 0 && ui.selectedItem < newRow) {
        requestSound(ui, SOUND_CLICK);
        forgetPlayer(ui.savedPlayers[ui.selectedItem]);
        ui.savedPlayerCount = loadPlayers(ui.savedPlayers, MAX_SAVED_PLAYERS);
        if (ui.selectedItem > ui.savedPlayerCount) {
            ui.selectedItem = ui.savedPlayerCount;
        }
        return NAME_ENTRY;
    }

    if (IsKeyPressed(KEY_ESCAPE) || (isMouseClick && ui.selectedItem == backItem)) {
        requestSound(ui, SOUND_CLICK);
        ui.selectedItem = 0;
        return TITLE;
    }

    bool isChosen = IsKeyPressed(KEY_ENTER) || (isMouseClick && ui.selectedItem >= 0 &&
                                                 ui.selectedItem < newRow);
    if (!isChosen) {
        return NAME_ENTRY;
    }
    requestSound(ui, SOUND_CLICK);
    if (ui.selectedItem >= 0 && ui.selectedItem < newRow) {
        return chooseName(ui, ui.savedPlayers[ui.selectedItem]);
    }
    if (isValidName(ui.typedText)) {
        return chooseName(ui, ui.typedText);
    }
    ui.errorMessage = "Use 1 to 12 letters, numbers or _.";
    return NAME_ENTRY;
}

// Draws the name entry screen: the remembered players as "Continue as" rows, the box for
// a new name with its rule and error, a hint and Back.
void drawNameEntry(const Textures& textures, const UiState& ui) {
    drawMovingBackground(textures.menuBackground, Color{12, 10, 16, 255});
    drawDarkLayer();

    drawPanel(340, 110, 600, 450);
    const char* heading = "Who is playing?";
    DrawText(heading, (SCREEN_WIDTH - MeasureText(heading, 40)) / 2, 140, 40, GOLD);

    const int newRow = ui.savedPlayerCount;
    for (int row = 0; row < newRow; row++) {
        int y = getNameRowY(row);
        bool isFocused = ui.selectedItem == row;
        if (isFocused) {
            DrawText(">", 420, y + 8, 30, GOLD);
        }
        std::string label = "Continue as " + ui.savedPlayers[row];
        DrawText(label.c_str(), 450, y + 8, 30, isFocused ? GOLD : RAYWHITE);
    }

    int newY = getNameRowY(newRow);
    bool isNewFocused = ui.selectedItem == newRow;
    if (isNewFocused) {
        DrawText(">", 420, newY + 8, 30, GOLD);
    }
    DrawText("New:", 450, newY + 8, 30, isNewFocused ? GOLD : RAYWHITE);
    drawTextInput(ui.typedText, 540, newY, 300, isNewFocused, "");
    const char* rule = "1 to 12 letters, numbers or _";
    if (!ui.errorMessage.empty()) {
        rule = ui.errorMessage.c_str();
    }
    DrawText(rule, 540, newY + 56, 20, ui.errorMessage.empty() ? LIGHTGRAY : ERROR_RED);

    const char* hint = newRow > 0 ? "Up/Down: choose   Enter: play   Delete: forget"
                                  : "Type your name and press Enter";
    DrawText(hint, (SCREEN_WIDTH - MeasureText(hint, 20)) / 2, 524, 20, LIGHTGRAY);

    drawButtonStyle("Back", BACK_BUTTON_X, BACK_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
                    ui.selectedItem == newRow + 1, false, textures);
}

// Draws the title screen and its route into the main menu.
void drawTitleScreen(const Textures& textures, const UiState& ui) {
    drawMovingBackground(textures.titleScreen, Color{12, 10, 16, 255});
    drawEmbers();
    float pulse = 0.65f + 0.35f * sinf(static_cast<float>(GetTime()) * 3.0f);
    drawFadedTextPlate("Press Enter to continue", SCREEN_WIDTH / 2, 540, 24, RAYWHITE, pulse);
    drawButtonStyle("Start", BACK_BUTTON_X, BACK_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
                    ui.selectedItem == 0, false, textures);
}

// Draws the main menu over the supplied corridor and logo art.
void drawMainMenu(const Textures& textures, const UiState& ui) {
    drawMovingBackground(textures.menuBackground, Color{12, 10, 16, 255});
    drawEmbers();

    if (textures.menuLogo.id != 0) {
        Rectangle source = {0.0f, 0.0f, static_cast<float>(textures.menuLogo.width),
                            static_cast<float>(textures.menuLogo.height)};
        Rectangle destination = {
            static_cast<float>((SCREEN_WIDTH - LOGO_WIDTH) / 2),
            40.0f + 5.0f * sinf(static_cast<float>(GetTime()) * 1.6f),
            static_cast<float>(LOGO_WIDTH), static_cast<float>(LOGO_HEIGHT)};
        DrawTexturePro(textures.menuLogo, source, destination, Vector2{0.0f, 0.0f}, 0.0f,
                       WHITE);
    }

    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        int buttonY = MENU_BUTTON_START_Y + i * BUTTON_SPACING;
        drawButtonStyle(MENU_ITEMS[i], MENU_BUTTON_X, buttonY, BUTTON_WIDTH,
                        BUTTON_HEIGHT, ui.selectedItem == i, false, textures);
    }
}

// Draws the leaderboard: the fastest result of every level on the left and the players
// with the most multiplayer wins on the right, then the Back button.
void drawLeaderboard(const Textures& textures, const UiState& ui) {
    drawMovingBackground(textures.menuBackground, Color{12, 10, 16, 255});
    drawDarkLayer();
    drawHeading("Leaderboard");

    drawPanel(60, 90, 600, 470);
    drawPanel(700, 90, 520, 470);

    DrawText("Fastest times", 84, 106, 26, GOLD);
    for (int i = 0; i < LEVEL_COUNT; i++) {
        // Each row slides in from the left, one after the other
        float rowProgress = getProgress(ui.screenSeconds, 0.05f * static_cast<float>(i), 0.3f);
        int shift = static_cast<int>((1.0f - rowProgress) * -40.0f);
        int y = 156 + i * 44;
        DrawText(TextFormat("%d  %s", i + 1, getLevel(i + 1)->place.c_str()), 84 + shift, y, 22,
                 Fade(RAYWHITE, rowProgress));
        if (!ui.hasLevelBest[i]) {
            DrawText("no result yet", 300 + shift, y + 2, 20, Fade(GRAY, rowProgress));
            continue;
        }
        const Result& best = ui.levelBest[i];
        int seconds = static_cast<int>(best.bestSeconds);
        DrawText(best.name.c_str(), 300 + shift, y + 2, 20, Fade(RAYWHITE, rowProgress));
        DrawText(TextFormat("%02d:%02d", seconds / 60, seconds % 60), 490 + shift, y + 2, 20,
                 Fade(GOLD, rowProgress));
        for (int star = 0; star < 3; star++) {
            const Texture2D& icon = star < best.stars ? textures.starFilled
                                                      : textures.starEmpty;
            drawIcon(icon, 556 + star * 30 + shift, y - 2, 28, rowProgress);
        }
    }

    DrawText("Most wins", 724, 106, 26, GOLD);
    if (ui.topWinCount == 0) {
        DrawText("No multiplayer wins yet.", 724, 160, 22, GRAY);
    }
    for (int i = 0; i < ui.topWinCount; i++) {
        float delay = 0.2f + 0.08f * static_cast<float>(i);
        float rowProgress = getProgress(ui.screenSeconds, delay, 0.3f);
        int shift = static_cast<int>((1.0f - rowProgress) * 40.0f);
        int y = 156 + i * 52;
        DrawText(TextFormat("%d", i + 1), 724 + shift, y, 26,
                 Fade(i == 0 ? GOLD : RAYWHITE, rowProgress));
        DrawText(ui.topWins[i].name.c_str(), 770 + shift, y, 26, Fade(RAYWHITE, rowProgress));
        const char* count =
            TextFormat(ui.topWins[i].wins == 1 ? "%d win" : "%d wins", ui.topWins[i].wins);
        DrawText(count, 1180 - MeasureText(count, 24) + shift, y + 2, 24,
                 Fade(GOLD, rowProgress));
    }

    drawButtonStyle("Back", BACK_BUTTON_X, BACK_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
                    ui.selectedItem == 0, false, textures);
}

// Draws the How to Play screen: the campaign rules on the left, the multiplayer rules on
// the right, each with a small picture, then the Back button.
void drawHowToPlay(const Textures& textures, const UiState& ui) {
    drawMovingBackground(textures.menuBackground, Color{12, 10, 16, 255});
    drawDarkLayer();
    drawHeading("How to Play");
    drawPanel(30, 72, 610, 500);
    drawPanel(660, 72, 590, 500);

    const Texture2D* campaignIcons[HOWTO_ROWS] = {
        &textures.howTo[HOWTO_TORCH], &textures.howTo[HOWTO_TORCH_BAR],
        &textures.howTo[HOWTO_OIL],   &textures.howTo[HOWTO_MAP],
        &textures.key,                &textures.howTo[HOWTO_DOOR],
        &textures.howTo[HOWTO_WATER], &textures.howTo[HOWTO_TRAP],
        &textures.howTo[HOWTO_STARS]};

    DrawText("Campaign (one player)", 56, 90, 26, GOLD);
    DrawText("Multiplayer (2 to 4 players)", 686, 90, 26, GOLD);
    for (int i = 0; i < HOWTO_ROWS; i++) {
        // Each row slides in from the side of its column, one after the other
        float rowProgress = getProgress(ui.screenSeconds, 0.05f * static_cast<float>(i), 0.3f);
        int shift = static_cast<int>((1.0f - rowProgress) * 40.0f);
        int y = 138 + i * 47;
        drawIcon(*campaignIcons[i], 56 - shift, y - 4, 40, rowProgress);
        DrawText(HOWTO_CAMPAIGN_TEXT[i], 112 - shift, y + 6, 19, Fade(RAYWHITE, rowProgress));
        drawIcon(textures.howTo[HOWTO_MULTIPLAYER_ICONS[i]], 686 + shift, y - 4, 40, rowProgress);
        DrawText(HOWTO_MULTIPLAYER_TEXT[i], 742 + shift, y + 6, 19, Fade(RAYWHITE, rowProgress));
    }

    drawButtonStyle("Back", BACK_BUTTON_X, BACK_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
                    ui.selectedItem == 0, false, textures);
}

// Draws the level intro: place and year, era, difficulty, best result, the fun fact,
// the hero panel, the maze size input, a hint and the Back button.
void drawLevelIntro(const Textures& textures, const UiState& ui) {
    const Level& level = *getLevel(ui.selectedLevel);
    // Each level has its own painting; it is dimmed so the panel stays readable
    const Texture2D& painting = textures.introBackgrounds[level.id - 1];
    if (painting.id != 0) {
        drawScreenBackground(painting, Color{12, 10, 16, 255});
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{0, 0, 0, 120});
    } else {
        drawMovingBackground(textures.menuBackground, Color{12, 10, 16, 255});
        drawDarkLayer();
    }

    const char* heading = level.place.c_str();
    if (level.year != 0) {
        heading = TextFormat("%s - %d", level.place.c_str(), level.year);
    }
    drawHeading(heading);
    int eraWidth = MeasureText(level.era.c_str(), 24);
    DrawText(level.era.c_str(), (SCREEN_WIDTH - eraWidth) / 2, 94, 24, RAYWHITE);

    drawPanel(140, 140, 1000, 390);

    const char* difficulty = "Difficulty: Easy";
    if (level.difficulty == 2) {
        difficulty = "Difficulty: Medium";
    } else if (level.difficulty == 3) {
        difficulty = "Difficulty: Hard";
    }
    DrawText(difficulty, 180, 168, 24, RAYWHITE);

    Result best = {};
    if (getBestResult(ui.playerName, level.id, best)) {
        DrawText(TextFormat("Best: %d stars", best.stars), 900, 168, 24, GOLD);
    } else {
        DrawText("Not finished yet", 900, 168, 24, GRAY);
    }

    drawWrappedText(level.funFact, 180, 220, 920, 26, RAYWHITE);

    DrawLine(180, 350, 1100, 350, Color{110, 78, 40, 255});

    // The portrait at its own size (128 px) with arrows that show the hero can be changed
    const Texture2D& portrait = textures.heroes[ui.characters[0]].portrait;
    drawIcon(portrait, 186, 368, 128, 1.0f);
    DrawText("<", 160, 418, 30, GOLD);
    DrawText(">", 324, 418, 30, GOLD);
    const Character* hero = getCharacter(ui.characters[0]);
    DrawText(hero->name.c_str(), 360, 376, 28, GOLD);
    DrawText(hero->power.c_str(), 360, 414, 22, RAYWHITE);
    if (ui.characters[0] == getSuggestedHero(ui.selectedLevel)) {
        DrawText("(suggested)", 360, 446, 20, LIGHTGRAY);
    }
    DrawText("Left/Right: change hero", 360, 480, 20, LIGHTGRAY);

    DrawText("Maze size (10 to 50):", 800, 376, 24, RAYWHITE);
    drawTextInput(ui.typedText, 800, 414, 200, true, ui.errorMessage);

    const char* hint = "Enter: start   Esc: back";
    int hintWidth = MeasureText(hint, 22);
    DrawText(hint, (SCREEN_WIDTH - hintWidth) / 2, 548, 22, LIGHTGRAY);

    drawButtonStyle("Back", INTRO_BACK_X, BACK_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
                    ui.selectedItem == 0, false, textures);
    drawButtonStyle("Start", INTRO_START_X, BACK_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
                    ui.selectedItem == 1, false, textures);
}

// Draws one campaign pin: grey when locked, orange when open, gold when finished,
// with its number, place name, status and a white ring when it is selected. The
// selected pin gets a pulsing ring. alpha (0 to 1) fades the whole
// pin in after an era switch.
void drawCampaignPin(const Textures& textures, const UiState& ui, int pin, float alpha) {
    int levelId = getLevelIdOfPin(ui.selectedEra, pin);
    const Level* level = getLevel(levelId);
    Vector2 center = getPinCenter(ui.selectedEra, pin);
    float labelBaseY = center.y;  // labels stay still while the pin floats
    // Every pin floats up and down a little, each at its own moment
    center.y += 3.0f * sinf(static_cast<float>(GetTime()) * 2.0f + static_cast<float>(pin) * 1.3f);
    bool isOpen = isPinOpen(ui, levelId);
    Result best = {};
    bool isFinished = getBestResult(ui.playerName, levelId, best);

    // selection goes from 0 (not selected) to 1 (selected) over a few frames
    float selection = ui.pinRingAlpha[pin];
    // The pin keeps its size (a whole-number scale); the ring shows the selection
    int size = PIN_SIZE;

    int frame = static_cast<int>(GetTime() * 6.0) % PIN_FRAMES;
    const Texture2D* pinTexture = &textures.pinLocked[frame];
    if (isFinished) {
        pinTexture = &textures.pinFinished[frame];
    } else if (isOpen) {
        pinTexture = &textures.pinOpen[frame];
    }
    float ringAlpha = selection * alpha;
    if (ringAlpha > 0.0f) {
        // pulse goes smoothly from 0 to 1 and back, about once every 1.8 seconds
        float pulse = (sinf(static_cast<float>(GetTime()) * 3.5f) + 1.0f) / 2.0f;
        float outerRadius = static_cast<float>(size) / 2.0f + 8.0f + pulse * 4.0f;
        DrawRing(center, outerRadius, outerRadius + 4.0f, 0.0f, 360.0f, 64,
                 Fade(RAYWHITE, (0.5f + 0.4f * (1.0f - pulse)) * ringAlpha));
    }
    drawIcon(*pinTexture, static_cast<int>(center.x) - size / 2,
             static_cast<int>(center.y) - size / 2, size, alpha);

    int labelX = static_cast<int>(center.x);
    int labelY = static_cast<int>(labelBaseY) + size / 2 + 10;
    // One plate per pin: the name and, after a dash, the stars or "locked"
    const char* name = TextFormat("%d. %s", levelId, level->place.c_str());
    if (isFinished) {
        drawFadedTextPlate(TextFormat(best.stars == 1 ? "%s - %d star" : "%s - %d stars", name,
                                      best.stars),
                           labelX, labelY, 20, GOLD, alpha);
    } else if (!isOpen) {
        drawFadedTextPlate(TextFormat("%s - locked", name), labelX, labelY, 20, GRAY, alpha);
    } else {
        drawFadedTextPlate(name, labelX, labelY, 20, RAYWHITE, alpha);
    }
}

// Draws the campaign map: era tabs, the era name, three pins, a hint and Back.
void drawCampaignMap(const Textures& textures, const UiState& ui) {
    int mapIndex = ui.selectedEra - 1;
    if (mapIndex >= 0 && mapIndex < ERA_MAP_COUNT && textures.eraMaps[mapIndex].id != 0) {
        drawScreenBackground(textures.eraMaps[mapIndex], Color{12, 10, 16, 255});
    } else {
        drawMovingBackground(textures.menuBackground, Color{12, 10, 16, 255});
        drawDarkLayer();
    }


    int previousEra = ui.previousEra;
    float secondsSinceSwitch = getTimeSinceEraSwitch(ui);
    float spinProgress = getEasedProgress(secondsSinceSwitch, ERA_SPIN_SECONDS);
    float fadeProgress = getEasedProgress(secondsSinceSwitch, PIN_FADE_SECONDS);

    for (int era = 1; era <= ERA_COUNT; era++) {
        // The new era grows and the old one shrinks while the tabs stay in place
        float selection = 0.0f;
        if (era == ui.selectedEra) {
            selection = spinProgress;
        } else if (era == previousEra) {
            selection = 1.0f - spinProgress;
        }
        drawEraTab(ERA_TAB_LABELS[era - 1], getEraSlot(era), selection, textures);
    }

    const char* eraName = getLevel(getLevelIdOfPin(ui.selectedEra, 0))->era.c_str();
    drawHeading(eraName);

    for (int pin = 0; pin < PINS_PER_ERA; pin++) {
        drawCampaignPin(textures, ui, pin, fadeProgress);
    }

    const char* hint =
        "Left/Right: choose maze   Up/Down: change era   Enter: open   Esc: back";
    drawTextPlate(hint, SCREEN_WIDTH / 2, 545, 20, LIGHTGRAY);

    bool isBackHovered = isPointerOverButton(GetMousePosition(), BACK_BUTTON_X,
                                             BACK_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT);
    drawButtonStyle("Back", BACK_BUTTON_X, BACK_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
                    isBackHovered, false, textures);
}

// Ends the level when the exit was reached or the torch went out. A win is saved with
// its stars. Returns RESULT when the level is over, otherwise PLAYING.
Screen checkLevelEnd(const GameState& game, UiState& ui) {
    bool isWin = game.winner == 0;
    if (!isWin && !game.players[0].isOut) {
        return PLAYING;
    }

    ui.resultSeconds = game.timeSeconds;
    ui.resultStars = 0;
    if (isWin) {
        ui.resultStars = calculateStars(game.maze.size, game.timeSeconds);
        if (isValidName(ui.playerName) && !ui.settings.isFreePlay) {
            Result result = {ui.playerName, game.level->id, game.timeSeconds,
                             ui.resultStars};
            saveResult(result);
        }
    }
    ui.selectedItem = 0;
    return RESULT;
}

// Updates the selection of itemCount buttons stacked from firstButtonY, like the main
// menu (mouse hover, Up/Down). Returns the index of the button activated by Enter or a
// left click, otherwise -1.
int updateButtonMenu(UiState& ui, int itemCount, int firstButtonY) {
    Vector2 pointer = GetMousePosition();
    Vector2 mouseDelta = GetMouseDelta();
    bool isMouseClick = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f || isMouseClick) {
        ui.isMouseActive = true;
    }

    if (ui.isMouseActive) {
        ui.selectedItem = -1;
        for (int i = 0; i < itemCount; i++) {
            int buttonY = firstButtonY + i * BUTTON_SPACING;
            if (isPointerOverButton(pointer, MENU_BUTTON_X, buttonY, BUTTON_WIDTH,
                                    BUTTON_HEIGHT)) {
                ui.selectedItem = i;
                break;
            }
        }
    }

    if (IsKeyPressed(KEY_UP)) {
        requestSound(ui, SOUND_CLICK);
        ui.isMouseActive = false;
        ui.selectedItem = moveSelection(ui.selectedItem, -1, itemCount);
        return -1;
    }
    if (IsKeyPressed(KEY_DOWN)) {
        requestSound(ui, SOUND_CLICK);
        ui.isMouseActive = false;
        ui.selectedItem = moveSelection(ui.selectedItem, 1, itemCount);
        return -1;
    }
    if (IsKeyPressed(KEY_ENTER)) {
        requestSound(ui, SOUND_CLICK);
        ui.isMouseActive = false;
        if (ui.selectedItem < 0) {
            ui.selectedItem = 0;
        }
        return ui.selectedItem;
    }
    if (isMouseClick && ui.selectedItem >= 0) {
        requestSound(ui, SOUND_CLICK);
        return ui.selectedItem;
    }
    return -1;
}

// Draws itemCount buttons stacked from firstButtonY, highlighting the selected one.
void drawButtonMenu(const Textures& textures, const UiState& ui, const char* const labels[],
                    int itemCount, int firstButtonY) {
    for (int i = 0; i < itemCount; i++) {
        int buttonY = firstButtonY + i * BUTTON_SPACING;
        drawButtonStyle(labels[i], MENU_BUTTON_X, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT,
                        ui.selectedItem == i, false, textures);
    }
}

// Draws the running maze: the maze, the torch light and the player inside the play
// area, then the HUD on top.
void drawGameplay(const GameState& game, const Textures& textures) {
    const Player& viewer = game.players[game.currentPlayer];
    BeginScissorMode(0, HUD_HEIGHT, GetScreenWidth(), GetScreenHeight() - HUD_HEIGHT);
    drawMaze(game, viewer, textures);
    drawTorchLight(game, viewer, textures);
    for (int i = 0; i < game.playerCount; i++) {
        const Player& other = game.players[i];
        if (i != game.currentPlayer && isLit(viewer, other.row, other.col)) {
            drawPlayerSeenBy(game, viewer, other, textures);
        }
    }
    drawPlayer(game, viewer, textures);
    EndScissorMode();
    drawHud(game, viewer, textures);
}

// Handles the pause screen. Returns PLAYING for Resume (or Esc) and Restart (after the
// game is freed, so a new maze starts), CAMPAIGN_MAP for Quit to map, otherwise PAUSED.
Screen updatePause(GameState& game, UiState& ui) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        requestSound(ui, SOUND_CLICK);
        return PLAYING;
    }
    int choice = updateButtonMenu(ui, PAUSE_ITEM_COUNT, PAUSE_BUTTON_Y);
    if (choice == 0) {
        return PLAYING;
    }
    if (choice == 1) {
        destroyGame(game);
        return PLAYING;
    }
    if (choice == 2) {
        bool isMultiplayer = game.isMultiplayer;
        destroyGame(game);
        ui.selectedItem = 0;
        if (isMultiplayer) {
            return MAIN_MENU;
        }
        return CAMPAIGN_MAP;
    }
    return PAUSED;
}

// Draws the paused maze under a dark layer with the heading and the pause buttons.
void drawPause(const GameState& game, const Textures& textures, const UiState& ui) {
    drawGameplay(game, textures);
    drawDarkLayer();
    int headingWidth = MeasureText("Paused", 38);
    DrawText("Paused", (SCREEN_WIDTH - headingWidth) / 2, 200, 38, GOLD);
    const char* const* items = game.isMultiplayer ? PAUSE_ITEMS_MP : PAUSE_ITEMS;
    drawButtonMenu(textures, ui, items, PAUSE_ITEM_COUNT, PAUSE_BUTTON_Y);
}

// Handles the result screen. Returns PLAYING for Play again (after the game is freed,
// so a new maze of the same level starts), CAMPAIGN_MAP for Back to map or Esc,
// otherwise RESULT.
Screen updateResult(GameState& game, UiState& ui) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        requestSound(ui, SOUND_CLICK);
        destroyGame(game);
        ui.selectedItem = 0;
        return CAMPAIGN_MAP;
    }
    int choice = updateButtonMenu(ui, RESULT_ITEM_COUNT, RESULT_BUTTON_Y);
    if (choice == 0) {
        destroyGame(game);
        return PLAYING;
    }
    if (choice == 1) {
        destroyGame(game);
        ui.selectedItem = 0;
        return CAMPAIGN_MAP;
    }
    return RESULT;
}

// Draws the result of the level: on a win the stars, time and best time, otherwise
// "Your torch went out", and the Play again and Back to map buttons.
void drawResult(const GameState& game, const Textures& textures, const UiState& ui) {
    bool isWin = ui.resultStars > 0;
    drawScreenBackground(isWin ? textures.resultWin : textures.resultLose,
                         Color{12, 10, 16, 255});

    drawEmbers();
    const char* heading = isWin ? "Maze complete!" : "Your torch went out";
    float headingProgress = getProgress(ui.screenSeconds, 0.0f, 0.4f);
    drawFadedTextPlate(heading, SCREEN_WIDTH / 2,
                       100 - static_cast<int>((1.0f - headingProgress) * 40.0f), 38,
                       isWin ? GOLD : ERROR_RED, headingProgress);

    if (isWin) {
        for (int i = 0; i < 3; i++) {
            // The stars pop in one after the other
            float starProgress = getProgress(ui.screenSeconds, 0.4f + 0.3f * static_cast<float>(i),
                                             0.4f);
            int starSize = static_cast<int>(80.0f * getPopScale(starProgress));
            const Texture2D& star = i + 1 <= ui.resultStars ? textures.starFilled
                                                            : textures.starEmpty;
            drawIcon(star, SCREEN_WIDTH / 2 - 80 + i * 80 - starSize / 2, 210 - starSize / 2,
                     starSize, starProgress > 0.0f ? 1.0f : 0.0f);
        }

        // The hero cheers on the floor left of the buttons, 4 times the sprite size
        const Texture2D* cheer = textures.heroes[ui.characters[0]].win;
        int frame = static_cast<int>(GetTime() * 6.0) % HERO_FRAMES;
        if (cheer[frame].id != 0) {
            Rectangle source = {0.0f, 0.0f, static_cast<float>(cheer[frame].width),
                                static_cast<float>(cheer[frame].height)};
            Rectangle destination = {360.0f, 420.0f, source.width * 4.0f,
                                     source.height * 4.0f};
            DrawTexturePro(cheer[frame], source, destination, Vector2{0.0f, 0.0f}, 0.0f,
                           WHITE);
        }

        int seconds = static_cast<int>(ui.resultSeconds);
        drawTextPlate(TextFormat("Time: %02d:%02d", seconds / 60, seconds % 60),
                      SCREEN_WIDTH / 2, 280, 28, RAYWHITE);

        Result best = {};
        if (game.level != nullptr && getBestResult(ui.playerName, game.level->id, best)) {
            int bestSeconds = static_cast<int>(best.bestSeconds);
            drawTextPlate(TextFormat("Best time: %02d:%02d", bestSeconds / 60, bestSeconds % 60),
                          SCREEN_WIDTH / 2, 336, 28, RAYWHITE);
        }
    }

    drawButtonMenu(textures, ui, RESULT_ITEMS, RESULT_ITEM_COUNT, RESULT_BUTTON_Y);
}

// Draws the label of one setup row at y: a ">" marker and gold text when the row is
// focused, otherwise white text.
void drawSetupLabel(const char* text, int y, bool isFocused) {
    if (isFocused) {
        DrawText(">", 270, y, 26, GOLD);
    }
    DrawText(text, 300, y, 26, isFocused ? GOLD : RAYWHITE);
}

// Draws the multiplayer setup form: arena, player count, maze size, one row per player
// with name and hero, the first error in red, a hint and the Back button.
void drawMultiplayerSetup(const Textures& textures, const UiState& ui) {
    drawMovingBackground(textures.menuBackground, Color{12, 10, 16, 255});
    drawDarkLayer();

    drawHeading("Multiplayer");
    drawPanel(200, 76, 880, 450);

    std::string arenaName = "Arena " + std::to_string(ui.arenaId);
    const Arena* arena = getArena(ui.arenaId);
    if (arena != nullptr) {
        arenaName = arena->name;
    }
    std::string arenaLabel = "Arena: < " + arenaName + " >";
    drawSetupLabel(arenaLabel.c_str(), 96, ui.selectedItem == 0);
    if (!isArenaOpen(ui, ui.arenaId)) {
        int arenaWidth = MeasureText(arenaLabel.c_str(), 26);
        DrawText(" (locked)", 300 + arenaWidth, 96, 26, GRAY);
    }

    if (arena != nullptr) {
        std::string traits = "Traps, doors and keys";
        if (arena->hasWater) {
            traits += ", water";
        }
        if (arena->isCold) {
            traits += ", cold (torch burns twice as fast)";
        }
        DrawText(traits.c_str(), 300, 128, 18, LIGHTGRAY);
    }

    std::string playersLabel = "Players: < " + std::to_string(ui.playerCount) + " >";
    drawSetupLabel(playersLabel.c_str(), 150, ui.selectedItem == 1);

    drawSetupLabel("Maze size (20 to 50):", 200, ui.selectedItem == 2);
    drawTextInput(ui.typedText, 700, 192, 120, ui.selectedItem == 2, "");

    for (int i = 0; i < ui.playerCount; i++) {
        int y = 270 + i * 56;
        bool isFocused = ui.selectedItem == 3 + i;
        std::string playerLabel = "Player " + std::to_string(i + 1);
        drawSetupLabel(playerLabel.c_str(), y, isFocused);
        drawTextInput(ui.playerNames[i], 430, y - 8, 260, isFocused, "");
        const Character* hero = getCharacter(ui.characters[i]);
        if (hero != nullptr) {
            std::string heroLabel = "< " + hero->name + " >";
            DrawText(heroLabel.c_str(), 720, y, 24, RAYWHITE);
        }
    }

    if (!ui.errorMessage.empty()) {
        int errorWidth = MeasureText(ui.errorMessage.c_str(), 22);
        DrawText(ui.errorMessage.c_str(), (SCREEN_WIDTH - errorWidth) / 2, 490, 22, ERROR_RED);
    }

    const char* hint = "Up/Down: row   Left/Right: change   Enter: start   Esc: back";
    int hintWidth = MeasureText(hint, 20);
    DrawText(hint, (SCREEN_WIDTH - hintWidth) / 2, 550, 20, LIGHTGRAY);

    bool isOverBack = isPointerOverButton(GetMousePosition(), BACK_BUTTON_X, BACK_BUTTON_Y,
                                          BUTTON_WIDTH, BUTTON_HEIGHT);
    drawButtonStyle("Back", BACK_BUTTON_X, BACK_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
                    ui.isMouseActive && isOverBack, false, textures);
}

// Handles the black pass screen between turns. Esc does nothing, so nobody can peek at
// the maze by pausing. Returns PLAYING when Enter is pressed, otherwise PASS_SCREEN.
Screen updatePassScreen() {
    if (IsKeyPressed(KEY_ENTER)) {
        return PLAYING;
    }
    return PASS_SCREEN;
}

// Looks for a winner after a move or a turn. A winner is saved with addWin. Returns
// MP_RESULT when the game is over, otherwise the screen given in otherwise.
Screen checkMultiplayerEnd(GameState& game, UiState& ui, Screen otherwise) {
    checkWinner(game);
    if (game.winner == NO_WINNER) {
        return otherwise;
    }
    if (game.winner >= 0 && game.winner < game.playerCount) {
        addWin(game.players[game.winner].name);
    }
    ui.selectedItem = 0;
    return MP_RESULT;
}

// Ends the turn of the current player: burns their fuel and passes the turn on, gives the
// next player their seconds, then looks for a winner. Returns MP_RESULT when the game is
// over, otherwise PASS_SCREEN.
Screen endMultiplayerTurn(GameState& game, UiState& ui) {
    startNextTurn(game);
    return checkMultiplayerEnd(game, ui, PASS_SCREEN);
}

// Runs one multiplayer frame. The player has a few seconds per turn (they shrink every
// round): hold an arrow key to walk (water is slower), 1 uses a Map, 2 places a Trap and
// Enter ends the turn early. The turn also ends when the time runs out or a trap springs.
// Returns PASS_SCREEN after a turn, MP_RESULT when the game is over, PAUSED for Esc,
// otherwise PLAYING.
Screen updateMultiplayerGame(GameState& game, UiState& ui, float deltaTime) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        ui.selectedItem = 0;
        return PAUSED;
    }
    for (int i = 0; i < game.playerCount; i++) {
        updateVisualPosition(game.players[i], deltaTime);
    }
    Player& player = game.players[game.currentPlayer];
    if (IsKeyPressed(KEY_ENTER)) {
        return endMultiplayerTurn(game, ui);
    }
    if (IsKeyPressed(KEY_ONE)) {
        useMap(game.maze, player);
    }
    if (IsKeyPressed(KEY_TWO)) {
        placeTrap(game.maze, player, game.currentPlayer);
    }

    int heldDirection = -1;
    if (IsKeyDown(KEY_UP)) {
        heldDirection = UP;
    } else if (IsKeyDown(KEY_DOWN)) {
        heldDirection = DOWN;
    } else if (IsKeyDown(KEY_LEFT)) {
        heldDirection = LEFT;
    } else if (IsKeyDown(KEY_RIGHT)) {
        heldDirection = RIGHT;
    }
    // The rules of the turn are in game.cpp; this screen only reads the keys
    bool isTurnOver = updateTurn(game, heldDirection, deltaTime);
    Screen next = checkMultiplayerEnd(game, ui, PLAYING);
    if (next != PLAYING) {
        return next;
    }
    if (isTurnOver) {
        return endMultiplayerTurn(game, ui);
    }
    return PLAYING;
}

// Draws the black pass screen with the name of the player whose turn comes next.
void drawPassScreen(const GameState& game, const Textures& textures) {
    drawMovingBackground(textures.passScreen, BLACK);
    // A warm glow breathes around the torch flame
    float glowPulse = 0.35f + 0.15f * sinf(static_cast<float>(GetTime()) * 2.5f);
    if (textures.glow.id != 0) {
        BeginBlendMode(BLEND_ADDITIVE);
        Rectangle glowSource = {0.0f, 0.0f, static_cast<float>(textures.glow.width),
                                static_cast<float>(textures.glow.height)};
        Rectangle glowDestination = {340.0f, -90.0f, 600.0f, 600.0f};
        DrawTexturePro(textures.glow, glowSource, glowDestination, Vector2{0.0f, 0.0f}, 0.0f,
                       Fade(WHITE, glowPulse));
        EndBlendMode();
    }
    drawEmbers();

    std::string message = "Pass the laptop.";
    if (game.players != nullptr && game.currentPlayer >= 0 &&
        game.currentPlayer < game.playerCount) {
        message = "Pass to " + game.players[game.currentPlayer].name + ".";
    }
    int messageWidth = MeasureText(message.c_str(), 44);
    int messageX = (SCREEN_WIDTH - messageWidth) / 2;
    DrawText(message.c_str(), messageX + 3, 563, 44, BLACK);
    DrawText(message.c_str(), messageX, 560, 44, GOLD);

    int enterWidth = MeasureText("Press Enter.", 28);
    int enterX = (SCREEN_WIDTH - enterWidth) / 2;
    float enterPulse = 0.6f + 0.4f * sinf(static_cast<float>(GetTime()) * 3.0f);
    DrawText("Press Enter.", enterX + 2, 642, 28, Fade(BLACK, enterPulse));
    DrawText("Press Enter.", enterX, 640, 28, Fade(RAYWHITE, enterPulse));
}

// Handles the multiplayer result screen. Returns PLAYING for Rematch (after the game is
// freed, so a new game with the same setup starts), MAIN_MENU for Menu or Esc,
// otherwise MP_RESULT.
Screen updateMultiplayerResult(GameState& game, UiState& ui) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        requestSound(ui, SOUND_CLICK);
        destroyGame(game);
        ui.selectedItem = 0;
        return MAIN_MENU;
    }
    int choice = updateButtonMenu(ui, MP_RESULT_ITEM_COUNT, MP_RESULT_BUTTON_Y);
    if (choice == 0) {
        destroyGame(game);
        return PLAYING;
    }
    if (choice == 1) {
        destroyGame(game);
        ui.selectedItem = 0;
        return MAIN_MENU;
    }
    return MP_RESULT;
}

// Draws the multiplayer result: the winner and their hero, or a draw, and the Rematch
// and Menu buttons.
void drawMultiplayerResult(const GameState& game, const Textures& textures, const UiState& ui) {
    // The painting stays still so the winner can stand on its middle altar
    drawScreenBackground(textures.multiplayerResult, Color{12, 10, 16, 255});
    drawEmbers();

    if (game.players != nullptr && game.winner >= 0 && game.winner < game.playerCount) {
        const Player& winner = game.players[game.winner];
        std::string heading = winner.name + " wins!";
        drawTextPlate(heading.c_str(), SCREEN_WIDTH / 2, 100, 40, GOLD);
        const Character* hero = getCharacter(winner.character);
        if (hero != nullptr) {
            int heroWidth = MeasureText(hero->name.c_str(), 20);
            DrawText(hero->name.c_str(), (SCREEN_WIDTH - heroWidth) / 2, 160, 20, LIGHTGRAY);
        }
        // The winner and the crown on the middle altar, the others on the side altars
        drawIcon(textures.iconCrown, 608, 190, 64, 1.0f);
        drawIcon(textures.heroes[winner.character].portrait, 576, 255, 128, 1.0f);
        const int sideAltarX[2] = {360, 856};
        int sideCount = 0;
        for (int i = 0; i < game.playerCount && sideCount < 2; i++) {
            if (i != game.winner) {
                drawIcon(textures.heroes[game.players[i].character].portrait,
                         sideAltarX[sideCount], 386, 64, 0.7f);
                sideCount++;
            }
        }
    } else if (game.winner == DRAW) {
        drawTextPlate("Draw: all torches went out", SCREEN_WIDTH / 2, 120, 40, ERROR_RED);
    }

    drawButtonMenu(textures, ui, MP_RESULT_ITEMS, MP_RESULT_ITEM_COUNT, MP_RESULT_BUTTON_Y);
}

// Returns the volume a settings row changes (rows 2 to 4), or nullptr for other rows.
static int* getVolumeSetting(Settings& settings, int row) {
    if (row == 2) {
        return &settings.menuMusicVolume;
    }
    if (row == 3) {
        return &settings.gameMusicVolume;
    }
    if (row == 4) {
        return &settings.effectsVolume;
    }
    return nullptr;
}

// Returns the top of a settings row on the screen.
static int getSettingsRowY(int row) {
    return SETTINGS_FIRST_ROW_Y + row * SETTINGS_ROW_STEP;
}

// Handles the settings screen. Rows 0 and 1 switch Sound and Free Play, rows 2 to 4 are
// volume sliders and item 5 is Back. Up/Down choose a row, Left/Right or Enter change it,
// the mouse can click a switch or drag a slider. Every change is saved. Returns MAIN_MENU
// for Back or Esc, otherwise SETTINGS.
Screen updateSettings(UiState& ui) {
    const int backItem = SETTINGS_ROW_COUNT;
    if (IsKeyPressed(KEY_ESCAPE)) {
        requestSound(ui, SOUND_CLICK);
        ui.selectedItem = 0;
        return MAIN_MENU;
    }

    Vector2 pointer = GetMousePosition();
    Vector2 mouseDelta = GetMouseDelta();
    bool isMouseClick = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f || isMouseClick) {
        ui.isMouseActive = true;
    }
    // A held mouse button keeps dragging the slider it started on
    bool isDragging = IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !isMouseClick &&
                      getVolumeSetting(ui.settings, ui.selectedItem) != nullptr;
    if (ui.isMouseActive && !isDragging) {
        ui.selectedItem = -1;
        for (int row = 0; row < SETTINGS_ROW_COUNT; row++) {
            int rowY = getSettingsRowY(row);
            if (pointer.x >= 380.0f && pointer.x <= 920.0f && pointer.y >= rowY - 12 &&
                pointer.y <= rowY + 36) {
                ui.selectedItem = row;
            }
        }
        if (isPointerOverButton(pointer, BACK_BUTTON_X, BACK_BUTTON_Y, BUTTON_WIDTH,
                                BUTTON_HEIGHT)) {
            ui.selectedItem = backItem;
        }
    }

    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN)) {
        requestSound(ui, SOUND_CLICK);
        ui.isMouseActive = false;
        ui.selectedItem =
            moveSelection(ui.selectedItem, IsKeyPressed(KEY_UP) ? -1 : 1, SETTINGS_ROW_COUNT + 1);
        return SETTINGS;
    }

    int step = 0;
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT)) {
        step = -1;
    } else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) {
        step = 1;
    }
    bool isPressed = IsKeyPressed(KEY_ENTER) || (isMouseClick && ui.selectedItem >= 0);
    bool hasChanged = false;

    int* volume = getVolumeSetting(ui.settings, ui.selectedItem);
    if (volume != nullptr) {
        int value = *volume + step;
        if (ui.isMouseActive && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            // The value follows the pointer along the slider bar
            float part = (pointer.x - SLIDER_X) / static_cast<float>(SLIDER_WIDTH);
            value = static_cast<int>(part * MAX_VOLUME + 0.5f);
        }
        value = value < 0 ? 0 : (value > MAX_VOLUME ? MAX_VOLUME : value);
        hasChanged = value != *volume;
        *volume = value;
    } else if ((ui.selectedItem == 0 || ui.selectedItem == 1) && (step != 0 || isPressed)) {
        if (ui.selectedItem == 0) {
            ui.settings.isSoundOn = !ui.settings.isSoundOn;
        } else {
            ui.settings.isFreePlay = !ui.settings.isFreePlay;
        }
        hasChanged = true;
    } else if (ui.selectedItem == backItem && isPressed) {
        requestSound(ui, SOUND_CLICK);
        ui.selectedItem = 0;
        return MAIN_MENU;
    }

    if (hasChanged) {
        requestSound(ui, SOUND_CLICK);
        saveSettings(ui.settings);
    }
    return SETTINGS;
}

// Draws the settings screen: the Sound and Free Play switches, three volume sliders, a
// note about Free Play and Back. The chosen row has a ">" marker and gold text.
void drawSettings(const Textures& textures, const UiState& ui) {
    drawMovingBackground(textures.menuBackground, Color{12, 10, 16, 255});
    drawDarkLayer();

    drawPanel(340, 120, 600, 440);
    int headingWidth = MeasureText("Settings", 40);
    DrawText("Settings", (SCREEN_WIDTH - headingWidth) / 2, 150, 40, GOLD);

    const char* const labels[SETTINGS_ROW_COUNT] = {"Sound", "Free Play", "Menu music",
                                                    "Game music", "Effects"};
    Settings settings = ui.settings;
    for (int row = 0; row < SETTINGS_ROW_COUNT; row++) {
        int y = getSettingsRowY(row);
        bool isFocused = ui.selectedItem == row;
        if (isFocused) {
            DrawText(">", 380, y, 30, GOLD);
        }
        DrawText(labels[row], 410, y, 30, isFocused ? GOLD : RAYWHITE);

        const int* volume = getVolumeSetting(settings, row);
        if (volume == nullptr) {
            bool isOn = row == 0 ? settings.isSoundOn : settings.isFreePlay;
            DrawText(isOn ? "On" : "Off", SLIDER_X, y, 30, isOn ? GOLD : GRAY);
            continue;
        }
        // The slider: a dark bar, a gold part for the volume and a knob at its end
        int fillWidth = SLIDER_WIDTH * *volume / MAX_VOLUME;
        Color fill = settings.isSoundOn ? GOLD : GRAY;
        DrawRectangle(SLIDER_X, y + 10, SLIDER_WIDTH, 10, Color{40, 32, 36, 255});
        DrawRectangle(SLIDER_X, y + 10, fillWidth, 10, fill);
        DrawRectangleLines(SLIDER_X, y + 10, SLIDER_WIDTH, 10, Color{110, 78, 40, 255});
        DrawRectangle(SLIDER_X + fillWidth - 5, y + 2, 10, 26, isFocused ? RAYWHITE : fill);
        DrawText(TextFormat("%d", *volume), SLIDER_X + SLIDER_WIDTH + 20, y, 30,
                 isFocused ? GOLD : RAYWHITE);
    }

    const char* note = "Free Play: all open, nothing saved.";
    DrawText(note, (SCREEN_WIDTH - MeasureText(note, 20)) / 2, 500, 20, LIGHTGRAY);

    drawButtonStyle("Back", BACK_BUTTON_X, BACK_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
                    ui.selectedItem == SETTINGS_ROW_COUNT, false, textures);
}

// Reads input and runs the logic of the current screen. Returns the screen to show
// next (the same one if nothing changed). Never draws.
Screen getNextScreen(Screen screen, GameState& game, UiState& ui, float deltaTime) {
    if (screen == TITLE) {
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) {
            ui.selectedItem = 0;
            return openNameEntry(ui);
        }
        Vector2 mouseDelta = GetMouseDelta();
        Vector2 pointer = GetMousePosition();
        bool isMouseClick = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f || isMouseClick) {
            ui.isMouseActive = true;
            ui.selectedItem =
                isPointerOverButton(pointer, BACK_BUTTON_X, BACK_BUTTON_Y, BUTTON_WIDTH,
                                    BUTTON_HEIGHT)
                    ? 0
                    : -1;
        }
        if (isMouseClick && ui.selectedItem == 0) {
            ui.selectedItem = 0;
            return openNameEntry(ui);
        }
        return TITLE;
    }

    if (screen == NAME_ENTRY) {
        return updateNameEntry(ui);
    }

    if (screen == PLAYING && game.isMultiplayer) {
        int playerIndex = game.currentPlayer;
        Player before = game.players[playerIndex];
        Screen next = updateMultiplayerGame(game, ui, deltaTime);
        requestPlayerSounds(game, playerIndex, before, ui);
        if (next == MP_RESULT && game.winner >= 0) {
            requestSound(ui, SOUND_WIN);
        }
        return next;
    }

    if (screen == PLAYING) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            ui.selectedItem = 0;
            return PAUSED;
        }
        int heldDirection = -1;
        if (IsKeyDown(KEY_UP)) {
            heldDirection = UP;
        } else if (IsKeyDown(KEY_DOWN)) {
            heldDirection = DOWN;
        } else if (IsKeyDown(KEY_LEFT)) {
            heldDirection = LEFT;
        } else if (IsKeyDown(KEY_RIGHT)) {
            heldDirection = RIGHT;
        }
        Player before = game.players[0];
        if (IsKeyPressed(KEY_M)) {
            useMap(game.maze, game.players[0]);
        }
        updateCampaign(game, heldDirection, deltaTime);
        updateVisualPosition(game.players[0], deltaTime);
        requestPlayerSounds(game, 0, before, ui);
        // After the exit or a dead torch the maze stays on screen and fades out first
        bool isLevelOver = game.winner == 0 || game.players[0].isOut;
        if (!isLevelOver) {
            ui.endBeatSeconds = 0.0f;
            return PLAYING;
        }
        if (ui.endBeatSeconds < END_BEAT_SECONDS) {
            ui.endBeatSeconds += deltaTime;
            return PLAYING;
        }
        ui.endBeatSeconds = 0.0f;
        Screen next = checkLevelEnd(game, ui);
        if (next == RESULT && ui.resultStars > 0) {
            requestSound(ui, SOUND_WIN);
        }
        return next;
    }

    if (screen == PAUSED) {
        return updatePause(game, ui);
    }

    if (screen == RESULT) {
        return updateResult(game, ui);
    }

    if (screen == MAIN_MENU) {
        return updateMainMenu(ui);
    }

    if (screen == CAMPAIGN_MAP) {
        return updateCampaignMap(ui);
    }

    if (screen == LEVEL_INTRO) {
        return updateLevelIntro(ui);
    }

    if (screen == SETTINGS) {
        return updateSettings(ui);
    }

    if (screen == MP_SETUP) {
        return updateMultiplayerSetup(ui);
    }

    if (screen == PASS_SCREEN) {
        return updatePassScreen();
    }

    if (screen == MP_RESULT) {
        return updateMultiplayerResult(game, ui);
    }

    if (screen == LEADERBOARD || screen == HOW_TO_PLAY) {
        return updateBackScreen(screen, ui);
    }

    return screen;
}

}  // namespace

// Reads input and runs the logic of the current screen. Returns the screen to show
// next (the same one if nothing changed). Never draws.
Screen updateScreen(Screen screen, GameState& game, UiState& ui, float deltaTime) {
    Screen nextScreen = getNextScreen(screen, game, ui, deltaTime);
    // The campaign map animations move on every frame the map is drawn, also the first one
    if (nextScreen == CAMPAIGN_MAP) {
        updateEraSwitch(ui);
        updatePinRings(ui);
    }
    return nextScreen;
}

// Draws the current screen. Never reads input or changes the game.
void drawScreen(Screen screen, const GameState& game, const UiState& ui,
                const Textures& textures) {
    if (screen == TITLE) {
        drawTitleScreen(textures, ui);
    } else if (screen == NAME_ENTRY) {
        drawNameEntry(textures, ui);
    } else if (screen == PLAYING && game.players != nullptr && game.playerCount > 0) {
        drawGameplay(game, textures);
        if (ui.endBeatSeconds > 0.0f) {
            float fade = ui.endBeatSeconds / END_BEAT_SECONDS;
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                          Fade(BLACK, fade > 1.0f ? 1.0f : fade));
        }
    } else if (screen == PAUSED && game.players != nullptr) {
        drawPause(game, textures, ui);
    } else if (screen == RESULT) {
        drawResult(game, textures, ui);
    } else if (screen == MAIN_MENU) {
        drawMainMenu(textures, ui);
    } else if (screen == CAMPAIGN_MAP) {
        drawCampaignMap(textures, ui);
    } else if (screen == LEVEL_INTRO) {
        drawLevelIntro(textures, ui);
    } else if (screen == SETTINGS) {
        drawSettings(textures, ui);
    } else if (screen == MP_SETUP) {
        drawMultiplayerSetup(textures, ui);
    } else if (screen == PASS_SCREEN) {
        drawPassScreen(game, textures);
    } else if (screen == MP_RESULT) {
        drawMultiplayerResult(game, textures, ui);
    } else if (screen == LEADERBOARD) {
        drawLeaderboard(textures, ui);
    } else if (screen == HOW_TO_PLAY) {
        drawHowToPlay(textures, ui);
    }

    // Every screen fades in from black when it opens
    if (ui.screenSeconds < SCREEN_FADE_SECONDS) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                      Fade(BLACK, 1.0f - ui.screenSeconds / SCREEN_FADE_SECONDS));
    }
}

// Draws a button and reports whether it was clicked.
bool drawButton(const char* label, int x, int y, int width, int height) {
    Vector2 pointer = GetMousePosition();
    bool isHovered = isPointerOverButton(pointer, x, y, width, height);
    Textures textures = {};
    bool isClicked = isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    return drawButtonStyle(label, x, y, width, height, isHovered, isClicked, textures);
}

// Draws a text box with the typed text, a blinking cursor when it has focus, and the
// error message in red under it when errorMessage is not empty.
void drawTextInput(const std::string& text, int x, int y, int width, bool hasFocus,
                   const std::string& errorMessage) {
    const int boxHeight = 48;
    const int fontSize = 28;
    Rectangle box = {static_cast<float>(x), static_cast<float>(y), static_cast<float>(width),
                     static_cast<float>(boxHeight)};
    DrawRectangleRec(box, Color{25, 20, 28, 255});
    DrawRectangleLinesEx(box, 2.0f, hasFocus ? GOLD : GRAY);

    int textX = x + 12;
    int textY = y + (boxHeight - fontSize) / 2;
    DrawText(text.c_str(), textX, textY, fontSize, RAYWHITE);

    // The cursor is visible for half a second, then hidden for half a second.
    if (hasFocus && static_cast<int>(GetTime() * 2) % 2 == 0) {
        int textWidth = MeasureText(text.c_str(), fontSize);
        DrawText("_", textX + textWidth, textY, fontSize, RAYWHITE);
    }

    if (!errorMessage.empty()) {
        DrawText(errorMessage.c_str(), x, y + 56, 20, ERROR_RED);
    }
}
