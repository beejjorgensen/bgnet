#!/usr/bin/env python3

import sys
import os
import os.path
from html.parser import HTMLParser

def html_str(tag, attrs, is_startend=False):
    """
    Rebuild the HTML string for a start or startend tag and given
    attributes.
    """

    r = f"<{tag}"

    for k, v in attrs:
        r += f' {k}="{v}"'

    if is_startend:
        r += " /"

    r += ">"

    return r

def attr_get(attrs, key):
    for k, v in attrs:
        if k.lower() == key.lower():
            return v

    return None

def attr_equals(attrs, key, value):
    for k, v in attrs:
        if k.lower() == key.lower() and v == value:
            return True

    return False

def attr_replace(attrs, key, value):
    for i, (k, v) in enumerate(attrs):
        if k.lower() == key.lower():
            attrs[i] = (k, value)
            return attrs[i]

    return None

def get_section_id_from_tag(tag, attrs):
    # Start of Title/Contents page
    if tag.lower() == "header":
        if attr_equals(attrs, "id", "title-block-header"):
            return "title_contents"

    # Start of Footnotes page
    elif tag.lower() == "section":
        if attr_equals(attrs, "id", "footnotes"):
            return "footnotes"

    # Start of any other page
    elif tag.lower() == "h1":
        if not attr_equals(attrs, "class", "title"):
            return attr_get(attrs, "id")

    return None

def file_name_for_section(s):
    if s == "title_contents":
        s = "index"

    return s + ".html"

class Inventory:
    def __init__(self, header, section_id_to_info, id_to_section_id):
        self.header = header
        self.section_id_to_info = section_id_to_info
        self.id_to_section_id = id_to_section_id

class SectionInfo:
    def __init__(self, prev_id=None, next_id=None):
        self.prev_id = prev_id
        self.next_id = next_id

    def __repr__(self):
        return f"SectionInfo({repr(self.prev_id)},{repr(self.next_id)})"

class InventoryHTMLParser(HTMLParser):
    """
    First pass HTML parser that gathers the following:

    * A string containing the header from the start of the file through
      the </head> tag.

    * A map of previous and next section IDs mapped by section. This
      will be used later to produce the nav header and footer.

    * A map of `id` attributes to section IDs, used later for patching
      up anchor tag targets.
    """

    def __init__(self):
        self.current_section_id = None
        self.section_id_to_info = {}
        self.record_header = False
        self.header = ""
        self.id_to_section_id = {}

        super().__init__(convert_charrefs=False)

    def feed(self, data):
        super().feed(data)

        inv = Inventory(self.header, self.section_id_to_info, \
                            self.id_to_section_id)
        return inv

    def switch_section_to(self, new_section):
        prev_section_id = self.current_section_id

        # Make a new Section Infor for previous and next
        new_section_info = SectionInfo(prev_section_id, None)

        # Add to the index
        self.section_id_to_info[new_section] = new_section_info

        # The current section is now the new section
        self.current_section_id = new_section

        # If we had a previous section, go back to its section info and
        # fix up the next to point at the new section

        if prev_section_id is not None:
            prev_section_info = self.section_id_to_info[prev_section_id]
            prev_section_info.next_id = self.current_section_id

    def add_id_section(self, attrs):
        id_attr = attr_get(attrs, "id")

        if id_attr is not None:
            assert id_attr not in self.id_to_section_id, \
                   f"{id_attr} already in id_section map"

            self.id_to_section_id[id_attr] = self.current_section_id

    def process(self, s):
        if self.record_header:
            self.header += s

    def handle_starttag(self, tag, attrs):
        # Check if we're switching to a new output file
        section_id = get_section_id_from_tag(tag, attrs)

        if section_id is not None:
            self.switch_section_to(section_id)

        # ID section mapping
        self.add_id_section(attrs)

        self.process(html_str(tag, attrs))

    def handle_endtag(self, tag):
        self.process(f"</{tag}>")

        if tag.lower() == "head":
            self.record_header = False 

    def handle_startendtag(self, tag, attrs):
        # ID section mapping
        self.add_id_section(attrs)

        self.process(html_str(tag, attrs, is_startend=True))

    def handle_data(self, data):
        self.process(data)

    def handle_entityref(self, name):
        self.process(f"&{name};")

    def handle_charref(self, data):
        self.process(f"&#{data};")

    def handle_comment(self, data):
        self.process(f"<!--{data}-->")

    def handle_decl(self, decl):
        if decl.lower().strip() == "doctype html":
            self.record_header = True

        self.process(f"<!{decl}>")

    def handle_pi(self, data):
        self.process(f"<?{data}>")

    def handle_unknowndecl(self, data):
        self.process(f"<![{data}]>")

INDEX_FILE_NAME = 'index.html'

