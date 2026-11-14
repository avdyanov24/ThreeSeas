# Three Seas: project documentation

School project "Labyrinth", C++17, 2026. The executable and the repository are called `crossroads`.

## 1. What the game is

Three Seas is a dark dungeon maze game set in Bulgarian history. Every maze is a real historical place. The player carries a torch that burns down, and only the lit area is visible. The goal is to find the exit before the flame dies.

- **Campaign:** 1 player, real time, 9 mazes in 3 eras. Hold the arrow keys to walk.
- **Multiplayer:** 2 to 4 players on one laptop. Every player starts in their own corner, the exit is hidden in a room far from all corners, and turns have a time limit.
- 6 historical heroes, each with one power that is always on.
- 4 items: oil, map, key and trap.
- Everything works offline. There are no accounts, no online features and no diagonal movement.

## 2. How to build and run

Requirements: CMake 3.20 or newer and a C++17 compiler. raylib 6.0 is downloaded by CMake (FetchContent).

macOS and Linux:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/crossroads
ctest --test-dir build --output-on-failure
```

Windows (Visual Studio developer prompt):

```bat
cmake -S . -B build
cmake --build build --config Release
build\Release\crossroads.exe
ctest --test-dir build -C Release --output-on-failure
```

CMake builds three targets:

| Target | What it is |
|---|---|
| `logic` | Library with the game logic. It does not use raylib. |
| `crossroads` | The game. Links `logic` and raylib. |
| `tests` | The QA tests. Links only `logic`, so they run without a window. |

Warnings are errors for our own code (`-Wall -Wextra -Werror`, `/W4 /WX` on Windows). The sound files in `assets/sounds` are copied next to the build, and the game saves its files in a `save/` folder next to where it is started.

## 3. How the code is split

The most important rule: the logic files never use `cin`, `cout` or raylib. They receive values and return results. Only the screen files draw, read keys or play sounds.

| File | Kind | Job |
|---|---|---|
| `types.h` | header | All enums and structs |
| `maze.h/.cpp` | logic | Create, generate and free the maze, place doors, water, traps and items, spawn corners, random exit |
| `game.h/.cpp` | logic | Movement, fuel, torch radius, items, traps, turns, turn time, stars, winner |
| `levels.h/.cpp` | logic | The 9 levels, 3 arenas, 6 heroes and the unlock checks |
| `validation.h/.cpp` | logic | Checks for sizes, numbers, names, player count and hero choice |
| `save.h/.cpp` | logic | Results, wins and settings files (`fstream` is allowed here) |
| `render.h/.cpp` | screen | Textures, drawing the maze, light, players and the HUD |
| `screens.h/.cpp` | screen | Menus, the campaign map, all other screens and their input |
| `audio.h/.cpp` | screen | Loading and playing the sounds |
| `main.cpp` | screen | Window, main loop, starting games, playing sounds |
| `tests/tests.cpp` | test | The QA test functions |

Code rules the team follows: no classes, templates, smart pointers, lambdas or `std::vector` for the graded arrays; no global or static variables (constants are fine); 4 spaces, at most 100 columns; a comment above every function; names of booleans start with `is`, `has` or `can`.

### How a frame works

`main` runs one loop. Each frame it calls `updateScreen` (reads input and changes the game, never draws), then `drawScreen` (draws, never changes the game). The current screen is one value of the `Screen` enum: title, name entry, main menu, campaign map, level intro, playing, paused, result, multiplayer setup, pass screen, multiplayer result, leaderboard, how to play and settings.

## 4. The main data types (`types.h`)

| Type | What it holds |
|---|---|
| `Tile` | `FLOOR`, `WALL`, `DOOR`, `WATER`, `EXIT` |
| `Item` | `NO_ITEM`, `OIL`, `MAP_ITEM`, `KEY`, `TRAP_ITEM` |
| `Cell` | One tile, one item and `trapOwner` (no trap, level trap, or the index of the player who placed it) |
| `Maze` | `size` and `cells`, a 2D dynamic array of `Cell` |
| `Player` | Name, hero, row and column, fuel, maps, traps, key, "torch out", turns to skip, and `seen`, the player's memory of the maze |
| `Level`, `Arena`, `Character` | The data tables: fuel factor, oil, traps, doors, water, cold; hero powers |
| `GameState` | The maze, the players, whose turn it is, the level, the time, the turn timer and the winner |
| `UiState` | Everything the screens remember: typed text, selected menu entry, chosen size, names, settings, leaderboard data, animation state |
| `Result`, `WinRecord`, `Settings` | What is saved in the files |

Three dynamic arrays are used: the maze (`Cell**`), the players (`Player*`) and each player's memory (`bool**`). Every `new` has a `delete[]`, and the pointers are set to `nullptr` afterwards. The memory is freed when a maze ends, when the player quits to the menu and when the window closes.

### Maze generation

`createMaze` makes a size x size maze that is all wall. `generatePaths` carves it with the recursive backtracker, which gives a perfect maze (one path between any two rooms). `connectExit` makes the bottom-right cell the exit. `placeFeatures` then adds the key, oil, maps, doors, water and level traps using the numbers of the level. Doors are only placed in walls that have floor on two opposite sides, so a maze can always be finished without any door.

In multiplayer `getSpawnCell` gives each player a corner (top-left, bottom-right, top-right, bottom-left), and `placeRandomExit` makes a random room the exit, at least one third of the maze size away from every corner.

### Saving

| File | Content |
|---|---|
| `save/results.txt` | The best (fastest) time and stars of each player on each level |
| `save/wins.txt` | The number of multiplayer wins of each player |
| `save/settings.txt` | Sound on or off and Free Play |

A missing or broken file is not an error: the game starts with empty data or the defaults.

## 5. The game rules

### Controls

| Key | Action |
|---|---|
| Arrow keys | Move (hold to keep walking) |
| `M` | Use a Map item |
| `1` / `2` | Multiplayer: use a Map / place a Trap |
| `Enter` | Confirm, or end your turn early |
| `Esc` | Pause menu or back |
| Mouse | Menus and buttons |

### The torch

- It burns 1 fuel every 2 seconds, twice as fast in cold places. When the fuel is 0 the torch is out and the maze must be played again.
- The light radius depends on the fuel: more than 60% gives 3 cells, 30% to 60% gives 2 cells, below 30% gives 1 cell and the screen flickers.
- Places you have seen stay dim on screen. Places you have never seen are black.

### Items and tiles

| Item | Effect |
|---|---|
| Oil | Refills 25% of the maximum fuel, never above the maximum |
| Map | Press `M` to reveal 4 cells around you |
| Key | Opens every locked door in the maze |
| Trap | Multiplayer: placed on your own cell, invisible to the others |

| Tile | Meaning |
|---|---|
| Floor, wall | Walkable, not walkable |
| Locked door | A shortcut through a wall. It is never needed to finish. |
| Water | Walkable at half speed |
| Hidden trap | Visible only when you are next to it. In the campaign it holds you for 2 seconds. In multiplayer the player loses the next turn. |
| Exit | In the campaign the bottom-right corner, in multiplayer a hidden room |

### Heroes

| Hero | Era | Power |
|---|---|---|
| Khan Asparuh | I | +50% starting fuel |
| Khan Krum | I | Sees all traps |
| Knyaz Boris I | I | Torch radius +1 |
| Tsar Simeon | I | Maps reveal double the area |
| Tsar Ivan Asen II | II | Opens doors without a key |
| Vasil Levski | III | Immune to traps |

### Campaign

- The nine mazes are in three eras. Era I adds traps and doors, Era II adds water and Era III adds cold.
- The start fuel is the maze size times the level's fuel factor, and the player chooses a size from 10 to 50.
- Stars: 3 stars for a time of at most 4 seconds per maze size, 2 stars for at most 7 seconds, 1 star for any finish.
- Only the first maze is open at the start. Finishing a maze opens the next one. Finishing the hard maze of an era opens the next era and that era's multiplayer arena. The first arena, Pliska, is open from the start. Heroes unlock when their era is entered. Free Play in the settings opens everything.

### Multiplayer

- 2 to 4 players, each hero only once per game. The maze size is 20 to 50.
- Start fuel is the maze size times 2, and every player starts with 1 trap.
- Each turn has a time limit: 8 seconds plus one fifth of the maze size seconds at first (12 seconds for size 20, 18 for size 50). The time gets 1 second shorter after every full round and never goes below half of the start time.
- During a turn the player holds the arrow keys to walk. A turn ends when the time is over, when the player presses `Enter`, or when the player springs a trap. Ending a turn burns 1 fuel (2 in the cold arena).
- Other players are drawn only when they are inside your torchlight. A black "Pass to <name>" screen separates the turns.
- The first player on the exit wins. A player with no fuel is out. If one torch is still burning, that player wins. If the last torches go out together, the game is a draw. A win is saved for the leaderboard.
- Arenas: Pliska (traps, doors, keys), Tsarevets (adds water) and Balkan Pass (adds cold).

## 6. How we worked

- **Team:** Aleksandar Dyanov (Scrum Master, frontend: headers, maze, drawing, art, reviews), Aleksandar Petrov (QA and backend: tests, QA plan, save files), Presiyan Tenev (frontend: screens, HUD, sound), Loris Zadikyan (backend: game rules, levels, input validation).
- **Sprints:** four one-week sprints plus a setup week, tracked as milestones on GitHub: Sprint 1 a walk through a random maze, Sprint 2 Era I playable from menu to result, Sprint 3 feature complete, Sprint 4 polish, documents and release v1.0.
- **Issues and pull requests:** every task was a GitHub issue with a "done when" list. Work was done on one branch per task and merged with a pull request that names the issue (`Closes #N`). Merges use merge commits, never squash, so every commit stays in the history.
- **Commits:** Conventional Commits (`feat:`, `fix:`, `test:`, `docs:`, `refactor:`, `style:`), at most 50 characters.
- **Definition of done:** it compiles with zero warnings, the naming and comment rules are followed, there is no `cin`, `cout` or raylib in the logic files, the tests are added and pass, and the issue is closed.

