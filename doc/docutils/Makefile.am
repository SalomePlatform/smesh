#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
#
# Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
# CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

include $(top_srcdir)/adm_local/unix/make_common_starter.am

pydocdir = $(docdir)/tui/SMESH/docutils

RSTFILES = \
	index.rst \
	overview.rst \
	docapi.rst

EXTRA_DIST += $(RSTFILES)

SPHINXOPTS      =
SOURCEDIR       = $(srcdir)
SPHINXBUILD     = sphinx-build
PAPEROPT_a4     = -D latex_paper_size=a4
ALLSPHINXOPTS   = -d doctrees $(PAPEROPT_a4) $(SPHINXOPTS) $(SOURCEDIR)

if SMESH_ENABLE_GUI
SPHINX_PYTHONPATH = $(prefix)/lib/python$(PYTHON_VERSION)/site-packages/salome:$(prefix)/lib64/python$(PYTHON_VERSION)/site-packages/salome:$(GUI_ROOT_DIR)/lib/salome:$(GUI_ROOT_DIR)/lib/python$(PYTHON_VERSION)/site-packages/salome:$(GUI_ROOT_DIR)/lib64/python$(PYTHON_VERSION)/site-packages/salome:$(KERNEL_ROOT_DIR)/bin/salome:$(KERNEL_ROOT_DIR)/lib/python$(PYTHON_VERSION)/site-packages/salome:$(KERNEL_ROOT_DIR)/lib64/python$(PYTHON_VERSION)/site-packages/salome:$(OMNIORB_ROOT)/lib/python$(PYTHON_VERSION)/site-packages:$(OMNIORB_ROOT)/lib64/python$(PYTHON_VERSION)/site-packages
else !SMESH_ENABLE_GUI
SPHINX_PYTHONPATH = $(prefix)/lib/python$(PYTHON_VERSION)/site-packages/salome:$(prefix)/lib64/python$(PYTHON_VERSION)/site-packages/salome:$(KERNEL_ROOT_DIR)/bin/salome:$(KERNEL_ROOT_DIR)/lib/python$(PYTHON_VERSION)/site-packages/salome:$(KERNEL_ROOT_DIR)/lib64/python$(PYTHON_VERSION)/site-packages/salome:$(OMNIORB_ROOT)/lib/python$(PYTHON_VERSION)/site-packages:$(OMNIORB_ROOT)/lib64/python$(PYTHON_VERSION)/site-packages
endif

if SMESH_ENABLE_GUI
SPHINX_LD_LIBRARY_PATH = $(GUI_ROOT_DIR)/lib/salome:$(KERNEL_ROOT_DIR)/lib/salome:$(OMNIORB_ROOT)/lib
else !SMESH_ENABLE_GUI
SPHINX_LD_LIBRARY_PATH = $(KERNEL_ROOT_DIR)/lib/salome:$(OMNIORB_ROOT)/lib
endif


.PHONY: latex

if SPHINX_IS_OK

html/index.html:$(RSTFILES)
	mkdir -p html doctrees
	PYTHONPATH=$(SPHINX_PYTHONPATH):${PYTHONPATH} \
	LD_LIBRARY_PATH=$(SPHINX_LD_LIBRARY_PATH):${LD_LIBRARY_PATH} \
	$(SPHINXBUILD) -c $(top_builddir)/doc/docutils -W -b html $(ALLSPHINXOPTS) html
	@echo
	@echo "Build finished. The HTML pages are in html."

else

html/index.html:
	@echo "Documentation for Python package not built. Sphinx was not present at configure time."

endif

latex:
	mkdir -p latex doctrees
	PYTHONPATH=$(SPHINX_PYTHONPATH):${PYTHONPATH} \
	LD_LIBRARY_PATH=$(SPHINX_LD_LIBRARY_PATH):${LD_LIBRARY_PATH} \
	$(SPHINXBUILD) -c $(top_builddir)/doc/docutils -W -b latex $(ALLSPHINXOPTS) latex
	@echo
	@echo "Build finished; the LaTeX files are in latex."
	@echo "Run \`make all-pdf' or \`make all-ps' in that directory to" \
	      "run these through (pdf)latex."

install-data-local: html/index.html
	test -z $(pydocdir) || mkdir -p $(DESTDIR)$(pydocdir)
	if test -d "html"; then b=; else b="$(srcdir)/"; fi; \
	cp -rf $$b"html"/* $(pydocdir) ; \
	if test -f $$b"latex"/smeshpy.pdf; then cp -f $$b"latex"/smeshpy.pdf $(pydocdir) ; fi;

uninstall-local:
	-test -d $(pydocdir) && chmod -R +w $(pydocdir) && rm -rf $(pydocdir)/*

clean-local:
	-rm -rf html latex doctrees
	if test -d "html"; then rm -rf html ; fi

dist-hook:
	-test -d html && cp -Rp html $(distdir)
