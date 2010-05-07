#  Copyright (C) 2007-2010  CEA/DEN, EDF R&D, OPEN CASCADE
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2.1 of the License.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
#  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

#  File   : Makefile.in
#  Author : Vasily Rusyaev (Open Cascade NN)
#  Module : doc
#
include $(top_srcdir)/adm_local/unix/make_common_starter.am

EXTRA_DIST += images static/doxygen.css static/footer.html

tuidocdir = $(docdir)/tui/SMESH
tuidoc_DATA = images/head.png images/smeshscreen.png

dist_doc_DATA = \
	extra/AddNetgenInSalome2.pdf	\
	extra/AddNetgenInSalome2.ps	\
	extra/AddNetgenInSalome2.sxw	\
	extra/PluginMeshers.html

EXTRA_DIST += extra/PluginMeshers.txt

dev_docs: doxyfile
	echo "Running doxygen in directory: "`pwd`; \
	$(DOXYGEN) $<;

clean-local:
	-rm -fr SMESH doxygen.bak

install-data-local:
	if test -d SMESH; then \
	  $(INSTALL) -d $(DESTDIR)$(docdir)/tui ; \
	  cp -rp SMESH $(DESTDIR)$(docdir)/tui ; \
	fi;

uninstall-local:
	rm -rf $(DESTDIR)$(docdir)/tui/SMESH
