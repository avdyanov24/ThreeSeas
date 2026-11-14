#include "game.h"

#include "levels.h"
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
    if (isMultiplayer) {
        placeRandomExit(game.maze);
    } else {
        connectExit(game.maze);
    }
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
        float fuelFactor = 2.0f;
        if (!game.isMultiplayer) {
            fuelFactor = game.level->fuelFactor;
        }
        if (game.isMultiplayer) {
            getSpawnCell(game.maze, i, player.row, player.col);
        }
        player.visualRow = static_cast<float>(player.row);
        player.visualCol = static_cast<float>(player.col);
        player.facing = DOWN;
        player.maxFuel = game.maze.size * fuelFactor;
        if (player.character == ASPARUH) {
            player.maxFuel *= 1.5f;
        }
        player.fuel = player.maxFuel;
        if (game.isMultiplayer) {
            player.traps = 1;
        }
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

    player.facing = direction;
    const int nextRow = player.row + rowStep(direction);
    const int nextCol = player.col + colStep(direction);
    if (!canEnter(maze, player, nextRow, nextCol)) {
        return false;
    }

    player.row = nextRow;
    player.col = nextCol;
    // Long enough to reach the next step, also on water (0.24 s), so a held arrow key
    // keeps the hero walking instead of flicking back to standing between cells
    player.walkSeconds = 0.3f;
    return true;
}

