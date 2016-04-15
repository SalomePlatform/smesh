# Copyright (C) 2012-2016  CEA/DEN, EDF R&D, OPEN CASCADE
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
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

##
# Common packages
##
SET(SUBDIRS_COMMON
  SMDS
  SMESHDS
  SMESHUtils
  Controls
  Driver
  MEDWrapper
  DriverMED
  DriverDAT
  DriverUNV
  DriverSTL
  DriverGMF
  SMESH
  SMESH_I
  SMESHClient
  SMESH_SWIG
  StdMeshers
  StdMeshers_I
  SMESH_PY
  Tools
)

IF(SALOME_SMESH_ENABLE_MEFISTO)
  SET(SUBDIRS_MEFISTO2 MEFISTO2)
ENDIF(SALOME_SMESH_ENABLE_MEFISTO)

##
# CGNS
##
IF(SALOME_SMESH_USE_CGNS)
  SET(SUBDIRS_CGNS 
    DriverCGNS
  )
ENDIF(SALOME_SMESH_USE_CGNS)

##
# GUI
##
IF(SALOME_BUILD_GUI)
  SET(SUBDIRS_GUI OBJECT 
                  SMESHFiltersSelection 
                  SMESHGUI 
                  PluginUtils
	          SMESH_SWIG_WITHIHM
	          StdMeshersGUI
  )
ENDIF(SALOME_BUILD_GUI)

SET(SUBDIRS
  ${SUBDIRS_COMMON}
  ${SUBDIRS_MEFISTO2}
  ${SUBDIRS_CGNS}
  ${SUBDIRS_GUI}
)

FOREACH(dir ${SUBDIRS})
 ADD_SUBDIRECTORY(${dir})
ENDFOREACH(dir ${SUBDIRS})
