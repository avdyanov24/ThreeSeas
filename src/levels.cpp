#include "levels.h"

const Level* getLevel(int) { return nullptr; }

const Arena* getArena(int) { return nullptr; }

const Character* getCharacter(CharacterId) { return nullptr; }

bool isLevelUnlocked(int, bool, bool) { return false; }

bool isCharacterUnlocked(CharacterId, int, bool) { return false; }

bool isArenaUnlocked(int, int, bool) { return false; }
