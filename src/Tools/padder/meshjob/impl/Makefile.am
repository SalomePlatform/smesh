# Copyright (C) 2011-2012  CEA/DEN, EDF R&D
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

AM_CFLAGS=$(SALOME_INCLUDES) -fexceptions

lib_LTLIBRARIES= libMeshJobManagerEngine.la

salomeinclude_HEADERS= \
	MeshJobManager_i.hxx


# =============================================================
# Definition of MeshJobManagerEngine construction
# =============================================================
libMeshJobManagerEngine_la_SOURCES  = \
	MeshJobManager_i.cxx

LIBXML_LIBS=@LIBXML_LIBS@
KERNEL_CXXFLAGS=@KERNEL_CXXFLAGS@

libMeshJobManagerEngine_la_CXXFLAGS = \
	-I$(top_builddir)/src/Tools/padder/meshjob/idl $(KERNEL_CXXFLAGS) \
	@CORBA_CXXFLAGS@ @CORBA_INCLUDES@ @LIBXML_INCLUDES@

libMeshJobManagerEngine_la_FFLAGS = -fexceptions
libMeshJobManagerEngine_la_LDFLAGS   = \
	$(top_builddir)/src/Tools/padder/meshjob/idl/libSalomeIDLSPADDER.la \
	@KERNEL_LDFLAGS@ -lSalomeContainer -lSalomeKernelHelpers -lSalomeLifeCycleCORBA \
	@LIBXML_LIBS@

# =============================================================
# Definition of the SPADDERPluginTester engine construction
# =============================================================
lib_LTLIBRARIES += libSPADDERPluginTesterEngine.la

libSPADDERPluginTesterEngine_la_SOURCES  = \
	SPADDERPluginTester_i.hxx \
        SPADDERPluginTester_i.cxx

nodist_libSPADDERPluginTesterEngine_la_SOURCES =

libSPADDERPluginTesterEngine_la_CXXFLAGS = \
	-I$(top_builddir)/src/Tools/padder/meshjob/idl $(KERNEL_CXXFLAGS) \
	@CORBA_CXXFLAGS@ @CORBA_INCLUDES@ @LIBXML_INCLUDES@

libSPADDERPluginTesterEngine_la_FFLAGS = -fexceptions
libSPADDERPluginTesterEngine_la_LIBADD = \
	$(top_builddir)/src/Tools/padder/meshjob/idl/libSalomeIDLSPADDER.la \
	@KERNEL_LDFLAGS@ -lSalomeContainer -lSalomeKernelHelpers \
	@LIBXML_LIBS@


# For testing SMESH
#
libSPADDERPluginTesterEngine_la_CXXFLAGS += \
	@GEOM_CXXFLAGS@ @MED_CXXFLAGS@ \
	-I$(top_builddir)/idl \
	-I$(top_srcdir)/src/SMESH \
	-I$(top_srcdir)/src/SMESH_I \
	-I$(top_srcdir)/src/SMESHDS \
	-I$(top_srcdir)/src/SMDS \
	-I$(top_srcdir)/src/SMESHUtils \
	$(VTK_INCLUDES) $(CAS_CPPFLAGS) $(BOOST_CPPFLAGS)

libSPADDERPluginTesterEngine_la_LIBADD += \
	$(top_builddir)/src/SMESH/libSMESHimpl.la \
	$(top_builddir)/src/SMESH_I/libSMESHEngine.la \
	$(top_builddir)/src/SMESHDS/libSMESHDS.la \
	$(top_builddir)/src/SMDS/libSMDS.la \
	$(top_builddir)/src/SMESHUtils/libSMESHUtils.la

