PACKAGE=bgnet
UPLOADDIR=beej71@alfalfa.dreamhost.com:~/beej.us/guide/$(PACKAGE)
BUILDDIR=./stage
BUILDTMP=./build_tmp
SRCDIR=./src

.PHONY: all
all:
	$(MAKE) -C ${SRCDIR}
	$(MAKE) -C source clean

.PHONY: stage
stage:
	mkdir -p $(BUILDDIR)/{pdf,html,translations,source}
	mkdir -p $(BUILDDIR)/html/$(PACKAGE)
	cp -v website/* website/.htaccess $(BUILDDIR)
	cp -v ${SRCDIR}/$(PACKAGE)*.pdf $(BUILDDIR)/pdf
	cp -v ${SRCDIR}/$(PACKAGE).html $(BUILDDIR)/html/index.html
	cp -v ${SRCDIR}/$(PACKAGE)-wide.html $(BUILDDIR)/html/index-wide.html
	cp -v ${SRCDIR}/split/* $(BUILDDIR)/html/$(PACKAGE)
	( cd $(BUILDDIR)/html; zip -r $(PACKAGE).zip $(PACKAGE); mv $(PACKAGE) split )
	mkdir -p $(BUILDDIR)/html/$(PACKAGE)
	cp -v ${SRCDIR}/split-wide/* $(BUILDDIR)/html/$(PACKAGE)
	( cd $(BUILDDIR)/html; zip -r $(PACKAGE)-wide.zip $(PACKAGE); mv $(PACKAGE) split-wide )
	cp -v ${SRCDIR}/{cs,dataencap}.svg $(BUILDDIR)/html/
	cp -v ${SRCDIR}/{cs,dataencap}.svg $(BUILDDIR)/html/split
	cp -v ${SRCDIR}/{cs,dataencap}.svg $(BUILDDIR)/html/split-wide
	cp -v translations/*.{pdf,html} $(BUILDDIR)/translations 2>/dev/null || : 
	cp -rv source/* $(BUILDDIR)/source
	mkdir -p $(BUILDTMP)/$(PACKAGE)_source
	cp -rv source/* $(BUILDTMP)/$(PACKAGE)_source
	( cd $(BUILDTMP); zip -r $(PACKAGE)_source.zip $(PACKAGE)_source )
	cp -v $(BUILDTMP)/$(PACKAGE)_source.zip $(BUILDDIR)/source
	rm -rf $(BUILDTMP)

.PHONY: upload
upload: pristine all stage
	rsync -rv -e ssh --delete $(BUILDDIR)/* $(BUILDDIR)/.htaccess $(UPLOADDIR)

.PHONY: fastupload
fastupload: all stage
	rsync -rv -e ssh --delete $(BUILDDIR)/* $(BUILDDIR)/.htaccess $(UPLOADDIR)

.PHONY: pristine
pristine: clean
	$(MAKE) -C ${SRCDIR} $@
	$(MAKE) -C source $@
	rm -rf $(BUILDDIR)

.PHONY: clean
clean:
	rm -rf 
	$(MAKE) -C ${SRCDIR} $@
	$(MAKE) -C source $@

