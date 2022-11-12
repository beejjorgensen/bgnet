#!/usr/bin/env python3

# Super hacky code to convert pandoc's single page HTML to multipage

# Imports:

import sys
import os
import re
import html.parser


if len(sys.argv) != 3:
    print(f"usage: bgsplit.py infile.html outdir", file=sys.stderr)
    sys.exit(1)

infile_name, outdir_name = sys.argv[1:]

try:
    os.mkdir(outdir_name)
except FileExistsError:
    pass

def line_equal(line, s):
    return line.strip() == s.strip()

# Parser that builds a map of ids to chapters (for anchor target
# filename addition), and a list of chapters (for header/footer nav
# creation)
class BGHTMLParser(html.parser.HTMLParser):

    def __init__(self):
        super().__init__()
        self.in_h1 = False
        self.chapter_id = None
        self.current_id = []
        self.id_chapter_map = {}
        self.chapter_list = []

    def handle_starttag(self, tag, attrs):
        attrs = dict(attrs)

        if 'class' in attrs and attrs['class'] == 'title':
            self.current_id.append("contents")
        elif 'id' in attrs:
            self.current_id.append(attrs['id'])

        if tag == "h1":
            self.in_h1 = True

            self.chapter_id = self.current_id[-1]
            self.chapter_list.append(self.chapter_id)

        if tag == "section" \
            and 'class' in attrs and attrs['class'] == "footnotes footnotes-end-of-document":

            self.chapter_id = "footnotes"

    def handle_endtag(self, tag):
        if self.current_id != []:

            cur_id = self.current_id.pop()
            self.id_chapter_map[cur_id] = self.chapter_id

        if tag == "h1":
            self.in_h1 = False

    def handle_data(self, data):
        pass


parser = BGHTMLParser()

# Inventory links and chapters
with open(infile_name) as fp:
    data = fp.read()

    parser.feed(data)
    parser.close()

# Break into separate files
with open(infile_name) as fp:
    preamble = ""
    in_preamble = True
    new_filename = None
    outfile = None
    badness = False
    chapter_list_index = None
    nav_html = ""

    for line in fp:

        # Read the preamble, head and all that
        if in_preamble:
            if line_equal(line, "</head>"):
                # Inject anything else in the header here
                # preamble += "whatever"
                preamble += '<style type="text/css">.bg-nav-outer { text-align:center; }</style>'
                in_preamble = False

            preamble += line

        # Lines to ignore
        if line_equal(line, "<body>") or line_equal(line, "</body>") or line_equal(line, "</html>"):
            continue

        # Filename switching
        # Check for contents page
        if line_equal(line, '<header id="title-block-header">'):
            new_filename = "index.html"
            chapter_list_index = 0

        # Check for footnotes page
        elif line_equal(line, '<section class="footnotes footnotes-end-of-document" role="doc-endnotes">'):
            new_filename = "footnotes.html"
            chapter_list_index = None

        # Check for regular h1 chapter
        else:
            m = re.search(r'<h1.*?id="(.*?)"', line)

            if m is not None:
                section_id = m.group(1)
                new_filename = section_id + ".html"

                chapter_list_index = parser.chapter_list.index(section_id)
                assert chapter_list_index != -1

        # We have a new file so close the old one
        if new_filename is not None and outfile is not None:
            # Write the footer nav
            outfile.write("<hr>\n" + nav_html)

            # Write the footer
            outfile.write("</body>\n</html>\n")
            outfile.close()
            outfile = None

        # We have a new file we haven't opened yet
        if new_filename is not None and outfile is None:
            outfile = open(os.sep.join((outdir_name, new_filename)), "w")
            outfile.write(preamble + "<body>\n")

            # Construct header and footer nav
            if chapter_list_index is not None:

                nav_html = ""

                if chapter_list_index > 0:

                    prev_chapter = parser.chapter_list[chapter_list_index-1]
                    if prev_chapter == "contents":
                        prev_chapter = "index"

                    nav_html += f'<a class="bg-nav-prev" href="{prev_chapter}.html">&laquo;Previous</a>'

                if chapter_list_index != 0:
                    if nav_html != "": nav_html += '&nbsp;|&nbsp;'
                    nav_html += f'<a class="bg-nav-home" href="index.html">Contents</a>'

                if chapter_list_index < len(parser.chapter_list) - 1:
                    next_chapter = parser.chapter_list[chapter_list_index+1]
                    if nav_html != "": nav_html += '&nbsp;|&nbsp;'
                    nav_html += f'<a class="bg-nav-next" href="{next_chapter}.html">Next&raquo;</a>'

                nav_html = f'<div class="bg-nav-outer">{nav_html}</div>'

                # Write the header nav
                outfile.write(nav_html + "\n<hr>\n")

            elif "footnotes.html" in outfile.name:
                outfile.write("<h1>Footnotes</h1>\n")


            new_filename = None

        # Anchor target substitution
        def anchor_sub(m):
            global badness

            target_id = m.group(1)

            if target_id in parser.id_chapter_map:
                filename = parser.id_chapter_map[target_id] + '.html'
            else:
                filename = ""
                print(f"Unknown id '{target_id}':")
                print(line)
                badness = True

            if filename == f"{target_id}.html":
                # For the first link in a file, just go to the filename
                href = filename
            else:
                # Otherwise go to the subsection
                href = f"{filename}#{target_id}"

            return f'href="{href}"'

        line = re.sub(r'href="#(.+?)"', anchor_sub, line);

        # Output
        if outfile is not None:
            outfile.write(line)
    
    if False and badness:
        sys.exit(2)

