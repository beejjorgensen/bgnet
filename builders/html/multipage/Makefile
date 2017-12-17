include ../common.mak

.PHONY: all
all: index.html $(IMGS) $(PACKAGE).css

index.html: $(VALIDFILE)
	( cd $(SRCDIR); ../bin/bg2html -header $(HEADER) -multipage -o ../builders/html/multipage -stylesheet $(PACKAGE).css $(PACKAGE).xml )

$(PACKAGE).css: $(CSS)
	cp $< $@

include ../commontargets.mak

