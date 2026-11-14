#ifndef SAVE_H
#define SAVE_H

#include "types.h"

// Reads save/results.txt into results and returns how many were read (at most
// maxCount). A missing file gives 0. Broken lines are skipped.
int loadResults(Result results[], int maxCount);

// Stores the result, keeping only the best (fastest) time for that name and level.
// Creates the save/ folder and file if needed. Returns false if writing failed.
bool saveResult(const Result& result);

// Looks up the best result of a player on a level. Returns false if there is none.
bool getBestResult(const std::string& name, int levelId, Result& best);

// Reads save/wins.txt into records and returns how many were read.
int loadWins(WinRecord records[], int maxCount);

// Adds 1 win for the player. Returns false if writing failed.
bool addWin(const std::string& name);

// Reads save/players.txt (one name per line, the last player first) into names and
// returns how many were read (at most maxCount). Invalid names and repeated names are
// skipped. A missing file gives 0.
int loadPlayers(std::string names[], int maxCount);

// Puts name first in save/players.txt, removing it from further down, and keeps at most
// MAX_SAVED_PLAYERS names. Returns false for an invalid name or if writing failed.
bool rememberPlayer(const std::string& name);

// Removes name from save/players.txt. Returns false if writing failed.
bool forgetPlayer(const std::string& name);

// Reads the last multiplayer setup from save/last_setup.txt ("name hero" per line, hero
// is 0 to 5) and returns how many players were read (at most maxCount). Broken lines are
// skipped.
int loadLastSetup(std::string names[], CharacterId heroes[], int maxCount);

// Writes the names and heroes of a multiplayer setup to save/last_setup.txt. Returns
// false if writing failed.
bool saveLastSetup(const std::string names[], const CharacterId heroes[], int count);

// Reads save/settings.txt. A missing or broken file gives the defaults (sound on,
// Free Play off, windowed, menu music 5, game music 4, effects 8). Volumes outside 0 to
// MAX_VOLUME are ignored.
Settings loadSettings();

// Writes save/settings.txt. Returns false if writing failed.
bool saveSettings(const Settings& settings);

#endif  // SAVE_H
