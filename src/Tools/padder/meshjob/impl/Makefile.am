
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
	-I$(builddir)/../idl $(KERNEL_CXXFLAGS) \
	@CORBA_CXXFLAGS@ @CORBA_INCLUDES@ @LIBXML_INCLUDES@

libMeshJobManagerEngine_la_FFLAGS = -fexceptions
libMeshJobManagerEngine_la_LDFLAGS   = \
	$(builddir)/../idl/libSalomeIDLSPADDER.la \
	@KERNEL_LDFLAGS@ -lSalomeContainer -lSalomeKernelHelpers \
	@LIBXML_LIBS@

# =============================================================
# Definition of the SPADDERPluginTester engine construction
# =============================================================
lib_LTLIBRARIES += libSPADDERPluginTesterEngine.la

libSPADDERPluginTesterEngine_la_SOURCES  = \
        SPADDERPluginTester_i.cxx

nodist_libSPADDERPluginTesterEngine_la_SOURCES =

libSPADDERPluginTesterEngine_la_CXXFLAGS = \
	-I$(builddir)/../idl $(KERNEL_CXXFLAGS) \
	@CORBA_CXXFLAGS@ @CORBA_INCLUDES@ @LIBXML_INCLUDES@

libSPADDERPluginTesterEngine_la_FFLAGS = -fexceptions
libSPADDERPluginTesterEngine_la_LIBADD = \
	$(builddir)/../idl/libSalomeIDLSPADDER.la \
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
	$(VTK_INCLUDES) $(CAS_CPPFLAGS)

libSPADDERPluginTesterEngine_la_LIBADD += \
	$(top_builddir)/src/SMESH/libSMESHimpl.la \
	$(top_builddir)/src/SMESH_I/libSMESHEngine.la \
	$(top_builddir)/src/SMESHDS/libSMESHDS.la \
	$(top_builddir)/src/SMDS/libSMDS.la \
	$(top_builddir)/src/SMESHUtils/libSMESHUtils.la

