#ifndef TYPES_H
#define TYPES_H

#include <string>

// ---------- Constants ----------

const int MIN_CAMPAIGN_SIZE = 10;
const int MIN_MULTIPLAYER_SIZE = 20;
const int MAX_SIZE = 50;
const int MIN_PLAYERS = 2;
const int MAX_PLAYERS = 4;
const int MAX_NAME_LENGTH = 12;
const int LEVEL_COUNT = 9;
const int ARENA_COUNT = 3;
const int CHARACTER_COUNT = 6;
const int MAX_RECORDS = 500;  // most lines read from a save file
const int LEADERBOARD_WINS = 5;  // players shown in the most-wins list

const int NO_TRAP = -1;     // Cell::trapOwner: no trap on this cell
const int LEVEL_TRAP = -2;  // Cell::trapOwner: trap placed by the level (campaign)
const int NO_WINNER = -1;   // GameState::winner: game still running
const int DRAW = -2;        // GameState::winner: all torches went out together

// ---------- Enums ----------

// What one square of the maze is made of
enum Tile { FLOOR, WALL, DOOR, WATER, EXIT };
enum Item { NO_ITEM, OIL, MAP_ITEM, KEY, TRAP_ITEM };
enum Direction { UP, DOWN, LEFT, RIGHT };
enum CharacterId { ASPARUH, KRUM, BORIS, SIMEON, IVAN_ASEN, LEVSKI };
enum Screen {
    TITLE,
    NAME_ENTRY,
    MAIN_MENU,
    CAMPAIGN_MAP,
    LEVEL_INTRO,
    PLAYING,
    PAUSED,
    RESULT,
    MP_SETUP,
    PASS_SCREEN,
    MP_RESULT,
    LEADERBOARD,
    HOW_TO_PLAY,
    SETTINGS
};
enum SoundId {
    SOUND_FOOTSTEP,
    SOUND_TORCH_CRACKLE,
    SOUND_LOW_TORCH,
    SOUND_PICKUP,
    SOUND_DOOR_UNLOCK,
    SOUND_TRAP_SNAP,
    SOUND_WIN,
    SOUND_TORCH_OUT,
    SOUND_CLICK,
    SOUND_TURN_CHIME
};

// ---------- Game structs ----------

struct Cell {
    Tile tile;
    Item item;
    int trapOwner;  // NO_TRAP, LEVEL_TRAP, or 0..3 = player index
};

struct Maze {
    int size;      // the maze is size x size
    Cell** cells;  // 2D dynamic array: cells[row][col]
};

struct Player {
    std::string name;
    CharacterId character;
    int row, col;
    float fuel, maxFuel;
    int maps, traps;
    bool hasKey;
    bool isOut;          // torch went out
    int turnsToSkip;     // multiplayer trap penalty
    float stuckSeconds;  // campaign trap penalty
    bool** seen;         // this player's memory of the maze: seen[row][col]
};

struct Level {
    int id;  // 1 to 9
    std::string place, era, funFact;
    int year;
    int difficulty;  // 1 easy, 2 medium, 3 hard
    float fuelFactor, oilPer100, trapsPer100;
    int doors;
    bool hasWater, isCold;
};

struct Arena {
    int id;  // 1 to 3
    std::string name;
    int era;  // 1 to 3, decides the look
    bool hasWater, isCold;
};

struct Character {
    CharacterId id;
    std::string name;
    int era;  // era in which the hero unlocks
    std::string power;
};

struct GameState {
    Maze maze;
    Player* players;  // 1D dynamic array (1 to 4 players)
    int playerCount, currentPlayer;
    const Level* level;
    bool isMultiplayer;
    float timeSeconds;
    int stepsLeft;  // multiplayer: steps left in this turn (3 at the start)
    float turnSecondsLeft;  // multiplayer: seconds left in this turn
    int turnNumber;  // multiplayer: 1 for the first turn, counts up after every turn
    int winner;     // player index, NO_WINNER or DRAW
};

// ---------- Save file records ----------

struct Result {
    std::string name;
    int levelId;
    float bestSeconds;
    int stars;
};

struct WinRecord {
    std::string name;
    int wins;
};

struct Settings {
    bool isSoundOn;
    bool isFreePlay;
};

// ---------- Screen state ----------

// Everything the screens remember between frames (typed text, selections...)
struct UiState {
    std::string playerName;
    std::string typedText;     // what is being typed in the current input
    std::string errorMessage;  // shown in red under the input, empty if none
    int selectedItem;          // highlighted menu entry
    float resultSeconds;       // time of the level that just ended
    int resultStars;           // stars of the level that just ended, 0 when the torch went out
    bool isMouseActive;
    bool isExitRequested;
    bool isMultiplayerGame;    // true when the game on screen is a multiplayer game
    int selectedEra;           // 1 to 3
    int selectedLevel;         // 1 to 9
    int chosenSize;            // maze size chosen on the level intro / MP setup
    int arenaId;               // multiplayer arena
    int playerCount;           // multiplayer players
    std::string playerNames[MAX_PLAYERS];
    CharacterId characters[MAX_PLAYERS];
    Settings settings;
    Result levelBest[LEVEL_COUNT];  // leaderboard: fastest result of each level
    bool hasLevelBest[LEVEL_COUNT];
    WinRecord topWins[LEADERBOARD_WINS];  // leaderboard: players with the most wins
    int topWinCount;
};

#endif  // TYPES_H
