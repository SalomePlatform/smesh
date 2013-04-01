# Copyright (C) 2007-2013  CEA/DEN, EDF R&D, OPEN CASCADE
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

#  File   : Makefile.in
#  Author : Vasily Rusyaev (Open Cascade NN)
#  Modified by : Alexander BORODIN (OCN) - autotools usage
#  Module : doc
#
include $(top_srcdir)/adm_local/unix/make_common_starter.am

EXTRA_DIST += images input static/footer.html static/doxygen.css

dist_salomescript_PYTHON = collect_mesh_methods.py

guidocdir = $(docdir)/gui/SMESH
guidoc_DATA = images/head.png

DOC_PYTHONPATH=$(prefix)/bin/salome:$(prefix)/lib/python$(PYTHON_VERSION)/site-packages/salome:$(MED_ROOT_DIR)/lib/python$(PYTHON_VERSION)/site-packages/salome:$(GEOM_ROOT_DIR)/bin/salome:$(GEOM_ROOT_DIR)/lib/python$(PYTHON_VERSION)/site-packages/salome:$(KERNEL_ROOT_DIR)/bin/salome:$(KERNEL_ROOT_DIR)/lib/python$(PYTHON_VERSION)/site-packages/salome:$(OMNIORB_ROOT)/lib/python$(PYTHON_VERSION)/site-packages:$(OMNIORB_ROOT)/lib64/python$(PYTHON_VERSION)/site-packages
DOC_SMESH_MeshersList=StdMeshers

# to have smesh.py in the documentation instead of smeshDC.py
# we create dummy smesh.py from the smeshDC.py
smesh.py: ../../../../src/SMESH_SWIG/smeshDC.py
	@awk '/^class Mesh:/ { mesh_found=1 } // { if (mesh_found) {print $$0; next} } /^ +(def|#)/ { match( $$0, /^ +/); print substr( $$0, 1+RLENGTH ); next } /^class smeshDC/ { next } //' \
	  $< > $@

tmp/smesh.py: $(top_srcdir)/src/SMESH_SWIG/StdMeshersDC.py $(srcdir)/collect_mesh_methods.py
	@mkdir -p tmp && PYTHONPATH=$(DOC_PYTHONPATH):${PYTHONPATH} SMESH_MeshersList=$(DOC_SMESH_MeshersList) $(PYTHON) $(srcdir)/collect_mesh_methods.py -o $@ StdMeshers

usr_docs: doxyfile_py doxyfile smesh.py tmp/smesh.py
	@$(DOXYGEN) doxyfile_py ;						\
	$(DOXYGEN) doxyfile

docs: usr_docs

clean-local:
	@for filen in `find . -maxdepth 1` ; do			\
	  case $${filen} in					\
	    ./Makefile | ./doxyfile | ./doxyfile_py ) ;;	\
	    . | .. | ./static ) ;;				\
	    *) echo "Removing $${filen}" ; rm -rf $${filen} ;;	\
	  esac ;						\
	done ;

install-data-local: usr_docs
	$(INSTALL) -d $(DESTDIR)$(docdir)/gui/SMESH
	@for filen in `find . -maxdepth 1` ; do							\
	  case $${filen} in									\
	    ./Makefile | ./doxyfile | ./doxyfile_py ) ;;					\
	    ./doxyfile.bak | ./doxyfile_py.bak ) ;;						\
	    . | .. | ./static ) ;;								\
	    *) echo "Installing $${filen}" ; cp -rp $${filen} $(DESTDIR)$(docdir)/gui/SMESH ;;	\
	  esac ;										\
	done ;                                                                                  \
	cp -rp $(srcdir)/images/head.png $(DESTDIR)$(docdir)/gui/SMESH/smeshpy_doc ;

uninstall-local:
	rm -rf $(DESTDIR)$(docdir)/gui/SMESH
