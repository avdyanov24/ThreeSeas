// QA test functions (T01-T24).
#include <iostream>
#include <string>
#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <fstream>

#include "game.h"
#include "maze.h"
#include "levels.h"
#include "validation.h"
#include "save.h"

// Prints a test result and increments failures when the test did not pass; returns nothing.
void check(const std::string& testName, bool passed, int& failures) {
    if (passed) {
        std::cout << "PASS " << testName << '\n';
    } else {
        std::cout << "FAIL " << testName << '\n';
        failures++;
    }
}

// Returns true if the maze exit can be reached from the start; returns false otherwise.
bool canReachExit(const Maze& maze) {
    if (maze.size <= 0 || maze.size > MAX_SIZE || maze.cells == nullptr) {
        return false;
    }

    bool visited[MAX_SIZE][MAX_SIZE];
    for (int row = 0; row < maze.size; row++) {
        for (int col = 0; col < maze.size; col++) {
            visited[row][col] = false;
        }
    }

    if (maze.cells[0][0].tile == WALL) {
        return false;
    }

    int stackRows[MAX_SIZE * MAX_SIZE];
    int stackCols[MAX_SIZE * MAX_SIZE];
    int stackSize = 0;
    stackRows[stackSize] = 0;
    stackCols[stackSize] = 0;
    stackSize++;
    visited[0][0] = true;

    Direction directions[4] = {UP, DOWN, LEFT, RIGHT};
    while (stackSize > 0) {
        stackSize--;
        int row = stackRows[stackSize];
        int col = stackCols[stackSize];

        if (row == maze.size - 1 && col == maze.size - 1) {
            return true;
        }

        for (int i = 0; i < 4; i++) {
            int nextRow = row + rowStep(directions[i]);
            int nextCol = col + colStep(directions[i]);
            if (isInside(maze, nextRow, nextCol) &&
                maze.cells[nextRow][nextCol].tile != WALL &&
                !visited[nextRow][nextCol]) {
                stackRows[stackSize] = nextRow;
                stackCols[stackSize] = nextCol;
                stackSize++;
                visited[nextRow][nextCol] = true;
            }
        }
    }

    return false;
}

// Allocates a small maze for movement tests and makes every cell a floor.
Maze createMovementTestMaze() {
    Maze maze = createMaze(5);
    for (int row = 0; row < maze.size; row++) {
        for (int col = 0; col < maze.size; col++) {
            maze.cells[row][col].tile = FLOOR;
        }
    }
    return maze;
}

// Checks campaign and multiplayer maze size limits; returns nothing.
void testSizeValidation(int& failures) {
    check("T01 campaign size 9 is rejected", !isValidSize(9, false), failures);
    check("T01 campaign size 51 is rejected", !isValidSize(51, false), failures);
    check("T01 campaign size 10 is accepted", isValidSize(10, false), failures);
    check("T01 campaign size 50 is accepted", isValidSize(50, false), failures);
    check("T01 multiplayer size 19 is rejected", !isValidSize(19, true), failures);
    check("T01 multiplayer size 51 is rejected", !isValidSize(51, true), failures);
    check("T01 multiplayer size 20 is accepted", isValidSize(20, true), failures);
    check("T01 multiplayer size 50 is accepted", isValidSize(50, true), failures);
}

// Checks whole-number parsing and preserves the number after invalid text; returns nothing.
void testWholeNumberParsing(int& failures) {
    int number = 7;
    check("T02 abc is rejected", !parseWholeNumber("abc", number), failures);
    check("T02 abc keeps the old number", number == 7, failures);
    check("T02 empty text is rejected", !parseWholeNumber("", number), failures);
    check("T02 empty text keeps the old number", number == 7, failures);
    check("T02 12a is rejected", !parseWholeNumber("12a", number), failures);
    check("T02 12a keeps the old number", number == 7, failures);
    check("T02 42 is accepted", parseWholeNumber("42", number), failures);
    check("T02 parsed number becomes 42", number == 42, failures);
}

// Checks accepted and rejected player names; returns nothing.
void testNameValidation(int& failures) {
    check("T03 empty name is rejected", !isValidName(""), failures);
    check("T03 13-character name is rejected", !isValidName("ABCDEFGHIJKLM"), failures);
    check("T03 12-character name is accepted", isValidName("ABCDEFGHIJKL"), failures);
    check("T03 name with a space is rejected", !isValidName("Ivan Petrov"), failures);
    check("T03 name with a hyphen is rejected", !isValidName("Ivan-1"), failures);
    check("T03 Ivan_1 is accepted", isValidName("Ivan_1"), failures);
}

// Checks maze dimensions, allocation, and wall initialization; returns nothing.
void testMazeSize(int& failures) {
    Maze maze = createMaze(10);
    check("T04 size 10 is recorded", maze.size == 10, failures);
    check("T04 size 10 cells are allocated", maze.cells != nullptr, failures);
    check("T04 size 10 first cell is a wall", maze.cells[0][0].tile == WALL, failures);
    check("T04 size 10 last cell is a wall",
          maze.cells[maze.size - 1][maze.size - 1].tile == WALL, failures);
    destroyMaze(maze);

    maze = createMaze(25);
    check("T04 size 25 is recorded", maze.size == 25, failures);
    check("T04 size 25 cells are allocated", maze.cells != nullptr, failures);
    check("T04 size 25 first cell is a wall", maze.cells[0][0].tile == WALL, failures);
    check("T04 size 25 last cell is a wall",
          maze.cells[maze.size - 1][maze.size - 1].tile == WALL, failures);
    destroyMaze(maze);

    maze = createMaze(50);
    check("T04 size 50 is recorded", maze.size == 50, failures);
    check("T04 size 50 cells are allocated", maze.cells != nullptr, failures);
    check("T04 size 50 first cell is a wall", maze.cells[0][0].tile == WALL, failures);
    check("T04 size 50 last cell is a wall",
          maze.cells[maze.size - 1][maze.size - 1].tile == WALL, failures);
    destroyMaze(maze);
}

