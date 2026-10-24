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
