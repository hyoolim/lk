# Build & Test

CMake with Ninja. Presets: `debug` (default) and `release`.
Output: `build/debug/lk` and `build/release/lk`.

**Configure before navigating code.** clangd (LSP) reads `compile_commands.json` from `build/debug/`. Run `cmake --preset debug` (configure only, no compilation) to generate it. Without it, clangd has no include paths and type resolution will be wrong.

Tests live in `tests/` as `.lk` files, run via ctest with the same presets.

To run a file: `./build/debug/lk -l stdlib <file.lk>` (`-l stdlib` loads the standard library)
