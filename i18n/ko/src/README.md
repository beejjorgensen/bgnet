# Beej's Guide to Network Programming source markdown

## Beej' extensions to markdown

These are brought to life with the `bin/preproc` script.

* `[[pagebreak]]`

  Issue a page break on the printed version. Renders as LaTeX
  `\newpage`.

* `[nh[word]]`

  Don't allow hypenation of this word. Translates to
  `\hyphenation{word}`. Underscores are prohibited due to LaTeX
  restrictions.

* `[ix[entry]]`

  Build a LaTeX index entry, substituting it as-is into an `\index{}`
  element. **Remember to escape your underscores with `\_` in these
  entries!** [LaTeX indexing examples
  here](https://en.wikibooks.org/wiki/LaTeX/Indexing#Sophisticated_indexing).

* `[ixtt[entry]]`

  For a single, simple index entry, renders in typewriter text with
  `\index{entry@\texttt{entry}}`.  More complex entries should be
  rendered by passing raw LaTeX into `[ix[]]`.

* `[fl[linktext|url]]`

  Footnote Link. Hyperlink linktext to the url and show the URL in a
  footnote. Translates to `[linktext](url)^[url]`.
    
* `[flx[linktext|file]]`

  Footnote Link to Beej's Examples. Automatically prepends
  `https://beej.us/guide/bgnet/examples/` to the file and shows the URL
  in a footnote.

* `[flr[link|id]]`

   Footnote Link to Beej's Redirect. Automatically prepends
   `https://beej.us/guide/url/` to the link id and shows the URL in a
   footnote.

* `[flrfc[link|num]]`

   Footnote Link to RFC. Automatically prepends
   `https://tools.ietf.org/html/rfc` to the RFC number and shows the URL
   in a footnote.
   

## pandoc markdown quirks

If you have multiple inline footnotes in the same paragraph but on
different lines of the markdown, all lines except the last must end with
a trailing space.

man page subsections are h4 `####` to keep from showing up in the
contents. Pandoc 2.8 should have a way to suppress h3 from contents.

Table rows need to be on one line for proper wrapping (newlines are
preserved in table cells).

Tables: the relative widths of the headers is reflected in the final
output.

Table header template:

```
| Macro           | Description                                            |
|-----------------|--------------------------------------------------------|
```

`<a name>` doesn't work. Use header `{#tags}`.

Fenced code with a standard language name can sometimes cause LaTeX to puke.

````
```c                   Don't do this

``` {.c}               Do this
``` {.c .numberLines}  Or this
````

Indexing done with latex `\index{foo}` markers. They don't show up in
HTML output.

LaTex indexing examples:

(Escape underscores with `\_`!)

```latex
\index{foo} plain element
\index{foo\_bar} element with underscore
\index{foo()@\texttt{foo()}} render foo() in monospace in index
\index{O\_NONBLOCK@\texttt{O\_NONBLOCK}} mono with underscores
\index{foo!bar} subindex bar in foo
\index{bind()@\texttt{bind()}!implicit} subindex with mono
```

[More LaTeX indexing examples
here](https://en.wikibooks.org/wiki/LaTeX/Indexing#Sophisticated_indexing).

Index entries should be at the main level and not in headers or bold or
italicized text (or they'll appear as seperate entries in the index).

Put a `\newpage` before each manpage to force a page break.

When editing `bgnet_amazon.md`, use `\newpage` to force widows onto the
next page.

Footnotes after links on sequential lines need a blank space at the end
of the line to work properly...?

```markdown
[Hey](url)^[footnote],   <-- Need a blank space at the end of this line
[Again](url2)^[footnote2].
```


