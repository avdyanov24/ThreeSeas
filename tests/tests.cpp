// QA test functions (T01-T20).
#include <iostream>
#include <string>
#include <cstdlib>
#include <cmath>

#include "game.h"
#include "maze.h"
#include "levels.h"
#include "validation.h"

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
    check("T07 left edge cell is floor", maze.cells[player.row][player.col].tile == FLOOR, failures);
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
         "In 811 Khan Krum trapped the Byzantine army here. Legend says he turned the emperor's skull "
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
         "In 1185 the brothers Asen and Peter rose up in Tarnovo and brought Bulgaria back to life.",
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
         "Cherven sat on a cliff above a river canyon. It had more medieval church inscriptions than "
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
         "At Klokotnitsa in 1230, Ivan Asen II rode into battle with the broken peace treaty on his "
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
         "The first shot of the April Uprising was fired at a bridge in Koprivshtitsa on 20 April 1876.",
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
         "In 1877 Bulgarian volunteers held Shipka against a far bigger army, then survived a frozen "
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

// Checks multiplayer step cost for a water cell and a floor cell.
void testWaterStepCost(int& failures) {
    Maze maze = createMovementTestMaze();
    maze.cells[2][2].tile = WATER;
    check("T14 water costs two steps", getStepCost(maze, 2, 2) == 2, failures);
    check("T14 floor costs one step", getStepCost(maze, 1, 1) == 1, failures);
    destroyMaze(maze);
}

// Prints the test summary and returns failure when any check fails.
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
    testWaterStepCost(failures);

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
