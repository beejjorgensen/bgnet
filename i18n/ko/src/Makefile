TITLE="Beej의 네트워크 프로그래밍 안내서"
SUBTITLE="인터넷 소켓 사용법"
AUTHOR='Brian “Beej Jorgensen” Hall'
VERSION_DATE="v3.1.11, Copyright © April 8, 2023"

GUIDE_ID=bgnet

PDF_MAINFONT="Pretendard"
PDF_SANSFONT="Pretendard"
PDF_MONOFONT="Pretendard"
#PDF_MAINFONT="DejaVu Serif"
#PDF_SANSFONT="DejaVu Sans"
#PDF_MONOFONT="DejaVu Sans Mono"

USLETTER_COLOR=$(GUIDE_ID)_usl_c_1.pdf $(GUIDE_ID)_usl_c_2.pdf
USLETTER_BW=$(GUIDE_ID)_usl_bw_1.pdf $(GUIDE_ID)_usl_bw_2.pdf
A4_COLOR=$(GUIDE_ID)_a4_c_1.pdf $(GUIDE_ID)_a4_c_2.pdf
A4_BW=$(GUIDE_ID)_a4_bw_1.pdf $(GUIDE_ID)_a4_bw_2.pdf
BOOKS=$(USLETTER_BW) $(USLETTER_COLOR) $(A4_BW) $(A4_COLOR)

HTML=$(GUIDE_ID).html $(GUIDE_ID)-wide.html
SPLIT_DIRS=split split-wide
GUIDE_MD=$(sort $(wildcard $(GUIDE_ID)_part_*.md))

PREPROC=../../../bin/preproc
PREPROC_MD=$(GUIDE_ID)_temp_preproc.md

SPLIT=../../../bin/bgsplit.py

COMMON_OPTS= \
	--variable title:$(TITLE) \
	--variable subtitle:$(SUBTITLE) \
	--variable author:$(AUTHOR) \
	--variable date:$(VERSION_DATE) \
	--number-sections \
	--toc \
	--wrap=none

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
	#-V indent \

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
COLOR=--highlight-style=tango   # color options

all: install-fonts $(HTML) split/index.html split-wide/index.html $(BOOKS)

install-fonts:
	bash ./install_fonts.sh

bg-css.html: common-css-src.html
	cat $^ > $@

bg-css-wide.html: common-css-src.html widescreen-css-src.html
	cat $^ > $@

$(GUIDE_ID).html: $(GUIDE_MD) bg-css.html
	$(PREPROC) $(GUIDE_MD) $(PREPROC_MD)
	pandoc $(HTML_OPTS) -s $(PREPROC_MD) -o $@ -H bg-css.html
	sed 's/src="\(.*\)\.pdf"/src="\1.svg"/g' $@ > $(GUIDE_ID)_temp.html # use svg images
	mv $(GUIDE_ID)_temp.html $@
	rm -f $(GUIDE_ID)_temp*

$(GUIDE_ID)-wide.html: $(GUIDE_MD) bg-css-wide.html
	$(PREPROC) $(GUIDE_MD) $(PREPROC_MD)
	pandoc $(HTML_OPTS) -s $(PREPROC_MD) -o $@ -H bg-css-wide.html
	sed 's/src="\(.*\)\.pdf"/src="\1.svg"/g' $@ > $(GUIDE_ID)_temp.html # use svg images
	mv $(GUIDE_ID)_temp.html $@
	rm -f $(GUIDE_ID)_temp*

split/index.html: $(GUIDE_ID).html
	$(SPLIT) $< split

split-wide/index.html: $(GUIDE_ID)-wide.html
	$(SPLIT) $< split-wide

$(GUIDE_ID).epub: $(GUIDE_MD)
	$(PREPROC) $^ $(PREPROC_MD)
	pandoc $(COMMON_OPTS) --webtex --metadata author=$(AUTHOR) --metadata title=$(TITLE) -o $@ $(PREPROC_MD)

$(GUIDE_ID)_quick.pdf: $(GUIDE_MD)
	$(PREPROC) $^ $(PREPROC_MD)
	pandoc $(PDF_OPTS) $(USLETTER) $(ONESIDE) $(COLOR) $(PREPROC_MD)
	xelatex $(GUIDE_ID)_temp.tex
	mv $(GUIDE_ID)_temp.pdf $@
	rm -f $(GUIDE_ID)_temp*

$(GUIDE_ID)_usl_c_1.pdf: $(GUIDE_MD)
	$(PREPROC) $^ $(PREPROC_MD)
	pandoc $(PDF_OPTS) $(USLETTER) $(ONESIDE) $(COLOR) $(PREPROC_MD)
	xelatex $(GUIDE_ID)_temp.tex
	makeindex $(GUIDE_ID)_temp.idx
	xelatex $(GUIDE_ID)_temp.tex
	xelatex $(GUIDE_ID)_temp.tex
	mv $(GUIDE_ID)_temp.pdf $@
	rm -f $(GUIDE_ID)_temp*

$(GUIDE_ID)_usl_c_2.pdf: $(GUIDE_MD)
	$(PREPROC) $^ $(PREPROC_MD)
	pandoc $(PDF_OPTS) $(USLETTER) $(TWOSIDE) $(COLOR) $(PREPROC_MD)
	xelatex $(GUIDE_ID)_temp.tex
	makeindex $(GUIDE_ID)_temp.idx
	xelatex $(GUIDE_ID)_temp.tex
	xelatex $(GUIDE_ID)_temp.tex
	mv $(GUIDE_ID)_temp.pdf $@
	rm -f $(GUIDE_ID)_temp*