class SplitHTMLParser(HTMLParser):
    """
    The Second Pass HTML parser.

    This splits the file into multiple outputs.

    It prepends the header, adds nav headers and footers, and rewrites
    the anchor tag targets to point to the proper files.
    """

    def __init__(self, out_directory, inv):
        self.header = inv.header
        self.section_id_to_info = inv.section_id_to_info
        self.id_to_section_id = inv.id_to_section_id
        self.out_directory = out_directory
        self.out_file = None
        self.current_section_id = None
        self.page_nav = ""
        self.handlingIndex = True
        self.switch_output_file(INDEX_FILE_NAME)

        super().__init__(convert_charrefs=False)

    def patch_href(self, attrs):
        old_href = attr_get(attrs, "href")

        # If this isn't an internal link, bail out
        if old_href is None or old_href[0] != "#":
            return
        
        old_id = old_href[1:]   # Strip hash
        section_id = self.id_to_section_id[old_id]

        section_file_name = file_name_for_section(section_id)

        new_href = f"{section_file_name}{old_href}"

        new_attr = attr_replace(attrs, "href", new_href)

        assert new_attr is not None, "href replace failed"

    def get_nav(self):
        info = self.section_id_to_info[self.current_section_id]
        html = '<div style="text-align:center">'

        if info.prev_id is not None:
            file = file_name_for_section(info.prev_id)
            span_style = ""
        else:
            file = ""
            span_style = ' style="visibility: hidden"'

        html += f'<span{span_style}><a href="{file}">Prev</a> | </span>'

        html += f'<a href="index.html">Contents</a>'

        if info.next_id is not None:
            file = file_name_for_section(info.next_id)
            span_style = ""
        else:
            file = ""
            span_style = ' style="visibility: hidden"'

        html += f'<span{span_style}> | <a href="{file}">Next</a></span>'

        html += "</div>"

        return html

    def write_header(self):
        self.page_nav = self.get_nav()

        self.out_file.write(self.header)
        self.out_file.write("\n<body>\n")
        self.out_file.write(self.page_nav)
        self.out_file.write("<hr>\n")

    def write_footer(self):
        self.out_file.write("<hr>")
        self.out_file.write(self.page_nav)
        self.out_file.write("</body>\n")
        self.out_file.write("</html>\n")

    def close_out_file(self):
        # Close any previously opened file
        if self.out_file is not None:
            self.write_footer()
            self.out_file.close()
            self.out_file = None
        
    def switch_output_file(self, file_name):
        try:
            os.mkdir(self.out_directory)
        except FileExistsError:
            pass

        self.close_out_file()

        file_path = os.path.sep.join((self.out_directory, file_name))
        self.out_file = open(file_path, "w")

        if file_name != INDEX_FILE_NAME:
          self.write_header()

    def output(self, s):
        if self.out_file is not None:
            self.out_file.write(s)

    def handle_starttag(self, tag, attrs):
        # Check if we're switching to a new output file
        section_id = get_section_id_from_tag(tag, attrs)

        if section_id is not None:
            self.current_section_id = section_id
            file_name = file_name_for_section(section_id)
            self.switch_output_file(file_name)

        # Fix anchor tag targets
        if tag.lower() == "a":
            self.patch_href(attrs)

        self.output(html_str(tag, attrs))

    def handle_endtag(self, tag):
        if tag.lower() == "html":
            self.close_out_file()

        self.output(f"</{tag}>")

    def handle_startendtag(self, tag, attrs):
        # Special case to strip the <hr/>s in footnotes
        if self.current_section_id == "footnotes":
            if tag.lower() == "hr":
                return

        self.output(html_str(tag, attrs, is_startend=True))

    def handle_data(self, data):
        self.output(data)

    def handle_entityref(self, name):
        self.output(f"&{name};")

    def handle_charref(self, data):
        self.output(f"&#{data};")

    def handle_comment(self, data):
        self.output(f"<!--{data}-->")

    def handle_decl(self, decl):
        self.output(f"<!{decl}>")

    def handle_pi(self, data):
        self.output(f"<?{data}>")

    def handle_unknowndecl(self, data):
        self.output(f"<![{data}]>")

def inventory_file(file_name):
    with open(file_name) as fp:
        parser = InventoryHTMLParser()
        data = fp.read()
        return parser.feed(data)

def split_file(file_name, out_directory, inv):
    with open(file_name) as fp:
        parser = SplitHTMLParser(out_directory, inv)
        data = fp.read()
        parser.feed(data)

def usage():
    print("usage: bgsplit.py infile.html outdirectory", file=sys.stderr)

def main(argv):
    
    try:
        file_name = argv[1]
        out_directory = argv[2]
    except:
        usage()
        return 1

    inv = inventory_file(file_name)

    split_file(file_name, out_directory, inv)

    return 0

if __name__ == "__main__":
    sys.exit(main(sys.argv))

