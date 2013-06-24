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
include $(top_srcdir)/adm_local/unix/make_common_starter.am

lib_LTLIBRARIES= libMEDWrapper.la

salomeinclude_HEADERS= \
	MED_Factory.hxx \
	MED_WrapperFactory.hxx

dist_libMEDWrapper_la_SOURCES= \
	MED_Factory.cxx

libMEDWrapper_la_CPPFLAGS= $(BOOST_CPPFLAGS) $(MED3_INCLUDES) \
	-I$(srcdir)/../Base
libMEDWrapper_la_CPPFLAGS+= -I$(KERNEL_ROOT_DIR)/include/salome
libMEDWrapper_la_CPPFLAGS+= -I$(srcdir)/../V2_2
libMEDWrapper_la_LDFLAGS= ../V2_2/libMEDWrapper_V2_2.la

# Executables targets
bin_PROGRAMS = MED_Test mprint_version

dist_MED_Test_SOURCES= MED_Test.cxx
MED_Test_CPPFLAGS= $(libMEDWrapper_la_CPPFLAGS)
MED_Test_LDADD= $(libMEDWrapper_la_LDFLAGS) ../Base/libMEDWrapperBase.la \
	$(BOOST_LIB_THREAD) $(BOOST_LIB_SYSTEM) libMEDWrapper.la

dist_mprint_version_SOURCES= mprint_version.cxx
mprint_version_CPPFLAGS= $(libMEDWrapper_la_CPPFLAGS)
mprint_version_LDADD= $(libMEDWrapper_la_LDFLAGS) ../Base/libMEDWrapperBase.la \
	$(BOOST_LIB_THREAD) $(BOOST_LIB_SYSTEM) libMEDWrapper.la $(MED3_LIBS_C_ONLY)