// Checks that generated mazes have a walkable start and an exit; returns nothing.
void testStartAndExitAreFloor(int& failures) {
    int sizes[4] = {10, 11, 25, 50};
    for (int i = 0; i < 4; i++) {
        srand(1);
        Maze maze = createMaze(sizes[i]);
        generatePaths(maze, 0, 0);
        connectExit(maze);

        int last = maze.size - 1;
        std::string sizeLabel = "T05 size " + std::to_string(maze.size);
        check(sizeLabel + " start is floor", maze.cells[0][0].tile == FLOOR, failures);
        check(sizeLabel + " exit is EXIT", maze.cells[last][last].tile == EXIT, failures);
        check(sizeLabel + " start is not a wall", maze.cells[0][0].tile != WALL, failures);
        check(sizeLabel + " exit is not a wall", maze.cells[last][last].tile != WALL, failures);

        destroyMaze(maze);
    }
}

// Checks that the exit is reachable in generated mazes; returns nothing.
void testExitIsReachable(int& failures) {
    int sizes[4] = {10, 11, 25, 50};
    for (int i = 0; i < 4; i++) {
        srand(1);
        Maze maze = createMaze(sizes[i]);
        generatePaths(maze, 0, 0);
        connectExit(maze);

        std::string sizeLabel = "T06 size " + std::to_string(maze.size);
        check(sizeLabel + " exit is reachable", canReachExit(maze), failures);
        destroyMaze(maze);
    }
}

// Checks that moves out of each maze edge fail without moving the player.
void testOutOfBoundsMovement(int& failures) {
    Maze maze = createMovementTestMaze();

    Player player{};
    player.row = 0;
    player.col = 2;
    check("T07 top edge cell is floor", maze.cells[player.row][player.col].tile == FLOOR, failures);
    check("T07 moving up from top edge is rejected", !tryMove(maze, player, UP), failures);
    check("T07 top edge position is unchanged", player.row == 0 && player.col == 2, failures);

    player.row = 4;
    player.col = 2;
    check("T07 bottom edge cell is floor",
          maze.cells[player.row][player.col].tile == FLOOR, failures);
    check("T07 moving down from bottom edge is rejected", !tryMove(maze, player, DOWN), failures);
    check("T07 bottom edge position is unchanged", player.row == 4 && player.col == 2, failures);

    player.row = 2;
    player.col = 0;
    check("T07 left edge cell is floor",
          maze.cells[player.row][player.col].tile == FLOOR, failures);
    check("T07 moving left from left edge is rejected", !tryMove(maze, player, LEFT), failures);
    check("T07 left edge position is unchanged", player.row == 2 && player.col == 0, failures);

    player.row = 2;
    player.col = 4;
    check("T07 right edge cell is floor",
          maze.cells[player.row][player.col].tile == FLOOR, failures);
    check("T07 moving right from right edge is rejected", !tryMove(maze, player, RIGHT), failures);
    check("T07 right edge position is unchanged", player.row == 2 && player.col == 4, failures);

    destroyMaze(maze);
}

// Checks that wall moves fail unchanged and a legal move succeeds by one cell.
void testWallAndLegalMovement(int& failures) {
    Maze maze = createMovementTestMaze();
    maze.cells[2][2].tile = WALL;

    Player player{};
    player.row = 2;
    player.col = 1;
    check("T08 target cell is a wall", maze.cells[2][2].tile == WALL, failures);
    check("T08 moving into a wall is rejected", !tryMove(maze, player, RIGHT), failures);
    check("T08 wall move leaves the position unchanged",
          player.row == 2 && player.col == 1, failures);

    check("T08 legal target cell is floor", maze.cells[1][1].tile == FLOOR, failures);
    check("T08 moving to an adjacent floor succeeds", tryMove(maze, player, UP), failures);
    check("T08 legal move changes only the row by one",
          player.row == 1 && player.col == 1, failures);

    destroyMaze(maze);
}

