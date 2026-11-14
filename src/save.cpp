#include "save.h"
#include "validation.h"

#include <filesystem>
#include <fstream>
#include <sstream>

// Reads valid result records from the save file and returns the number stored.
int loadResults(Result results[], int maxCount) {
    if (results == nullptr || maxCount <= 0) {
        return 0;
    }

    std::ifstream input("save/results.txt");
    if (!input.is_open()) {
        return 0;
    }

    int resultCount = 0;
    std::string line;
    while (resultCount < maxCount && std::getline(input, line)) {
        std::istringstream lineStream(line);
        Result result;
        std::string extra;

        if (!(lineStream >> result.name >> result.levelId >> result.bestSeconds >> result.stars)) {
            continue;
        }
        if (lineStream >> extra) {
            continue;
        }
        if (result.levelId < 1 || result.levelId > 9) {
            continue;
        }
        if (result.stars < 1 || result.stars > 3) {
            continue;
        }
        if (!(result.bestSeconds > 0.0f)) {
            continue;
        }

        results[resultCount] = result;
        resultCount++;
    }

    return resultCount;
}

// Saves a result while keeping the fastest time and the most stars for that player and level.
bool saveResult(const Result& result) {
    Result results[MAX_RECORDS];
    int resultCount = loadResults(results, MAX_RECORDS);
    bool hasMatchingResult = false;

    for (int i = 0; i < resultCount; i++) {
        if (results[i].name == result.name && results[i].levelId == result.levelId) {
            if (result.bestSeconds < results[i].bestSeconds) {
                // A faster time never lowers the stars that were already earned
                int bestStars = result.stars;
                if (results[i].stars > bestStars) {
                    bestStars = results[i].stars;
                }
                results[i] = result;
                results[i].stars = bestStars;
            }
            hasMatchingResult = true;
            break;
        }
    }

    if (!hasMatchingResult) {
        if (resultCount >= MAX_RECORDS) {
            return false;
        }
        results[resultCount] = result;
        resultCount++;
    }

    std::filesystem::create_directories("save");
    std::ofstream output("save/results.txt");
    if (!output.is_open()) {
        return false;
    }

    for (int i = 0; i < resultCount; i++) {
        output << results[i].name << ' ' << results[i].levelId << ' ' << results[i].bestSeconds
               << ' ' << results[i].stars << '\n';
    }

    return true;
}

// Finds and copies a player's best result for the requested level.
bool getBestResult(const std::string& name, int levelId, Result& best) {
    Result results[MAX_RECORDS];
    int resultCount = loadResults(results, MAX_RECORDS);

    for (int i = 0; i < resultCount; i++) {
        if (results[i].name == name && results[i].levelId == levelId) {
            best = results[i];
            return true;
        }
    }

    return false;
}

// Reads valid win records from the save file and returns the number stored.
int loadWins(WinRecord records[], int maxCount) {
    if (records == nullptr || maxCount <= 0) {
        return 0;
    }

    std::ifstream input("save/wins.txt");
    if (!input.is_open()) {
        return 0;
    }

    int recordCount = 0;
    std::string line;
    while (recordCount < maxCount && std::getline(input, line)) {
        std::istringstream lineStream(line);
        WinRecord record;
        std::string extra;

        if (!(lineStream >> record.name >> record.wins)) {
            continue;
        }
        if (lineStream >> extra) {
            continue;
        }
        if (record.wins < 1) {
            continue;
        }

        records[recordCount] = record;
        recordCount++;
    }

    return recordCount;
}

// Adds a win for a player and saves all win records.
bool addWin(const std::string& name) {
    WinRecord records[MAX_RECORDS];
    int recordCount = loadWins(records, MAX_RECORDS);
    bool hasMatchingRecord = false;

    for (int i = 0; i < recordCount; i++) {
        if (records[i].name == name) {
            records[i].wins++;
            hasMatchingRecord = true;
            break;
        }
    }

    if (!hasMatchingRecord) {
        if (recordCount >= MAX_RECORDS) {
            return false;
        }
        records[recordCount].name = name;
        records[recordCount].wins = 1;
        recordCount++;
    }

    std::filesystem::create_directories("save");
    std::ofstream output("save/wins.txt");
    if (!output.is_open()) {
        return false;
    }

    for (int i = 0; i < recordCount; i++) {
        output << records[i].name << ' ' << records[i].wins << '\n';
    }

    return true;
}

