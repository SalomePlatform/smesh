#  Copyright (C) 2005  CEA/DEN, EDF R&D, OPEN CASCADE, PRINCIPIA R&D
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
#  Modified by : Alexander BORODIN (OCN) - autotools usage
#  Module : doc

include $(top_srcdir)/adm_local/unix/make_common_starter.am

EXTRA_DIST+=SMESH

doxygen=@DOXYGEN@

usr_docs:
	cd ./SMESH; \
	echo "Processing smeshDC.py file: "; \
	$(doxygen) ./doxyfile_py; \
	sed 's|../../../share/salome|$(top_srcdir)|' ./doxyfile > ./doxyfile1; \
	mv -f doxyfile1 doxyfile; \
	echo "Running doxygen in directory: "`pwd`; \
	$(doxygen) ./doxyfile;

docs: usr_docs

clean-local:
	rm -fr `ls | grep -v "Makefile"`
	rm -fr SMESH

install-data-local: usr_docs
	mkdir -p $(docdir)/gui
	cp -rf SMESH $(docdir)/gui
	-find $(PWD) -name CVS -prune -exec rm -rf {} \;

uninstall-local:
	rm -rf $(docdir)/gui/SMESH

docguidir=$(docdir)/gui/SMESH

nodist_docgui_DATA= SMESH/doxyfile
nodist_docgui_DATA+= SMESH/doxyfile_py
EXTRA_DIST+= SMESH/doxyfile.in
EXTRA_DIST+= SMESH/doxyfile_py.in
