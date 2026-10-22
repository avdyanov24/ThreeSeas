#ifndef GAME_H
#define GAME_H

#include "types.h"

// Returns a game with no maze and no players: cells and players are nullptr,
// counts are 0, level is nullptr and winner is NO_WINNER.
GameState createEmptyGame();

// Frees the maze and the players. Call it when the window closes.
void destroyGame(GameState& game);

// Frees any old maze, then builds a new one: createMaze, generatePaths(0, 0),
// connectExit and placeFeatures. Sets game.level, isMultiplayer, timeSeconds = 0,
// winner = NO_WINNER and currentPlayer = 0. Does not create players.
void startLevel(GameState& game, const Level& level, int size, bool isMultiplayer);

// Creates count players (1 to 4) at (0, 0), with their names and characters.
// Call it after startLevel. Start fuel: campaign size * level fuelFactor,
// multiplayer size * 2 (Asparuh gets +50%). Multiplayer players start with 1 trap,
// campaign players with 0. Allocates the seen arrays and marks the first lit cells.
void createPlayers(GameState& game, int count, const std::string names[],
                   const CharacterId characters[]);

// Frees every seen array and the players array, then sets players to nullptr
// and playerCount to 0.
void destroyPlayers(GameState& game);

// Returns true if the player may stand on (row, col): inside the maze, not a WALL,
// and a DOOR only if the player has the key or is IVAN_ASEN.
bool canEnter(const Maze& maze, const Player& player, int row, int col);

// Moves the player one cell in the given direction.
// Returns false (and does not move) if the cell is outside the maze,
// a wall, or a locked door the player cannot open.
// Only changes the position: items, traps and the seen cells are handled by
// pickUpItem and updateSeen.
bool tryMove(Maze& maze, Player& player, Direction direction);

// Multiplayer: returns how many of the 3 steps entering (row, col) costs:
// 2 for WATER, otherwise 1.
int getStepCost(const Maze& maze, int row, int col);

// One frame of the campaign. heldDirection is -1 when no arrow key is held,
// otherwise a Direction value. Adds deltaTime to the timer, moves the player one
// cell every 0.12 s (0.24 s on water), handles being stuck, calls pickUpItem and
// updateSeen after each move, burns fuel by time and sets game.winner to 0 when
// the exit is reached.
void updateCampaign(GameState& game, int heldDirection, float deltaTime);

// Subtracts amount from the fuel (never below 0). Sets isOut when it reaches 0.
void burnFuel(Player& player, float amount);

// Returns how many cells the torch lights: 3 above 60% fuel, 2 from 30% to 60%,
// 1 below 30%, 0 when the torch is out. BORIS adds 1 (but not when out).
int getTorchRadius(const Player& player);

// Returns true if (row, col) is inside the torchlight circle of the player.
bool isLit(const Player& player, int row, int col);

// Marks every lit cell of the maze as seen[row][col] = true.
void updateSeen(const Maze& maze, Player& player);

// Handles the cell the player just entered: oil adds 25% of maxFuel (never above
// maxFuel), a map adds to maps, a key sets hasKey, a trap item adds to traps.
// The item is removed from the cell. Also springs a hidden trap on that cell:
// campaign sets stuckSeconds to 2, multiplayer sets turnsToSkip to 1 when the
// trap belongs to someone else (LEVSKI is immune, the owner is safe). A sprung
// trap disappears.
void pickUpItem(GameState& game, Player& player);

// Uses one Map item: reveals a radius of 4 cells around the player (8 for SIMEON)
// by marking them as seen. Does nothing if the player has no map.
void useMap(const Maze& maze, Player& player);

// Multiplayer: puts one of the player's traps on their cell with trapOwner set to
// playerIndex. Returns false if the player has no trap or the cell already has one.
bool placeTrap(Maze& maze, Player& player, int playerIndex);

// Returns true if the player stands on the EXIT tile.
bool hasReachedExit(const Maze& maze, const Player& player);

// Returns 3 stars for seconds <= size * 4, 2 stars for seconds <= size * 7,
// otherwise 1.
int calculateStars(int size, float seconds);

// Multiplayer: ends the current turn. Burns the fuel of the current player (1,
// or 2 when the level is cold), then picks the next player who is not out,
// skipping and wrapping around. A player with turnsToSkip loses that turn.
// Resets stepsLeft to 3.
void nextTurn(GameState& game);

// Sets game.winner: the index of the first player on the exit, or of the last
// player whose torch is still burning, or DRAW if all torches went out in the
// same round. Leaves NO_WINNER if the game goes on.
void checkWinner(GameState& game);

#endif  // GAME_H
