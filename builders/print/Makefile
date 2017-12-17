FOP=fop
PACKAGE=bgnet

BASE=../..
VALIDFILE=$(BASE)/$(PACKAGE).valid
SRCPATH=$(BASE)/src
SRCXML=$(SRCPATH)/$(PACKAGE).xml
BINPATH=$(BASE)/bin
LIBPATH=$(BASE)/lib
IMGPATH=$(BASE)/src/images
IMGS=cs-300-3.334.png dataencap-300-4.736.png
FOPOPTIONS=-c fop.xconf
STD_FONT_OPTS=-sans "Liberation Sans" -serif "Liberation Serif" -mono "Liberation Mono" 

#DESTS=$(PACKAGE)_USLetter.pdf $(PACKAGE)_USLetter_2.pdf \
#	$(PACKAGE)_USLetter.ps.gz $(PACKAGE)_USLetter_2.ps.gz \
#	$(PACKAGE)_A4.pdf $(PACKAGE)_A4_2.pdf \
#	$(PACKAGE)_A4.ps.gz $(PACKAGE)_A4_2.ps.gz

DESTS=$(PACKAGE)_USLetter.pdf $(PACKAGE)_USLetter_2.pdf \
	$(PACKAGE)_A4.pdf $(PACKAGE)_A4_2.pdf

PYTHONPATH=../lib:$(LIBPATH)
export PYTHONPATH

.PHONY: all
all: $(VALIDFILE) $(DESTS)

$(VALIDFILE): $(SRCXML)
	$(BINPATH)/bgvalidate $< $@

# implicit rules -----------

%.pdf: %.fo
	FOP_OPTS=-Xms4096m $(FOP) $(FOPOPTIONS) $< -pdf $@

%.ps: %.fo
	FOP_OPTS=-Xms4096m $(FOP) $(FOPOPTIONS) $< -ps $@

%.ps.gz: %.ps
	gzip -f $<

# FO -----------------------

$(PACKAGE)_USLetter.fo: $(VALIDFILE)
	( cd $(SRCPATH); ../bin/bg2fo $(STD_FONT_OPTS) -o ../builders/print/$@ $(PACKAGE).xml )

$(PACKAGE)_USLetter_2.fo: $(VALIDFILE)
	( cd $(SRCPATH); ../bin/bg2fo $(STD_FONT_OPTS) -2 -o ../builders/print/$@ $(PACKAGE).xml )

$(PACKAGE)_A4.fo: $(VALIDFILE)
	( cd $(SRCPATH); ../bin/bg2fo $(STD_FONT_OPTS) -a4 -o ../builders/print/$@ $(PACKAGE).xml )

$(PACKAGE)_A4_2.fo: $(VALIDFILE)
	( cd $(SRCPATH); ../bin/bg2fo $(STD_FONT_OPTS) -a4 -2 -o ../builders/print/$@ $(PACKAGE).xml )

$(PACKAGE)_tech_2.fo: $(VALIDFILE)
	( cd $(SRCPATH); ../bin/bg2fo -fat -bastard $(STD_FONT_OPTS) -tech -2 -suppress -o ../builders/print/$@ $(PACKAGE).xml )

$(PACKAGE)_crownquattro_2.fo: $(VALIDFILE)
	( cd $(SRCPATH); ../bin/bg2fo -fat -bgcolor '#cccccc' -bastard $(STD_FONT_OPTS) -crownquattro -2 -suppress -o ../builders/print/$@ $(PACKAGE).xml )

# PDF ----------------------

$(PACKAGE)_USLetter.pdf: $(PACKAGE)_USLetter.fo $(IMGS)
$(PACKAGE)_USLetter_2.pdf: $(PACKAGE)_USLetter_2.fo $(IMGS)
$(PACKAGE)_A4.pdf: $(PACKAGE)_A4.fo $(IMGS)
$(PACKAGE)_A4_2.pdf: $(PACKAGE)_A4_2.fo $(IMGS)
$(PACKAGE)_tech_2.pdf: $(PACKAGE)_tech_2.fo $(IMGS)
$(PACKAGE)_crownquattro_2.pdf: $(PACKAGE)_crownquattro_2.fo $(IMGS)

# PS -----------------------

$(PACKAGE)_USLetter.ps: $(PACKAGE)_USLetter.fo $(IMGS)
$(PACKAGE)_USLetter_2.ps: $(PACKAGE)_USLetter_2.fo $(IMGS)
$(PACKAGE)_A4.ps: $(PACKAGE)_A4.fo $(IMGS)
$(PACKAGE)_A4_2.ps: $(PACKAGE)_A4_2.fo $(IMGS)

$(PACKAGE)_USLetter.ps.gz: $(PACKAGE)_USLetter.ps
$(PACKAGE)_USLetter_2.ps.gz: $(PACKAGE)_USLetter_2.ps
$(PACKAGE)_A4.ps.gz: $(PACKAGE)_A4.ps
$(PACKAGE)_A4_2.ps.gz: $(PACKAGE)_A4_2.ps

# IMAGES -------------------
dataencap-300-4.736.png: $(IMGPATH)/dataencap-300-4.736.png
	cp $< $@

cs-300-3.334.png: $(IMGPATH)/cs-300-3.334.png
	cp $< $@

amazon-72-1.805.png: $(IMGPATH)/amazon-72-1.805.png
	cp $< $@

.PHONY: pristine
pristine: clean
	rm -f *.pdf *.ps *.ps.gz

.PHONY: clean
clean:
	rm -f *.fo *.png

