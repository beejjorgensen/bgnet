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
BOOKS=$(USLETTER_BW) $(USLETTER_COLOR) $(A4_BW) $(A4_COLOR)

HTML=bgnet.html

PANDOC_BOOK_OPTS= \
	--template=bg-template.latex \
	--pdf-engine=xelatex \
	--variable mainfont=$(PDF_MAINFONT) \
	--variable sansfont=$(PDF_SANSFONT) \
	--variable monofont=$(PDF_MONOFONT) \
	-V documentclass=book \
	-o bgnet_temp.tex \
	--toc

all: $(HTML) $(BOOKS)

bgnet.html: bgnet.md meta-html.yaml bg-css.html
	pandoc -s bgnet.md meta-html.yaml -o bgnet.html --toc -H bg-css.html
	sed -i 's/src="\(.*\)\.pdf"/src="\1.svg"/g' bgnet.html  # use svg images

bgnet.pdf: bgnet.md meta-letter-1.yaml
	# unused
	pandoc --template=bg-template.latex --pdf-engine=xelatex --variable mainfont="DejaVu Serif" --variable sansfont=Arial -V documentclass=book bgnet.md meta-letter-1.yaml -o bgnet.pdf --toc

bgnet_usl_c_1.pdf: bgnet.md meta-letter-1.yaml
	pandoc $(PANDOC_BOOK_OPTS) $^
	xelatex bgnet_temp.tex
	makeindex bgnet_temp.idx
	xelatex bgnet_temp.tex
	xelatex bgnet_temp.tex
	mv bgnet_temp.pdf $@
	rm -f bgnet_temp.*

bgnet_usl_c_2.pdf: bgnet.md meta-letter-2.yaml
	pandoc $(PANDOC_BOOK_OPTS) $^
	xelatex bgnet_temp.tex
	makeindex bgnet_temp.idx
	xelatex bgnet_temp.tex
	xelatex bgnet_temp.tex
	mv bgnet_temp.pdf $@
	rm -f bgnet_temp.*

bgnet_a4_c_1.pdf: bgnet.md meta-a4-1.yaml
	pandoc $(PANDOC_BOOK_OPTS) $^
	xelatex bgnet_temp.tex
	makeindex bgnet_temp.idx
	xelatex bgnet_temp.tex
	xelatex bgnet_temp.tex
	mv bgnet_temp.pdf $@
	rm -f bgnet_temp.*

bgnet_a4_c_2.pdf: bgnet.md meta-a4-2.yaml
	pandoc $(PANDOC_BOOK_OPTS) $^
	xelatex bgnet_temp.tex
	makeindex bgnet_temp.idx
	xelatex bgnet_temp.tex
	xelatex bgnet_temp.tex
	mv bgnet_temp.pdf $@
	rm -f bgnet_temp.*

bgnet_usl_bw_1.pdf: bgnet.md meta-letter-1.yaml
	pandoc $(PANDOC_BOOK_OPTS) --no-highlight $^
	xelatex bgnet_temp.tex
	makeindex bgnet_temp.idx
	xelatex bgnet_temp.tex
	xelatex bgnet_temp.tex
	mv bgnet_temp.pdf $@
	rm -f bgnet_temp.*

bgnet_usl_bw_2.pdf: bgnet.md meta-letter-2.yaml
	pandoc $(PANDOC_BOOK_OPTS) --no-highlight $^
	xelatex bgnet_temp.tex
	makeindex bgnet_temp.idx
	xelatex bgnet_temp.tex
	xelatex bgnet_temp.tex
	mv bgnet_temp.pdf $@
	rm -f bgnet_temp.*

bgnet_a4_bw_1.pdf: bgnet.md meta-a4-1.yaml
	pandoc $(PANDOC_BOOK_OPTS) --no-highlight $^
	xelatex bgnet_temp.tex
	makeindex bgnet_temp.idx
	xelatex bgnet_temp.tex
	xelatex bgnet_temp.tex
	mv bgnet_temp.pdf $@
	rm -f bgnet_temp.*

bgnet_a4_bw_2.pdf: bgnet.md meta-a4-2.yaml
	pandoc $(PANDOC_BOOK_OPTS) --no-highlight $^
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