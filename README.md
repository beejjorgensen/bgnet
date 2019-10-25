# Beej's Guide to Network Programming

This is the source for Beej's Guide to Network Programming.

If you merely wish to read the guide, please visit the [Beej's Guide to
Network Programming](https://beej.us/guide/bgnet/) website.

This is here so that Beej has everything in a repo and so translators
can easily clone it.

## Build Instructions

You'll need:

* [Gnu make](https://www.gnu.org/software/make/)
* [Pandoc 2.7.3+](https://pandoc.org/)

1. Type `make` from the top-level directory.

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

## Pull Requests

Please keep these on the scale of typo and bug fixes. That way I don't
have to consider any copyright issues when merging changes.

## TODO

### Content

* File transfer example maybe in son of data encapsulation
* Multicast?
* Event IO?

### Bug fixes

* When pandoc 2.8 comes up, switch all man page subheaders to h3 and supress
  them from the table of contents.

