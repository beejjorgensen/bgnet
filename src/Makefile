TITLE="Beej's Guide to Network Programming"
SUBTITLE="Using Internet Sockets"
AUTHOR='Brian "Beej Jorgensen" Hall'
VERSION_DATE="v3.0.22, Copyright © November 1, 2019"

PDF_MAINFONT="Liberation Serif"
PDF_SANSFONT="Liberation Sans"
PDF_MONOFONT="Liberation Mono"
#PDF_MAINFONT="DejaVu Serif"
#PDF_SANSFONT="DejaVu Sans"
#PDF_MONOFONT="DejaVu Sans Mono"

USLETTER_COLOR=bgnet_usl_c_1.pdf bgnet_usl_c_2.pdf
USLETTER_BW=bgnet_usl_bw_1.pdf bgnet_usl_bw_2.pdf
A4_COLOR=bgnet_a4_c_1.pdf bgnet_a4_c_2.pdf
A4_BW=bgnet_a4_bw_1.pdf bgnet_a4_bw_2.pdf
BOOKS=$(USLETTER_BW) $(USLETTER_COLOR) $(A4_BW) $(A4_COLOR) bgnet_lulu.pdf

HTML=bgnet.html

COMMON_OPTS= \
	--variable title:$(TITLE) \
	--variable subtitle:$(SUBTITLE) \
	--variable author:$(AUTHOR) \
	--variable date:$(VERSION_DATE) \
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
	-o bgnet_temp.tex \
	$(COMMON_OPTS)

HTML_OPTS=$(COMMON_OPTS) \
	--metadata title:$(TITLE)

ONESIDE=--variable classoption:oneside
TWOSIDE=--variable classoption:twoside
USLETTER=--variable papersize:letter
A4=--variable papersize:a4
CROWNQUARTO=--variable geometry:"paperwidth=7.444in,paperheight=9.681in,top=1in,bottom=1in,left=1in,right=1.5in" # Lulu press
CROWNQUARTO_AMAZON=--variable geometry:"paperwidth=7.444in,paperheight=9.681in,top=1in,bottom=1in,left=1.25in,right=1.25in" # Amazon
SIZE_75x925_AMAZON=--variable geometry:"paperwidth=7.5in,paperheight=9.25in,top=1in,bottom=1in,left=1.125in,right=1.375in" # Amazon 7.25" x 9.5"
BLANKLAST=-A latex/after_blank.latex # add a blank last page
BW=--no-highlight  # black and white options

all: $(HTML) $(BOOKS)

bgnet.html: bgnet.md bg-css.html
	pandoc $(HTML_OPTS) -s bgnet.md -o bgnet.html -H bg-css.html
	sed -i 's/src="\(.*\)\.pdf"/src="\1.svg"/g' bgnet.html  # use svg images

bgnet_usl_c_1.pdf: bgnet.md
	pandoc $(PDF_OPTS) $(USLETTER) $(ONESIDE) $^
	xelatex bgnet_temp.tex
	makeindex bgnet_temp.idx
	xelatex bgnet_temp.tex
	xelatex bgnet_temp.tex
	mv bgnet_temp.pdf $@
	rm -f bgnet_temp.*

bgnet_usl_c_2.pdf: bgnet.md
	pandoc $(PDF_OPTS) $(USLETTER) $(TWOSIDE) $^
	xelatex bgnet_temp.tex
	makeindex bgnet_temp.idx
	xelatex bgnet_temp.tex
	xelatex bgnet_temp.tex
	mv bgnet_temp.pdf $@
	rm -f bgnet_temp.*

bgnet_a4_c_1.pdf: bgnet.md
	pandoc $(PDF_OPTS) $(A4) $(ONESIDE) $^
	xelatex bgnet_temp.tex
	makeindex bgnet_temp.idx
	xelatex bgnet_temp.tex
	xelatex bgnet_temp.tex
	mv bgnet_temp.pdf $@
	rm -f bgnet_temp.*

bgnet_a4_c_2.pdf: bgnet.md
	pandoc $(PDF_OPTS) $(A4) $(TWOSIDE) $^
	xelatex bgnet_temp.tex
	makeindex bgnet_temp.idx
	xelatex bgnet_temp.tex
	xelatex bgnet_temp.tex
	mv bgnet_temp.pdf $@
	rm -f bgnet_temp.*

bgnet_usl_bw_1.pdf: bgnet.md
	pandoc $(PDF_OPTS) $(USLETTER) $(ONESIDE) $(BW) $^
	xelatex bgnet_temp.tex
	makeindex bgnet_temp.idx
	xelatex bgnet_temp.tex
	xelatex bgnet_temp.tex
	mv bgnet_temp.pdf $@
	rm -f bgnet_temp.*

bgnet_usl_bw_2.pdf: bgnet.md
	pandoc $(PDF_OPTS) $(USLETTER) $(TWOSIDE) $(BW) $^
	xelatex bgnet_temp.tex
	makeindex bgnet_temp.idx
	xelatex bgnet_temp.tex
	xelatex bgnet_temp.tex
	mv bgnet_temp.pdf $@
	rm -f bgnet_temp.*

bgnet_a4_bw_1.pdf: bgnet.md
	pandoc $(PDF_OPTS) $(A4) $(ONESIDE) $(BW) $^
	xelatex bgnet_temp.tex
	makeindex bgnet_temp.idx
	xelatex bgnet_temp.tex
	xelatex bgnet_temp.tex
	mv bgnet_temp.pdf $@
	rm -f bgnet_temp.*

bgnet_a4_bw_2.pdf: bgnet.md
	pandoc $(PDF_OPTS) $(A4) $(TWOSIDE) $(BW) $^
	xelatex bgnet_temp.tex
	makeindex bgnet_temp.idx
	xelatex bgnet_temp.tex
	xelatex bgnet_temp.tex
	mv bgnet_temp.pdf $@
	rm -f bgnet_temp.*

bgnet_lulu.pdf: bgnet_lulu.md
	pandoc $(PDF_OPTS) $(TWOSIDE) $(CROWNQUARTO) $(BLANKLAST) $^
	xelatex bgnet_temp.tex
	makeindex bgnet_temp.idx
	xelatex bgnet_temp.tex
	xelatex bgnet_temp.tex
	mv bgnet_temp.pdf $@
	rm -f bgnet_temp.*

bgnet_amazon.pdf: bgnet_amazon.md
	pandoc $(PDF_OPTS) $(TWOSIDE) $(SIZE_75x925_AMAZON) $(BLANKLAST) $^
	xelatex bgnet_temp.tex
	makeindex bgnet_temp.idx
	xelatex bgnet_temp.tex
	xelatex bgnet_temp.tex
	mv bgnet_temp.pdf $@
	rm -f bgnet_temp.*

clean:
	rm -f bgnet_temp.*

pristine: clean
	rm -f $(HTML) $(BOOKS)

.PHONY: all, html, clean, pristine