// Checks every field in the nine level records and getLevel's boundary behavior.
void testLevelData(int& failures) {
    static const Level expectedLevels[] = {
        {1,
         "Pliska",
         "First Bulgarian State",
         "Pliska was Bulgaria's first capital, from 681 until 893.",
         681,
         1,
         4.0f,
         3.0f,
         0.0f,
         0,
         false,
         false},
        {2,
         "Varbitsa Pass",
         "First Bulgarian State",
         "In 811 Khan Krum trapped the Byzantine army here. Legend says he turned the emperor's "
         "skull "
         "into a silver cup.",
         811,
         2,
         3.5f,
         2.0f,
         2.0f,
         0,
         false,
         false},
        {3,
         "Preslav",
         "First Bulgarian State",
         "Under Simeon, Preslav became the heart of Bulgaria's Golden Age of books and learning.",
         893,
         3,
         3.0f,
         1.5f,
         1.0f,
         3,
         false,
         false},
        {4,
         "Tarnovo",
         "Second Bulgarian State",
         "In 1185 the brothers Asen and Peter rose up in Tarnovo and brought Bulgaria back to "
         "life.",
         1185,
         1,
         3.75f,
         3.0f,
         0.0f,
         2,
         false,
         false},
        {5,
         "Cherven",
         "Second Bulgarian State",
         "Cherven sat on a cliff above a river canyon. It had more medieval church inscriptions "
         "than "
         "the capital Tarnovo.",
         0,
         2,
         3.25f,
         2.0f,
         0.0f,
         0,
         true,
         false},
        {6,
         "Klokotnitsa",
         "Second Bulgarian State",
         "At Klokotnitsa in 1230, Ivan Asen II rode into battle with the broken peace treaty on "
         "his "
         "spear.",
         1230,
         3,
         2.75f,
         1.5f,
         2.0f,
         3,
         true,
         false},
        {7,
         "Koprivshtitsa",
         "Road to Liberation",
         "The first shot of the April Uprising was fired at a bridge in Koprivshtitsa on 20 April "
         "1876.",
         1876,
         1,
         3.5f,
         2.5f,
         1.0f,
         2,
         false,
         false},
        {8,
         "Plovdiv",
         "Road to Liberation",
         "On 6 September 1885 Bulgaria was united again, starting in Plovdiv.",
         1885,
         2,
         3.0f,
         2.0f,
         0.0f,
         2,
         true,
         false},
        {9,
         "Shipka",
         "Road to Liberation",
         "In 1877 Bulgarian volunteers held Shipka against a far bigger army, then survived a "
         "frozen "
         "winter on the peak.",
         1877,
         3,
         2.5f,
         1.5f,
         2.0f,
         3,
         true,
         true}};

    for (int i = 0; i < LEVEL_COUNT; i++) {
        const Level* actual = getLevel(i + 1);
        const Level& expected = expectedLevels[i];
        const bool matches = actual != nullptr && actual->id == expected.id &&
                             actual->place == expected.place && actual->era == expected.era &&
                             actual->funFact == expected.funFact && actual->year == expected.year &&
                             actual->difficulty == expected.difficulty &&
                             actual->fuelFactor == expected.fuelFactor &&
                             actual->oilPer100 == expected.oilPer100 &&
                             actual->trapsPer100 == expected.trapsPer100 &&
                             actual->doors == expected.doors &&
                             actual->hasWater == expected.hasWater &&
                             actual->isCold == expected.isCold;
        check("T21 level record " + std::to_string(i + 1) + " matches", matches, failures);
    }

    const Level* firstLevel = getLevel(1);
    const Level* lastLevel = getLevel(LEVEL_COUNT);
    check("T21 invalid ids return nullptr",
          getLevel(-1) == nullptr && getLevel(0) == nullptr && getLevel(LEVEL_COUNT + 1) == nullptr,
          failures);
    check("T21 returned pointers remain valid",
          firstLevel != nullptr && lastLevel != nullptr && firstLevel->place == "Pliska" &&
              lastLevel->place == "Shipka",
          failures);
}

// Checks real campaign setup, elapsed time, fuel, movement, memory and cleanup.
void testCampaignHudState(int& failures) {
    GameState game = createEmptyGame();
    check("HUD empty game has no winner", game.winner == NO_WINNER, failures);
    const Level& level = *getLevel(1);
    startLevel(game, level, 10, false);
    std::string names[1] = {"Player"};
    CharacterId characters[1] = {KRUM};
    createPlayers(game, 1, names, characters);
    Player& player = game.players[0];
    check("HUD campaign starts with level fuel and no elapsed time",
          player.fuel == 40.0f && player.maxFuel == 40.0f && game.timeSeconds == 0.0f,
          failures);
    check("HUD starting torch reveals nearby cells only",
          player.seen[0][0] && player.seen[0][3] && !player.seen[3][3], failures);
    game.maze.cells[0][1].tile = FLOOR;
    updateCampaign(game, RIGHT, 0.12f);
    check("HUD campaign movement advances time and burns real fuel",
          player.col == 1 && std::fabs(game.timeSeconds - 0.12f) < 0.001f &&
              std::fabs(player.fuel - 39.94f) < 0.001f && player.seen[0][4], failures);
    updateCampaign(game, -1, 1.0f);
    check("HUD idle campaign still advances time and fuel",
          player.col == 1 && std::fabs(game.timeSeconds - 1.12f) < 0.001f &&
              std::fabs(player.fuel - 39.44f) < 0.001f, failures);
    player.row = game.maze.size - 1;
    player.col = game.maze.size - 2;
    updateCampaign(game, RIGHT, 0.12f);
    check("HUD exit ends the campaign", game.winner == 0, failures);
    float finishTime = game.timeSeconds;
    float finishFuel = player.fuel;
    updateCampaign(game, -1, 2.0f);
    check("HUD finish freezes the displayed result",
          game.timeSeconds == finishTime && player.fuel == finishFuel, failures);
    startLevel(game, *getLevel(9), 50, false);
    characters[0] = ASPARUH;
    createPlayers(game, 1, names, characters);
    check("HUD restart replaces state and applies Asparuh fuel",
          game.maze.size == 50 && game.timeSeconds == 0.0f && game.winner == NO_WINNER &&
              game.players[0].maxFuel == 187.5f, failures);
    updateCampaign(game, -1, 1.0f);
    check("HUD cold campaign burns twice the normal fuel",
          game.players[0].fuel == 186.5f, failures);
    burnFuel(game.players[0], 1000.0f);
    float outTime = game.timeSeconds;
    updateCampaign(game, RIGHT, 1.0f);
    check("HUD empty torch stops movement and time",
          game.players[0].isOut && game.players[0].fuel == 0.0f &&
              game.players[0].col == 0 && game.timeSeconds == outTime, failures);
    destroyGame(game);
    destroyGame(game);
    check("HUD cleanup can run twice",
          game.players == nullptr && game.playerCount == 0 && game.maze.cells == nullptr &&
              game.maze.size == 0, failures);
}

