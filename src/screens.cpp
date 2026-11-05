#include "screens.h"

#include <cmath>

#include "game.h"
#include "levels.h"
#include "save.h"
#include "validation.h"

namespace {

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
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
const int MENU_ITEM_COUNT = 6;
const int LOGO_WIDTH = 366;
const int LOGO_HEIGHT = 144;
const char* const MENU_ITEMS[MENU_ITEM_COUNT] = {
    "Campaign", "Multiplayer", "Leaderboard", "How to Play", "Settings", "Exit"};
const int SETTINGS_ITEM_COUNT = 3;
const int SETTINGS_BUTTON_Y = 300;
const int PAUSE_ITEM_COUNT = 3;
const int PAUSE_BUTTON_Y = 300;
const char* const PAUSE_ITEMS[PAUSE_ITEM_COUNT] = {"Resume", "Restart", "Quit to map"};
const int RESULT_ITEM_COUNT = 2;
const int RESULT_BUTTON_Y = 440;
const char* const RESULT_ITEMS[RESULT_ITEM_COUNT] = {"Play again", "Back to map"};
const int MP_RESULT_ITEM_COUNT = 2;
const int MP_RESULT_BUTTON_Y = 440;
const char* const MP_RESULT_ITEMS[MP_RESULT_ITEM_COUNT] = {"Rematch", "Menu"};
const int ERA_COUNT = 3;
const int PINS_PER_ERA = 3;
const int PIN_RADIUS = 40;
const int PIN_Y = 360;
const int PIN_X[PINS_PER_ERA] = {340, 640, 940};
const int ERA_TAB_Y = 100;
// Three tabs fill the screen width, so a tab that slides out ends fully off-screen
const int ERA_TAB_WIDTH = BUTTON_NATIVE_WIDTH * 13;
const int ERA_TAB_SPACING = SCREEN_WIDTH / ERA_COUNT;
const float ERA_TAB_MIDDLE_SCALE = 1.1f;
const float ERA_TAB_SIDE_SCALE = 0.8f;
const float PIN_RING_FADE_SPEED = 4.0f;
const float ERA_SPIN_SECONDS = 0.55f;
const float PIN_FADE_SECONDS = 1.3f;
const float PIN_SELECTED_GROWTH = 0.2f;
const char* const ERA_TAB_LABELS[ERA_COUNT] = {"Era I", "Era II", "Era III"};

// Tests whether the pointer is inside a button's screen rectangle.
bool isPointerOverButton(Vector2 pointer, int x, int y, int width, int height) {
    Rectangle bounds = {static_cast<float>(x), static_cast<float>(y),
                        static_cast<float>(width), static_cast<float>(height)};
    return CheckCollisionPointRec(pointer, bounds);
}

// Draws the shared pixel-art button style and returns the caller's click result.
bool drawButtonStyle(const char* label, int x, int y, int width, int height,
                     bool isHighlighted, bool isClicked, const Textures& textures) {
    const Texture2D& buttonTexture =
        isHighlighted ? textures.buttonHover : textures.buttonNormal;
    Rectangle destination = {static_cast<float>(x), static_cast<float>(y),
                             static_cast<float>(width), static_cast<float>(height)};

    if (buttonTexture.id != 0) {
        Rectangle source = {0.0f, 0.0f, static_cast<float>(buttonTexture.width),
                            static_cast<float>(buttonTexture.height)};
        DrawTexturePro(buttonTexture, source, destination, Vector2{0.0f, 0.0f}, 0.0f,
                       WHITE);
    } else {
        DrawRectangleRec(destination, isHighlighted ? Color{125, 63, 24, 255}
                                                   : Color{45, 35, 32, 255});
        DrawRectangleLinesEx(destination, 2.0f, GOLD);
    }

    int fontSize = 22;
    int textWidth = MeasureText(label, fontSize);
    DrawText(label, x + (width - textWidth) / 2, y + (height - fontSize) / 2, fontSize,
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

// Returns where an era's tab sits next to the selected era: -1 left, 0 middle, 1 right.
int getEraSlot(int era, int selectedEra) {
    return (era - selectedEra + ERA_COUNT + 1) % ERA_COUNT - 1;
}

// Returns the screen rectangle of an era tab at a slot (can be between slots while it
// moves). The middle tab is the biggest; tabs shrink the further they are from it.
Rectangle getEraTabRect(float slot) {
    float distance = fabsf(slot) > 2.0f ? 2.0f : fabsf(slot);
    float scale =
        ERA_TAB_MIDDLE_SCALE - (ERA_TAB_MIDDLE_SCALE - ERA_TAB_SIDE_SCALE) * distance;
    float width = ERA_TAB_WIDTH * scale;
    float height = BUTTON_HEIGHT * scale;
    float centerX = SCREEN_WIDTH / 2.0f + slot * ERA_TAB_SPACING;
    float centerY = ERA_TAB_Y + BUTTON_HEIGHT / 2.0f;
    return Rectangle{centerX - width / 2.0f, centerY - height / 2.0f, width, height};
}

// Draws one era tab at a slot. Tabs away from the middle are darker, as if they stand
// in the shadow of the middle one.
void drawEraTab(const char* label, float slot, bool isSelected, const Textures& textures) {
    Rectangle bounds = getEraTabRect(slot);
    float distance = fabsf(slot) > 1.0f ? 1.0f : fabsf(slot);
    unsigned char light = static_cast<unsigned char>(255.0f - 140.0f * distance);
    Color shade = {light, light, light, 255};

    const Texture2D& texture = isSelected ? textures.buttonHover : textures.buttonNormal;
    if (texture.id != 0) {
        Rectangle source = {0.0f, 0.0f, static_cast<float>(texture.width),
                            static_cast<float>(texture.height)};
        DrawTexturePro(texture, source, bounds, Vector2{0.0f, 0.0f}, 0.0f, shade);
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

// Notices when the selected era changes. Sets previousEra to the era shown before the
// last switch and returns the seconds since that switch.
float getTimeSinceEraSwitch(int selectedEra, int& previousEra) {
    // static: the animation has to remember the last era and the switch time between
    // frames, and UiState (types.h) must not be changed in this task
    static int shownEra = 0;
    static int lastEra = 0;
    static double switchTime = 0.0;
    if (selectedEra != shownEra) {
        lastEra = shownEra == 0 ? selectedEra : shownEra;
        shownEra = selectedEra;
        switchTime = GetTime();
    }
    previousEra = lastEra;
    return static_cast<float>(GetTime() - switchTime);
}

// Moves a pin's selection a little towards selected or not selected each frame, so its
// ring fades and its size grows smoothly. Returns the selection from 0 to 1.
float getPinRingAlpha(int pin, bool isSelected) {
    // static: each pin's fade has to continue from where it was last frame
    static float ringAlpha[PINS_PER_ERA] = {0.0f, 0.0f, 0.0f};
    float step = GetFrameTime() * PIN_RING_FADE_SPEED;
    if (isSelected) {
        ringAlpha[pin] = ringAlpha[pin] + step > 1.0f ? 1.0f : ringAlpha[pin] + step;
    } else {
        ringAlpha[pin] = ringAlpha[pin] - step < 0.0f ? 0.0f : ringAlpha[pin] - step;
    }
    return ringAlpha[pin];
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
        ui.selectedItem = 0;
        return MAIN_MENU;
    }

    int rowCount = 3 + ui.playerCount;
    if (IsKeyPressed(KEY_UP)) {
        ui.isMouseActive = false;
        ui.selectedItem = moveSelection(ui.selectedItem, -1, rowCount);
    }
    if (IsKeyPressed(KEY_DOWN)) {
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
        ui.errorMessage = getSetupError(ui);
        if (ui.errorMessage.empty()) {
            int size = 0;
            parseWholeNumber(ui.typedText, size);
            ui.chosenSize = size;
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
    if (ui.isMouseActive) {
        ui.selectedItem =
            isPointerOverButton(pointer, BACK_BUTTON_X, BACK_BUTTON_Y, BUTTON_WIDTH,
                                BUTTON_HEIGHT)
                ? 0
                : -1;
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
        ui.characters[0] = getNextUnlockedHero(ui, -1);
    }
    if (IsKeyPressed(KEY_RIGHT)) {
        ui.characters[0] = getNextUnlockedHero(ui, 1);
    }

    if (IsKeyPressed(KEY_ENTER)) {
        int size = 0;
        if (parseWholeNumber(ui.typedText, size) && isValidSize(size, false)) {
            ui.chosenSize = size;
            ui.errorMessage = "";
            return PLAYING;
        }
        ui.errorMessage = "Enter a whole number from 10 to 50.";
        return LEVEL_INTRO;
    }

    if (IsKeyPressed(KEY_ESCAPE) || (isMouseClick && ui.selectedItem == 0)) {
        ui.selectedItem = 0;
        return CAMPAIGN_MAP;
    }
    return LEVEL_INTRO;
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
            return LEADERBOARD;
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
        ui.isMouseActive = false;
        ui.selectedItem = moveSelection(ui.selectedItem, -1, MENU_ITEM_COUNT);
        return MAIN_MENU;
    }
    if (IsKeyPressed(KEY_DOWN)) {
        ui.isMouseActive = false;
        ui.selectedItem = moveSelection(ui.selectedItem, 1, MENU_ITEM_COUNT);
        return MAIN_MENU;
    }
    if (IsKeyPressed(KEY_ENTER)) {
        ui.isMouseActive = false;
        if (ui.selectedItem < 0) {
            ui.selectedItem = 0;
        }
        return selectMenuItem(ui.selectedItem, ui);
    }
    if (isMouseClick && ui.selectedItem >= 0) {
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
            Vector2 center = {static_cast<float>(PIN_X[i]), static_cast<float>(PIN_Y)};
            if (CheckCollisionPointCircle(pointer, center, static_cast<float>(PIN_RADIUS))) {
                ui.selectedItem = i;
                break;
            }
        }
    }

    if (isMouseClick) {
        for (int era = 1; era <= ERA_COUNT; era++) {
            float slot = static_cast<float>(getEraSlot(era, ui.selectedEra));
            if (CheckCollisionPointRec(pointer, getEraTabRect(slot))) {
                ui.selectedEra = era;
                ui.selectedItem = 0;
                return CAMPAIGN_MAP;
            }
        }
        if (isPointerOverButton(pointer, BACK_BUTTON_X, BACK_BUTTON_Y, BUTTON_WIDTH,
                                BUTTON_HEIGHT)) {
            ui.selectedItem = 0;
            return MAIN_MENU;
        }
        return openSelectedPin(ui);
    }

    if (IsKeyPressed(KEY_LEFT)) {
        ui.isMouseActive = false;
        ui.selectedItem = moveSelection(ui.selectedItem, -1, PINS_PER_ERA);
        return CAMPAIGN_MAP;
    }
    if (IsKeyPressed(KEY_RIGHT)) {
        ui.isMouseActive = false;
        ui.selectedItem = moveSelection(ui.selectedItem, 1, PINS_PER_ERA);
        return CAMPAIGN_MAP;
    }
    // The eras go round in a circle: after Era III comes Era I again
    if (IsKeyPressed(KEY_UP)) {
        ui.isMouseActive = false;
        ui.selectedEra = ui.selectedEra % ERA_COUNT + 1;
        ui.selectedItem = 0;
        return CAMPAIGN_MAP;
    }
    if (IsKeyPressed(KEY_DOWN)) {
        ui.isMouseActive = false;
        ui.selectedEra = (ui.selectedEra + ERA_COUNT - 2) % ERA_COUNT + 1;
        ui.selectedItem = 0;
        return CAMPAIGN_MAP;
    }
    if (IsKeyPressed(KEY_ENTER)) {
        ui.isMouseActive = false;
        return openSelectedPin(ui);
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
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
        ui.selectedItem = 0;
        return MAIN_MENU;
    }
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN)) {
        ui.isMouseActive = false;
        ui.selectedItem = 0;
        return screen;
    }
    if (IsKeyPressed(KEY_ENTER)) {
        ui.isMouseActive = false;
        return MAIN_MENU;
    }
    if (isMouseClick && ui.selectedItem == 0) {
        return MAIN_MENU;
    }
    return screen;
}

// Prepares the name entry screen with the last accepted name. Returns NAME_ENTRY.
Screen openNameEntry(UiState& ui) {
    ui.typedText = ui.playerName;
    ui.errorMessage = "";
    ui.selectedItem = -1;
    return NAME_ENTRY;
}

// Handles typing, Backspace, Enter and the way back on the name entry screen. Returns
// MAIN_MENU for a valid name, TITLE for Esc or Back, otherwise NAME_ENTRY.
Screen updateNameEntry(UiState& ui) {
    Vector2 pointer = GetMousePosition();
    bool isMouseClick = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    ui.selectedItem =
        isPointerOverButton(pointer, BACK_BUTTON_X, BACK_BUTTON_Y, BUTTON_WIDTH,
                            BUTTON_HEIGHT)
            ? 0
            : -1;

    int key = GetCharPressed();
    while (key > 0) {
        if (key >= 32 && key <= 126 &&
            static_cast<int>(ui.typedText.size()) < MAX_NAME_LENGTH) {
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

    if (IsKeyPressed(KEY_ENTER)) {
        if (isValidName(ui.typedText)) {
            ui.playerName = ui.typedText;
            ui.errorMessage = "";
            ui.selectedItem = 0;
            return MAIN_MENU;
        }
        ui.errorMessage = "Use 1-12 letters or numbers, no spaces.";
        return NAME_ENTRY;
    }

    if (IsKeyPressed(KEY_ESCAPE) || (isMouseClick && ui.selectedItem == 0)) {
        ui.selectedItem = 0;
        return TITLE;
    }
    return NAME_ENTRY;
}

// Draws the name entry screen: heading, text input with its error, hint and Back.
void drawNameEntry(const Textures& textures, const UiState& ui) {
    drawScreenBackground(textures.menuBackground, Color{12, 10, 16, 255});
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{0, 0, 0, 165});

    const char* heading = "Enter your name";
    int headingWidth = MeasureText(heading, 38);
    DrawText(heading, (SCREEN_WIDTH - headingWidth) / 2, 200, 38, GOLD);

    drawTextInput(ui.typedText, (SCREEN_WIDTH - 400) / 2, 300, 400, true, ui.errorMessage);

    const char* hint = "Press Enter to continue";
    int hintWidth = MeasureText(hint, 24);
    DrawText(hint, (SCREEN_WIDTH - hintWidth) / 2, 420, 24, RAYWHITE);

    drawButtonStyle("Back", BACK_BUTTON_X, BACK_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
                    ui.selectedItem == 0, false, textures);
}

// Draws the title screen and its route into the main menu.
void drawTitleScreen(const Textures& textures, const UiState& ui) {
    drawScreenBackground(textures.titleScreen, Color{12, 10, 16, 255});
    DrawText("Press Enter to continue", 440, 560, 24, RAYWHITE);
    drawButtonStyle("Back", BACK_BUTTON_X, BACK_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
                    ui.selectedItem == 0, false, textures);
}

// Draws the main menu over the supplied corridor and logo art.
void drawMainMenu(const Textures& textures, const UiState& ui) {
    drawScreenBackground(textures.menuBackground, Color{12, 10, 16, 255});

    if (textures.menuLogo.id != 0) {
        Rectangle source = {0.0f, 0.0f, static_cast<float>(textures.menuLogo.width),
                            static_cast<float>(textures.menuLogo.height)};
        Rectangle destination = {
            static_cast<float>((SCREEN_WIDTH - LOGO_WIDTH) / 2), 40.0f,
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

// Draws the campaign connection point or a pending screen with a Back button.
void drawBackScreen(Screen screen, const Textures& textures, const UiState& ui) {
    drawScreenBackground(textures.menuBackground, Color{12, 10, 16, 255});
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{0, 0, 0, 165});

    const char* heading = "Campaign";
    const char* message = "Campaign maze is not connected yet.";
    if (screen == LEADERBOARD) {
        heading = "Leaderboard";
        message = "Leaderboard is pending.";
    } else if (screen == HOW_TO_PLAY) {
        heading = "How to Play";
        message = "How to Play is pending.";
    }

    int headingWidth = MeasureText(heading, 38);
    int messageWidth = MeasureText(message, 24);
    DrawText(heading, (SCREEN_WIDTH - headingWidth) / 2, 250, 38, GOLD);
    DrawText(message, (SCREEN_WIDTH - messageWidth) / 2, 330, 24, RAYWHITE);
    drawButtonStyle("Back", BACK_BUTTON_X, BACK_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
                    ui.selectedItem == 0, false, textures);
}

// Draws the level intro: place and year, era, difficulty, best result, the fun fact,
// the hero panel, the maze size input, a hint and the Back button.
void drawLevelIntro(const Textures& textures, const UiState& ui) {
    const Level& level = *getLevel(ui.selectedLevel);
    drawScreenBackground(textures.menuBackground, Color{12, 10, 16, 255});
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{0, 0, 0, 165});

    const char* heading = level.place.c_str();
    if (level.year != 0) {
        heading = TextFormat("%s - %d", level.place.c_str(), level.year);
    }
    int headingWidth = MeasureText(heading, 44);
    DrawText(heading, (SCREEN_WIDTH - headingWidth) / 2, 50, 44, GOLD);

    int eraWidth = MeasureText(level.era.c_str(), 24);
    DrawText(level.era.c_str(), (SCREEN_WIDTH - eraWidth) / 2, 110, 24, RAYWHITE);

    const char* difficulty = "Difficulty: Easy";
    if (level.difficulty == 2) {
        difficulty = "Difficulty: Medium";
    } else if (level.difficulty == 3) {
        difficulty = "Difficulty: Hard";
    }
    DrawText(difficulty, 140, 160, 24, RAYWHITE);

    Result best = {};
    if (getBestResult(ui.playerName, level.id, best)) {
        DrawText(TextFormat("Best: %d stars", best.stars), 140, 195, 24, GOLD);
    } else {
        DrawText("Not finished yet", 140, 195, 24, GRAY);
    }

    drawWrappedText(level.funFact, 140, 250, 1000, 26, RAYWHITE);

    const Texture2D& portrait = textures.heroes[ui.characters[0]].portrait;
    if (portrait.id != 0) {
        DrawTexture(portrait, 140, 380, WHITE);
    }
    const Character* hero = getCharacter(ui.characters[0]);
    DrawText(hero->name.c_str(), 290, 390, 28, GOLD);
    DrawText(hero->power.c_str(), 290, 430, 22, RAYWHITE);
    if (ui.characters[0] == getSuggestedHero(ui.selectedLevel)) {
        DrawText("(suggested)", 290, 465, 20, LIGHTGRAY);
    }
    DrawText("Left/Right: change hero", 290, 490, 20, LIGHTGRAY);

    DrawText("Maze size (10 to 50):", 760, 390, 24, RAYWHITE);
    drawTextInput(ui.typedText, 760, 430, 200, true, ui.errorMessage);

    const char* hint = "Enter: start   Esc: back";
    int hintWidth = MeasureText(hint, 22);
    DrawText(hint, (SCREEN_WIDTH - hintWidth) / 2, 545, 22, LIGHTGRAY);

    drawButtonStyle("Back", BACK_BUTTON_X, BACK_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
                    ui.selectedItem == 0, false, textures);
}

// Draws one campaign pin: grey when locked, orange when open, gold when finished,
// with its number, place name, status and a white ring when it is selected. The
// selected pin smoothly grows bigger than the others. alpha (0 to 1) fades the whole
// pin in after an era switch.
void drawCampaignPin(const UiState& ui, int pin, float alpha) {
    int levelId = getLevelIdOfPin(ui.selectedEra, pin);
    const Level* level = getLevel(levelId);
    int x = PIN_X[pin];
    bool isOpen = isPinOpen(ui, levelId);
    Result best = {};
    bool isFinished = getBestResult(ui.playerName, levelId, best);

    // selection goes from 0 (not selected) to 1 (selected) over a few frames
    float selection = getPinRingAlpha(pin, ui.selectedItem == pin);
    float scale = 1.0f + PIN_SELECTED_GROWTH * selection;
    // the text under the pin moves down as the pin grows, so they never overlap
    int textShift = static_cast<int>(90.0f * (scale - 1.0f));

    Color pinColor = Color{90, 90, 90, 255};
    if (isFinished) {
        pinColor = GOLD;
    } else if (isOpen) {
        pinColor = ORANGE;
    }
    if (isOpen || isFinished) {
        DrawCircle(x, PIN_Y, 50.0f * scale, Fade(pinColor, 0.35f * alpha));
    }
    DrawCircle(x, PIN_Y, PIN_RADIUS * scale, Fade(pinColor, alpha));

    const char* number = TextFormat("%d", levelId);
    int numberSize = static_cast<int>(28.0f * scale);
    int numberWidth = MeasureText(number, numberSize);
    DrawText(number, x - numberWidth / 2, PIN_Y - numberSize / 2, numberSize,
             Fade(BLACK, alpha));

    const char* place = level->place.c_str();
    int placeWidth = MeasureText(place, 22);
    DrawText(place, x - placeWidth / 2, PIN_Y + 66 + textShift, 22, Fade(RAYWHITE, alpha));

    if (isFinished) {
        const char* stars = TextFormat("Stars: %d", best.stars);
        int starsWidth = MeasureText(stars, 18);
        DrawText(stars, x - starsWidth / 2, PIN_Y + 96 + textShift, 18, Fade(GOLD, alpha));
    } else if (!isOpen) {
        int lockedWidth = MeasureText("Locked", 18);
        DrawText("Locked", x - lockedWidth / 2, PIN_Y + 96 + textShift, 18,
                 Fade(GRAY, alpha));
    }

    float ringAlpha = selection * alpha;
    if (ringAlpha > 0.0f) {
        // pulse goes smoothly from 0 to 1 and back, about once every 1.8 seconds
        float pulse = (sinf(static_cast<float>(GetTime()) * 3.5f) + 1.0f) / 2.0f;
        Vector2 center = {static_cast<float>(x), static_cast<float>(PIN_Y)};
        float outerRadius = (57.0f + pulse * 3.0f) * scale;
        DrawRing(center, outerRadius, outerRadius + 6.0f, 0.0f, 360.0f, 64,
                 Fade(RAYWHITE, 0.25f * (1.0f - pulse) * ringAlpha));
        DrawRing(center, outerRadius - 5.0f, outerRadius, 0.0f, 360.0f, 64,
                 Fade(RAYWHITE, (0.75f + 0.25f * pulse) * ringAlpha));
    }
}

// Draws the campaign map: era tabs, the era name, three pins, a hint and Back.
void drawCampaignMap(const Textures& textures, const UiState& ui) {
    drawScreenBackground(textures.menuBackground, Color{12, 10, 16, 255});
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{0, 0, 0, 165});

    int headingWidth = MeasureText("Campaign", 38);
    DrawText("Campaign", (SCREEN_WIDTH - headingWidth) / 2, 40, 38, GOLD);

    int previousEra = ui.selectedEra;
    float secondsSinceSwitch = getTimeSinceEraSwitch(ui.selectedEra, previousEra);
    float spinProgress = getEasedProgress(secondsSinceSwitch, ERA_SPIN_SECONDS);
    float fadeProgress = getEasedProgress(secondsSinceSwitch, PIN_FADE_SECONDS);

    for (int era = 1; era <= ERA_COUNT; era++) {
        int newSlot = getEraSlot(era, ui.selectedEra);
        int oldSlot = getEraSlot(era, previousEra);
        // Every tab moves one slot the same way; the tab that wraps around is moved
        // by -2 or +2 slots, which becomes the same one-slot move here
        int shift = oldSlot - newSlot;
        if (shift > 1) {
            shift -= ERA_COUNT;
        } else if (shift < -1) {
            shift += ERA_COUNT;
        }
        bool isSelected = era == ui.selectedEra;
        float slot =
            static_cast<float>(newSlot) + static_cast<float>(shift) * (1.0f - spinProgress);
        drawEraTab(ERA_TAB_LABELS[era - 1], slot, isSelected, textures);
        // The wrapping tab also slides out on the side it left, so it never jumps
        if (oldSlot != newSlot + shift && spinProgress < 1.0f) {
            float leavingSlot =
                static_cast<float>(oldSlot) - static_cast<float>(shift) * spinProgress;
            drawEraTab(ERA_TAB_LABELS[era - 1], leavingSlot, isSelected, textures);
        }
    }

    const char* eraName = getLevel(getLevelIdOfPin(ui.selectedEra, 0))->era.c_str();
    int eraWidth = MeasureText(eraName, 26);
    DrawText(eraName, (SCREEN_WIDTH - eraWidth) / 2, 200, 26, Fade(RAYWHITE, fadeProgress));

    for (int pin = 0; pin < PINS_PER_ERA; pin++) {
        drawCampaignPin(ui, pin, fadeProgress);
    }

    const char* hint =
        "Left/Right: choose maze   Up/Down: change era   Enter: open   Esc: back";
    int hintWidth = MeasureText(hint, 20);
    DrawText(hint, (SCREEN_WIDTH - hintWidth) / 2, 520, 20, LIGHTGRAY);

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
        if (isValidName(ui.playerName)) {
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
        ui.isMouseActive = false;
        ui.selectedItem = moveSelection(ui.selectedItem, -1, itemCount);
        return -1;
    }
    if (IsKeyPressed(KEY_DOWN)) {
        ui.isMouseActive = false;
        ui.selectedItem = moveSelection(ui.selectedItem, 1, itemCount);
        return -1;
    }
    if (IsKeyPressed(KEY_ENTER)) {
        ui.isMouseActive = false;
        if (ui.selectedItem < 0) {
            ui.selectedItem = 0;
        }
        return ui.selectedItem;
    }
    if (isMouseClick && ui.selectedItem >= 0) {
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
    drawPlayer(game, viewer, textures);
    EndScissorMode();
    drawHud(game, viewer, textures);
}

// Handles the pause screen. Returns PLAYING for Resume (or Esc) and Restart (after the
// game is freed, so a new maze starts), CAMPAIGN_MAP for Quit to map, otherwise PAUSED.
Screen updatePause(GameState& game, UiState& ui) {
    if (IsKeyPressed(KEY_ESCAPE)) {
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
        destroyGame(game);
        ui.selectedItem = 0;
        return CAMPAIGN_MAP;
    }
    return PAUSED;
}

// Draws the paused maze under a dark layer with the heading and the pause buttons.
void drawPause(const GameState& game, const Textures& textures, const UiState& ui) {
    drawGameplay(game, textures);
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{0, 0, 0, 165});
    int headingWidth = MeasureText("Paused", 38);
    DrawText("Paused", (SCREEN_WIDTH - headingWidth) / 2, 200, 38, GOLD);
    drawButtonMenu(textures, ui, PAUSE_ITEMS, PAUSE_ITEM_COUNT, PAUSE_BUTTON_Y);
}

// Handles the result screen. Returns PLAYING for Play again (after the game is freed,
// so a new maze of the same level starts), CAMPAIGN_MAP for Back to map or Esc,
// otherwise RESULT.
Screen updateResult(GameState& game, UiState& ui) {
    if (IsKeyPressed(KEY_ESCAPE)) {
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
    drawScreenBackground(textures.menuBackground, Color{12, 10, 16, 255});
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{0, 0, 0, 165});

    bool isWin = ui.resultStars > 0;
    const char* heading = isWin ? "Maze complete!" : "Your torch went out";
    int headingWidth = MeasureText(heading, 38);
    DrawText(heading, (SCREEN_WIDTH - headingWidth) / 2, 130, 38, isWin ? GOLD : RED);

    if (isWin) {
        const float starX[3] = {540.0f, 640.0f, 740.0f};
        for (int i = 0; i < 3; i++) {
            Color starColor = i + 1 <= ui.resultStars ? GOLD : Color{70, 60, 60, 255};
            DrawPoly(Vector2{starX[i], 250.0f}, 4, 36.0f, 45.0f, starColor);
        }

        int seconds = static_cast<int>(ui.resultSeconds);
        const char* timeText = TextFormat("Time: %02d:%02d", seconds / 60, seconds % 60);
        int timeWidth = MeasureText(timeText, 28);
        DrawText(timeText, (SCREEN_WIDTH - timeWidth) / 2, 330, 28, RAYWHITE);

        Result best = {};
        if (game.level != nullptr && getBestResult(ui.playerName, game.level->id, best)) {
            int bestSeconds = static_cast<int>(best.bestSeconds);
            const char* bestTime =
                TextFormat("Best time: %02d:%02d", bestSeconds / 60, bestSeconds % 60);
            int bestWidth = MeasureText(bestTime, 28);
            DrawText(bestTime, (SCREEN_WIDTH - bestWidth) / 2, 370, 28, RAYWHITE);
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
    drawScreenBackground(textures.menuBackground, Color{12, 10, 16, 255});
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{0, 0, 0, 165});

    int headingWidth = MeasureText("Multiplayer", 38);
    DrawText("Multiplayer", (SCREEN_WIDTH - headingWidth) / 2, 30, 38, GOLD);

    std::string arenaName = "Arena " + std::to_string(ui.arenaId);
    const Arena* arena = getArena(ui.arenaId);
    if (arena != nullptr) {
        arenaName = arena->name;
    }
    std::string arenaLabel = "Arena: < " + arenaName + " >";
    drawSetupLabel(arenaLabel.c_str(), 100, ui.selectedItem == 0);
    if (!isArenaOpen(ui, ui.arenaId)) {
        int arenaWidth = MeasureText(arenaLabel.c_str(), 26);
        DrawText(" (locked)", 300 + arenaWidth, 100, 26, GRAY);
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
        DrawText(ui.errorMessage.c_str(), (SCREEN_WIDTH - errorWidth) / 2, 515, 22, RED);
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

// Draws the black pass screen with the name of the player whose turn comes next.
void drawPassScreen(const GameState& game) {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);

    std::string message = "Pass the laptop.";
    if (game.players != nullptr && game.currentPlayer >= 0 &&
        game.currentPlayer < game.playerCount) {
        message = "Pass to " + game.players[game.currentPlayer].name + ".";
    }
    int messageWidth = MeasureText(message.c_str(), 44);
    DrawText(message.c_str(), (SCREEN_WIDTH - messageWidth) / 2, 290, 44, GOLD);

    int enterWidth = MeasureText("Press Enter.", 28);
    DrawText("Press Enter.", (SCREEN_WIDTH - enterWidth) / 2, 370, 28, RAYWHITE);
}

// Handles the multiplayer result screen. Returns PLAYING for Rematch (after the game is
// freed, so a new game with the same setup starts), MAIN_MENU for Menu or Esc,
// otherwise MP_RESULT.
Screen updateMultiplayerResult(GameState& game, UiState& ui) {
    if (IsKeyPressed(KEY_ESCAPE)) {
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
    drawScreenBackground(textures.menuBackground, Color{12, 10, 16, 255});
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{0, 0, 0, 165});

    if (game.players != nullptr && game.winner >= 0 && game.winner < game.playerCount) {
        const Player& winner = game.players[game.winner];
        std::string heading = winner.name + " wins!";
        int headingWidth = MeasureText(heading.c_str(), 38);
        DrawText(heading.c_str(), (SCREEN_WIDTH - headingWidth) / 2, 200, 38, GOLD);
        const Character* hero = getCharacter(winner.character);
        if (hero != nullptr) {
            int heroWidth = MeasureText(hero->name.c_str(), 28);
            DrawText(hero->name.c_str(), (SCREEN_WIDTH - heroWidth) / 2, 270, 28, RAYWHITE);
        }
    } else if (game.winner == DRAW) {
        const char* heading = "Draw: all torches went out";
        int headingWidth = MeasureText(heading, 38);
        DrawText(heading, (SCREEN_WIDTH - headingWidth) / 2, 200, 38, RED);
    }

    drawButtonMenu(textures, ui, MP_RESULT_ITEMS, MP_RESULT_ITEM_COUNT, MP_RESULT_BUTTON_Y);
}

// Handles the settings screen: switches Sound or Free Play and saves the settings.
// Returns MAIN_MENU for Back or Esc, otherwise SETTINGS.
Screen updateSettings(UiState& ui) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        ui.selectedItem = 0;
        return MAIN_MENU;
    }
    int choice = updateButtonMenu(ui, SETTINGS_ITEM_COUNT, SETTINGS_BUTTON_Y);
    if (choice == 0) {
        ui.settings.isSoundOn = !ui.settings.isSoundOn;
        saveSettings(ui.settings);
    } else if (choice == 1) {
        ui.settings.isFreePlay = !ui.settings.isFreePlay;
        saveSettings(ui.settings);
    } else if (choice == 2) {
        ui.selectedItem = 0;
        return MAIN_MENU;
    }
    return SETTINGS;
}

// Draws the settings screen: the heading, the Sound, Free Play and Back buttons, and a
// note about Free Play.
void drawSettings(const Textures& textures, const UiState& ui) {
    drawScreenBackground(textures.menuBackground, Color{12, 10, 16, 255});
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{0, 0, 0, 165});

    int headingWidth = MeasureText("Settings", 38);
    DrawText("Settings", (SCREEN_WIDTH - headingWidth) / 2, 200, 38, GOLD);

    // TextFormat reuses its buffers, so each label is kept in its own variable first
    const char* soundLabel = TextFormat("Sound: %s", ui.settings.isSoundOn ? "On" : "Off");
    const char* freePlayLabel =
        TextFormat("Free Play: %s", ui.settings.isFreePlay ? "On" : "Off");
    const char* const labels[SETTINGS_ITEM_COUNT] = {soundLabel, freePlayLabel, "Back"};
    drawButtonMenu(textures, ui, labels, SETTINGS_ITEM_COUNT, SETTINGS_BUTTON_Y);

    const char* note = "Free Play opens every maze, hero and arena. It does not save progress.";
    int noteWidth = MeasureText(note, 20);
    DrawText(note, (SCREEN_WIDTH - noteWidth) / 2, 560, 20, LIGHTGRAY);
}

}  // namespace

// Reads input and runs the logic of the current screen. Returns the screen to show
// next (the same one if nothing changed). Never draws.
Screen updateScreen(Screen screen, GameState& game, UiState& ui, float deltaTime) {
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
        if (IsKeyPressed(KEY_M)) {
            useMap(game.maze, game.players[0]);
        }
        updateCampaign(game, heldDirection, deltaTime);
        return checkLevelEnd(game, ui);
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

// Draws the current screen. Never reads input or changes the game.
void drawScreen(Screen screen, const GameState& game, const UiState& ui,
                const Textures& textures) {
    if (screen == TITLE) {
        drawTitleScreen(textures, ui);
    } else if (screen == NAME_ENTRY) {
        drawNameEntry(textures, ui);
    } else if (screen == PLAYING && game.players != nullptr && game.playerCount > 0) {
        drawGameplay(game, textures);
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
        drawPassScreen(game);
    } else if (screen == MP_RESULT) {
        drawMultiplayerResult(game, textures, ui);
    } else if (screen == LEADERBOARD || screen == HOW_TO_PLAY) {
        drawBackScreen(screen, textures, ui);
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
        DrawText(errorMessage.c_str(), x, y + 56, 20, RED);
    }
}
