TITLE="Beej's Guide to Network Programming"
SUBTITLE="Using Internet Sockets"
AUTHOR='Brian “Beej Jorgensen” Hall'
VERSION_DATE="v3.1.5, Copyright © November 20, 2020"

GUIDE_ID=bgnet

PDF_MAINFONT="Liberation Serif"
PDF_SANSFONT="Liberation Sans"
PDF_MONOFONT="Liberation Mono"
#PDF_MAINFONT="DejaVu Serif"
#PDF_SANSFONT="DejaVu Sans"
#PDF_MONOFONT="DejaVu Sans Mono"

USLETTER_COLOR=$(GUIDE_ID)_usl_c_1.pdf $(GUIDE_ID)_usl_c_2.pdf
USLETTER_BW=$(GUIDE_ID)_usl_bw_1.pdf $(GUIDE_ID)_usl_bw_2.pdf
A4_COLOR=$(GUIDE_ID)_a4_c_1.pdf $(GUIDE_ID)_a4_c_2.pdf
A4_BW=$(GUIDE_ID)_a4_bw_1.pdf $(GUIDE_ID)_a4_bw_2.pdf
BOOKS=$(USLETTER_BW) $(USLETTER_COLOR) $(A4_BW) $(A4_COLOR)

HTML=$(GUIDE_ID).html $(GUIDE_ID)-wide.html

PREPROC=../bin/preproc
PREPROC_MD=$(GUIDE_ID)_temp_preproc.md

COMMON_OPTS= \
	--variable title:$(TITLE) \
	--variable subtitle:$(SUBTITLE) \
	--variable author:$(AUTHOR) \
	--variable date:$(VERSION_DATE) \
	--number-sections \
	--toc

PDF_OPTS= \
	-H latex/header_index.latex \
	-A latex/after_index.latex \
	--pdf-engine=xelatex \
	--variable mainfont=$(PDF_MAINFONT) \
	--variable sansfont=$(PDF_SANSFONT) \
	--variable monofont=$(PDF_MONOFONT) \
	--variable geometry:"top=1in,bottom=1in" \
	-V documentclass=book \
	-o $(GUIDE_ID)_temp.tex \
	$(COMMON_OPTS)

HTML_OPTS=$(COMMON_OPTS) \
	--metadata title:$(TITLE) \
	--mathjax

ONESIDE=--variable classoption:oneside
TWOSIDE=--variable classoption:twoside
USLETTER=--variable papersize:letter
A4=--variable papersize:a4
CROWNQUARTO=--variable geometry:"paperwidth=7.444in,paperheight=9.681in,top=1in,bottom=1in,left=1in,right=1.5in" # Lulu press
CROWNQUARTO_AMAZON=--variable geometry:"paperwidth=7.444in,paperheight=9.681in,top=1in,bottom=1in,left=1.25in,right=1.25in" # Amazon
#SIZE_75x925_AMAZON=--variable geometry:"paperwidth=7.5in,paperheight=9.25in,top=1in,bottom=1in,left=1.125in,right=1.375in" # Amazon 7.5" x 9.25", margins too far inside
SIZE_75x925_AMAZON=--variable geometry:"paperwidth=7.5in,paperheight=9.25in,top=1in,bottom=1in,left=1.25in,right=1.25in" # Amazon 7.5" x 9.25"
BLANKLAST=-A latex/after_blank.latex # add a blank last page
BW=--no-highlight  # black and white options

all: $(HTML) $(BOOKS)

bg-css-wide.html: bg-css.html body-wide-css.html
	cat $^ > $@

$(GUIDE_ID).html: $(GUIDE_ID).md bg-css.html
	$(PREPROC) $< $(PREPROC_MD)
	pandoc $(HTML_OPTS) -s $(PREPROC_MD) -o $@ -H bg-css.html
	sed 's/src="\(.*\)\.pdf"/src="\1.svg"/g' $@ > $(GUIDE_ID)_temp.html # use svg images
	mv $(GUIDE_ID)_temp.html $@
	rm -f $(GUIDE_ID)_temp*

$(GUIDE_ID)-wide.html: $(GUIDE_ID).md bg-css-wide.html
	$(PREPROC) $< $(PREPROC_MD)
	pandoc $(HTML_OPTS) -s $(PREPROC_MD) -o $@ -H bg-css-wide.html
	sed 's/src="\(.*\)\.pdf"/src="\1.svg"/g' $@ > $(GUIDE_ID)_temp.html # use svg images
	mv $(GUIDE_ID)_temp.html $@
	rm -f $(GUIDE_ID)_temp*

$(GUIDE_ID)_usl_c_1.pdf: $(GUIDE_ID).md
	$(PREPROC) $< $(PREPROC_MD)
	pandoc $(PDF_OPTS) $(USLETTER) $(ONESIDE) $(PREPROC_MD)
	xelatex $(GUIDE_ID)_temp.tex
	makeindex $(GUIDE_ID)_temp.idx
	xelatex $(GUIDE_ID)_temp.tex
	xelatex $(GUIDE_ID)_temp.tex
	mv $(GUIDE_ID)_temp.pdf $@
	rm -f $(GUIDE_ID)_temp*

$(GUIDE_ID)_usl_c_2.pdf: $(GUIDE_ID).md
	$(PREPROC) $< $(PREPROC_MD)
	pandoc $(PDF_OPTS) $(USLETTER) $(TWOSIDE) $(PREPROC_MD)
	xelatex $(GUIDE_ID)_temp.tex
	makeindex $(GUIDE_ID)_temp.idx
	xelatex $(GUIDE_ID)_temp.tex
	xelatex $(GUIDE_ID)_temp.tex
	mv $(GUIDE_ID)_temp.pdf $@
	rm -f $(GUIDE_ID)_temp*

