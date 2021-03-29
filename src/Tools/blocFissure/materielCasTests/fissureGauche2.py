# -*- coding: utf-8 -*-
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

import logging

import sys
import salome

salome.salome_init()

import salome_notebook
notebook = salome_notebook.notebook

import os
from blocFissure import gmu

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
Circle_1 = geompy.MakeCircle(O, OX, 500)
Extrusion_1 = geompy.MakePrismVecH2Ways(Circle_1, OX, 500)
Vertex_1 = geompy.MakeVertex(500, 0, 0)
Circle_3 = geompy.MakeCircle(Vertex_1, OZ, 300)
Extrusion_2 = geompy.MakePrismVecH(Circle_3, OZ, 1000)
Partition_1 = geompy.MakePartition([Extrusion_1], [Extrusion_2], [], [], geompy.ShapeType["FACE"], 0, [], 0)
[Face_1,Face_2] = geompy.SubShapes(Partition_1, [18, 13])
FaceFissExt = geompy.MakeFuse(Face_2, Face_1)
geompy.ExportBREP(FaceFissExt, os.path.join(gmu.pathBloc, "materielCasTests", "faceGauche2Fiss.brep"))
Vertex_2 = geompy.MakeVertex(0, -500, 0)
Vertex_3 = geompy.MakeVertex(400, 500, 800)
objetSain = geompy.MakeBoxTwoPnt(Vertex_3, Vertex_2)
Rotation_1 = geompy.MakeRotation(Extrusion_1, OX, 180*math.pi/180.0)
Partition_2 = geompy.MakePartition([Rotation_1], [Extrusion_2], [], [], geompy.ShapeType["FACE"], 0, [], 0)
geompy.addToStudy( Partition_2, 'Partition_2' )
[FaceFissExtSimple] = geompy.SubShapes(Partition_2, [13])
geompy.addToStudyInFather( Partition_2, FaceFissExtSimple, 'FaceFissExtSimple' )
Plane_1 = geompy.MakePlaneLCS(None, 2000, 3)
FaceFissExtCoupe = geompy.MakePartition([FaceFissExtSimple], [Plane_1], [], [], geompy.ShapeType["FACE"], 0, [], 0)
geompy.ExportBREP(FaceFissExtCoupe, os.path.join(gmu.pathBloc, "materielCasTests", "faceGauche2FissCoupe.brep"))
geompy.addToStudy( O, 'O' )
geompy.addToStudy( OX, 'OX' )
geompy.addToStudy( OY, 'OY' )
geompy.addToStudy( OZ, 'OZ' )
geompy.addToStudy( Circle_1, 'Circle_1' )
geompy.addToStudy( Extrusion_1, 'Extrusion_1' )
geompy.addToStudy( Vertex_1, 'Vertex_1' )
geompy.addToStudy( Circle_3, 'Circle_3' )
geompy.addToStudy( Extrusion_2, 'Extrusion_2' )
geompy.addToStudy( Partition_1, 'Partition_1' )
geompy.addToStudyInFather( Partition_1, Face_1, 'Face_1' )
geompy.addToStudyInFather( Partition_1, Face_2, 'Face_2' )
geompy.addToStudy( FaceFissExt, 'FaceFissExt' )
geompy.addToStudy( Vertex_2, 'Vertex_2' )
geompy.addToStudy( Vertex_3, 'Vertex_3' )
geompy.addToStudy( objetSain, 'objetSain' )
geompy.addToStudy( Rotation_1, 'Rotation_1' )
geompy.addToStudy( Plane_1, 'Plane_1' )
geompy.addToStudy( FaceFissExtCoupe, 'FaceFissExtCoupe' )

###
### SMESH component
###

import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New()
from salome.StdMeshers import StdMeshersBuilder
Mesh_1 = smesh.Mesh(objetSain)
smesh.SetName(Mesh_1, 'Mesh_1')
Regular_1D = Mesh_1.Segment()
Nb_Segments_1 = Regular_1D.NumberOfSegments(15,[],[  ])
Nb_Segments_1.SetDistrType( 0 )
Quadrangle_2D = Mesh_1.Quadrangle(algo=smeshBuilder.QUADRANGLE)
Hexa_3D = Mesh_1.Hexahedron(algo=smeshBuilder.Hexa)

is_done = Mesh_1.Compute()
text = "Mesh_1.Compute"
if is_done:
  logging.info(text+" OK")
else:
  text = "Erreur au calcul du maillage.\n" + text
  logging.info(text)
  raise Exception(text)

Mesh_1.ExportMED(os.path.join(gmu.pathBloc, "materielCasTests", "boiteSaine.med"))

## set object names
smesh.SetName(Mesh_1.GetMesh(), 'Mesh_1')
smesh.SetName(Regular_1D.GetAlgorithm(), 'Regular_1D')
smesh.SetName(Nb_Segments_1, 'Nb. Segments_1')
smesh.SetName(Quadrangle_2D.GetAlgorithm(), 'Quadrangle_2D')
smesh.SetName(Hexa_3D.GetAlgorithm(), 'Hexa_3D')

if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()
