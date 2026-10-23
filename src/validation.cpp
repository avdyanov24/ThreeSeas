#include "validation.h"

// Converts text to a number. Returns true when every character is a digit.
bool parseWholeNumber(const std::string& text, int& number) {
    if (text.empty() || text.length() > 9) {
        return false;
    }

    int parsedNumber = 0;
    for (int i = 0; i < text.length(); i++) {
        char character = text[i];
        if (character < '0' || character > '9') {
            return false;
        }
        parsedNumber = parsedNumber * 10 + (character - '0');
    }

    number = parsedNumber;
    return true;
}

// Checks whether a maze size is in the allowed range. Returns true when valid.
bool isValidSize(int size, bool isMultiplayer) {
    int minimumSize = MIN_CAMPAIGN_SIZE;
    if (isMultiplayer) {
        minimumSize = MIN_MULTIPLAYER_SIZE;
    }

    if (size < minimumSize || size > MAX_SIZE) {
        return false;
    }

    return true;
}

bool isValidName(const std::string&) { return false; }

bool isValidPlayerCount(int) { return false; }

bool isCharacterTaken(const Player*, int, CharacterId) { return false; }