## 7. Testing

`tests/tests.cpp` has simple test functions and no test library. Each one prepares a small situation, calls one logic function and checks the result with a `check` helper that prints `PASS` or `FAIL`. The test program returns 0 only if every check passed, so `ctest` can run it.

| Tests | What they check |
|---|---|
| T01 to T03 | Size, number and name validation |
| T04 to T08 | Maze size, start and exit are floor, the exit is reachable, moving outside the maze and into walls |
| T09 to T12 | Doors and keys, oil, torch radius, torch out |
| T13 to T15 | Trap effects, water step cost, campaign stars |
| T16 to T17 | Next player selection and the multiplayer winner |
| T18 to T20 | Save and load, destroying a maze, unlocks and Free Play |
| T21 | The data of all nine levels |
| T22 to T24 | Placing features, the multiplayer start (spawns, random exit, fuel and traps), turn time and the player count and hero checks |
| T25 to T26 | The multiplayer turn rules, the Map and Trap items and the arena and hero tables |

All tests pass. The QA test plan in `docs/qa_test_plan.md` lists the purpose, input, expected result and actual result for each test. Besides the unit tests, scripts generated every campaign maze size from 10 to 50 for all nine levels, and multiplayer games on every arena with 2, 3 and 4 players. In all of them the exit was reachable and the games ended with a winner.

## 8. Credits

- Pixel art generated with ChatGPT and post-processed with Python.
- Graphics and input: [raylib](https://www.raylib.com/) (zlib license).
- Sounds: 10 sound effects generated for this project (CC0, public domain). Font: raylib default font.