// Returns 0.24 seconds per step on water, otherwise 0.12.
float getStepSeconds(const Maze& maze, const Player& player) {
    if (maze.cells[player.row][player.col].tile == WATER) {
        return 0.24f;
    }
    return 0.12f;
}

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
    float previousTime = game.timeSeconds;
    game.timeSeconds += deltaTime;
    float fuelPerSecond = 0.5f;
    if (game.level->isCold) {
        fuelPerSecond = 1.0f;
    }
    burnFuel(player, deltaTime * fuelPerSecond);
    if (player.isOut) {
        return;
    }
    if (player.stuckSeconds > 0.0f) {
        player.stuckSeconds -= deltaTime;
        if (player.stuckSeconds < 0.0f) {
            player.stuckSeconds = 0.0f;
        }
        return;
    }
    float movementInterval = getStepSeconds(game.maze, player);
    int previousStep = static_cast<int>(previousTime / movementInterval);
    int currentStep = static_cast<int>(game.timeSeconds / movementInterval);
    if (heldDirection >= UP && heldDirection <= RIGHT && previousStep != currentStep) {
        if (tryMove(game.maze, player, static_cast<Direction>(heldDirection))) {
            pickUpItem(game, player);
            updateSeen(game.maze, player);
            if (hasReachedExit(game.maze, player)) {
                game.winner = 0;
            }
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
    float fuelRatio = player.fuel / player.maxFuel;
    int radius = 1;
    if (fuelRatio > 0.6f) {
        radius = 3;
    } else if (fuelRatio >= 0.3f) {
        radius = 2;
    }
    if (player.character == BORIS) {
        radius++;
    }
    return radius;
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

// Collects the cell's item and applies any hidden trap on the cell.
void pickUpItem(GameState& game, Player& player) {
    Cell& cell = game.maze.cells[player.row][player.col];
    switch (cell.item) {
        case OIL:
            player.fuel += player.maxFuel * 0.25f;
            if (player.fuel > player.maxFuel) {
                player.fuel = player.maxFuel;
            }
            break;
        case MAP_ITEM:
            player.maps++;
            break;
        case KEY:
            player.hasKey = true;
            break;
        case TRAP_ITEM:
            player.traps++;
            break;
        case NO_ITEM:
            break;
    }
    if (cell.item != NO_ITEM) {
        cell.item = NO_ITEM;
    }

    if (cell.trapOwner != NO_TRAP) {
        int playerIndex = -1;
        for (int i = 0; i < game.playerCount; i++) {
            if (&game.players[i] == &player) {
                playerIndex = i;
                break;
            }
        }
        if (player.character != LEVSKI && cell.trapOwner != playerIndex) {
            if (game.isMultiplayer) {
                player.turnsToSkip = 1;
            } else {
                player.stuckSeconds = 2.0f;
            }
            cell.trapOwner = NO_TRAP;
        }
    }
}

// Reveals cells around the player and uses one map.
void useMap(const Maze& maze, Player& player) {
    if (player.maps <= 0 || player.seen == nullptr) {
        return;
    }

    int radius = 4;
    if (player.character == SIMEON) {
        radius = 8;
    }
    int radiusSquared = radius * radius;
    for (int row = 0; row < maze.size; row++) {
        for (int col = 0; col < maze.size; col++) {
            int rowDistance = row - player.row;
            int colDistance = col - player.col;
            if (rowDistance * rowDistance + colDistance * colDistance <= radiusSquared) {
                player.seen[row][col] = true;
            }
        }
    }
    player.maps--;
}

// Places one of the player's traps on their cell and returns whether it succeeded.
bool placeTrap(Maze& maze, Player& player, int playerIndex) {
    if (player.traps <= 0) {
        return false;
    }

    Cell& cell = maze.cells[player.row][player.col];
    if (cell.trapOwner != NO_TRAP) {
        return false;
    }

    cell.trapOwner = playerIndex;
    player.traps--;
    return true;
}

// Moves one drawn coordinate towards its target by at most step and returns the new value.
static float approachValue(float current, float target, float step) {
    if (current + step >= target && current - step <= target) {
        return target;
    }
    return current < target ? current + step : current - step;
}

// Slides the drawn position of the player towards their cell at 10 cells per second.
void updateVisualPosition(Player& player, float deltaTime) {
    float step = deltaTime * 10.0f;
    player.walkSeconds = player.walkSeconds > deltaTime ? player.walkSeconds - deltaTime : 0.0f;
    player.visualRow = approachValue(player.visualRow, static_cast<float>(player.row), step);
    player.visualCol = approachValue(player.visualCol, static_cast<float>(player.col), step);
}

// Returns whether the player is standing on the maze exit.
bool hasReachedExit(const Maze& maze, const Player& player) {
    return isInside(maze, player.row, player.col) &&
           maze.cells[player.row][player.col].tile == EXIT;
}

// Returns 3, 2, or 1 star based on completion time.
int calculateStars(int size, float seconds) {
    if (seconds <= size * 4) {
        return 3;
    }
    if (seconds <= size * 7) {
        return 2;
    }
    return 1;
}

// Burns the current player's fuel and advances to the next player who can take a turn.
void nextTurn(GameState& game) {
    if (game.players == nullptr || game.playerCount == 0) {
        return;
    }

    float fuelCost = 1.0f;
    if (game.level != nullptr && game.level->isCold) {
        fuelCost = 2.0f;
    }
    burnFuel(game.players[game.currentPlayer], fuelCost);

    for (int i = 1; i <= 2 * game.playerCount; i++) {
        int index = (game.currentPlayer + i) % game.playerCount;
        Player& player = game.players[index];
        if (player.isOut) {
            continue;
        }
        if (player.turnsToSkip > 0) {
            player.turnsToSkip--;
            continue;
        }

        game.currentPlayer = index;
        break;
    }
}

// Counts down the turn, walks the current player and returns whether the turn is over.
bool updateTurn(GameState& game, int heldDirection, float deltaTime) {
    if (game.players == nullptr || game.playerCount == 0 || deltaTime <= 0.0f) {
        return false;
    }
    Player& player = game.players[game.currentPlayer];
    float previousTime = game.timeSeconds;
    game.timeSeconds += deltaTime;
    game.turnSecondsLeft -= deltaTime;

    // A step happens each time the game time passes a whole number of steps
    float stepSeconds = getStepSeconds(game.maze, player);
    int previousStep = static_cast<int>(previousTime / stepSeconds);
    int currentStep = static_cast<int>(game.timeSeconds / stepSeconds);
    bool hasSprungTrap = false;
    if (heldDirection >= UP && heldDirection <= RIGHT && previousStep != currentStep &&
        tryMove(game.maze, player, static_cast<Direction>(heldDirection))) {
        pickUpItem(game, player);
        updateSeen(game.maze, player);
        hasSprungTrap = player.turnsToSkip > 0;
    }
    return hasSprungTrap || game.turnSecondsLeft <= 0.0f;
}

// Passes the turn on and resets the turn timer for the next player.
void startNextTurn(GameState& game) {
    nextTurn(game);
    game.turnNumber++;
    game.turnSecondsLeft = getTurnSeconds(game.turnNumber, game.playerCount, game.maze.size);
}

// Returns the seconds of a turn: 8 + size / 5 at first, 1 less after every full round,
// never below half of the start time.
float getTurnSeconds(int turnNumber, int playerCount, int mazeSize) {
    float startSeconds = 8.0f + static_cast<float>(mazeSize) / 5.0f;
    int round = 0;
    if (turnNumber > 1 && playerCount > 0) {
        round = (turnNumber - 1) / playerCount;
    }
    float seconds = startSeconds - static_cast<float>(round);
    if (seconds < startSeconds / 2.0f) {
        seconds = startSeconds / 2.0f;
    }
    return seconds;
}

// Sets the winner to the first player at the exit, the last active player, or DRAW.
void checkWinner(GameState& game) {
    if (game.players == nullptr || game.playerCount == 0) {
        return;
    }

    for (int i = 0; i < game.playerCount; i++) {
        if (hasReachedExit(game.maze, game.players[i])) {
            game.winner = i;
            return;
        }
    }

    int activePlayerCount = 0;
    int activePlayerIndex = -1;
    for (int i = 0; i < game.playerCount; i++) {
        if (!game.players[i].isOut) {
            activePlayerCount++;
            activePlayerIndex = i;
        }
    }

    if (activePlayerCount == 1) {
        game.winner = activePlayerIndex;
        return;
    }
    if (activePlayerCount == 0) {
        game.winner = DRAW;
        return;
    }
}

// Builds the tutorial game with a fixed maze and generous fuel.
void startTutorial(GameState& game, const std::string& name, CharacterId hero) {
    startLevel(game, *getLevel(1), TUTORIAL_SIZE, false);
    buildTutorialMaze(game.maze);
    std::string names[1] = {name};
    CharacterId heroes[1] = {hero};
    createPlayers(game, 1, names, heroes);
    game.players[0].maxFuel = 60.0f;
    game.players[0].fuel = 60.0f;
}

// Returns true when the player has done what the tutorial step asks for.
static bool isTutorialStepDone(const GameState& game, int step) {
    const Player& player = game.players[0];
    const Maze& maze = game.maze;
    switch (step) {
        case 0:
            return player.row != 0 || player.col != 0;
        case 1:
            return maze.cells[0][5].item == NO_ITEM;
        case 2:
            return maze.cells[2][3].item == NO_ITEM;
        case 3:
            return player.maps == 0;
        case 4:
            return player.hasKey;
        case 5:
            return player.row > 6 || (player.row == 6 && player.col < 7);
        case 6:
            return player.row >= 8;
        case 7:
            return game.winner == 0;
        default:
            return true;
    }
}

// Moves the tutorial on past every step that is already done.
int getTutorialStep(const GameState& game, int step) {
    if (game.players == nullptr || game.playerCount == 0) {
        return step;
    }
    if (game.winner == 0) {
        return TUTORIAL_DONE_STEP;
    }
    while (step < TUTORIAL_DONE_STEP && isTutorialStepDone(game, step)) {
        step++;
    }
    return step;
}