// Checks the fuel thresholds used by the HUD and maze lighting.
void testTorchState(int& failures) {
    Player player = {};
    player.character = KRUM;
    player.maxFuel = 100.0f;
    player.fuel = 100.0f;
    check("T11 full torch lights three cells", getTorchRadius(player) == 3, failures);
    player.fuel = 60.0f;
    check("T11 sixty percent uses radius two", getTorchRadius(player) == 2, failures);
    player.fuel = 30.0f;
    check("T11 thirty percent uses radius two", getTorchRadius(player) == 2, failures);
    player.fuel = 29.0f;
    check("T11 low torch uses radius one", getTorchRadius(player) == 1, failures);
    player.character = BORIS;
    check("T11 Boris adds one radius cell", getTorchRadius(player) == 2, failures);
    check("T11 light circle excludes diagonals beyond its radius",
          isLit(player, 0, 2) && !isLit(player, 2, 2), failures);
    burnFuel(player, -1.0f);
    check("T12 negative fuel loss cannot add fuel", player.fuel == 29.0f, failures);
    burnFuel(player, 100.0f);
    check("T12 empty torch has no light, even for Boris",
          player.fuel == 0.0f && player.isOut && getTorchRadius(player) == 0 &&
              !isLit(player, 0, 0), failures);
}

// Checks door access for a player without a key, with a key, and IVAN_ASEN.
void testDoorAccess(int& failures) {
    Maze maze = createMovementTestMaze();
    maze.cells[2][2].tile = DOOR;

    Player withoutKey{};
    Player withKey{};
    withKey.hasKey = true;
    Player ivanAsen{};
    ivanAsen.character = IVAN_ASEN;

    check("T09 player without a key cannot enter a door",
          !canEnter(maze, withoutKey, 2, 2), failures);
    check("T09 player with a key can enter a door", canEnter(maze, withKey, 2, 2), failures);
    check("T09 IVAN_ASEN can enter a door without a key",
          canEnter(maze, ivanAsen, 2, 2), failures);

    destroyMaze(maze);
}

// Checks oil refills 25% of maximum fuel and caps fuel at the maximum.
void testOilRefill(int& failures) {
    GameState game = createEmptyGame();
    game.maze = createMovementTestMaze();

    Player player{};
    player.row = 0;
    player.col = 0;
    player.maxFuel = 100.0f;
    player.fuel = 50.0f;
    game.maze.cells[0][0].item = OIL;
    pickUpItem(game, player);
    check("T10 oil adds 25 fuel", player.fuel == 75.0f, failures);
    check("T10 oil item is removed", game.maze.cells[0][0].item == NO_ITEM, failures);

    game.maze.cells[0][0].item = OIL;
    player.fuel = 90.0f;
    pickUpItem(game, player);
    check("T10 oil refill does not exceed maximum fuel", player.fuel == 100.0f, failures);

    destroyMaze(game.maze);
}

// Checks torch radius at three fuel levels for a normal player and BORIS.
void testTorchRadius(int& failures) {
    Player player{};
    player.character = KRUM;
    player.maxFuel = 100.0f;
    player.fuel = 100.0f;
    check("T11 normal player at 100 fuel has radius 3", getTorchRadius(player) == 3, failures);
    player.fuel = 50.0f;
    check("T11 normal player at 50 fuel has radius 2", getTorchRadius(player) == 2, failures);
    player.fuel = 10.0f;
    check("T11 normal player at 10 fuel has radius 1", getTorchRadius(player) == 1, failures);

    player.character = BORIS;
    player.fuel = 100.0f;
    check("T11 BORIS at 100 fuel has radius 4", getTorchRadius(player) == 4, failures);
    player.fuel = 50.0f;
    check("T11 BORIS at 50 fuel has radius 3", getTorchRadius(player) == 3, failures);
    player.fuel = 10.0f;
    check("T11 BORIS at 10 fuel has radius 2", getTorchRadius(player) == 2, failures);
}

// Checks that burning the last fuel sets the torch-out state.
void testTorchOut(int& failures) {
    Player player{};
    player.fuel = 1.0f;
    player.maxFuel = 100.0f;
    burnFuel(player, 1.0f);
    check("T12 burning the last fuel sets fuel to zero", player.fuel == 0.0f, failures);
    check("T12 burning the last fuel sets torch out", player.isOut, failures);
}

// Checks campaign star ratings at the three documented time thresholds.
void testCampaignStars(int& failures) {
    check("T15 size 10 at 40 seconds earns 3 stars", calculateStars(10, 40.0f) == 3, failures);
    check("T15 size 10 at 70 seconds earns 2 stars", calculateStars(10, 70.0f) == 2, failures);
    check("T15 size 10 at 71 seconds earns 1 star", calculateStars(10, 71.0f) == 1, failures);
}

// Checks multiplayer trap effects for another player, the owner, and LEVSKI.
void testTrapEffects(int& failures) {
    GameState game = createEmptyGame();
    game.maze = createMovementTestMaze();
    game.isMultiplayer = true;
    Player players[3] = {};
    game.players = players;
    game.playerCount = 3;
    players[0].character = KRUM;
    players[1].character = KRUM;
    players[2].character = LEVSKI;

    game.maze.cells[1][1].tile = FLOOR;
    game.maze.cells[1][1].item = NO_ITEM;
    game.maze.cells[1][1].trapOwner = 0;
    players[1].row = 1;
    players[1].col = 1;
    pickUpItem(game, players[1]);
    check("T13 another player loses a turn to a trap", players[1].turnsToSkip == 1, failures);
    check("T13 sprung trap is removed",
          game.maze.cells[1][1].trapOwner == NO_TRAP, failures);

    game.maze.cells[1][1].trapOwner = 0;
    players[0].row = 1;
    players[0].col = 1;
    pickUpItem(game, players[0]);
    check("T13 trap owner does not lose a turn", players[0].turnsToSkip == 0, failures);
    check("T13 owner trap remains", game.maze.cells[1][1].trapOwner == 0, failures);

    players[2].row = 1;
    players[2].col = 1;
    pickUpItem(game, players[2]);
    check("T13 LEVSKI does not lose a turn", players[2].turnsToSkip == 0, failures);
    check("T13 LEVSKI leaves the trap in place",
          game.maze.cells[1][1].trapOwner == 0, failures);

    destroyMaze(game.maze);
}

