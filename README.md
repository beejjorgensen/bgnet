# Beej's Guide to Network Programming

This is the source for Beej's Guide to Network Programming.

If you merely wish to read the guide, please visit the [Beej's Guide to
Network Programming](https://beej.us/guide/bgnet/) website.

This is here so that Beej has everything in a repo and so translators
can easily clone it.

## Build Instructions

### Dependencies

* [Gnu make](https://www.gnu.org/software/make/) (XCode make works, too)
* [Python 3+](https://www.python.org/)
* [Pandoc 2.7.3+](https://pandoc.org/)
* XeLaTeX (can be found in [TeX Live](https://www.tug.org/texlive/))
* [Liberation fonts](https://en.wikipedia.org/wiki/Liberation_fonts) (sans, serif, mono)

Mac dependencies install (reopen terminal after doing this):

```
xcode-select --install                  # installs make
brew install python                     # installs Python3
brew install pandoc
brew install mactex                     # installs XeLaTeX
brew tap homebrew/cask-fonts
brew install font-liberation            # installs Liberation fonts
```

You might have to add something like this to your path to find `xelatex`:

```
PATH=$PATH:/usr/local/texlive/2021/bin/universal-darwin
```

### Dependency: Build System

This depends on an external repo to build: [Beej's Guide Build System
for Pandoc](https://github.com/beejjorgensen/bgbspd).

You'll want to clone that repo as a sibling to this one:

```
mystuff-->bggit
      \-->bgbspd
```

The Makefiles here will look for the build system there.

You can override the `bgbspd` directory before running `make` like this:

```
export BGBSPD_BUILD_DIR=/some/path/to/bgbspd
```

### Build

1. Type `make all` from the top-level directory.

   If you have Gnu Make, it should work fine.  Other makes might work as
   well.  Windows users might want to check out Cygwin.

2. Type `make stage` to copy all the build products and website to the
   `stage` directory.

3. There is no step three.

You can also `cd` to the `src` directory and `make`.

`make clean` cleans, and `make pristine` cleans to "original" state.

To embed your own fonts in the PDFs, see the `src/Makefile` for examples.

The `upload` target in the root `Makefile` demonstrates the build steps
for a complete release.  You'll need to change the `UPLOADDIR` macro in
the top-level `Makefile` to point to your host if you want to use that.
You're free to upload whatever versions you desire individually, as
well.

### Build via Docker

If you don't want to mess with a local setup, you can build via Docker.

1. Run `docker build -t beej-bgnet-builder .` from the top-level directory.

2. Run `docker run --rm -v "$PWD":/guide -ti beej-bgnet-builder`.

   This will mount the project where the image expects it, and run `make
   pristine all stage`, leaving your `./stage` directory ready to be published.

## Pull Requests

Please keep these on the scale of typo and bug fixes. That way I don't
have to consider any copyright issues when merging changes.

## TODO

### Content

* File transfer example maybe in son of data encapsulation
* Multicast?
* Event IO?

