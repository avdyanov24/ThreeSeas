#ifndef VALIDATION_H
#define VALIDATION_H

#include "types.h"

// Converts text to a number. Returns false (number unchanged) if the text is empty,
// contains anything but digits, or is longer than 9 digits.
bool parseWholeNumber(const std::string& text, int& number);

// Campaign: true for 10 to 50. Multiplayer: true for 20 to 50.
bool isValidSize(int size, bool isMultiplayer);

// True for 1 to 12 characters that are letters, digits or '_' (no spaces).
bool isValidName(const std::string& name);

// True for 2 to 4.
bool isValidPlayerCount(int count);

// True if one of the first count players already has this character.
bool isCharacterTaken(const Player* players, int count, CharacterId character);

#endif  // VALIDATION_H