// Checks that walking out of a water cell takes twice as long as out of a floor cell.
void testWaterSlowsWalking(int& failures) {
    GameState game = createEmptyGame();
    startLevel(game, *getLevel(1), 10, false);
    std::string names[1] = {"Player"};
    CharacterId characters[1] = {KRUM};
    createPlayers(game, 1, names, characters);
    Player& player = game.players[0];
    game.maze.cells[0][1].tile = WATER;
    game.maze.cells[0][2].tile = FLOOR;
    game.maze.cells[0][3].tile = FLOOR;
    player.row = 0;
    player.col = 1;
    updateCampaign(game, RIGHT, 0.12f);
    check("T14 leaving water takes longer than 0.12 s", player.col == 1, failures);
    updateCampaign(game, RIGHT, 0.12f);
    check("T14 leaving water works after 0.24 s", player.col == 2, failures);
    updateCampaign(game, RIGHT, 0.12f);
    check("T14 leaving floor takes 0.12 s", player.col == 3, failures);
    destroyGame(game);
}

// Checks that nextTurn skips players who are out or have a skipped turn.
void testNextPlayer(int& failures) {
    GameState game = createEmptyGame();
    Player players[3] = {};
    game.players = players;
    game.playerCount = 3;
    players[0].maxFuel = 100.0f;
    players[0].fuel = 50.0f;
    players[1].maxFuel = 100.0f;
    players[1].fuel = 50.0f;
    players[2].maxFuel = 100.0f;
    players[2].fuel = 50.0f;

    game.currentPlayer = 2;
    players[0].isOut = true;
    nextTurn(game);
    check("T16 next active player wraps past player 0", game.currentPlayer == 1, failures);
    check("T16 ending the turn burns one fuel", players[2].fuel == 49.0f, failures);

    players[0].isOut = true;
    players[1].turnsToSkip = 1;
    game.currentPlayer = 2;
    nextTurn(game);
    check("T16 skipped player returns selection to player 2", game.currentPlayer == 2, failures);
    check("T16 skipped player's penalty is cleared", players[1].turnsToSkip == 0, failures);
}

// Checks multiplayer winners for the exit, last burning player, draw, and ongoing game.
void testMultiplayerWinner(int& failures) {
    GameState game = createEmptyGame();
    game.maze = createMovementTestMaze();
    game.maze.cells[2][2].tile = EXIT;
    Player players[3] = {};
    game.players = players;
    game.playerCount = 3;
    players[0].maxFuel = 100.0f;
    players[0].fuel = 50.0f;
    players[0].row = 0;
    players[0].col = 0;
    players[1].maxFuel = 100.0f;
    players[1].fuel = 50.0f;
    players[1].row = 0;
    players[1].col = 0;
    players[2].maxFuel = 100.0f;
    players[2].fuel = 50.0f;
    players[2].row = 0;
    players[2].col = 0;

    players[1].row = 2;
    players[1].col = 2;
    checkWinner(game);
    check("T17 player on the exit wins", game.winner == 1, failures);

    game.winner = NO_WINNER;
    players[1].row = 0;
    players[1].col = 0;
    players[0].isOut = true;
    players[2].isOut = true;
    checkWinner(game);
    check("T17 last player with fuel wins", game.winner == 1, failures);

    game.winner = NO_WINNER;
    players[0].isOut = true;
    players[1].isOut = true;
    players[2].isOut = true;
    checkWinner(game);
    check("T17 all torches out results in a draw", game.winner == DRAW, failures);

    game.winner = NO_WINNER;
    players[0].isOut = false;
    players[1].isOut = false;
    players[2].isOut = false;
    checkWinner(game);
    check("T17 game continues when no player reaches the exit",
          game.winner == NO_WINNER, failures);

    destroyMaze(game.maze);
}

// Checks result saving, loading, missing-file handling, and keeping the best time.
void testSaveAndLoadResults(int& failures) {
    std::remove("save/results.txt");
    Result results[MAX_RECORDS] = {};
    int count = {};
    count = loadResults(results, MAX_RECORDS);
    check("T18 missing results file loads zero records", count == 0, failures);

    check("T18 first result saves successfully",
          saveResult({"Ivan", 3, 84.5f, 2}), failures);
    count = loadResults(results, MAX_RECORDS);
    check("T18 saved result loads one record", count == 1, failures);
    check("T18 loaded result has the saved name", results[0].name == "Ivan", failures);
    check("T18 loaded result has the saved level", results[0].levelId == 3, failures);
    check("T18 loaded result has the saved time", results[0].bestSeconds == 84.5f, failures);
    check("T18 loaded result has the saved stars", results[0].stars == 2, failures);

    check("T18 slower result saves successfully",
          saveResult({"Ivan", 3, 90.0f, 1}), failures);
    count = loadResults(results, MAX_RECORDS);
    check("T18 slower result keeps one record", count == 1, failures);
    check("T18 slower result preserves the best time", results[0].bestSeconds == 84.5f, failures);
    check("T18 faster result saves successfully", saveResult({"Ivan", 3, 70.0f, 1}), failures);
    count = loadResults(results, MAX_RECORDS);
    check("T18 faster result replaces the best time", results[0].bestSeconds == 70.0f, failures);
    check("T18 faster result never lowers the stars", results[0].stars == 2, failures);
    std::remove("save/results.txt");
}

