#include "maze.h"

#include <cstdlib>

// Creates a size x size maze on the heap, every cell a WALL with no item and no trap
Maze createMaze(int size) {
    Maze maze;
    maze.size = size;
    maze.cells = new Cell*[size];
    for (int row = 0; row < size; row++) {
        maze.cells[row] = new Cell[size];
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
        delete[] maze.cells[row];
    }
    delete[] maze.cells;
    maze.cells = nullptr;
    maze.size = 0;
}

// Returns true if (row, col) is a cell of the maze
bool isInside(const Maze& maze, int row, int col) {
    return row >= 0 && row < maze.size && col >= 0 && col < maze.size;
}

// Returns the row change for a direction: -1 for UP, 1 for DOWN, otherwise 0
int rowStep(Direction direction) {
    if (direction == UP) {
        return -1;
    }
    if (direction == DOWN) {
        return 1;
    }
    return 0;
}

// Returns the column change for a direction: -1 for LEFT, 1 for RIGHT, otherwise 0
int colStep(Direction direction) {
    if (direction == LEFT) {
        return -1;
    }
    if (direction == RIGHT) {
        return 1;
    }
    return 0;
}

// Puts the 4 directions in a random order (Fisher-Yates shuffle)
void shuffleDirections(Direction order[4]) {
    for (int i = 3; i > 0; i--) {
        int j = rand() % (i + 1);
        Direction saved = order[i];
        order[i] = order[j];
        order[j] = saved;
    }
}

// Carves paths from the room (row, col) with the recursive backtracker.
// Rooms are the cells with even coordinates; the cells between two rooms are walls
// that get knocked down when we walk from one room to the next.
void generatePaths(Maze& maze, int row, int col) {
    maze.cells[row][col].tile = FLOOR;
    Direction order[4] = {UP, DOWN, LEFT, RIGHT};
    shuffleDirections(order);
    for (int i = 0; i < 4; i++) {
        int nextRow = row + rowStep(order[i]) * 2;
        int nextCol = col + colStep(order[i]) * 2;
        if (isInside(maze, nextRow, nextCol) && maze.cells[nextRow][nextCol].tile == WALL) {
            maze.cells[row + rowStep(order[i])][col + colStep(order[i])].tile = FLOOR;
            generatePaths(maze, nextRow, nextCol);
        }
    }
}

// Makes the bottom-right cell the EXIT and connects it to the maze.
// For an odd size that cell is already a room. For an even size the last row and
// column are not rooms, so we carve the cell next to the last room first.
void connectExit(Maze& maze) {
    int last = maze.size - 1;
    if (last % 2 == 1) {
        maze.cells[last - 1][last].tile = FLOOR;
    }
    maze.cells[last][last].tile = EXIT;
}

void placeFeatures(Maze&, const Level&) {}
