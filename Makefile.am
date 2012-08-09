# Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
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

# -* Makefile *- 
# Author : Patrick GOLDBRONN (CEA)
# Date : 28/06/2001
# Modified by : Alexander BORODIN (OCN) - autotools usage
#
include $(top_srcdir)/adm_local/unix/make_common_starter.am

if SMESH_ENABLE_GUI
  ACLOCAL_AMFLAGS = -I adm_local/unix/config_files			\
                    -I ${GUI_ROOT_DIR}/adm_local/unix/config_files	\
                    -I ${KERNEL_ROOT_DIR}/salome_adm/unix/config_files	\
                    -I ${MED_ROOT_DIR}/adm_local/unix/config_files	\
                    -I ${GEOM_ROOT_DIR}/adm_local/unix/config_files
else !SMESH_ENABLE_GUI
  ACLOCAL_AMFLAGS = -I adm_local/unix/config_files			\
                    -I ${KERNEL_ROOT_DIR}/salome_adm/unix/config_files	\
                    -I ${MED_ROOT_DIR}/adm_local/unix/config_files	\
                    -I ${GEOM_ROOT_DIR}/adm_local/unix/config_files
endif

SUBDIRS = idl adm_local resources src bin doc

DIST_SUBDIRS = idl adm_local resources src bin doc

DISTCLEANFILES = a.out aclocal.m4 configure local-install.sh hack_libtool

salomeinclude_DATA = SMESH_version.h

EXTRA_DIST +=		\
	build_configure	\
	clean_configure	\
	build_cmake     \
	build_cmake.bat \
	LICENCE

dist-hook:
	rm -rf `find $(distdir) -name CVS`

usr_docs:
	(cd doc && $(MAKE) $(AM_MAKEFLAGS) usr_docs)

docs: usr_docs

dev_docs:
	(cd doc && $(MAKE) $(AM_MAKEFLAGS) dev_docs)