// Checks destroyMaze clears the maze pointer and size.
void testDestroyMaze(int& failures) {
    Maze maze = createMaze(10);
    destroyMaze(maze);
    check("T19 destroyMaze clears cells", maze.cells == nullptr, failures);
    check("T19 destroyMaze clears size", maze.size == 0, failures);
}

// Checks level, character, and arena unlock rules with Free Play.
void testUnlocks(int& failures) {
    check("T20 level 2 is locked without a prior result",
          !isLevelUnlocked(2, false, false), failures);
    check("T20 KRUM is locked before era 1 is entered",
          !isCharacterUnlocked(KRUM, 0, false), failures);
    check("T20 arena 1 is open from the start", isArenaUnlocked(1, 0, false), failures);
    check("T20 arena 2 is locked before an era is finished",
          !isArenaUnlocked(2, 0, false), failures);
    check("T20 arena 3 is locked after one era", !isArenaUnlocked(3, 1, false), failures);
    check("T20 Free Play unlocks level 2", isLevelUnlocked(2, false, true), failures);
    check("T20 Free Play unlocks KRUM", isCharacterUnlocked(KRUM, 0, true), failures);
    check("T20 Free Play unlocks arena 1", isArenaUnlocked(1, 0, true), failures);
    check("T20 prior result unlocks level 2", isLevelUnlocked(2, true, false), failures);
    check("T20 entering era 1 unlocks KRUM", isCharacterUnlocked(KRUM, 1, false), failures);
    check("T20 finishing era 1 unlocks arena 2", isArenaUnlocked(2, 1, false), failures);
    check("T20 finishing era 2 unlocks arena 3", isArenaUnlocked(3, 2, false), failures);
}

// Checks placeFeatures: start and exit stay clear, one key, doors only between floors, water,
// level traps away from the start, nothing on the spawn corners, and the exit stays reachable.
void testPlaceFeatures(int& failures) {
    srand(22);
    const Level* level = nullptr;
    for (int id = 1; id <= LEVEL_COUNT; id++) {
        const Level* candidate = getLevel(id);
        if (candidate != nullptr && candidate->doors > 0 && candidate->hasWater) {
            level = candidate;
            break;
        }
    }
    check("T22 a level with doors and water exists", level != nullptr, failures);
    if (level == nullptr) {
        return;
    }

    Maze maze = createMaze(30);
    generatePaths(maze, 0, 0);
    connectExit(maze);
    placeFeatures(maze, *level);

    int keys = 0;
    int doors = 0;
    int water = 0;
    int levelTraps = 0;
    int trapsNearStart = 0;
    int badDoors = 0;
    for (int row = 0; row < maze.size; row++) {
        for (int col = 0; col < maze.size; col++) {
            const Cell& cell = maze.cells[row][col];
            if (cell.item == KEY) {
                keys++;
            }
            if (cell.tile == WATER) {
                water++;
            }
            if (cell.trapOwner == LEVEL_TRAP) {
                levelTraps++;
                if (row + col < 3) {
                    trapsNearStart++;
                }
            }
            if (cell.tile == DOOR) {
                doors++;
                bool isOpenVertically = row > 0 && row < maze.size - 1 &&
                                        maze.cells[row - 1][col].tile != WALL &&
                                        maze.cells[row - 1][col].tile != DOOR &&
                                        maze.cells[row + 1][col].tile != WALL &&
                                        maze.cells[row + 1][col].tile != DOOR;
                bool isOpenHorizontally = col > 0 && col < maze.size - 1 &&
                                          maze.cells[row][col - 1].tile != WALL &&
                                          maze.cells[row][col - 1].tile != DOOR &&
                                          maze.cells[row][col + 1].tile != WALL &&
                                          maze.cells[row][col + 1].tile != DOOR;
                if (!isOpenVertically && !isOpenHorizontally) {
                    badDoors++;
                }
            }
        }
    }

    int itemsOnCorners = 0;
    for (int index = 0; index < 4; index++) {
        int row = 0;
        int col = 0;
        getSpawnCell(maze, index, row, col);
        if (maze.cells[row][col].item != NO_ITEM || maze.cells[row][col].trapOwner != NO_TRAP) {
            itemsOnCorners++;
        }
    }

    check("T22 start cell is floor with no item",
          maze.cells[0][0].tile == FLOOR && maze.cells[0][0].item == NO_ITEM, failures);
    check("T22 exit cell is still EXIT", maze.cells[29][29].tile == EXIT, failures);
    check("T22 level with doors has exactly one key", keys == 1, failures);
    check("T22 doors are placed and not more than the level asks",
          doors > 0 && doors <= level->doors, failures);
    check("T22 every door has floor on two opposite sides", badDoors == 0, failures);
    check("T22 water is placed", water > 0, failures);
    check("T22 level traps are placed", levelTraps > 0, failures);
    check("T22 no trap next to the start", trapsNearStart == 0, failures);
    check("T22 nothing lies on the spawn corners", itemsOnCorners == 0, failures);
    check("T22 exit is reachable after placing features", canReachExit(maze), failures);
    destroyMaze(maze);

    const Level* plainLevel = getLevel(1);
    Maze plain = createMaze(20);
    generatePaths(plain, 0, 0);
    connectExit(plain);
    placeFeatures(plain, *plainLevel);
    int plainKeys = 0;
    int plainDoors = 0;
    int plainWater = 0;
    for (int row = 0; row < plain.size; row++) {
        for (int col = 0; col < plain.size; col++) {
            if (plain.cells[row][col].item == KEY) {
                plainKeys++;
            }
            if (plain.cells[row][col].tile == DOOR) {
                plainDoors++;
            }
            if (plain.cells[row][col].tile == WATER) {
                plainWater++;
            }
        }
    }
    check("T22 level without doors has no key and no door", plainKeys == 0 && plainDoors == 0,
          failures);
    check("T22 level without water has no water", plainWater == 0, failures);
    destroyMaze(plain);
}

