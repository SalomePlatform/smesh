#  Copyright (C) 2007-2008  CEA/DEN, EDF R&D, OPEN CASCADE
#
#  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
#  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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

EXTRA_DIST += SMESH

dev_docs:
	cp -fr $(srcdir)/SMESH ./INPUT; \
	cp -f ./SMESH/doxyfile ./INPUT; \
	cd INPUT; \
	echo "DOXYGEN SUPPORT PYTHON - @DOXYGEN_WITH_PYTHON@"; \
	if( test "x@DOXYGEN_WITH_PYTHON@" = "xyes"); then \
	  sed 's|python_extension_must_be_here|*.py|' doxyfile > doxyfile1; \
	else \
	  sed 's|python_extension_must_be_here||' doxyfile > doxyfile1; \
	fi; \
	mv -f doxyfile1 doxyfile; \
	if( test "x@DOXYGEN_WITH_STL@" = "xyes"); then \
	  sed -e 's|BUILTIN_STL_SUPPORT    = NO|BUILTIN_STL_SUPPORT    = YES|' doxyfile > doxyfile1; \
	  mv -f doxyfile1 doxyfile; \
	fi; \
	$(DOXYGEN) -u doxyfile; \
	echo "Running doxygen in directory:"`pwd`; \
	$(DOXYGEN) doxyfile; \
	cd ../; \
	cp -fr $(srcdir)/SMESH/sources/ SMESH ; \
	rm -fr INPUT

doctuidir=$(docdir)/tui/SMESH

nodist_doctui_DATA= SMESH/doxyfile
EXTRA_DIST+= SMESH/doxyfile.in

install-data-local:
	if test -d SMESH; then \
	  $(INSTALL) -d $(docdir)/tui ; \
	  cp -rp SMESH $(docdir)/tui ; \
	  find $(docdir)/tui -name CVS -prune -exec rm -rf {} ";" ; \
	fi;

uninstall-local:
	rm -rf $(docdir)/tui/SMESH
