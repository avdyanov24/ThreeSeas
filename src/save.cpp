#include "save.h"

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
        ++resultCount;
    }

    return resultCount;
}

// Saves a result while keeping the fastest time for that player and level.
bool saveResult(const Result& result) {
    Result results[MAX_RECORDS];
    int resultCount = loadResults(results, MAX_RECORDS);
    bool hasMatchingResult = false;

    for (int i = 0; i < resultCount; ++i) {
        if (results[i].name == result.name && results[i].levelId == result.levelId) {
            if (result.bestSeconds < results[i].bestSeconds) {
                results[i] = result;
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
        ++resultCount;
    }

    std::filesystem::create_directories("save");
    std::ofstream output("save/results.txt");
    if (!output.is_open()) {
        return false;
    }

    for (int i = 0; i < resultCount; ++i) {
        output << results[i].name << ' ' << results[i].levelId << ' ' << results[i].bestSeconds
               << ' ' << results[i].stars << '\n';
    }

    return true;
}

// Finds and copies a player's best result for the requested level.
bool getBestResult(const std::string& name, int levelId, Result& best) {
    Result results[MAX_RECORDS];
    int resultCount = loadResults(results, MAX_RECORDS);

    for (int i = 0; i < resultCount; ++i) {
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
        ++recordCount;
    }

    return recordCount;
}

// Adds a win for a player and saves all win records.
bool addWin(const std::string& name) {
    WinRecord records[MAX_RECORDS];
    int recordCount = loadWins(records, MAX_RECORDS);
    bool hasMatchingRecord = false;

    for (int i = 0; i < recordCount; ++i) {
        if (records[i].name == name) {
            ++records[i].wins;
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
        ++recordCount;
    }

    std::filesystem::create_directories("save");
    std::ofstream output("save/wins.txt");
    if (!output.is_open()) {
        return false;
    }

    for (int i = 0; i < recordCount; ++i) {
        output << records[i].name << ' ' << records[i].wins << '\n';
    }

    return true;
}

// Loads sound and Free Play settings, using their defaults when unavailable.
Settings loadSettings() {
    Settings settings;
    settings.isSoundOn = true;
    settings.isFreePlay = false;

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
        if (value != 0 && value != 1) {
            continue;
        }
        if (word == "sound") {
            settings.isSoundOn = (value == 1);
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

    return true;
}
