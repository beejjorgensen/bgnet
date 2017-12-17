PACKAGE=bgnet
UPLOADDIR=beej71@alfalfa.dreamhost.com:~/beej.us/guide/$(PACKAGE)
BUILDDIR=./build

.PHONY: all
all:
	$(MAKE) -C builders

.PHONY: buildcp
buildcp:
	mkdir -p $(BUILDDIR)/{pdf,html/single,html/multi,html/archive,translations,examples}
	cp -v website/* $(BUILDDIR)
	cp -v builders/print/*.pdf $(BUILDDIR)/pdf
	cp -v builders/html/$(PACKAGE)*.{tgz,zip} $(BUILDDIR)/html/archive
	cp -v builders/html/singlepage/*.{html,css,png} $(BUILDDIR)/html/single
	cp -v builders/html/multipage/*.{html,css,png} $(BUILDDIR)/html/multi
	cp -v translations/*.pdf $(BUILDDIR)/translations 2>/dev/null || : 
	cp -v examples/*.c $(BUILDDIR)/examples
	cp -v examples/Makefile $(BUILDDIR)/examples

.PHONY: upload
upload: pristine all buildcp
	rsync -rv -e ssh --delete build/* $(UPLOADDIR)

.PHONY: pristine
pristine: clean
	$(MAKE) -C builders $@
	$(MAKE) -C examples $@
	rm -rf $(BUILDDIR)
	rm -f lib/*.pyc

.PHONY: clean
clean:
	$(MAKE) -C builders $@
	$(MAKE) -C examples $@
	rm -f $(PACKAGE).valid

