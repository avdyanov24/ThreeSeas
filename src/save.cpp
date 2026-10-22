#include "save.h"

int loadResults(Result[], int) { return 0; }

bool saveResult(const Result&) { return false; }

bool getBestResult(const std::string&, int, Result&) { return false; }

int loadWins(WinRecord[], int) { return 0; }

bool addWin(const std::string&) { return false; }

Settings loadSettings() { return Settings{}; }

bool saveSettings(const Settings&) { return false; }
