PACKAGE=bgnet
UPLOADDIR=beej71@alfalfa.dreamhost.com:~/beej.us/guide/$(PACKAGE)
BUILDDIR=./stage

.PHONY: all
all:
	$(MAKE) -C src

.PHONY: stage
stage:
	mkdir -p $(BUILDDIR)/{pdf,html,translations,examples}
	cp -v website/* $(BUILDDIR)
	cp -v src/bgnet*.pdf $(BUILDDIR)/pdf
	cp -v src/bgnet.html $(BUILDDIR)/html/index.html
	cp -v translations/*.pdf $(BUILDDIR)/translations 2>/dev/null || : 
	cp -v examples/*.c $(BUILDDIR)/examples
	cp -v examples/Makefile $(BUILDDIR)/examples

.PHONY: upload
upload: pristine all stage
	rsync -rv -e ssh --delete ${BUILDDIR}/* $(UPLOADDIR)

.PHONY: pristine
pristine: clean
	$(MAKE) -C src $@
	$(MAKE) -C examples $@
	rm -rf $(BUILDDIR)

.PHONY: clean
clean:
	$(MAKE) -C src $@
	$(MAKE) -C examples $@

