#include "game.h"

GameState createEmptyGame() { return GameState{}; }

void destroyGame(GameState&) {}

void startLevel(GameState&, const Level&, int, bool) {}

void createPlayers(GameState&, int, const std::string[], const CharacterId[]) {}

void destroyPlayers(GameState&) {}

bool canEnter(const Maze&, const Player&, int, int) { return false; }

bool tryMove(Maze&, Player&, Direction) { return false; }

int getStepCost(const Maze&, int, int) { return 0; }

void updateCampaign(GameState&, int, float) {}

void burnFuel(Player&, float) {}

int getTorchRadius(const Player&) { return 0; }

bool isLit(const Player&, int, int) { return false; }

void updateSeen(const Maze&, Player&) {}

void pickUpItem(GameState&, Player&) {}

void useMap(const Maze&, Player&) {}

bool placeTrap(Maze&, Player&, int) { return false; }

bool hasReachedExit(const Maze&, const Player&) { return false; }

int calculateStars(int, float) { return 0; }

void nextTurn(GameState&) {}

void checkWinner(GameState&) {}
