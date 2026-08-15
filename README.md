# c2arm
A lightweight teaching-level C compiler targeting Linux ARM64!

## Phase 1 Goals
* Bare and merged `int` declarations, plus `int` reassignment
* `void`/`void` functions
* Single `.c` file, no libraries or headers

## Design Philosophy
As this is a teaching tool, modularity is valued over speed. Everything is designed to make the full pipeline of a C compiler incredibly clear!

## How to Run
`make` runs the `Makefile`, and `./compiler file.c` will run it.

To execute the program, simple type `./out`. If not on Linux ARM64, run

``` bash
qemu-aarch64 ./out
```

to get the same result.

## Note on AI usage
This code was written wholly and completely by the author of this repository. Anthropic's Claude was quite helpful for  debugging and research, but never to write any of the code in this repository.
