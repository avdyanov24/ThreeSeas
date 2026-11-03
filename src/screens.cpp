#include "screens.h"

#include "game.h"
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

// Opens the first playable campaign maze.
Screen openCampaign() {
    return PLAYING;
}

// Sets up the destination screen for a selected main-menu entry.
Screen selectMenuItem(int selectedItem, UiState& ui) {
    ui.selectedItem = 0;
    switch (selectedItem) {
        case 0:
            return openCampaign();
        case 1:
            return MP_SETUP;
        case 2:
            return LEADERBOARD;
        case 3:
            return HOW_TO_PLAY;
        case 4:
            return SETTINGS;
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
    if (screen == MP_SETUP) {
        heading = "Multiplayer";
        message = "Multiplayer is pending.";
    } else if (screen == LEADERBOARD) {
        heading = "Leaderboard";
        message = "Leaderboard is pending.";
    } else if (screen == HOW_TO_PLAY) {
        heading = "How to Play";
        message = "How to Play is pending.";
    } else if (screen == SETTINGS) {
        heading = "Settings";
        message = "Settings are pending.";
    }

    int headingWidth = MeasureText(heading, 38);
    int messageWidth = MeasureText(message, 24);
    DrawText(heading, (SCREEN_WIDTH - headingWidth) / 2, 250, 38, GOLD);
    DrawText(message, (SCREEN_WIDTH - messageWidth) / 2, 330, 24, RAYWHITE);
    drawButtonStyle("Back", BACK_BUTTON_X, BACK_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
                    ui.selectedItem == 0, false, textures);
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

// Handles the result screen: Enter plays the same level again on a new maze, Esc goes
// back to the campaign map and frees the game.
Screen updateResult(GameState& game, UiState& ui) {
    if (IsKeyPressed(KEY_ENTER)) {
        return PLAYING;
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        destroyGame(game);
        ui.selectedItem = 0;
        return CAMPAIGN_MAP;
    }
    return RESULT;
}

// Draws the result of the level: win with time, stars and best time, or torch out.
void drawResult(const GameState& game, const Textures& textures, const UiState& ui) {
    drawScreenBackground(textures.menuBackground, Color{12, 10, 16, 255});
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{0, 0, 0, 165});

    bool isWin = ui.resultStars > 0;
    const char* heading = isWin ? "Maze complete!" : "Your torch went out";
    int headingWidth = MeasureText(heading, 38);
    DrawText(heading, (SCREEN_WIDTH - headingWidth) / 2, 220, 38, isWin ? GOLD : RED);

    if (isWin) {
        int seconds = static_cast<int>(ui.resultSeconds);
        DrawText(TextFormat("Time: %02d:%02d", seconds / 60, seconds % 60), 500, 300, 28,
                 RAYWHITE);
        DrawText(TextFormat("Stars: %d", ui.resultStars), 500, 340, 28, GOLD);
        Result best = {};
        if (game.level != nullptr && getBestResult(ui.playerName, game.level->id, best)) {
            int bestSeconds = static_cast<int>(best.bestSeconds);
            DrawText(TextFormat("Best time: %02d:%02d", bestSeconds / 60, bestSeconds % 60),
                     500, 380, 28, RAYWHITE);
        }
    }

    const char* hint = "Enter: play again   Esc: back to the map";
    int hintWidth = MeasureText(hint, 22);
    DrawText(hint, (SCREEN_WIDTH - hintWidth) / 2, 500, 22, LIGHTGRAY);
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
            destroyGame(game);
            ui.selectedItem = 0;
            return CAMPAIGN_MAP;
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

    if (screen == RESULT) {
        return updateResult(game, ui);
    }

    if (screen == MAIN_MENU) {
        return updateMainMenu(ui);
    }

    if (screen == CAMPAIGN_MAP || screen == MP_SETUP || screen == LEADERBOARD ||
        screen == HOW_TO_PLAY || screen == SETTINGS) {
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
        const Player& viewer = game.players[game.currentPlayer];
        BeginScissorMode(0, HUD_HEIGHT, GetScreenWidth(), GetScreenHeight() - HUD_HEIGHT);
        drawMaze(game, viewer, textures);
        drawTorchLight(game, viewer, textures);
        drawPlayer(game, viewer, textures);
        EndScissorMode();
        drawHud(game, viewer, textures);
    } else if (screen == RESULT) {
        drawResult(game, textures, ui);
    } else if (screen == MAIN_MENU) {
        drawMainMenu(textures, ui);
    } else if (screen == CAMPAIGN_MAP || screen == MP_SETUP || screen == LEADERBOARD ||
               screen == HOW_TO_PLAY || screen == SETTINGS) {
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
