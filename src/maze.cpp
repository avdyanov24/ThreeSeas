#include "maze.h"

Maze createMaze(int) { return Maze{}; }

void destroyMaze(Maze&) {}

bool isInside(const Maze&, int, int) { return false; }

int rowStep(Direction) { return 0; }

int colStep(Direction) { return 0; }

void shuffleDirections(Direction[4]) {}

void generatePaths(Maze&, int, int) {}

void connectExit(Maze&) {}

void placeFeatures(Maze&, const Level&) {}
