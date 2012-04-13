#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
#
# Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
# CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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

#  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
#  File   : smeshpy.py
#  Module : SMESH
#
import salome
import SMESH

from SALOME_utilities import *

#=============================================================================

class smeshpy:
    _geom = None
    _smesh = None
    _studyId = None

    #--------------------------------------------------------------------------

    def __init__(self):
        try:
            self._geom = salome.lcc.FindOrLoadComponent("FactoryServer","GEOM")
            self._smesh = salome.lcc.FindOrLoadComponent("FactoryServer","SMESH")
        except:
            MESSAGE( "exception in smeshpy:__init__" )
        self._study = salome.myStudy
        self._smesh.SetCurrentStudy(self._study)

    #--------------------------------------------------------------------------

    def CreateMesh(self, shapeId):
        try:
            shape = salome.IDToObject(shapeId)
            aMesh = self._smesh.CreateMesh(shape)
            return aMesh
        except:
            MESSAGE( "exception in smeshpy:Init" )
            return None

    #--------------------------------------------------------------------------

    def CreateHypothesis(self, name, libname):
        try:
            hyp = self._smesh.CreateHypothesis(name, libname)
            return hyp
        except:
            MESSAGE( "exception in smeshpy:CreateHypothesis" )
            return None    

    #--------------------------------------------------------------------------

    def Compute(self, mesh, shapeId):
        try:
            shape = salome.IDToObject(shapeId)
            ret=self._smesh.Compute(mesh, shape)
            return ret
        except:
            MESSAGE( "exception in smeshpy:Compute" )
            return 0    

#=============================================================================
##    #--------------------------------------------------------------------------

##def SmeshInit(shapeId):
##    import salome
##    import SMESH
##    geom = salome.lcc.FindOrLoadComponent("FactoryServer", "GEOM")
##    smesh = salome.lcc.FindOrLoadComponent("FactoryServer", "SMESH")
##    shape = salome.IDToObject(shapeId)
##    studyId = salome.myStudyId
##    MESSAGE( str(studyId) )
##    aMesh = smesh.Init(geom, studyId, shape)
##    return aMesh

##    #--------------------------------------------------------------------------
