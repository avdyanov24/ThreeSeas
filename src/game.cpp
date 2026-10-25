#include "game.h"

#include "maze.h"

GameState createEmptyGame() { return GameState{}; }

void destroyGame(GameState&) {}

void startLevel(GameState&, const Level&, int, bool) {}

void createPlayers(GameState&, int, const std::string[], const CharacterId[]) {}

void destroyPlayers(GameState&) {}

// Returns whether the player can enter the specified maze cell.
bool canEnter(const Maze& maze, const Player& player, int row, int col) {
    if (!isInside(maze, row, col)) {
        return false;
    }

    const Tile tile = maze.cells[row][col].tile;
    if (tile == WALL) {
        return false;
    }

    if (tile == DOOR && !player.hasKey && player.character != IVAN_ASEN) {
        return false;
    }

    return true;
}

// Moves the player one cell if the destination is valid; otherwise leaves position unchanged.
bool tryMove(Maze& maze, Player& player, Direction direction) {
    if (direction != UP && direction != DOWN && direction != LEFT && direction != RIGHT) {
        return false;
    }

    const int nextRow = player.row + rowStep(direction);
    const int nextCol = player.col + colStep(direction);
    if (!canEnter(maze, player, nextRow, nextCol)) {
        return false;
    }

    player.row = nextRow;
    player.col = nextCol;
    return true;
}

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
