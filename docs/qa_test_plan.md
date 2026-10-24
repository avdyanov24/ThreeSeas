# QA Test Plan

### T01: Size validation
- **Purpose:** Check that campaign maze sizes follow the allowed limits.
- **Function tested:** `isValidSize`
- **Input:** Campaign sizes 9, 10, 50, and 51.
- **Expected result:** 9 and 51 are rejected; 10 and 50 are accepted.
- **Actual result:** Not run yet

### T02: Number parsing
- **Purpose:** Check that only non-empty digit text is parsed as a number.
- **Function tested:** `parseWholeNumber`
- **Input:** `abc`, empty text, and `12a`.
- **Expected result:** Each input is rejected and the number remains unchanged.
- **Actual result:** Not run yet

### T03: Name validation
- **Purpose:** Check the allowed characters and length for player names.
- **Function tested:** `isValidName`
- **Input:** `Petrov_1`, `Abcdefghijkl`, empty text, `Abcdefghijklm`, and `Petrov 1`.
- **Expected result:** Names of 1 to 12 letters, digits, or underscores are accepted; the other inputs are rejected.
- **Actual result:** Not run yet

### T04: Maze dimensions
- **Purpose:** Check that maze creation makes a square maze of the requested size.
- **Function tested:** `createMaze`
- **Input:** Create a maze with size 10.
- **Expected result:** The maze has 10 rows and 10 columns.
- **Actual result:** Not run yet

### T05: Start and exit tiles
- **Purpose:** Check that generated mazes have a floor start and an exit at the bottom-right.
- **Function tested:** `generatePaths`, `connectExit`
- **Input:** Generate mazes of sizes 10 and 11, then connect each exit.
- **Expected result:** Cell (0, 0) is FLOOR and cell (size-1, size-1) is EXIT in both mazes.
- **Actual result:** Not run yet

### T06: Exit is reachable
- **Purpose:** Check that the exit can be reached from the start in a generated maze.
- **Function tested:** `generatePaths`, `connectExit`
- **Input:** Generate and connect the exit in a maze of size 10; search from (0, 0) using flood fill.
- **Expected result:** Flood fill can reach the EXIT at (9, 9).
- **Actual result:** Not run yet

### T07: Move outside the maze
- **Purpose:** Check that a player cannot move beyond the maze boundary.
- **Function tested:** `tryMove`
- **Input:** In a size 10 maze, put the player at (0, 0) and try to move UP.
- **Expected result:** The move returns false and the player's position stays at (0, 0).
- **Actual result:** Not run yet

### T08: Move into a wall
- **Purpose:** Check that a player cannot move into a wall.
- **Function tested:** `tryMove`
- **Input:** In a size 10 maze, put a player at (0, 0) and try to move RIGHT into a WALL.
- **Expected result:** The move returns false and the player's position stays at (0, 0).
- **Actual result:** Not run yet
