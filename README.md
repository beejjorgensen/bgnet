# Beej's Guide to Network Programming

This is the source for Beej's Guide to Network Programming.

If you merely wish to read the guide, please visit the [Beej's Guide to
Network Programming](https://beej.us/guide/bgnet/) website.

This is here so that Beej has everything in a repo and so translators
can easily clone it.

## Build Instructions

You'll need:

* [Gnu make](https://www.gnu.org/software/make/)
* [Python 2.4+](https://www.python.org/)
* [Apache Xerces-C](https://xerces.apache.org/xerces-c/) (for
  validation; see step 0, below)
* [Apache FOP](https://xmlgraphics.apache.org/fop/) (or hack in some
  other FO processor for print output)

0. If you don't have Xerces-C, go to `bin/bgvalidate` and uncomment the
   `disable=1` line to disable validation.

1. Type `make` from the top-level directory.

   If you have Gnu Make, it should work fine.  Other makes might work as
   well.  Windows users might want to check out Cygwin.

2. Type `make buildcp` to copy all the build products and website to the
   `build` directory.

3. There is no step three.

You can also `cd` to anywhere in the `builders` directory tree and
`make`.

`make clean` cleans, and `make pristine` cleans to "original" state.

To embed your own fonts in the PDFs, see the file
`builders/print/fop.xconf` which already embeds the Liberation Fonts
into the PDF.

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

### In `bg2fo`

* URL hyperlinks in PDF
* Auto-manpage links in <func> tags.

      <code file="..." />
      <code annotate="y">  ]]><foo>blah</foo><![CDATA[

### In `bg2html`

* Auto-manpage links in <func> tags.

      <code file="..." />
      <code annotate="y">  ]]><foo>blah</foo><![CDATA[

