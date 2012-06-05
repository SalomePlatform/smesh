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

# SMESH SMESHGUI : GUI for SMESH component
# File   : Makefile.am
# Author : Alexander BORODIN, Open CASCADE S.A.S.
#
include $(top_srcdir)/adm_local/unix/make_common_starter.am

# ===============================================================
# Swig targets
# ===============================================================
# (cf. http://www.geocities.com/foetsch/python/swig_linux.htm)
#
# Step 1: build the wrapping source files with swig
#
# libSALOME_LifeCycleCORBA.i -- swig --> swig_wrap.cpp
#                                        libSALOME_Swig.py
#
# Step 2: build the dynamic library from cpp built source files and
#         dependant libraries.
#
# swig_wrap.cpp -- gcc --> swig_wrap.o    |-- link --> _libSMESH_Swig.la
#                          +              |
#                          dependant libs |
#
# The file libSMESH_Swig.py will be installed to the
# <prefix>/bin/salome directory.
# The library _libSMESH_Swig.so will be installed to the 
# <prefix>/lib/python<version>/site-package/salome directory.
#

SWIG_FLAGS    = @SWIG_FLAGS@ -I$(srcdir) -I$(srcdir)/../SMESHGUI
SWIG_SOURCES  = libSMESH_Swig.i

salomeinclude_HEADERS = $(SWIG_SOURCES) libSMESH_Swig.h

# Libraries targets
lib_LTLIBRARIES                  = _libSMESH_Swig.la
_libSMESH_Swig_la_SOURCES        = $(SWIG_SOURCES) libSMESH_Swig.h libSMESH_Swig.cxx
nodist__libSMESH_Swig_la_SOURCES = swig_wrap.cpp

_libSMESH_Swig_la_CPPFLAGS =	\
	$(QT_INCLUDES)		\
	$(PYTHON_INCLUDES)	\
	$(CAS_CPPFLAGS)		\
	$(VTK_INCLUDES)		\
	$(OGL_INCLUDES) 	\
	$(KERNEL_CXXFLAGS)	\
	$(GUI_CXXFLAGS)		\
	$(MED_CXXFLAGS)		\
	$(GEOM_CXXFLAGS)	\
	$(CORBA_CXXFLAGS)	\
	$(CORBA_INCLUDES)	\
	$(BOOST_CPPFLAGS)	\
	-I$(srcdir)             \
	-I$(srcdir)/../SMESHGUI	\
	-I$(top_builddir)/idl

_libSMESH_Swig_la_LDFLAGS  = -module
_libSMESH_Swig_la_LIBADD   = ../SMESHGUI/libSMESH.la			\
	$(KERNEL_LDFLAGS) -lSalomeGenericObj -lSALOMELocalTrace		\
	$(GUI_LDFLAGS) -lCAM -lsuit -lqtx -lSalomeApp -lstd -lEvent	\
	$(PYTHON_LIBS) $(QT_MT_LIBS)

swig_wrap.cpp : $(SWIG_SOURCES)
	$(SWIG) $(SWIG_FLAGS) -o $@ $<

libSMESH_Swig.py: swig_wrap.cpp

CLEANFILES = swig_wrap.cpp libSMESH_Swig.py

#
# ===============================================================
# Files to be installed
# ===============================================================
#

# Scripts to be installed (distributed)
dist_salomescript_DATA =

# Scripts to be installed (non-distributed)
nodist_salomescript_DATA = libSMESH_Swig.py
