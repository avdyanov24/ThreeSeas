# AGENTS.md

Rules for every AI assistant working on this repository. Read all of it before you write or change anything. It is distilled from the team handbook (v1.0, 17 Oct 2026).

## 0. Rules that override everything

1. **Follow this file. Where it is silent, pick the simplest option that fits the rules below and say what you chose.**
2. **Never put yourself in git history.** No `Co-Authored-By`, no "Generated with" line, no AI name in a commit message, author or committer. Commits are made and pushed from the user's own GitHub account. Do not commit or push at all unless the user asks.
3. **Do not add files, folders, tools or config the user has not asked for.** No extra `.md` files, no `.clang-format`, no CI workflow, no helper scripts, no "nice to have" extras.
4. **Keep headers and `types.h` stable.** Change them only when the task needs it, and say so.
5. **No mock data, no invented history.** Every historical fact comes from this file or the handbook.
6. **Everyone must be able to explain their code.** Write simple code a student can explain line by line. Prefer the boring solution.
7. **English only** in code, comments, commit messages, issues, docs and in-game text. Bulgarian names are written in Latin letters (Krum, Simeon, Tarnovo).
8. Only edit what the task needs. One task, one branch, small changes.

## 1. The project

**Three Seas** is a dark dungeon maze game set in Bulgarian history. Every maze is a real historical place. The player carries a torch that burns down; only the torch-lit area is visible. Find the exit before the flame dies.

- School project "Labyrinth", C++17, 2026. Built with **raylib 6.0** (pinned) and **CMake** (FetchContent).
- Repository and executable codename: **crossroads**. The handbook says the real name "Three Seas" is revealed only at the presentation.
- Modes: **Campaign** (1 player, real-time, 9 mazes in 3 eras) and **Multiplayer** (2 to 4 players, pass the laptop, turn-based, each player sees only their own light).
- 6 historical heroes, one always-on power each. 4 items: oil, map, key, trap.
- Everything works offline. No online features, databases, accounts, quizzes, dice, chasing monsters or diagonal movement.
- Team: **AD** Aleksandar Dyanov (Scrum Master + frontend: headers, maze, drawing, art, reviews), **AP** Aleksandar Petrov (QA + backend: tests, QA doc, save, balancing, history fact check), **PT** Presiyan Tenev (frontend: screens, HUD, sound), **LZ** Loris Zadikyan (backend: game rules, levels, validation).

Design pillars: dark and tense; real places and real history; strategy not luck (the only randomness is the maze layout); simple to explain (if a feature cannot be explained in two sentences, cut it).

### Current state

Skeleton only: folders, `CMakeLists.txt`, placeholder `.h`/`.cpp` files (one-line comments), and the finished art in `art/`. The final headers with stub functions still have to be written.

## 2. The golden rule: logic vs. screen

- **Logic files** (`maze`, `game`, `levels`, `validation`, `save`) **never** use `cin`, `cout` or raylib. They receive values and return results. (`save` may use `fstream`.)
- **Screen files** (`render`, `screens`, `audio`, `main`) are the only files that draw, read keys, or play sounds.
- `tests/tests.cpp` may use `cout` to print results. It links only the logic files and runs without a window.

## 3. Folder and file structure

```
crossroads/
├── .github/            issue templates, PR template, CODEOWNERS
├── art/                finished pixel art (PNG only)
├── assets/             textures/  sounds/  fonts/  maps/   (what the game loads)
├── docs/               screenshots, technical doc, QA doc, presentation
├── src/
│   ├── main.cpp                  window + main loop
│   ├── types.h                   all enums and structs
│   ├── maze.h / maze.cpp         create, generate, destroy the maze
│   ├── game.h / game.cpp         movement, fuel, items, turns, winning
│   ├── levels.h / levels.cpp     the 9 levels, 3 arenas, 6 characters
│   ├── validation.h / .cpp       checking user input
│   ├── save.h / save.cpp         leaderboard and settings files
│   ├── render.h / render.cpp     drawing the maze, light, HUD
│   ├── screens.h / screens.cpp   menus and all other screens
│   └── audio.h / audio.cpp       loading and playing sounds
├── tests/tests.cpp     QA test functions
├── .gitignore          ignores build/ and save/
├── CMakeLists.txt
└── README.md
```

