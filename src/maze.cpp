#include "maze.h"

// Creates a size x size maze on the heap, every cell a WALL with no item and no trap
Maze createMaze(int size) {
    Maze maze;
    maze.size = size;
    maze.cells = new Cell*[size];  // list of row pointers
    for (int row = 0; row < size; row++) {
        maze.cells[row] = new Cell[size];  // one row of cells
        for (int col = 0; col < size; col++) {
            maze.cells[row][col].tile = WALL;
            maze.cells[row][col].item = NO_ITEM;
            maze.cells[row][col].trapOwner = NO_TRAP;
        }
    }
    return maze;
}

// Frees all memory of the maze. Safe to call twice: the second call does nothing.
void destroyMaze(Maze& maze) {
    for (int row = 0; row < maze.size; row++) {
        delete[] maze.cells[row];  // free each row
    }
    delete[] maze.cells;  // free the row list
    maze.cells = nullptr;
    maze.size = 0;
}

// Returns true if (row, col) is a cell of the maze
bool isInside(const Maze& maze, int row, int col) {
    return row >= 0 && row < maze.size && col >= 0 && col < maze.size;
}

int rowStep(Direction) { return 0; }

int colStep(Direction) { return 0; }

void shuffleDirections(Direction[4]) {}

void generatePaths(Maze&, int, int) {}

void connectExit(Maze&) {}

void placeFeatures(Maze&, const Level&) {}
