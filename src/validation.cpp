#include "validation.h"

#include <cstddef>

// Converts text to a number. Returns true when every character is a digit.
bool parseWholeNumber(const std::string& text, int& number) {
    if (text.empty() || text.length() > 9) {
        return false;
    }

    int parsedNumber = 0;
    for (std::size_t i = 0; i < text.length(); i++) {
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

// Checks whether a name has an allowed length and characters. Returns true when valid.
bool isValidName(const std::string& name) {
    if (name.empty() || name.length() > MAX_NAME_LENGTH) {
        return false;
    }

    for (std::size_t i = 0; i < name.length(); i++) {
        char character = name[i];
        bool isLowercaseLetter = character >= 'a' && character <= 'z';
        bool isUppercaseLetter = character >= 'A' && character <= 'Z';
        bool isDigit = character >= '0' && character <= '9';
        if (!isLowercaseLetter && !isUppercaseLetter && !isDigit && character != '_') {
            return false;
        }
    }

    return true;
}

bool isValidPlayerCount(int) { return false; }

bool isCharacterTaken(const Player*, int, CharacterId) { return false; }