| Module | Job | Owner | cin/cout or raylib? |
|---|---|---|---|
| main.cpp | Window, main loop, switching screens | AD | raylib yes |
| types.h | All enums and structs | AD | no |
| maze | Create, generate, place features, destroy | AD | no |
| game | Moving (AD); fuel, items, traps, turns, stars, winner (LZ) | AD / LZ | no |
| levels | Level, arena, character tables; unlock checks | LZ | no |
| validation | Input checks | LZ | no |
| save | Read and write save files | AP | no (fstream ok) |
| render | Draw tiles, light, players, HUD | AD | raylib yes |
| screens | Menus, map, inputs, results | PT | raylib yes |
| audio | Sounds | PT | raylib yes |
| tests | Test functions | AP | cout yes |

One person per file at a time. Do not edit a file owned by someone else without being asked.

## 4. How to write code

### 4.1 C++ we use, and don't

| Use | Do NOT use |
|---|---|
| `int`, `float`, `bool`, `char`, `std::string` | Classes, inheritance, templates |
| `if`, `switch`, `for`, `while` | `std::vector` for the three graded arrays |
| Functions with parameters and return values | Smart pointers, lambdas |
| References `&` and `const &` for parameters | Global variables (except constants) |
| `struct` and `enum` | Anything a team member cannot explain |
| Pointers + `new` / `delete[]` for dynamic arrays | |
| Header (`.h`) and source (`.cpp`) files | |
| Recursion (only in `generatePaths`) | |
| `rand()` / `srand()`, `fstream`, `const` values | |
| About 15 raylib functions | |

Rule of thumb: pointers for dynamic memory, references for function parameters, `const &` when a function only reads.

### 4.2 Naming

| What | Style | Example |
|---|---|---|
| Files | snake_case | `maze.cpp`, `render.h` |
| Functions | camelCase, start with a verb | `movePlayer()`, `drawHud()` |
| Variables, parameters | camelCase, full words | `torchRadius`, `playerCount` |
| Booleans | start with `is` / `has` / `can` | `isOut`, `hasKey`, `canEnter` |
| Structs, enum types | PascalCase | `Player`, `Tile` |
| Enum values, constants | UPPER_SNAKE_CASE | `WALL`, `MAX_SIZE` |
| Loop counters | short names allowed only here | `i`, `row`, `col` |

### 4.3 Comments and formatting

- A comment above **every** function: what it does and what it returns.
- Comment tricky lines to explain *why*, not *what*.
- No commented-out old code.
- Format: LLVM style, 4-space indent, 100 columns. Match the surrounding code.
- Small functions, one job each. Headers say what exists; `.cpp` says how.

### 4.4 Warnings

Code must compile with **zero warnings**. The build uses `-Wall -Wextra -Werror` (Mac/Linux/MinGW) and `/W4 /WX` (MSVC).

### 4.5 Data types (`types.h`, AD's design)

```cpp
enum Tile { FLOOR, WALL, DOOR, WATER, EXIT };
enum Item { NO_ITEM, OIL, MAP_ITEM, KEY, TRAP_ITEM };
enum Direction { UP, DOWN, LEFT, RIGHT };
enum CharacterId { ASPARUH, KRUM, BORIS, SIMEON, IVAN_ASEN, LEVSKI };
struct Cell   { Tile tile; Item item; int trapOwner; };   // -1 none, -2 level trap, 0..3 player
struct Maze   { int size; Cell** cells; };                // size x size, 2D dynamic array
struct Player {
    std::string name; CharacterId character; int row, col;
    float fuel, maxFuel; int maps, traps; bool hasKey;
    bool isOut; int turnsToSkip; float stuckSeconds; bool** seen;
};
struct Level  { int id; std::string place, era, funFact; int year, difficulty;
                float fuelFactor, oilPer100, trapsPer100; int doors; bool hasWater, isCold; };
struct GameState { Maze maze; Player* players; int playerCount, currentPlayer;
                   const Level* level; bool isMultiplayer; float timeSeconds; int winner; };
```

Winner: `-1` nobody yet, `-2` draw.

### 4.6 Dynamic memory (10 graded points)

