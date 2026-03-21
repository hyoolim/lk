# Code Style

## Standard
C11. Style choices worth noting:
- Variable declarations inline, close to first use (including `for` loop init)
- `noreturn` / `stdnoreturn.h` — not `__attribute__((noreturn))`
- `_Static_assert` to document implicit assumptions (e.g. alignment requirements, struct layout)

## Formatting
`.clang-format` is present — run `clang-format -i <file>` to format.

## Blank line rules
- One blank line between functions
- One blank line before and after all control flow statements (if/for/while/switch), including braceless ones — except no preceding blank line if it's the first thing in a block, and no trailing blank line if it's the last
- One blank line before `else`, regardless of whether braces are involved (`} else {`, `} else`, `else {`, bare `else`)
- One blank line before a comment, except when it's the first thing in a block

## Naming
- `size`: size of a C data structure in bytes
- `length`: number of elements in a dynamic array, or number of characters in a string
- `at`: index into a string or list
- `first`: pointer to the start of the active range within a buffer (may not be the buffer's start when sliced)
- `start` / `end`: absolute index bounds for a slicing operation
- `offset` / `limit`: relative slicing — `offset` skips n elements from the front, `limit` caps the length; together equivalent to `start = offset`, `end = offset + limit`

## Comments
- Only where the logic isn't self-evident
- Capitalize the first word; no trailing period unless separating multiple sentences within the same comment
- Write out full words even when the C identifiers are abbreviated (e.g. "vector" not "vec", "character" not "char")

## Conventions
- All lk object structs begin with `struct lk_common o` as their first member
- Boolean fields use `bool`, not `int` or `uint8_t`
- Use `#pragma region <name>` / `#pragma endregion` for IDE folding section markers (clang silently ignores them)
