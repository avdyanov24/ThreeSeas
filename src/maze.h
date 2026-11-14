#ifndef MAZE_H
#define MAZE_H

#include "types.h"

// Creates a size x size maze on the heap. Every cell is a WALL with no item and
// trapOwner NO_TRAP. The size must already be validated (10 to 50).
Maze createMaze(int size);

// Frees every row and the row list, then sets cells to nullptr and size to 0.
// Safe to call on an already destroyed maze.
void destroyMaze(Maze& maze);

// Returns true if (row, col) is a cell of the maze.
bool isInside(const Maze& maze, int row, int col);

// Returns the row change for a direction: -1 for UP, 1 for DOWN, otherwise 0.
int rowStep(Direction direction);

// Returns the column change for a direction: -1 for LEFT, 1 for RIGHT, otherwise 0.
int colStep(Direction direction);

// Puts the 4 directions in a random order (uses rand()).
void shuffleDirections(Direction order[4]);

// Carves paths with the recursive backtracker, starting from the room (row, col).
// Rooms are the cells with even coordinates. Call it once with (0, 0) on a maze
// that is completely WALL. The result is a perfect maze (one path between any two cells).
void generatePaths(Maze& maze, int row, int col);

// Makes sure the bottom-right cell (size-1, size-1) is the EXIT and is reachable.
// For an even size that cell is not a room, so a 2-cell connection is carved to it.
// The EXIT tile is walkable, so it counts as floor.
void connectExit(Maze& maze);

// Multiplayer: returns the room where player 0 to 3 starts, one corner each: top-left,
// bottom-right, top-right, bottom-left. Corners are rooms, so they are always floor.
void getSpawnCell(const Maze& maze, int index, int& row, int& col);

// Multiplayer: makes a random room the EXIT, at least size / 3 steps (row + col distance)
// away from every spawn corner. Falls back to connectExit if no room is far enough.
// Call it once, after generatePaths, instead of connectExit.
void placeRandomExit(Maze& maze);

// Adds oil, maps, the key, doors, water and level traps using the numbers in level.
// Never on the start (0, 0) or the exit; traps never next to the start.
// Doors only go on walls with floor on two opposite sides, so the maze can always
// be finished without them. Level traps get trapOwner LEVEL_TRAP.
void placeFeatures(Maze& maze, const Level& level);

// Fills a TUTORIAL_SIZE maze with the fixed tutorial layout: a winding path from the
// start (0, 0) past oil, a map, a key, a locked door and a hidden trap to the EXIT at
// (9, 0). Returns false and changes nothing when the maze is not TUTORIAL_SIZE.
bool buildTutorialMaze(Maze& maze);

#endif  // MAZE_H
