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
- One blank line between a block of variable declarations and the first non-declaration statement
- One blank line before and after multiline statements (if/for/while/switch) — except no preceding blank line if it's the first thing in a block, and no trailing blank line if it's the last
- One blank line before `else`, regardless of whether braces are involved (`} else {`, `} else`, `else {`, bare `else`)
- One blank line before a comment, except when it's the first thing in a block

## Comments
- Only where the logic isn't self-evident
- Capitalize the first word; no trailing period unless separating multiple sentences within the same comment

## Conventions
- All lk object structs begin with `struct lk_common o` as their first member
- Boolean fields use `bool`, not `int` or `uint8_t`
