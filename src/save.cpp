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

int loadWins(WinRecord[], int) { return 0; }

bool addWin(const std::string&) { return false; }

Settings loadSettings() { return Settings{}; }

bool saveSettings(const Settings&) { return false; }
