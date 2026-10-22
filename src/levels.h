#ifndef LEVELS_H
#define LEVELS_H

#include "types.h"

// Returns the level with this id (1 to 9), or nullptr for any other id.
const Level* getLevel(int id);

// Returns the multiplayer arena with this id (1 to 3), or nullptr.
const Arena* getArena(int id);

// Returns the hero with this id.
const Character* getCharacter(CharacterId id);

// Level 1 is always open. Any other level is open if the player has a result for
// the previous level. Free Play opens everything.
bool isLevelUnlocked(int levelId, bool hasResultForPreviousLevel, bool isFreePlay);

// A hero is unlocked once the player has entered the hero's era.
bool isCharacterUnlocked(CharacterId id, int highestEraEntered, bool isFreePlay);

// Arena 1 needs 1 finished era, arena 2 needs 2, arena 3 needs 3.
// Free Play opens everything.
bool isArenaUnlocked(int arenaId, int erasFinished, bool isFreePlay);

#endif  // LEVELS_H
