// QA test functions (T01-T20).
#include <iostream>
#include <string>
#include <cstdlib>

#include "maze.h"
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

// Prints the test summary and returns failure when any check fails.
int main() {
	int failures = 0;
    testSizeValidation(failures);
    testWholeNumberParsing(failures);
    testNameValidation(failures);
    testMazeSize(failures);
    testStartAndExitAreFloor(failures);
    testExitIsReachable(failures);

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
