# to be included from singlepage/Makefile and multipage/Makefile

PACKAGE=bgnet
BASE=../../..
SRCDIR=$(BASE)/src
SRC=$(SRCDIR)/$(PACKAGE).xml
CSS=$(SRCDIR)/$(PACKAGE).css
VALIDFILE=$(BASE)/$(PACKAGE).valid
BINPATH=$(BASE)/bin
LIBPATH=$(BASE)/lib
IMGPATH=$(SRCDIR)/images
IMGS=cs-120-3.334.png dataencap-120-4.736.png
HEADER="Beej's Guide to Network Programming"

PYTHONPATH=../lib:$(LIBPATH)
export PYTHONPATH