$(GUIDE_ID)_a4_c_1.pdf: $(GUIDE_ID).md
	$(PREPROC) $< $(PREPROC_MD)
	pandoc $(PDF_OPTS) $(A4) $(ONESIDE) $(PREPROC_MD)
	xelatex $(GUIDE_ID)_temp.tex
	makeindex $(GUIDE_ID)_temp.idx
	xelatex $(GUIDE_ID)_temp.tex
	xelatex $(GUIDE_ID)_temp.tex
	mv $(GUIDE_ID)_temp.pdf $@
	rm -f $(GUIDE_ID)_temp*

$(GUIDE_ID)_a4_c_2.pdf: $(GUIDE_ID).md
	$(PREPROC) $< $(PREPROC_MD)
	pandoc $(PDF_OPTS) $(A4) $(TWOSIDE) $(PREPROC_MD)
	xelatex $(GUIDE_ID)_temp.tex
	makeindex $(GUIDE_ID)_temp.idx
	xelatex $(GUIDE_ID)_temp.tex
	xelatex $(GUIDE_ID)_temp.tex
	mv $(GUIDE_ID)_temp.pdf $@
	rm -f $(GUIDE_ID)_temp*

$(GUIDE_ID)_usl_bw_1.pdf: $(GUIDE_ID).md
	$(PREPROC) $< $(PREPROC_MD)
	pandoc $(PDF_OPTS) $(USLETTER) $(ONESIDE) $(BW) $(PREPROC_MD)
	xelatex $(GUIDE_ID)_temp.tex
	makeindex $(GUIDE_ID)_temp.idx
	xelatex $(GUIDE_ID)_temp.tex
	xelatex $(GUIDE_ID)_temp.tex
	mv $(GUIDE_ID)_temp.pdf $@
	rm -f $(GUIDE_ID)_temp*

$(GUIDE_ID)_usl_bw_2.pdf: $(GUIDE_ID).md
	$(PREPROC) $< $(PREPROC_MD)
	pandoc $(PDF_OPTS) $(USLETTER) $(TWOSIDE) $(BW) $(PREPROC_MD)
	xelatex $(GUIDE_ID)_temp.tex
	makeindex $(GUIDE_ID)_temp.idx
	xelatex $(GUIDE_ID)_temp.tex
	xelatex $(GUIDE_ID)_temp.tex
	mv $(GUIDE_ID)_temp.pdf $@
	rm -f $(GUIDE_ID)_temp*

$(GUIDE_ID)_a4_bw_1.pdf: $(GUIDE_ID).md
	$(PREPROC) $< $(PREPROC_MD)
	pandoc $(PDF_OPTS) $(A4) $(ONESIDE) $(BW) $(PREPROC_MD)
	xelatex $(GUIDE_ID)_temp.tex
	makeindex $(GUIDE_ID)_temp.idx
	xelatex $(GUIDE_ID)_temp.tex
	xelatex $(GUIDE_ID)_temp.tex
	mv $(GUIDE_ID)_temp.pdf $@
	rm -f $(GUIDE_ID)_temp*

$(GUIDE_ID)_a4_bw_2.pdf: $(GUIDE_ID).md
	$(PREPROC) $< $(PREPROC_MD)
	pandoc $(PDF_OPTS) $(A4) $(TWOSIDE) $(BW) $(PREPROC_MD)
	xelatex $(GUIDE_ID)_temp.tex
	makeindex $(GUIDE_ID)_temp.idx
	xelatex $(GUIDE_ID)_temp.tex
	xelatex $(GUIDE_ID)_temp.tex
	mv $(GUIDE_ID)_temp.pdf $@
	rm -f $(GUIDE_ID)_temp*

$(GUIDE_ID)_lulu.pdf: $(GUIDE_ID)_lulu.md
	$(PREPROC) $< $(PREPROC_MD)
	pandoc $(PDF_OPTS) $(TWOSIDE) $(CROWNQUARTO) $(BLANKLAST) $(PREPROC_MD)
	xelatex $(GUIDE_ID)_temp.tex
	makeindex $(GUIDE_ID)_temp.idx
	xelatex $(GUIDE_ID)_temp.tex
	xelatex $(GUIDE_ID)_temp.tex
	mv $(GUIDE_ID)_temp.pdf $@
	rm -f $(GUIDE_ID)_temp*

$(GUIDE_ID)_amazon.pdf: $(GUIDE_ID)_amazon.md
	$(PREPROC) $< $(PREPROC_MD)
	pandoc $(PDF_OPTS) $(TWOSIDE) $(SIZE_75x925_AMAZON) $(BLANKLAST) $(PREPROC_MD)
	xelatex $(GUIDE_ID)_temp.tex
	makeindex $(GUIDE_ID)_temp.idx
	xelatex $(GUIDE_ID)_temp.tex
	xelatex $(GUIDE_ID)_temp.tex
	mv $(GUIDE_ID)_temp.pdf $@
	rm -f $(GUIDE_ID)_temp*

clean:
	rm -f $(GUIDE_ID)_temp* bg-css-wide.html

pristine: clean
	rm -f $(HTML) $(BOOKS)

.PHONY: all, html, clean, pristine
