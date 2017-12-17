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

2. There is no step two.

You can also `cd` to anywhere in the `output` directory tree and `make`.

`make clean` cleans, and `make pristine` cleans to "original" state.

To embed your own fonts in the PDFs, see the file
`output/print/fop.xconf` which already embeds the Liberation Fonts into
the PDF.

The `makeupload` script demonstrates the build steps for a complete
release.  You'll need to change the `upload` target in the top-level
`Makefile` to point to your host if you want to use that.  You're free
to upload whatever versions you desire individually, as well.

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

