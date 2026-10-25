#include "game.h"

#include "maze.h"

// Returns an empty game with no winner yet.
GameState createEmptyGame() {
    GameState game = {};
    game.winner = NO_WINNER;
    return game;
}

// Frees players before their maze, then resets the game.
void destroyGame(GameState& game) {
    destroyPlayers(game);
    destroyMaze(game.maze);
    game = createEmptyGame();
}

// Replaces the old game with a generated maze using the selected level.
void startLevel(GameState& game, const Level& level, int size, bool isMultiplayer) {
    destroyGame(game);
    game.maze = createMaze(size);
    generatePaths(game.maze, 0, 0);
    connectExit(game.maze);
    placeFeatures(game.maze, level);
    game.level = &level;
    game.isMultiplayer = isMultiplayer;
}

// Creates players and their separate memories, with fuel from the level rules.
void createPlayers(GameState& game, int count, const std::string names[],
                   const CharacterId characters[]) {
    destroyPlayers(game);
    game.players = new Player[count]{};
    game.playerCount = count;
    game.currentPlayer = 0;
    for (int i = 0; i < count; i++) {
        Player& player = game.players[i];
        player.name = names[i];
        player.character = characters[i];
        float factor = game.isMultiplayer ? 2.0f : game.level->fuelFactor;
        player.maxFuel = game.maze.size * factor;
        if (player.character == ASPARUH) {
            player.maxFuel *= 1.5f;
        }
        player.fuel = player.maxFuel;
        player.traps = game.isMultiplayer ? 1 : 0;
        player.seen = new bool*[game.maze.size];
        for (int row = 0; row < game.maze.size; row++) {
            player.seen[row] = new bool[game.maze.size]{};
        }
        updateSeen(game.maze, player);
    }
}

// Frees every player's memory and the player list.
void destroyPlayers(GameState& game) {
    for (int i = 0; i < game.playerCount; i++) {
        for (int row = 0; row < game.maze.size; row++) {
            delete[] game.players[i].seen[row];
            game.players[i].seen[row] = nullptr;
        }
        delete[] game.players[i].seen;
        game.players[i].seen = nullptr;
    }
    delete[] game.players;
    game.players = nullptr;
    game.playerCount = 0;
}

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

// Advances campaign time and fuel, and moves at the shared walking interval.
void updateCampaign(GameState& game, int heldDirection, float deltaTime) {
    if (game.players == nullptr || game.playerCount == 0 || game.level == nullptr ||
        game.winner != NO_WINNER || deltaTime <= 0.0f) {
        return;
    }
    Player& player = game.players[0];
    if (player.isOut) {
        return;
    }
    float oldTime = game.timeSeconds;
    game.timeSeconds += deltaTime;
    burnFuel(player, deltaTime * (game.level->isCold ? 1.0f : 0.5f));
    if (player.isOut) {
        return;
    }
    float interval = game.maze.cells[player.row][player.col].tile == WATER ? 0.24f : 0.12f;
    if (heldDirection >= UP && heldDirection <= RIGHT &&
        static_cast<int>(oldTime / interval) != static_cast<int>(game.timeSeconds / interval)) {
        tryMove(game.maze, player, static_cast<Direction>(heldDirection));
        updateSeen(game.maze, player);
        if (hasReachedExit(game.maze, player)) {
            game.winner = 0;
        }
    }
}

// Removes fuel without going below zero and marks an extinguished torch.
void burnFuel(Player& player, float amount) {
    if (amount <= 0.0f) {
        return;
    }
    player.fuel -= amount;
    if (player.fuel <= 0.0f) {
        player.fuel = 0.0f;
        player.isOut = true;
    }
}

// Returns the light radius from fuel, including Boris's extra cell.
int getTorchRadius(const Player& player) {
    if (player.isOut || player.fuel <= 0.0f || player.maxFuel <= 0.0f) {
        return 0;
    }
    float ratio = player.fuel / player.maxFuel;
    int radius = ratio > 0.6f ? 3 : (ratio >= 0.3f ? 2 : 1);
    return radius + (player.character == BORIS ? 1 : 0);
}

// Returns whether a cell lies in the player's burning torch circle.
bool isLit(const Player& player, int row, int col) {
    if (player.isOut || player.fuel <= 0.0f) {
        return false;
    }
    int rowDistance = row - player.row;
    int colDistance = col - player.col;
    int radius = getTorchRadius(player);
    return rowDistance * rowDistance + colDistance * colDistance <= radius * radius;
}

// Permanently remembers every cell currently lit by the player.
void updateSeen(const Maze& maze, Player& player) {
    if (player.seen == nullptr) {
        return;
    }
    for (int row = 0; row < maze.size; row++) {
        for (int col = 0; col < maze.size; col++) {
            if (isLit(player, row, col)) {
                player.seen[row][col] = true;
            }
        }
    }
}

void pickUpItem(GameState&, Player&) {}

void useMap(const Maze&, Player&) {}

bool placeTrap(Maze&, Player&, int) { return false; }

// Returns whether the player is standing on the maze exit.
bool hasReachedExit(const Maze& maze, const Player& player) {
    return isInside(maze, player.row, player.col) &&
           maze.cells[player.row][player.col].tile == EXIT;
}

int calculateStars(int, float) { return 0; }

void nextTurn(GameState&) {}

void checkWinner(GameState&) {}