// Checks the multiplayer start: one corner per player, a random exit far from every corner,
// and the starting fuel and traps of the players.
void testMultiplayerStart(int& failures) {
    srand(23);
    Maze maze = createMaze(30);
    int cornerRows[4] = {};
    int cornerCols[4] = {};
    for (int index = 0; index < 4; index++) {
        getSpawnCell(maze, index, cornerRows[index], cornerCols[index]);
    }
    check("T23 player 0 starts top-left", cornerRows[0] == 0 && cornerCols[0] == 0, failures);
    check("T23 player 1 starts bottom-right",
          cornerRows[1] == 28 && cornerCols[1] == 28, failures);
    check("T23 player 2 starts top-right", cornerRows[2] == 0 && cornerCols[2] == 28, failures);
    check("T23 player 3 starts bottom-left", cornerRows[3] == 28 && cornerCols[3] == 0, failures);
    destroyMaze(maze);

    bool isExitAlwaysSingle = true;
    bool isExitAlwaysFar = true;
    for (int attempt = 0; attempt < 5; attempt++) {
        Maze arena = createMaze(30);
        generatePaths(arena, 0, 0);
        placeRandomExit(arena);
        int exits = 0;
        for (int row = 0; row < arena.size; row++) {
            for (int col = 0; col < arena.size; col++) {
                if (arena.cells[row][col].tile != EXIT) {
                    continue;
                }
                exits++;
                for (int index = 0; index < 4; index++) {
                    int distance = std::abs(row - cornerRows[index]) +
                                   std::abs(col - cornerCols[index]);
                    if (distance < arena.size / 3) {
                        isExitAlwaysFar = false;
                    }
                }
            }
        }
        if (exits != 1) {
            isExitAlwaysSingle = false;
        }
        destroyMaze(arena);
    }
    check("T23 there is exactly one exit", isExitAlwaysSingle, failures);
    check("T23 the exit is at least size / 3 from every corner", isExitAlwaysFar, failures);

    GameState game = createEmptyGame();
    startLevel(game, *getLevel(1), 30, true);
    std::string names[4] = {"Ana", "Boris", "Cvetan", "Dimo"};
    CharacterId characters[4] = {ASPARUH, KRUM, BORIS, SIMEON};
    createPlayers(game, 4, names, characters);
    bool isEveryoneInOwnCorner = game.playerCount == 4;
    bool hasEveryoneOneTrap = true;
    for (int index = 0; index < game.playerCount; index++) {
        if (game.players[index].row != cornerRows[index] ||
            game.players[index].col != cornerCols[index]) {
            isEveryoneInOwnCorner = false;
        }
        if (game.players[index].traps != 1) {
            hasEveryoneOneTrap = false;
        }
    }
    check("T23 every player starts in their own corner", isEveryoneInOwnCorner, failures);
    check("T23 every multiplayer player starts with one trap", hasEveryoneOneTrap, failures);
    check("T23 fuel is size * 2", game.players[1].maxFuel == 60.0f, failures);
    check("T23 Asparuh gets 50 percent more fuel", game.players[0].maxFuel == 90.0f, failures);
    destroyGame(game);
}

// Checks the time of a multiplayer turn and the player count and hero checks.
void testTurnTimeAndSetup(int& failures) {
    check("T24 size 20 gives 12 seconds", getTurnSeconds(1, 2, 20) == 12.0f, failures);
    check("T24 size 50 gives 18 seconds", getTurnSeconds(1, 2, 50) == 18.0f, failures);
    check("T24 a full round takes one second away", getTurnSeconds(3, 2, 20) == 11.0f, failures);
    check("T24 still the start time inside the first round", getTurnSeconds(2, 2, 20) == 12.0f,
          failures);
    check("T24 time never drops below half", getTurnSeconds(100, 2, 20) == 6.0f, failures);

    check("T24 1 player is not allowed", !isValidPlayerCount(1), failures);
    check("T24 2 players are allowed", isValidPlayerCount(2), failures);
    check("T24 4 players are allowed", isValidPlayerCount(4), failures);
    check("T24 5 players are not allowed", !isValidPlayerCount(5), failures);

    Player players[3] = {};
    players[0].character = KRUM;
    players[1].character = BORIS;
    players[2].character = LEVSKI;
    check("T24 a chosen hero is taken", isCharacterTaken(players, 2, BORIS), failures);
    check("T24 a free hero is not taken", !isCharacterTaken(players, 2, SIMEON), failures);
    check("T24 players after the count are ignored", !isCharacterTaken(players, 2, LEVSKI),
          failures);
    check("T24 nobody has a hero when the count is 0", !isCharacterTaken(players, 0, KRUM),
          failures);
}

