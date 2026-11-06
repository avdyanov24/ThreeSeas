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

// Returns the room of the corner where player index 0 to 3 starts.
void getSpawnCell(const Maze& maze, int index, int& row, int& col) {
    int last = (maze.size - 1) / 2 * 2;
    row = 0;
    col = 0;
    if (index == 1 || index == 3) {
        row = last;
    }
    if (index == 1 || index == 2) {
        col = last;
    }
}

// Returns the distance in steps along rows and columns between two cells.
static int getCellDistance(int rowA, int colA, int rowB, int colB) {
    int rowDistance = rowA > rowB ? rowA - rowB : rowB - rowA;
    int colDistance = colA > colB ? colA - colB : colB - colA;
    return rowDistance + colDistance;
}

// Returns true if (row, col) is one of the four spawn corners.
static bool isSpawnCorner(const Maze& maze, int row, int col) {
    for (int i = 0; i < 4; i++) {
        int spawnRow = 0;
        int spawnCol = 0;
        getSpawnCell(maze, i, spawnRow, spawnCol);
        if (row == spawnRow && col == spawnCol) {
            return true;
        }
    }
    return false;
}

// Makes a random room far from every corner the EXIT, or the bottom-right cell if none is.
void placeRandomExit(Maze& maze) {
    int* rooms = new int[maze.size * maze.size];
    int count = 0;
    int minDistance = maze.size / 3;
    for (int row = 0; row < maze.size; row += 2) {
        for (int col = 0; col < maze.size; col += 2) {
            bool isFarEnough = maze.cells[row][col].tile == FLOOR;
            for (int i = 0; i < 4 && isFarEnough; i++) {
                int spawnRow = 0;
                int spawnCol = 0;
                getSpawnCell(maze, i, spawnRow, spawnCol);
                if (getCellDistance(row, col, spawnRow, spawnCol) < minDistance) {
                    isFarEnough = false;
                }
            }
            if (isFarEnough) {
                rooms[count] = row * maze.size + col;
                count++;
            }
        }
    }
    if (count == 0) {
        connectExit(maze);
    } else {
        int picked = rooms[rand() % count];
        maze.cells[picked / maze.size][picked % maze.size].tile = EXIT;
    }
    delete[] rooms;
    rooms = nullptr;
}

// Stores the position (row * size + col) of every free floor cell in cells: a FLOOR
// cell with no item and no trap, except the four spawn corners. Returns how many were stored.
static int collectFreeFloor(const Maze& maze, int cells[]) {
    int count = 0;
    for (int row = 0; row < maze.size; row++) {
        for (int col = 0; col < maze.size; col++) {
            const Cell& cell = maze.cells[row][col];
            if (cell.tile == FLOOR && cell.item == NO_ITEM && cell.trapOwner == NO_TRAP &&
                !isSpawnCorner(maze, row, col)) {
                cells[count] = row * maze.size + col;
                count++;
            }
        }
    }
    return count;
}

// Puts the first count entries of cells in a random order (Fisher-Yates shuffle).
static void shuffleCells(int cells[], int count) {
    for (int i = count - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int saved = cells[i];
        cells[i] = cells[j];
        cells[j] = saved;
    }
}

// Returns how many things belong in a maze of this size when the level asks for
// perHundred of them on every 100 cells (rounded to the nearest whole number).
static int getAmount(int size, float perHundred) {
    return static_cast<int>(size * size / 100.0f * perHundred + 0.5f);
}

// Puts up to amount of the item on random free floor cells, never closer to the start than
// minDistance (counted as row + col). A LEVEL_TRAP is a trap, so it becomes a trapOwner.
static void scatterOnFloor(Maze& maze, Item item, bool isTrap, int amount, int minDistance) {
    int* cells = new int[maze.size * maze.size];
    int count = collectFreeFloor(maze, cells);
    shuffleCells(cells, count);
    int placed = 0;
    for (int i = 0; i < count && placed < amount; i++) {
        int row = cells[i] / maze.size;
        int col = cells[i] % maze.size;
        if (row + col < minDistance) {
            continue;
        }
        if (isTrap) {
            maze.cells[row][col].trapOwner = LEVEL_TRAP;
        } else {
            maze.cells[row][col].item = item;
        }
        placed++;
    }
    delete[] cells;
    cells = nullptr;
}

// Turns up to amount walls into doors. A door only goes on an inner wall that has floor on
// two opposite sides, so every door is a shortcut and the maze can be finished without it.
static void placeDoors(Maze& maze, int amount) {
    int* candidates = new int[maze.size * maze.size];
    int count = 0;
    for (int row = 1; row < maze.size - 1; row++) {
        for (int col = 1; col < maze.size - 1; col++) {
            if (maze.cells[row][col].tile != WALL) {
                continue;
            }
            bool isVertical = maze.cells[row - 1][col].tile == FLOOR &&
                              maze.cells[row + 1][col].tile == FLOOR;
            bool isHorizontal = maze.cells[row][col - 1].tile == FLOOR &&
                                maze.cells[row][col + 1].tile == FLOOR;
            if (isVertical || isHorizontal) {
                candidates[count] = row * maze.size + col;
                count++;
            }
        }
    }
    shuffleCells(candidates, count);
    for (int i = 0; i < count && i < amount; i++) {
        maze.cells[candidates[i] / maze.size][candidates[i] % maze.size].tile = DOOR;
    }
    delete[] candidates;
    candidates = nullptr;
}

// Turns about 8% of the free floor cells into water (never next to the start).
static void placeWater(Maze& maze) {
    int* cells = new int[maze.size * maze.size];
    int count = collectFreeFloor(maze, cells);
    shuffleCells(cells, count);
    int amount = count * 8 / 100;
    int placed = 0;
    for (int i = 0; i < count && placed < amount; i++) {
        int row = cells[i] / maze.size;
        int col = cells[i] % maze.size;
        if (row + col < 3) {
            continue;
        }
        maze.cells[row][col].tile = WATER;
        placed++;
    }
    delete[] cells;
    cells = nullptr;
}

// Adds doors, water, level traps, the key, oil and maps. Doors and water come first,
// because they need plain floor around them; items go on the floor that is left.
void placeFeatures(Maze& maze, const Level& level) {
    placeDoors(maze, level.doors);
    if (level.hasWater) {
        placeWater(maze);
    }
    scatterOnFloor(maze, NO_ITEM, true, getAmount(maze.size, level.trapsPer100), 3);
    if (level.doors > 0) {
        scatterOnFloor(maze, KEY, false, 1, 3);
    }
    scatterOnFloor(maze, OIL, false, getAmount(maze.size, level.oilPer100), 0);
    scatterOnFloor(maze, MAP_ITEM, false, maze.size * maze.size / 300 + 1, 0);
}
