# -*- coding: iso-8859-1 -*-
# Copyright (C) 2014-2021  EDF R&D
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

import sys
import salome

salome.salome_init()

import salome_notebook

###
### GEOM component
###

import GEOM
from salome.geom import geomBuilder
import math
import SALOMEDS

geompy = geomBuilder.New()

O = geompy.MakeVertex(0, 0, 0)
OX = geompy.MakeVectorDXDYDZ(1, 0, 0)
OY = geompy.MakeVectorDXDYDZ(0, 1, 0)
OZ = geompy.MakeVectorDXDYDZ(0, 0, 1)
Box_1 = geompy.MakeBoxDXDYDZ(200, 200, 200)
Vertex_1 = geompy.MakeVertex(0, 0, 100)
Disk_1 = geompy.MakeDiskPntVecR(Vertex_1, OZ, 50)
Rotation_1 = geompy.MakeRotation(Box_1, OZ, 45*math.pi/180.0)
geompy.TranslateDXDYDZ(Rotation_1, -50, -250, 0)
Cut_1 = geompy.MakeCut(Disk_1, Rotation_1)
geompy.Export(Cut_1, "disk.brep", "BREP")
geompy.addToStudy( O, 'O' )
geompy.addToStudy( OX, 'OX' )
geompy.addToStudy( OY, 'OY' )
geompy.addToStudy( OZ, 'OZ' )
geompy.addToStudy( Box_1, 'Box_1' )
geompy.addToStudy( Vertex_1, 'Vertex_1' )
geompy.addToStudy( Disk_1, 'Disk_1' )
geompy.addToStudy( Rotation_1, 'Rotation_1' )
geompy.addToStudy( Cut_1, 'Cut_1' )

###
### SMESH component
###

import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New()
from salome.StdMeshers import StdMeshersBuilder
boite = smesh.Mesh(Box_1)
Regular_1D = boite.Segment()
Nb_Segments_1 = Regular_1D.NumberOfSegments(15)
Nb_Segments_1.SetDistrType( 0 )
Quadrangle_2D = boite.Quadrangle(algo=smeshBuilder.QUADRANGLE)
Hexa_3D = boite.Hexahedron(algo=smeshBuilder.Hexa)
isDone = boite.Compute()
smesh.SetName(boite, 'boite')
boite.ExportMED(r'boite.med')

## set object names
smesh.SetName(boite.GetMesh(), 'boite')
smesh.SetName(Regular_1D.GetAlgorithm(), 'Regular_1D')
smesh.SetName(Nb_Segments_1, 'Nb. Segments_1')
smesh.SetName(Quadrangle_2D.GetAlgorithm(), 'Quadrangle_2D')
smesh.SetName(Hexa_3D.GetAlgorithm(), 'Hexa_3D')

import os
from blocFissure import gmu
from blocFissure.gmu import initLog
#initLog.setDebug()
initLog.setVerbose()

from blocFissure.gmu.casStandard import casStandard

dicoParams = dict(nomCas            = 'angleCube2',
                  maillageSain      = 'boite.med',
                  CAOFaceFissure    = "disk.brep",
                  edgeFiss          = [4],
                  lgInfluence       = 50,
                  meshBrep          = (5,10),
                  rayonPipe         = 15,
                  lenSegPipe        = 20,
                  nbSegRad          = 5,
                  nbSegCercle       = 12,
                  areteFaceFissure  = 2)

execInstance = casStandard(dicoParams)

if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()
