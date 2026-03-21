# Link Programming Language

Link is a systems language currently in early development. It is dynamically typed and prototype-based today, with class-based OOP and static typing planned. The current implementation is a tree-walking interpreter written in C11. The file extension is `.lk`, the executable is `lk`, and the C identifier prefix is `lk_`.

Core stdlib is in `stdlib/`, loaded at runtime via `-l stdlib`. `stdlib/Init.lk` is the primary stdlib file.

## Instructions

- For writing code: @.ai/code.md
- For building and testing: @.ai/build.md