$(GUIDE_ID)_a4_c_1.pdf: $(GUIDE_MD)
	$(PREPROC) $^ $(PREPROC_MD)
	pandoc $(PDF_OPTS) $(A4) $(ONESIDE) $(COLOR) $(PREPROC_MD)
	xelatex $(GUIDE_ID)_temp.tex
	makeindex $(GUIDE_ID)_temp.idx
	xelatex $(GUIDE_ID)_temp.tex
	xelatex $(GUIDE_ID)_temp.tex
	mv $(GUIDE_ID)_temp.pdf $@
	rm -f $(GUIDE_ID)_temp*

$(GUIDE_ID)_a4_c_2.pdf: $(GUIDE_MD)
	$(PREPROC) $^ $(PREPROC_MD)
	pandoc $(PDF_OPTS) $(A4) $(TWOSIDE) $(COLOR) $(PREPROC_MD)
	xelatex $(GUIDE_ID)_temp.tex
	makeindex $(GUIDE_ID)_temp.idx
	xelatex $(GUIDE_ID)_temp.tex
	xelatex $(GUIDE_ID)_temp.tex
	mv $(GUIDE_ID)_temp.pdf $@
	rm -f $(GUIDE_ID)_temp*

$(GUIDE_ID)_usl_bw_1.pdf: $(GUIDE_MD)
	$(PREPROC) $^ $(PREPROC_MD)
	pandoc $(PDF_OPTS) $(USLETTER) $(ONESIDE) $(BW) $(PREPROC_MD)
	xelatex $(GUIDE_ID)_temp.tex
	makeindex $(GUIDE_ID)_temp.idx
	xelatex $(GUIDE_ID)_temp.tex
	xelatex $(GUIDE_ID)_temp.tex
	mv $(GUIDE_ID)_temp.pdf $@
	rm -f $(GUIDE_ID)_temp*

$(GUIDE_ID)_usl_bw_2.pdf: $(GUIDE_MD)
	$(PREPROC) $^ $(PREPROC_MD)
	pandoc $(PDF_OPTS) $(USLETTER) $(TWOSIDE) $(BW) $(PREPROC_MD)
	xelatex $(GUIDE_ID)_temp.tex
	makeindex $(GUIDE_ID)_temp.idx
	xelatex $(GUIDE_ID)_temp.tex
	xelatex $(GUIDE_ID)_temp.tex
	mv $(GUIDE_ID)_temp.pdf $@
	rm -f $(GUIDE_ID)_temp*

$(GUIDE_ID)_a4_bw_1.pdf: $(GUIDE_MD)
	$(PREPROC) $^ $(PREPROC_MD)
	pandoc $(PDF_OPTS) $(A4) $(ONESIDE) $(BW) $(PREPROC_MD)
	xelatex $(GUIDE_ID)_temp.tex
	makeindex $(GUIDE_ID)_temp.idx
	xelatex $(GUIDE_ID)_temp.tex
	xelatex $(GUIDE_ID)_temp.tex
	mv $(GUIDE_ID)_temp.pdf $@
	rm -f $(GUIDE_ID)_temp*

$(GUIDE_ID)_a4_bw_2.pdf: $(GUIDE_MD)
	$(PREPROC) $^ $(PREPROC_MD)
	pandoc $(PDF_OPTS) $(A4) $(TWOSIDE) $(BW) $(PREPROC_MD)
	xelatex $(GUIDE_ID)_temp.tex
	makeindex $(GUIDE_ID)_temp.idx
	xelatex $(GUIDE_ID)_temp.tex
	xelatex $(GUIDE_ID)_temp.tex
	mv $(GUIDE_ID)_temp.pdf $@
	rm -f $(GUIDE_ID)_temp*

$(GUIDE_ID)_lulu.md: $(GUIDE_MD)
	$(PREPROC) $^ $@

$(GUIDE_ID)_lulu.pdf: $(GUIDE_ID)_lulu.md
	pandoc $(PDF_OPTS) $(TWOSIDE) $(CROWNQUARTO) $(BLANKLAST) $(COLOR) $(PREPROC_MD)
	xelatex $(GUIDE_ID)_temp.tex
	makeindex $(GUIDE_ID)_temp.idx
	xelatex $(GUIDE_ID)_temp.tex
	xelatex $(GUIDE_ID)_temp.tex
	mv $(GUIDE_ID)_temp.pdf $@
	rm -f $(GUIDE_ID)_temp*

$(GUIDE_ID)_amazon.md: $(GUIDE_MD)
	$(PREPROC) --book $^ $@

$(GUIDE_ID)_amazon.pdf: $(GUIDE_ID)_amazon.md
	pandoc $(PDF_OPTS) $(TWOSIDE) $(SIZE_75x925_AMAZON) $(BLANKLAST) $(COLOR) $(GUIDE_ID)_amazon.md
	xelatex $(GUIDE_ID)_temp.tex
	makeindex $(GUIDE_ID)_temp.idx
	xelatex $(GUIDE_ID)_temp.tex
	xelatex $(GUIDE_ID)_temp.tex
	mv $(GUIDE_ID)_temp.pdf $@
	rm -f $(GUIDE_ID)_temp*

clean:
	rm -f $(GUIDE_ID)_temp* $(GUIDE_ID)_quick.pdf bg-css*.html

pristine: clean
	rm -f $(HTML) $(BOOKS)
	rm -rf $(SPLIT_DIRS)
 
.PHONY: all, html, clean, pristine, install-fonts
