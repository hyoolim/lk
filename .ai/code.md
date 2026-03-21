# Code Style

## Standard
C11. Style choices worth noting:
- Variable declarations inline, close to first use (including `for` loop init)
- `noreturn` / `stdnoreturn.h` — not `__attribute__((noreturn))`
- `_Static_assert` to document implicit assumptions (e.g. alignment requirements, struct layout)

## Formatting
`.clang-format` is present — run `clang-format -i <file>` to format.

## Conventions
- All lk object structs begin with `struct lk_common o` as their first member
- Boolean fields use `bool`, not `int` or `uint8_t`