// Checks the multiplayer turn: walking, slower water, a trap and the time end the turn, and
// the next turn gets its own seconds.
void testTurnRules(int& failures) {
    GameState game = createEmptyGame();
    startLevel(game, *getLevel(1), 10, true);
    std::string names[2] = {"Ana", "Boris"};
    CharacterId characters[2] = {ASPARUH, KRUM};
    createPlayers(game, 2, names, characters);
    game.turnNumber = 1;
    game.turnSecondsLeft = getTurnSeconds(1, 2, 10);
    for (int col = 1; col <= 4; col++) {
        game.maze.cells[0][col] = {FLOOR, NO_ITEM, NO_TRAP};
    }
    Player& player = game.players[0];

    bool isTurnOver = updateTurn(game, RIGHT, 0.12f);
    check("T25 holding an arrow walks one cell", player.col == 1 && !isTurnOver, failures);
    check("T25 the turn time counts down",
          std::fabs(game.turnSecondsLeft - 9.88f) < 0.001f, failures);

    game.maze.cells[0][1].tile = WATER;
    game.timeSeconds = 0.0f;
    updateTurn(game, RIGHT, 0.12f);
    check("T25 leaving water takes longer than 0.12 s", player.col == 1, failures);
    updateTurn(game, RIGHT, 0.12f);
    check("T25 leaving water works after 0.24 s", player.col == 2, failures);

    game.maze.cells[0][3].trapOwner = 1;
    game.timeSeconds = 0.0f;
    isTurnOver = updateTurn(game, RIGHT, 0.12f);
    check("T25 another player's trap ends the turn",
          isTurnOver && player.col == 3 && player.turnsToSkip == 1, failures);

    game.turnSecondsLeft = 0.05f;
    check("T25 the turn ends when the time runs out", updateTurn(game, -1, 0.1f), failures);

    startNextTurn(game);
    check("T25 the next player takes the turn", game.currentPlayer == 1, failures);
    check("T25 the next turn gets its own seconds",
          game.turnNumber == 2 && game.turnSecondsLeft == getTurnSeconds(2, 2, 10), failures);
    destroyGame(game);
}

// Checks the Map and Trap items and the arena and hero tables.
void testItemsAndData(int& failures) {
    GameState game = createEmptyGame();
    startLevel(game, *getLevel(1), 10, true);
    std::string names[2] = {"Ana", "Boris"};
    CharacterId characters[2] = {ASPARUH, KRUM};
    createPlayers(game, 2, names, characters);
    Player& player = game.players[1];

    player.maps = 1;
    check("T26 a far cell is not seen before the map", !player.seen[5][9], failures);
    useMap(game.maze, player);
    check("T26 the map reveals 4 cells around the player", player.seen[5][9], failures);
    check("T26 using a map spends it", player.maps == 0, failures);
    player.seen[5][9] = false;
    useMap(game.maze, player);
    check("T26 without a map nothing is revealed", !player.seen[5][9], failures);

    check("T26 a trap can be placed", placeTrap(game.maze, player, 1), failures);
    check("T26 the trap belongs to the player",
          game.maze.cells[player.row][player.col].trapOwner == 1 && player.traps == 0,
          failures);
    check("T26 no trap is placed without one", !placeTrap(game.maze, player, 1), failures);
    destroyGame(game);

    check("T26 arena 1 is Pliska", getArena(1) != nullptr && getArena(1)->name == "Pliska",
          failures);
    check("T26 arena 3 is cold", getArena(3) != nullptr && getArena(3)->isCold, failures);
    check("T26 arena ids outside 1 to 3 are rejected",
          getArena(0) == nullptr && getArena(4) == nullptr, failures);
    check("T26 Levski unlocks in era III",
          getCharacter(LEVSKI) != nullptr && getCharacter(LEVSKI)->era == 3, failures);
}

// Checks that the settings and the three volumes are saved and loaded, and that a
// missing file or a volume out of range gives the defaults.
void testSettingsAndVolumes(int& failures) {
    std::remove("save/settings.txt");
    Settings settings = loadSettings();
    check("T27 a missing file gives the default volumes",
          settings.menuMusicVolume == 5 && settings.gameMusicVolume == 4 &&
              settings.effectsVolume == 8 && settings.isSoundOn && !settings.isFreePlay,
          failures);

    settings.isSoundOn = false;
    settings.menuMusicVolume = 0;
    settings.gameMusicVolume = 10;
    settings.effectsVolume = 3;
    check("T27 settings save successfully", saveSettings(settings), failures);
    Settings loaded = loadSettings();
    check("T27 saved volumes load back",
          loaded.menuMusicVolume == 0 && loaded.gameMusicVolume == 10 &&
              loaded.effectsVolume == 3 && !loaded.isSoundOn,
          failures);

    std::ofstream broken("save/settings.txt");
    broken << "menumusic 11\neffects -1\ngamemusic 7\n";
    broken.close();
    loaded = loadSettings();
    check("T27 volumes out of range are ignored",
          loaded.menuMusicVolume == 5 && loaded.effectsVolume == 8 && loaded.gameMusicVolume == 7,
          failures);
    std::remove("save/settings.txt");
}

// Runs every test and returns 0 only if every check passed.
int main() {
    int failures = 0;
    testSizeValidation(failures);
    testWholeNumberParsing(failures);
    testNameValidation(failures);
    testMazeSize(failures);
    testStartAndExitAreFloor(failures);
    testExitIsReachable(failures);
    testOutOfBoundsMovement(failures);
    testWallAndLegalMovement(failures);
    testLevelData(failures);
    testCampaignHudState(failures);
    testTorchState(failures);
    testDoorAccess(failures);
    testOilRefill(failures);
    testTorchRadius(failures);
    testTorchOut(failures);
    testCampaignStars(failures);
    testTrapEffects(failures);
    testWaterSlowsWalking(failures);
    testNextPlayer(failures);
    testMultiplayerWinner(failures);
    testSaveAndLoadResults(failures);
    testDestroyMaze(failures);
    testUnlocks(failures);
    testPlaceFeatures(failures);
    testMultiplayerStart(failures);
    testTurnTimeAndSetup(failures);
    testTurnRules(failures);
    testItemsAndData(failures);
    testSettingsAndVolumes(failures);

    if (failures == 0) {
        std::cout << "All tests passed\n";
    } else {
        std::cout << failures << " checks failed\n";
    }

    if (failures == 0) {
        return 0;
    }

    return 1;
}
