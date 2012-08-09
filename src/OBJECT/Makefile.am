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

#  SMESH OBJECT : interactive object for SMESH visualization
#  File   : Makefile.in
#  Modified by : Alexander BORODIN (OCN) - autotools usage
#  Module : SMESH
#
include $(top_srcdir)/adm_local/unix/make_common_starter.am

# header files 
salomeinclude_HEADERS = \
	SMESH_Actor.h \
	SMESH_Object.h \
	SMESH_ObjectDef.h \
	SMESH_ActorUtils.h \
	SMESH_ActorDef.h \
	SMESH_DeviceActor.h \
	SMESH_PreviewActorsCollection.h \
	SMESH_ExtractGeometry.h \
	SMESH_FaceOrientationFilter.h \
	SMESH_ScalarBarActor.h \
	SMESH_NodeLabelActor.h \
	SMESH_CellLabelActor.h

# Libraries targets

lib_LTLIBRARIES = libSMESHObject.la
dist_libSMESHObject_la_SOURCES = \
	SMESH_Object.cxx \
	SMESH_DeviceActor.cxx \
	SMESH_Actor.cxx \
	SMESH_PreviewActorsCollection.cxx \
	SMESH_ExtractGeometry.cxx \
	SMESH_ActorUtils.cxx \
	SMESH_FaceOrientationFilter.cxx \
	SMESH_ScalarBarActor.cxx \
	SMESH_NodeLabelActor.cxx \
	SMESH_CellLabelActor.cxx 

libSMESHObject_la_CPPFLAGS = \
        $(QT_INCLUDES) \
        $(KERNEL_CXXFLAGS) \
        $(GUI_CXXFLAGS) \
	$(MED_CXXFLAGS) \
	$(QWT_INCLUDES) \
	$(GEOM_CXXFLAGS) \
	$(CAS_CPPFLAGS) \
        $(VTK_INCLUDES) \
	$(CORBA_INCLUDES) \
	$(CORBA_CXXFLAGS) \
        $(BOOST_CPPFLAGS) \
        $(QT_INCLUDES) \
	-I$(srcdir)/../Controls \
	-I$(srcdir)/../SMDS \
	-I$(srcdir)/../SMESHDS \
	-I$(srcdir)/../Driver \
	-I$(srcdir)/../DriverMED \
	-I$(srcdir)/../SMESH \
	-I$(srcdir)/../SMESHClient \
	-I$(srcdir)/../SMESHUtils \
	-I$(top_builddir)/idl

libSMESHObject_la_LDFLAGS  = \
        ../SMESHClient/libSMESHClient.la \
        ../SMDS/libSMDS.la \
        ../Controls/libSMESHControls.la \
        $(GUI_LDFLAGS) -lSalomeApp -lSalomeObject -lSVTK -lVTKViewer -lqtx -lsuit -lstd \
	-lPlot2d \
	-lSPlot2d \
	$(GEOM_LDFLAGS) -lGEOMObject \
	$(CAS_KERNEL) \
        $(VTK_LIBS) $(QT_MT_LIBS)
