# Beej's Guide to C Examples

`exercises/`: solutions for exercises
`examples/`: code from the main chapters
`examples/atomics/`: code from the main atomics chapter
`examples/threads/`: code from the main threads chapter
`man/`: code from the reference pages

Type `make` in a particular source directory to build those sources.

## Build Notes

The `Makefile`s call `gcc` to build, and are expected to run on a
Unix-like (e.g. Linux, BSD, Mac OSX, Windows WSL).

Linking:

* Math programs are linked with `-lm`.
* Threaded programs are linked with `-lpthread`.
* Atomic programs are linked with `-latomic`.

Not all features are necessarily available on all platforms.
