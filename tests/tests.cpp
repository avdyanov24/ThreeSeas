// QA test functions (T01-T20).
#include <iostream>
#include <string>
#include <cstdlib>

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

// Prints the test summary and returns failure when any check fails.
int main() {
	int failures = 0;
    testSizeValidation(failures);
    testWholeNumberParsing(failures);
    testNameValidation(failures);
    testMazeSize(failures);
    testStartAndExitAreFloor(failures);
    testExitIsReachable(failures);
    testLevelData(failures);

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
