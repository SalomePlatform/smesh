# Copyright (C) 2007-2011  CEA/DEN, EDF R&D, OPEN CASCADE
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

#  Author : Guillaume Boulant (EDF/R&D)

include $(top_srcdir)/adm_local/unix/make_common_starter.am

EXTRA_DIST += images input

#
# The simplest way to extends the documentation of SMESH with the
# documentation for PADDER is to copy the local source files (*.doc
# and *.png) in the source directory of SMESH documentation. Theses
# pages will be automagically included in the SMESH documentation
# generating process (make usr_docs executed at installation step) and
# will be inserted in the page pages.html that lists the "Related
# pages". 
#

DOC_FILES = \
	padder_userguide.doc

DOCDIR=$(top_srcdir)/doc/salome/gui/SMESH/input

IMG_FILES = \
	SMESH_spadder_end.png                            \
	SMESH_spadder_inputdialog_concrete.png           \
	SMESH_spadder_inputdialog_start.png              \
	SMESH_spadder_inputdialog_steelbar.png           \
	SMESH_spadder_menu.png                           \
	SMESH_spadder_plugindialog_compute_finished.png  \
	SMESH_spadder_plugindialog_compute_ready.png     \
	SMESH_spadder_plugindialog_compute_running.png   \
	SMESH_spadder_plugindialog_published.png         \
	SMESH_spadder_plugindialog_start.png             \
	SMESH_spadder_start.png


IMGDIR=$(top_srcdir)/doc/salome/gui/SMESH/images


all:
	@mkdir -p $(DOCDIR); \
	for docfile in $(DOC_FILES); do  \
	    if [ ! -h $(DOCDIR)/$${docfile} ]; then \
	        echo "Linking file $${docfile} to $(DOCDIR)/$${docfile}"; \
	        ln -s ../$(srcdir)/input/$${docfile} $(DOCDIR)/$${docfile}; \
	    fi; \
	done; \
	for imgfile in $(IMG_FILES); do  \
	    if [ ! -h $(IMGDIR)/$${imgfile} ]; then \
	        echo "Linking file $${imgfile} to $(IMGDIR)/$${imgfile}"; \
	        ln -s ../$(srcdir)/images/$${imgfile} $(IMGDIR)/$${imgfile}; \
	    fi; \
	done;

clean-local:
	@ls | grep -v -e Makefile -e doxyfile | xargs rm -rf; \
	for docfile in $(DOC_FILES); do  \
	    echo "Removing file $${docfile} from $(DOCDIR)"; \
	    rm $(DOCDIR)/$${docfile}; \
	done; \
	for imgfile in $(IMG_FILES); do  \
	    echo "Removing file $${imgfile} from $(IMGDIR)"; \
	    rm $(IMGDIR)/$${imgfile}; \
	done;

#
# For test purpose, we let the user generate a local dosygen
# documentation including only the local pages  
#
test_docs: doxyfile 
	echo "===========================================" ; \
	echo "Generating PADDER documentation" ;             \
	echo "===========================================" ; \
	$(DOXYGEN) doxyfile ;
