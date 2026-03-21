# Build & Test

CMake with Ninja. Presets: `debug` (default) and `release`.
Output: `build/debug/lk` and `build/release/lk`.

Tests live in `tests/` as `.lk` files, run via ctest with the same presets.

To run a file: `./build/debug/lk -l stdlib <file.lk>` (`-l stdlib` loads the standard library)