// Loads the switches and volumes, using their defaults for anything missing or broken.
Settings loadSettings() {
    Settings settings;
    settings.isSoundOn = true;
    settings.isFreePlay = false;
    settings.isFullscreen = false;
    settings.menuMusicVolume = 5;
    settings.gameMusicVolume = 4;
    settings.effectsVolume = 8;

    std::ifstream input("save/settings.txt");
    if (!input.is_open()) {
        return settings;
    }

    std::string line;
    while (std::getline(input, line)) {
        std::istringstream lineStream(line);
        std::string word;
        std::string extra;
        int value;

        if (!(lineStream >> word >> value)) {
            continue;
        }
        if (lineStream >> extra) {
            continue;
        }
        // Volumes go from 0 to MAX_VOLUME, the switches are 0 or 1
        bool isVolume = word == "menumusic" || word == "gamemusic" || word == "effects";
        if (isVolume && value >= 0 && value <= MAX_VOLUME) {
            if (word == "menumusic") {
                settings.menuMusicVolume = value;
            } else if (word == "gamemusic") {
                settings.gameMusicVolume = value;
            } else {
                settings.effectsVolume = value;
            }
            continue;
        }
        if (value != 0 && value != 1) {
            continue;
        }
        if (word == "sound") {
            settings.isSoundOn = (value == 1);
        }
        if (word == "fullscreen") {
            settings.isFullscreen = (value == 1);
        }
        if (word == "freeplay") {
            settings.isFreePlay = (value == 1);
        }
    }

    return settings;
}

// Saves sound and Free Play settings to the settings file.
bool saveSettings(const Settings& settings) {
    std::filesystem::create_directories("save");
    std::ofstream output("save/settings.txt");
    if (!output.is_open()) {
        return false;
    }

    output << "sound " << (settings.isSoundOn ? 1 : 0) << '\n';
    output << "freeplay " << (settings.isFreePlay ? 1 : 0) << '\n';
    output << "fullscreen " << (settings.isFullscreen ? 1 : 0) << '\n';
    output << "menumusic " << settings.menuMusicVolume << '\n';
    output << "gamemusic " << settings.gameMusicVolume << '\n';
    output << "effects " << settings.effectsVolume << '\n';

    return true;
}

// Writes the names to save/players.txt, one per line. Returns false if writing failed.
static bool writePlayers(const std::string names[], int count) {
    std::filesystem::create_directories("save");
    std::ofstream output("save/players.txt");
    if (!output.is_open()) {
        return false;
    }
    for (int i = 0; i < count; i++) {
        output << names[i] << '\n';
    }
    return true;
}

// Reads the remembered names, skipping invalid and repeated ones.
int loadPlayers(std::string names[], int maxCount) {
    std::ifstream input("save/players.txt");
    if (!input.is_open()) {
        return 0;
    }
    int count = 0;
    std::string line;
    while (count < maxCount && std::getline(input, line)) {
        if (!isValidName(line)) {
            continue;
        }
        bool isRepeated = false;
        for (int i = 0; i < count; i++) {
            if (names[i] == line) {
                isRepeated = true;
            }
        }
        if (!isRepeated) {
            names[count] = line;
            count++;
        }
    }
    return count;
}

// Moves name to the front of the remembered names.
bool rememberPlayer(const std::string& name) {
    if (!isValidName(name)) {
        return false;
    }
    std::string names[MAX_SAVED_PLAYERS];
    int count = loadPlayers(names, MAX_SAVED_PLAYERS);
    std::string ordered[MAX_SAVED_PLAYERS];
    ordered[0] = name;
    int orderedCount = 1;
    for (int i = 0; i < count && orderedCount < MAX_SAVED_PLAYERS; i++) {
        if (names[i] != name) {
            ordered[orderedCount] = names[i];
            orderedCount++;
        }
    }
    return writePlayers(ordered, orderedCount);
}

// Removes name from the remembered names.
bool forgetPlayer(const std::string& name) {
    std::string names[MAX_SAVED_PLAYERS];
    int count = loadPlayers(names, MAX_SAVED_PLAYERS);
    std::string kept[MAX_SAVED_PLAYERS];
    int keptCount = 0;
    for (int i = 0; i < count; i++) {
        if (names[i] != name) {
            kept[keptCount] = names[i];
            keptCount++;
        }
    }
    return writePlayers(kept, keptCount);
}

// Reads the names and heroes of the last multiplayer game.
int loadLastSetup(std::string names[], CharacterId heroes[], int maxCount) {
    std::ifstream input("save/last_setup.txt");
    if (!input.is_open()) {
        return 0;
    }
    int count = 0;
    std::string line;
    while (count < maxCount && std::getline(input, line)) {
        std::istringstream lineStream(line);
        std::string name;
        int hero = -1;
        if (!(lineStream >> name >> hero) || !isValidName(name) || hero < ASPARUH ||
            hero > LEVSKI) {
            continue;
        }
        names[count] = name;
        heroes[count] = static_cast<CharacterId>(hero);
        count++;
    }
    return count;
}

// Writes one "name hero" line per player.
bool saveLastSetup(const std::string names[], const CharacterId heroes[], int count) {
    std::filesystem::create_directories("save");
    std::ofstream output("save/last_setup.txt");
    if (!output.is_open()) {
        return false;
    }
    for (int i = 0; i < count; i++) {
        output << names[i] << ' ' << static_cast<int>(heroes[i]) << '\n';
    }
    return true;
}
