# QA Test Plan

### T01: Size validation

- **Purpose:** Check that campaign maze sizes follow the allowed limits.
- **Function tested:** `isValidSize`
- **Input:** Campaign sizes 9, 10, 50, and 51.
- **Expected result:** 9 and 51 are rejected; 10 and 50 are accepted.
- **Actual result:** Passed on 3 Nov 2026 (8 checks, all PASS)

### T02: Number parsing

- **Purpose:** Check that only non-empty digit text is parsed as a number.
- **Function tested:** `parseWholeNumber`
- **Input:** `abc`, empty text, and `12a`.
- **Expected result:** Each input is rejected and the number remains unchanged.
- **Actual result:** Passed on 3 Nov 2026 (8 checks, all PASS)

### T03: Name validation

- **Purpose:** Check the allowed characters and length for player names.
- **Function tested:** `isValidName`
- **Input:** `Petrov_1`, `Abcdefghijkl`, empty text, `Abcdefghijklm`, and `Petrov 1`.
- **Expected result:** Names of 1 to 12 letters, digits, or underscores are accepted; the other inputs are rejected.
- **Actual result:** Passed on 3 Nov 2026 (6 checks, all PASS)

### T04: Maze dimensions

- **Purpose:** Check that maze creation makes a square maze of the requested size.
- **Function tested:** `createMaze`
- **Input:** Create a maze with size 10.
- **Expected result:** The maze has 10 rows and 10 columns.
- **Actual result:** Passed on 3 Nov 2026 (12 checks, all PASS)

### T05: Start and exit tiles

- **Purpose:** Check that generated mazes have a floor start and an exit at the bottom-right.
- **Function tested:** `generatePaths`, `connectExit`
- **Input:** Generate mazes of sizes 10 and 11, then connect each exit.
- **Expected result:** Cell (0, 0) is FLOOR and cell (size-1, size-1) is EXIT in both mazes.
- **Actual result:** Passed on 3 Nov 2026 (16 checks, all PASS)

### T06: Exit is reachable

- **Purpose:** Check that the exit can be reached from the start in a generated maze.
- **Function tested:** `generatePaths`, `connectExit`
- **Input:** Generate and connect the exit in a maze of size 10; search from (0, 0) using flood fill.
- **Expected result:** Flood fill can reach the EXIT at (9, 9).
- **Actual result:** Passed on 3 Nov 2026 (4 checks, all PASS)

### T07: Move outside the maze

- **Purpose:** Check that a player cannot move beyond the maze boundary.
- **Function tested:** `tryMove`
- **Input:** In a size 10 maze, put the player at (0, 0) and try to move UP.
- **Expected result:** The move returns false and the player's position stays at (0, 0).
- **Actual result:** Passed on 3 Nov 2026 (12 checks, all PASS)

### T08: Move into a wall

- **Purpose:** Check that a player cannot move into a wall.
- **Function tested:** `tryMove`
- **Input:** In a size 10 maze, put a player at (0, 0) and try to move RIGHT into a WALL.
- **Expected result:** The move returns false and the player's position stays at (0, 0).
- **Actual result:** Passed on 3 Nov 2026 (6 checks, all PASS)

### T09: Door access

- **Purpose:** Check that a locked door needs a key unless the player is IVAN_ASEN.
- **Function tested:** `canEnter`
- **Input:** Check a DOOR cell for a player without a key, a player with a key, and an IVAN_ASEN player.
- **Expected result:** The player without a key cannot enter; the other two can enter.
- **Actual result:** Passed on 3 Nov 2026 (3 checks, all PASS)

### T10: Oil fuel refill

- **Purpose:** Check that oil adds 25% of maximum fuel without going over the maximum.
- **Function tested:** `pickUpItem`
- **Input:** Pick up OIL with maxFuel 100 and fuel 50, then with fuel 90.
- **Expected result:** Fuel becomes 75 after the first pickup and 100 after the second.
- **Actual result:** Passed on 3 Nov 2026 (3 checks, all PASS)

### T11: Torch radius

- **Purpose:** Check torch radius at 100%, 50%, and 10% fuel, including BORIS.
- **Function tested:** `getTorchRadius`
- **Input:** Check normal players and BORIS with maxFuel 100 and fuel 100, 50, and 10.
- **Expected result:** Normal radii are 3, 2, and 1; BORIS adds 1 at each fuel level.
- **Actual result:** Passed on 3 Nov 2026 (12 checks, all PASS)

