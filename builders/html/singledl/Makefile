include ../common.mak

PACKAGENAME=$(PACKAGE)_singlepage
ZIPNAME=$(PACKAGENAME).zip
TGZNAME=$(PACKAGENAME).tgz
PACKAGEZIP=../$(ZIPNAME)
PACKAGETGZ=../$(TGZNAME)

.PHONY: all
all: $(PACKAGEZIP) $(PACKAGETGZ) autoclean

$(PACKAGEZIP): index.html $(IMGS) $(PACKAGE).css
	( mv *.css *.png $(PACKAGE) 2> /dev/null; exit 0 )
	rm -f $@; zip -r $@ $(PACKAGE)

$(PACKAGETGZ): index.html $(IMGS) $(PACKAGE).css
	( mv *.css *.png $(PACKAGE) 2> /dev/null; exit 0 )
	tar czvf $@ $(PACKAGE)

index.html: $(SRC)
	( mkdir $(PACKAGE) 2> /dev/null; exit 0 )
	( cd $(SRCDIR); ../bin/bg2html -o ../builders/html/singledl/$(PACKAGE)/$@ -stylesheet $(PACKAGE).css $(PACKAGE).xml )

$(PACKAGE).css: $(CSS)
	cp $< $@

.PHONY: autoclean
autoclean:
	rm -rf $(PACKAGE)

include ../commontargets.mak

