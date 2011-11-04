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

# This Makefile is responsible of generating the client and server
# implementation of IDL interfaces for both C++ and python usage.
# The building process of the C++ files is in charge of each source
# package and then is not manage here.
#
include $(top_srcdir)/adm_local/unix/make_common_starter.am

BUILT_SOURCES = \
	MESHJOBSK.cc

IDL_FILES = \
	MESHJOB.idl

salomeidl_DATA = $(IDL_FILES) 

lib_LTLIBRARIES = libSalomeIDLSPADDER.la
libSalomeIDLSPADDER_la_SOURCES      =
nodist_libSalomeIDLSPADDER_la_SOURCES = $(BUILT_SOURCES)
nodist_salomeinclude_HEADERS= $(IDL_FILES:%idl=%hh)

OMNIORB_CXXFLAGS=@OMNIORB_CXXFLAGS@ @OMNIORB_INCLUDES@
OMNIORB_LIBS=@OMNIORB_LIBS@

libSalomeIDLSPADDER_la_CXXFLAGS = \
	$(KERNEL_CXXFLAGS) \
	$(OMNIORB_CXXFLAGS) \
	-I.

libSalomeIDLSPADDER_la_LIBADD     = \
	$(KERNEL_LDFLAGS) -lSalomeIDLKernel \
	$(OMNIORB_LIBS)


# These variables defines the building process of CORBA files
IDLCXXFLAGS = \
        -bcxx -I. \
	@OMNIORB_IDLCXXFLAGS@ \
        -I$(KERNEL_ROOT_DIR)/idl/salome

IDLPYFLAGS  = \
	-I. \
	@OMNIORB_IDLPYFLAGS@ \
        -I$(KERNEL_ROOT_DIR)/idl/salome

##########################################################
SUFFIXES = .idl .hh SK.cc

%SK.cc %.hh : %.idl
	$(OMNIORB_IDL) $(IDLCXXFLAGS) $<

%_idl.py : %.idl
	$(OMNIORB_IDL) $(IDLPYFLAGS)  $<

CLEANFILES = *.hh *SK.cc *.py *.hxx *.cxx

EXTRA_DIST += $(IDL_FILES)

install-data-local: $(IDL_FILES)
	$(INSTALL) -d  $(DESTDIR)$(salomepythondir)
	ls $^ | while read file; do \
	$(OMNIORB_IDL) $(IDLPYFLAGS) -C$(DESTDIR)$(salomepythondir) $$file ; \
	done

uninstall-local:
	rm -rf $(DESTDIR)$(salomepythondir)/*

