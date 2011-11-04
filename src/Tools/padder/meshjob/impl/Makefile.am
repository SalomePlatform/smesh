
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


LIBXML_INCLUDES=@LIBXML_INCLUDES@
LIBXML_LIBS=@LIBXML_LIBS@
KERNEL_CXXFLAGS=@KERNEL_CXXFLAGS@
#KERNEL_LIBS=@KERNEL_LDFLAGS@ -lSalomeContainer -lOpUtil -lSalomeDSCContainer -lSalomeDSCSuperv -lSalomeDatastream -lSalomeDSCSupervBasic -lCalciumC -lSalomeKernelHelpers
KERNEL_LIBS = \
	@KERNEL_LDFLAGS@ \
	-lSalomeContainer

# -lSalomeLauncher -lSalomeKernelHelpers \
#	-lSalomeGenericObj -lSalomeIDLKernel

OMNIORB_CXXFLAGS=

libMeshJobManagerEngine_la_CXXFLAGS = \
	-I$(builddir)/../idl $(KERNEL_CXXFLAGS) \
	@CORBA_CXXFLAGS@ @CORBA_INCLUDES@ \
	$(LIBXML_INCLUDES)

libMeshJobManagerEngine_la_LDFLAGS   = \
	$(builddir)/../idl/libSalomeIDLSPADDER.la \
	$(KERNEL_LIBS) \
	$(LIBXML_LIBS)
