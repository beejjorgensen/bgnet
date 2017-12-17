include ../common.mak

.PHONY: all
all: $(PACKAGE).html $(IMGS) $(PACKAGE).css

$(PACKAGE).html: $(VALIDFILE)
	( cd $(SRCDIR); ../bin/bg2html -o ../builders/html/singlepage/$@ -stylesheet $(PACKAGE).css $(PACKAGE).xml )

$(PACKAGE).css: $(CSS)
	cp $< $@

include ../commontargets.mak

