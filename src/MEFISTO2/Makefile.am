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

#  File   : Makefile.in
#  Author : 
#  Modified by : Alexander BORODIN (OCN) - autotools usage
#  Module : SMESH
#
include $(top_srcdir)/adm_local/unix/make_common_starter.am

# header files 
salomeinclude_HEADERS = \
	aptrte.h Rn.h

# Libraries targets
lib_LTLIBRARIES = libMEFISTO2D.la
dist_libMEFISTO2D_la_SOURCES = \
	aptrte.cxx \
	trte.f

# additionnal information to compil and link file
libMEFISTO2D_la_CPPFLAGS = \
	$(CAS_CPPFLAGS) \
	$(KERNEL_CXXFLAGS) \
	@PLATFORM_INCLUDES@

libMEFISTO2D_la_LDFLAGS = \
	$(CAS_LDFLAGS) -lTKernel -lTKMath \
	$(KERNEL_LDFLAGS) -lSALOMELocalTrace \
	$(FCLIBS)

#if USE_GFORTRAN
#libMEFISTO2D_la_LDFLAGS += -lgfortran
#else !USE_GFORTRAN
#libMEFISTO2D_la_LDFLAGS += -lg2c
#endif

OBSOLETE_FILES = areteideale.f

EXTRA_DIST += $(OBSOLETE_FILES)