- Three dynamic arrays: `Cell**` (maze), `Player*` (players), `bool**` (each player's memory).
- Every `new` has a matching `delete[]`. After `delete[]`, set the pointer to `nullptr`.
- Free memory when a maze ends, when quitting to the menu, and when the window closes.
- No `std::vector` for these three arrays.

### 4.7 Planned function list

| Module | Functions |
|---|---|
| maze | createMaze, destroyMaze, isInside, generatePaths (recursive), connectExit, placeFeatures |
| game | startLevel, createPlayers, destroyPlayers, canEnter, tryMove, updateCampaign, burnFuel, getTorchRadius, isLit, updateSeen, pickUpItem, useMap, placeTrap, hasReachedExit, calculateStars, nextTurn, checkWinner |
| levels | getLevel, getArena, getCharacter, isLevelUnlocked, isCharacterUnlocked |
| validation | parseWholeNumber, isValidSize, isValidName, isValidPlayerCount, isCharacterTaken |
| save | loadResults, saveResult, getBestResult, loadWins, addWin, loadSettings, saveSettings |
| render | loadTextures, unloadTextures, drawMaze, drawTorchLight, drawPlayer, drawHud |
| screens | one `update...` and one `draw...` function per screen, plus drawButton, drawTextInput |
| audio | loadSounds, playSound, setSoundEnabled, unloadSounds |

### 4.8 Maze generation (recursive backtracker)

1. Fill the whole maze with walls. "Rooms" are cells with even coordinates; cells between them are walls that can be knocked down.
2. Start in room (0,0), make it floor. Visit the 4 neighbouring rooms (2 cells away) in random order.
3. If a neighbour is still a wall: knock down the wall between and recurse from that room. When no unvisited neighbours remain, return (backtrack).
4. For even sizes the bottom-right cell is not a room: `connectExit` carves a 2-cell connection to it.

Result: a perfect maze (exactly one path between any two cells). Doors then add loops. **Start is always (0,0), exit always (size-1, size-1), both always floor. Every maze must be finishable without doors, keys, or water.**

### 4.9 Placing features

| Feature | Rule |
|---|---|
| Doors | Only on wall cells with floor on two opposite sides; always an extra shortcut |
| Key | One key if the maze has doors, on a random floor cell in the half far from the start |
| Water | Short "river" lines on floor cells. Walkable (slow), never blocks the path |
| Traps | Random floor cells, never on the start, exit, or next to the start |
| Oil, maps | Random floor cells, never on the start or exit |

### 4.10 Torch, memory, and one rule set for two modes

- `isLit`: cell is lit if `dRow*dRow + dCol*dCol <= radius*radius`. After every move `updateSeen` marks lit cells as seen. Drawing has 3 states: lit (full colour), seen (dim, items hidden), unknown (black).
- **Campaign** (real-time): `updateCampaign(game, heldDirection, deltaTime)` moves one cell per 0.12 s (0.24 s on water) and burns fuel by time.
- **Multiplayer** (turn-based): each arrow press calls `tryMove`; after 3 steps or an item, `nextTurn` burns fuel and picks the next player who is not out.
- Both modes share `tryMove`, `pickUpItem`, `getTorchRadius`, `hasReachedExit`. Write once, use twice.

### 4.11 Save files

Plain text in `save/` (git-ignored), read and written with `fstream`, created automatically. One record per line, values separated by spaces (so names cannot contain spaces).

| File | Line format | Example |
|---|---|---|
| `save/results.txt` | `name levelId bestSeconds stars` | `Ivan 3 84.5 2` |
| `save/wins.txt` | `name wins` | `Ivan 5` |
| `save/settings.txt` | `key value` | `sound 1` / `freeplay 0` |

A maze is unlocked when the player has a result for the maze before it. **Missing or broken files must never crash the game**: start with empty data and skip broken lines.

### 4.12 Input validation

| Input | Rule | Error message |
|---|---|---|
| Maze size (campaign) | Whole number 10 to 50 | "Enter a whole number from 10 to 50." |
| Maze size (multiplayer) | Whole number 20 to 50 | "Enter a whole number from 20 to 50." |
| Any number | Digits only, not empty (`12a`, `abc`, `""` rejected) | same as above |
| Player name | 1 to 12 characters: letters, digits, `_`; no spaces | "Use 1–12 letters or numbers, no spaces." |
| Player count | 2 to 4 | "Choose 2 to 4 players." |
| Character (multiplayer) | Not already taken | "Already taken by <name>." |

### 4.13 Main loop and screens

```cpp
enum Screen { TITLE, NAME_ENTRY, MAIN_MENU, CAMPAIGN_MAP, LEVEL_INTRO, PLAYING, PAUSED,
              RESULT, MP_SETUP, PASS_SCREEN, MP_RESULT, LEADERBOARD, HOW_TO_PLAY, SETTINGS };
// per frame: screen = updateScreen(screen, game, deltaTime);  // input + logic
//            BeginDrawing(); drawScreen(screen, game); EndDrawing();   // drawing only
```

Window 1280x720, 60 FPS, camera follows the player (a 50x50 maze must work).

## 5. Game rules (the numbers are starting values; AP tunes)

### 5.1 Controls

Arrow keys move (hold to walk, about 8 cells/s, arrows only, no diagonals). `M` uses a Map. `Esc` pause/back. Mouse/Enter for menus. Multiplayer: `1` Map, `2` Trap, `Enter` ends the turn early.

### 5.2 Torch

- Burns 1 fuel per 2 s (twice as fast in cold places). Oil refills 25% of max fuel, never above max. At 0 fuel: torch out, restart the same maze (new random layout).
- Radius: more than 60% fuel = 3 cells; 30% to 60% = 2; below 30% = 1 (screen flickers); 0 = out. Boris I adds +1.

### 5.3 Items and tiles

| Item | Effect |
|---|---|
| Oil | +25% of max fuel |
| Map | Press M: permanently reveals radius 4 around you (Simeon: 8) |
| Key | Opens every locked door in the maze; you keep it (Ivan Asen II needs none) |

| Tile | Meaning |
|---|---|
| Floor / Wall | Walkable / not |
| Locked door | Shortcut through a wall, never needed to finish |
| Water | Walkable at half speed (multiplayer: costs 2 of 3 steps) |
| Hidden trap | Invisible until adjacent. Campaign: stuck 2 s then it disappears. Multiplayer: lose next turn, trap used up |
| Exit | Bottom-right corner |

### 5.4 Stars (campaign)

3 stars: time <= size x 4 s. 2 stars: <= size x 7 s. 1 star: any finish. Reaching the exit with at least 1 star unlocks the next maze.

### 5.5 Progress

Only Pliska is open at the start. Finishing a maze opens the next. Finishing an era's hard maze opens the next era and that era's multiplayer arena. Characters unlock when their era is entered. **Free Play** (Settings): unlocks everything instantly, is not saved as progress.

### 5.6 Multiplayer

2 to 4 players, each character once per game. Maze size 20 to 50 (30 suggested). Start fuel = size x 2. On a turn: move up to 3 steps **or** use one item (ends the turn); torch loses 1 fuel (2 in the cold arena). Every player starts with 1 Trap. Traps sit on your own cell, invisible to others (except Krum), harmless to you. First player on the exit wins; a player at 0 fuel is out; last torch burning wins; last torches out in the same round = draw. A black "Pass to <name>. Press Enter." screen separates turns.

Arenas: I Pliska (traps, doors, keys), II Tsarevets/Tarnovo (+ water), III Balkan Pass (+ cold, torch x2). Each unlocks when its era is finished.

### 5.7 Heroes

| Hero | Era | Power |
|---|---|---|
| Khan Asparuh | I | +50% starting fuel |
| Khan Krum | I | Sees all traps |
| Knyaz Boris I | I | Torch radius +1 |
| Tsar Simeon | I | Maps reveal double the area |
| Tsar Ivan Asen II | II | Opens doors without a key |
| Vasil Levski | III | Immune to traps |

In the campaign the player picks any unlocked hero; the era's hero (Krum, Ivan Asen II, Levski) is "suggested". Art rule: respectful look, no gore. Krum's skull cup is mentioned only in a fun fact, never drawn.

### 5.8 Levels and difficulty

Difficulty comes only from these numbers. Changing difficulty means changing numbers, never new code.

| # | Maze | Year | Era | Start fuel | Oil/100 cells | Traps/100 | Doors | Water | Cold |
|---|---|---|---|---|---|---|---|---|---|
| 1 | Pliska | 681 | I | size x 4.0 | 3 | 0 | 0 | no | no |
| 2 | Varbitsa Pass | 811 | I | size x 3.5 | 2 | 2 | 0 | no | no |
| 3 | Preslav | 893 | I | size x 3.0 | 1.5 | 1 | 3 | no | no |
| 4 | Tarnovo | 1185 | II | size x 3.75 | 3 | 0 | 2 | no | no |
| 5 | Cherven | 13th-14th c. | II | size x 3.25 | 2 | 0 | 0 | yes | no |
| 6 | Klokotnitsa | 1230 | II | size x 2.75 | 1.5 | 2 | 3 | yes | no |
| 7 | Koprivshtitsa | 1876 | III | size x 3.5 | 2.5 | 1 | 2 | no | no |
| 8 | Plovdiv | 1885 | III | size x 3.0 | 2 | 0 | 2 | yes | no |
| 9 | Shipka | 1877 | III | size x 2.5 | 1.5 | 2 | 3 | yes | yes |

Every maze has 2 Map items on easy mazes and 1 on medium and hard. Mazes with doors have exactly 1 Key. Each era adds one mechanic: I traps and doors, II water, III cold.

## 6. History (must be 100% correct)

One or two short sentences per level, shown on the level intro screen. Use only these.

| Maze | Fun fact |
|---|---|
| Pliska (681) | "Pliska was Bulgaria's first capital, from 681 until 893." |
| Varbitsa Pass (811) | "In 811 Khan Krum trapped the Byzantine army here. Legend says he turned the emperor's skull into a silver cup." |
| Preslav (893) | "Under Simeon, Preslav became the heart of Bulgaria's Golden Age of books and learning." |
| Tarnovo (1185) | "In 1185 the brothers Asen and Peter rose up in Tarnovo and brought Bulgaria back to life." |
| Cherven (13th-14th c.) | "Cherven sat on a cliff above a river canyon. It had more medieval church inscriptions than the capital Tarnovo." |
| Klokotnitsa (1230) | "At Klokotnitsa in 1230, Ivan Asen II rode into battle with the broken peace treaty on his spear." |
| Koprivshtitsa (1876) | "The first shot of the April Uprising was fired at a bridge in Koprivshtitsa on 20 April 1876." |
| Plovdiv (1885) | "On 6 September 1885 Bulgaria was united again, starting in Plovdiv." |
| Shipka (1877) | "In 1877 Bulgarian volunteers held Shipka against a far bigger army, then survived a frozen winter on the peak." |

Eras: I First Bulgarian State (681-1018, map at Simeon's peak), II Second Bulgarian State (1185-1396, map at Ivan Asen II's peak), III Road to Liberation (1870s-1885). Levski died in 1873, before the Era III events; he stays as the era's hero because his committee network prepared the 1876 uprising. The cold mechanic comes from the "Winter Shipka" defence. Do not add facts, dates or quotes that are not in this file or the handbook.

## 7. Screens, UI and art

### 7.1 Screens

Title -> Name entry -> Main menu (Campaign, Multiplayer, Leaderboard, How to Play, Settings, Exit). Campaign map (era tabs, 3 pins: grey locked, glowing open, gold finished) -> Level intro (place, year, fun fact, difficulty, best stars, character, size input) -> Gameplay (+ Pause: Resume, Restart, Quit to map) -> Result (win: time, stars, best time; lose: "Your torch went out"). Multiplayer: setup (arena, players, names + characters, size) -> Pass screen -> turns -> MP result (winner or draw, Rematch/Menu). Settings: sound on/off, Free Play on/off.

### 7.2 UX rules

- Every screen has a way back (Back button and `Esc`).
- Everything works with the keyboard; menus also with the mouse.
- Errors in red under the input, plain words (see 4.12).
- Buttons look the same everywhere. Never more than 6 buttons on one screen.
- Something always shows the game is alive (a small animation or sound per action).

### 7.3 HUD

Torch bar with flame icon and percentage (red and flickering below 30%). Campaign: timer. Multiplayer: player name, turn number, steps left. Inventory: Map x n, Trap x n, Key. Place and year (e.g. "Varbitsa Pass · 811") and a hero portrait.

### 7.4 Visual style

Pixel art, old dark dungeon. Very dark background, warm torch orange, parchment and gold for menus. One free-licence pixel font (e.g. Press Start 2P, OFL). A dark layer over the maze with a soft, slightly flickering circle of light around the player. **Always scale by whole numbers with nearest-neighbour** so pixels stay sharp.

### 7.5 Art in `art/`

The shipped art uses a **32 px native grid** (the handbook's 16 px was replaced because 16 px turned walls, doors and faces to mush). Draw at 2x (64 px on screen).

| Asset | Size |
|---|---|
| Tiles, props, items, effects, icons, pins | 32x32 |
| Characters | 32x48 (left-facing is a mirror of the side view, done in code) |
| Portraits | 128x128 |
| Intro paintings, era maps, screens | 1280x720 |

Layout: `01_ui`, `02_era_maps` (3 maps + pins), `03_levels/NN_name` (`tiles/`, `props/`, `intro_background/`), `04_items`, `05_characters/<name>` (`sprites/`, `portrait/`), `06_effects`, `07_screens`. Keep file names as they are. Do not redraw or recolour art without being asked.

### 7.6 Sounds

Footstep, torch crackle (loop), low-torch warning (below 30%), pickups, door unlock, trap snap, win jingle / torch out, UI click, multiplayer turn chime. Only CC0 sources (Kenney.nl, freesound CC0) or self-generated.

### 7.7 Credits rule

Everything not made by the team (AI images, sounds, font, reference maps) is listed with source and licence in the README and the technical documentation. Be honest: it protects the "own elements" points.

## 8. Build and test

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/crossroads
ctest --test-dir build --output-on-failure
```

CMake targets: `logic` (library, no raylib), `crossroads` (executable, links `logic` + raylib), `tests` (links `logic` only). Never make `logic` depend on raylib.

### 8.1 Tests (`tests/tests.cpp`, AP)

Simple test functions, no test library. Each test sets up a small situation, calls one logic function, and checks with a `check()` helper that prints PASS or FAIL. Required list:

T01 size validation (9, 51 rejected; 10, 50 accepted) · T02 number parsing (`abc`, `""`, `12a` rejected) · T03 name validation · T04 `createMaze(n)` makes n x n · T05 start and exit are floor (odd and even sizes) · T06 exit reachable (flood fill) · T07 moving outside fails · T08 moving into a wall fails · T09 doors (blocked, key opens, Ivan Asen II opens) · T10 oil +25% capped · T11 torch radius at 100/50/10%, Boris +1 · T12 torch out at 0 · T13 traps (others lose a turn, owner safe, Levski immune, Krum sees) · T14 water costs 2 steps in multiplayer · T15 stars · T16 turns skip players who are out and wrap around · T17 winner (first at exit, last torch, draw) · T18 save and load round-trip, missing file does not crash · T19 after `destroyMaze` pointer is `nullptr` and size 0 · T20 unlocks and Free Play.

Write tests from the headers, even before the code exists. Any logic change needs its test added or updated, and all tests must pass.

## 9. Git workflow

- `main` always builds and runs. Nobody pushes to it directly; every change goes through a pull request.
- One branch per task: `feature/…`, `fix/…`, `docs/…`, `test/…`, lowercase with hyphens (`feature/maze-generation`). Delete after merge.
- **Merge commits only, never squash** (commits are graded).
- Pull `main` every day before starting; push every day, even unfinished work on your branch.
- Small pull requests (ideally under 300 changed lines). Title like a commit. Description says what changed and `Closes #N`. Answer every review comment on the same branch.
- Never rewrite or force-push shared history unless the user says so.

### Commit messages (Conventional Commits)

Format `type: what the commit does`. Types: `feat`, `fix`, `test`, `docs`, `refactor`, `style`, `chore`. Imperative mood ("add", not "added"), title at most 50 characters, no full stop, a description below when the why is not obvious. One idea per commit. Never "fix", "update" or "asdf".

Examples: `feat: add torch radius based on fuel`, `fix: stop player walking through locked doors`, `test: add wall collision test`, `docs: describe maze generation in README`, `chore: add Era II tileset`.

### Definition of Done

Compiles with zero warnings · naming rules followed · comment above every function · no `cin`/`cout`/raylib in logic files · tests added or updated and passing · pull request reviewed and merged · issue closed.

## 10. Scope priority

| Priority | Features |
|---|---|
| Must | Maze generation, movement, torch and memory, oil / map / key / doors, the 3 Era I mazes, menus, name and size validation, result and stars, dynamic memory, tests, documents |
| Should | Eras II and III, traps, water, cold, Multiplayer Arena I, characters and powers, save files and leaderboard, sound, Free Play |
| Could | Arenas II and III, animations, screen shake, animated map borders |
| Won't | Online play, databases, quizzes, dice, chasing monsters, AI inside the game |

Feature freeze: Sunday 8 November 2026. After that only fixes, comments, formatting and documents. Deadline and release v1.0: Sunday 15 November 2026.

## 11. Checklist before you hand work back

1. Did I only touch what was asked, and add nothing unrequested?
2. Did I keep headers and `types.h` unchanged unless the task needed it?
3. Zero warnings? Naming, comments and formatting match 4.1 to 4.3?
4. No `cin`/`cout`/raylib in logic files? No `std::vector` for the graded arrays? Every `new` has its `delete[]`?
5. Tests added or updated and passing?
6. Did I state any assumption I made?
7. No AI attribution anywhere in commits, and no commit or push unless the user asked?
