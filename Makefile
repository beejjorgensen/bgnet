PACKAGE=bgnet
UPLOADDIR=beej71@alfalfa.dreamhost.com:~/beej.us/guide/$(PACKAGE)
BUILDDIR=./build

.PHONY: all
all:
	$(MAKE) -C builders

.PHONY: buildcp
buildcp:
	mkdir -p $(BUILDDIR)/{pdf,html/single,html/multi,html/archive,translations}
	cp -v website/* $(BUILDDIR)
	cp -v builders/print/*.pdf $(BUILDDIR)/pdf
	cp -v builders/html/$(PACKAGE)*.{tgz,zip} $(BUILDDIR)/html/archive
	cp -v builders/html/singlepage/*.{html,css,png} $(BUILDDIR)/html/single
	cp -v builders/html/multipage/*.{html,css,png} $(BUILDDIR)/html/multi
	cp -v translations/*.pdf $(BUILDDIR)/translations 2>/dev/null || : 

.PHONY: upload
upload: pristine all buildcp
	rsync -rv -e ssh --delete build/* $(UPLOADDIR)

.PHONY: pristine
pristine: clean
	$(MAKE) -C builders $@
	rm -rf $(BUILDDIR)
	rm -f lib/*.pyc

.PHONY: clean
clean:
	$(MAKE) -C builders $@
	rm -f $(PACKAGE).valid

