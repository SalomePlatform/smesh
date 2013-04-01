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
#  Author : Patrick GOLDBRONN (CEA)
#  Modified by : Alexander BORODIN (OCN) - autotools usage
#  Module : SMESH
#
include $(top_srcdir)/adm_local/unix/make_common_starter.am

if WITH_CGNS
  DriverCGNS_SUDIR = DriverCGNS
endif

SUBDIRS = \
	SMDS \
	SMESHDS \
	SMESHUtils \
	Controls \
	Driver \
	DriverMED \
	DriverDAT \
	DriverUNV \
	DriverSTL \
	DriverGMF \
	$(DriverCGNS_SUDIR) \
	SMESH \
	SMESH_I \
	SMESHClient \
	SMESH_SWIG \
	MEFISTO2 \
	StdMeshers \
	StdMeshers_I \
	SMESH_PY \
	Tools

if SMESH_ENABLE_GUI
  SUBDIRS += \
	OBJECT \
	SMESHFiltersSelection \
	SMESHGUI \
	PluginUtils \
	SMESH_SWIG_WITHIHM \
	StdMeshersGUI
endif

DIST_SUBDIRS = 	SMDS SMESHDS Controls Driver DriverMED DriverDAT DriverUNV DriverSTL DriverCGNS	\
		DriverGMF \
		SMESHUtils SMESH SMESH_I SMESHClient SMESH_SWIG MEFISTO2 StdMeshers StdMeshers_I OBJECT 	\
		SMESHFiltersSelection SMESHGUI PluginUtils SMESH_SWIG_WITHIHM StdMeshersGUI SMESH_PY Tools