### T12: Torch out

- **Purpose:** Check that the torch is out when fuel reaches 0.
- **Function tested:** `burnFuel`
- **Input:** Burn 1 fuel from a player with fuel 1.
- **Expected result:** Fuel is 0 and isOut is true.
- **Actual result:** Passed on 3 Nov 2026 (4 checks, all PASS)

### T13: Trap effects

- **Purpose:** Check trap effects and note that no header function checks Krum's trap visibility.
- **Function tested:** `pickUpItem`
- **Input:** In multiplayer, enter another player's trap as a different player, its owner, and LEVSKI; check Krum visibility.
- **Expected result:** Another player loses a turn; the owner and LEVSKI do not. Krum visibility is not testable here.
- **Actual result:** Passed on 5 Nov 2026 (3 checks, all PASS)

### T14: Water step cost

- **Purpose:** Check that entering WATER costs 2 multiplayer steps.
- **Function tested:** `getStepCost`
- **Input:** Check the step cost for a WATER cell in a multiplayer maze.
- **Expected result:** The step cost is 2.
- **Actual result:** Passed on 5 Nov 2026 (5 checks, all PASS)

### T15: Campaign stars

- **Purpose:** Check star ratings at the documented time limits.
- **Function tested:** `calculateStars`
- **Input:** For size 10, check times of 40, 70, and 71 seconds.
- **Expected result:** The results are 3, 2, and 1 star, respectively.
- **Actual result:** Passed on 3 Nov 2026 (3 checks, all PASS)

### T16: Next player selection

- **Purpose:** Check that nextTurn skips players who are out and wraps around.
- **Function tested:** `nextTurn`
- **Input:** With 3 players, currentPlayer 2 is active and player 0 is out; call nextTurn.
- **Expected result:** The next currentPlayer is 1.
- **Actual result:** Passed on 6 Nov 2026 (2 checks, all PASS)

### T17: Multiplayer winner

- **Purpose:** Check winner selection for an exit, the last burning torch, and a draw.
- **Function tested:** `checkWinner`
- **Input:** Check a player at EXIT, one player with fuel remaining, and all players out in the same round.
- **Expected result:** The player at EXIT wins; otherwise the last burning player wins, or the result is DRAW.
- **Actual result:** Passed on 6 Nov 2026 (6 checks, all PASS)

### T18: Save and load results

- **Purpose:** Check that results can be saved and loaded and a missing file is handled.
- **Function tested:** `saveResult`, `loadResults`
- **Input:** Save and load the documented record `Ivan 3 84.5 2`; load results when `save/results.txt` is missing.
- **Expected result:** The record is read back; a missing file returns 0.
- **Actual result:** Passed on 6 Nov 2026 (1 checks, all PASS)

### T19: Destroy maze

- **Purpose:** Check that destroying a maze clears its pointer and size.
- **Function tested:** `destroyMaze`
- **Input:** Create a size 10 maze and call destroyMaze.
- **Expected result:** `cells` is nullptr and `size` is 0.
- **Actual result:** Passed on 6 Nov 2026 (3 checks, all PASS)

### T20: Unlocks and Free Play

- **Purpose:** Check level, character, and arena unlock rules with and without Free Play.
- **Function tested:** `isLevelUnlocked`, `isCharacterUnlocked`, `isArenaUnlocked`
- **Input:** Check level 2 without a previous result; KRUM before era 1 is entered; arena 1 with 0 eras finished; then check each with Free Play.
- **Expected result:** The first checks are locked; Free Play opens each one.
- **Actual result:** Passed on 6 Nov 2026 (4 checks, all PASS)

### T22: Place features

- **Purpose:** Check that doors, the key, oil, maps and level traps are placed in legal cells.
- **Function tested:** `placeFeatures`
- **Input:** Place the features of level 3 on a size 30 maze and of level 1 on a size 20 maze.
- **Expected result:** The start and exit stay clear, there is one key and the level's doors, items lie on floor, traps are not next to the start, the exit is reachable without doors, and a level without doors has no key.
- **Actual result:** Passed on 6 Nov 2026 (9 checks, all PASS)
