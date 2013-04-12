# Makefile for Sphinx documentation
#

include $(top_srcdir)/adm_local/unix/make_common_starter.am

yamsdocdir=$(docdir)/gui/SMESH/yams


RSTFILES = lct.rst \
	   index.rst \
	   editHypo.rst \
	   Mandatory_params.rst \
	   Generics_params.rst \
	   Advanced_params.rst

EXTRA_DIST += $(RSTFILES)

# You can set these variables from the command line.
SPHINXOPTS    =
SOURCEDIR     = $(srcdir)
SPHINXBUILD   = sphinx-build
PAPER         =
BUILDDIR      = _build
CONF_FILE_DIR = .

# Internal variables.
PAPEROPT_a4     = -D latex_paper_size=a4
PAPEROPT_letter = -D latex_paper_size=letter
ALLSPHINXOPTS   = -d $(BUILDDIR)/doctrees -c $(CONF_FILE_DIR) $(PAPEROPT_$(PAPER)) $(SPHINXOPTS) $(SOURCEDIR)

.PHONY: help clean html latexpdf 

all:
	make html

help:
	@echo "Please use \`make <target>' where <target> is one of"
	@echo "  html       to make standalone HTML files"
	@echo "  dirhtml    to make HTML files named index.html in directories"
	@echo "  singlehtml to make a single large HTML file"
	@echo "  pickle     to make pickle files"
	@echo "  json       to make JSON files"
	@echo "  htmlhelp   to make HTML files and a HTML help project"
	@echo "  qthelp     to make HTML files and a qthelp project"
	@echo "  devhelp    to make HTML files and a Devhelp project"
	@echo "  epub       to make an epub"
	@echo "  latex      to make LaTeX files, you can set PAPER=a4 or PAPER=letter"
	@echo "  latexpdf   to make LaTeX files and run them through pdflatex"
	@echo "  text       to make text files"
	@echo "  man        to make manual pages"
	@echo "  changes    to make an overview of all changed/added/deprecated items"
	@echo "  linkcheck  to check all external links for integrity"
	@echo "  doctest    to run all doctests embedded in the documentation (if enabled)"

install-data-local: all
	test -z $(yamsdocdir) || mkdir -p $(yamsdocdir)
	cp -rf $(BUILDDIR)/html/* $(yamsdocdir) ;

clean:
	-rm -rf $(BUILDDIR)/*

html:
	$(SPHINXBUILD) -b html $(ALLSPHINXOPTS) $(BUILDDIR)/html
	@echo
	@echo "Build finished. The HTML pages are in $(BUILDDIR)/html."

latexpdf:
	$(SPHINXBUILD) -b latex $(ALLSPHINXOPTS) $(BUILDDIR)/latex
	@echo "Running LaTeX files through pdflatex..."
	$(MAKE) -C $(BUILDDIR)/latex all-pdf
	@echo "pdflatex finished; the PDF files are in $(BUILDDIR)/latex."

