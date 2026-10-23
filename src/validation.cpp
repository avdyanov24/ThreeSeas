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

bool isValidSize(int, bool) { return false; }

bool isValidName(const std::string&) { return false; }

bool isValidPlayerCount(int) { return false; }

bool isCharacterTaken(const Player*, int, CharacterId) { return false; }
