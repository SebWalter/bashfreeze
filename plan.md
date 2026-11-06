1. **Fix screen resize issue:**
   - In `bashfreeze.cpp`, the arguments for `resize_and_move` are `height, width, starty, startx, display_rows`. However, in `handleTableSizeChange` they are passing `0, 0, 0, 0, per_table_rows` which results in 0 height/width windows, thus making the TUI disappear or break when resizing.
   - We need to correctly calculate the heights and positions in `handleTableSizeChange` and `updateTableDimensions` to reflect real widths (from `getmaxyx`) and heights. Or just merge them into one logic piece.

2. **Remove stray debug prints:**
   - There's a debug print like "Create ... main processes" still remaining.
   - We should locate it (probably in `processreader.cpp`) and remove it.

3. **Pre-commit checks:**
   - Re-run test/verification pre-commit steps.

4. **Submit:**
   - Submit the fixes.
