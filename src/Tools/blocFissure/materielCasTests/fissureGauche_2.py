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
"""Géométrie et maillage de base nécessaire au cas-test :
. faceGauche_2
"""

import os
import math

import logging

import salome
from salome.smesh import smeshBuilder
from salome.StdMeshers import StdMeshersBuilder
import GEOM
import SMESH
import SALOMEDS

from blocFissure import gmu
from blocFissure.gmu.geomsmesh import geompy
from blocFissure.gmu.geomsmesh import geomPublish
from blocFissure.gmu.geomsmesh import geomPublishInFather

from blocFissure.gmu.triedreBase import triedreBase
from blocFissure.gmu.putName import putName
from blocFissure.gmu import initLog

###
### GEOM component
###

O, OX, OY, OZ = triedreBase()

Circle_1 = geompy.MakeCircle(O, OX, 500.)
Extrusion_1 = geompy.MakePrismVecH2Ways(Circle_1, OX, 500)
Vertex_1 = geompy.MakeVertex(500., 0., 0.)
Circle_3 = geompy.MakeCircle(Vertex_1, OZ, 300)
Extrusion_2 = geompy.MakePrismVecH(Circle_3, OZ, 1000)
Partition_1 = geompy.MakePartition([Extrusion_1], [Extrusion_2], [], [], geompy.ShapeType["FACE"], 0, [], 0)
[Face_1,Face_2] = geompy.SubShapes(Partition_1, [18, 13])

FaceFissExt = geompy.MakeFuse(Face_2, Face_1)
geompy.addToStudy( FaceFissExt, 'FaceFissExt' )
geompy.ExportBREP(FaceFissExt, os.path.join(gmu.pathBloc, "materielCasTests", "faceGauche2Fiss.brep"))

Vertex_2 = geompy.MakeVertex(0, -500, 0)
Vertex_3 = geompy.MakeVertex(400, 500, 800)
objetSain = geompy.MakeBoxTwoPnt(Vertex_3, Vertex_2)
geompy.addToStudy( objetSain, 'objetSain' )

Rotation_1 = geompy.MakeRotation(Extrusion_1, OX, math.pi)
Partition_2 = geompy.MakePartition([Rotation_1], [Extrusion_2], [], [], geompy.ShapeType["FACE"], 0, [], 0)
geompy.addToStudy( Partition_2, 'Partition_2' )
[FaceFissExtSimple] = geompy.SubShapes(Partition_2, [13])
geompy.addToStudyInFather( Partition_2, FaceFissExtSimple, 'FaceFissExtSimple' )
Plane_1 = geompy.MakePlaneLCS(None, 2000., 3)

FaceFissExtCoupe = geompy.MakePartition([FaceFissExtSimple], [Plane_1], [], [], geompy.ShapeType["FACE"], 0, [], 0)
geompy.addToStudy( FaceFissExtCoupe, 'FaceFissExtCoupe' )

geompy.ExportBREP(FaceFissExtCoupe, os.path.join(gmu.pathBloc, "materielCasTests", "faceGauche2FissCoupe.brep"))

geomPublish(initLog.debug, Circle_1, 'Circle_1' )
geomPublish(initLog.debug, Extrusion_1, 'Extrusion_1' )
geomPublish(initLog.debug, Vertex_1, 'Vertex_1' )
geomPublish(initLog.debug, Circle_3, 'Circle_3' )
geomPublish(initLog.debug, Extrusion_2, 'Extrusion_2' )
geomPublish(initLog.debug, Partition_1, 'Partition_1' )
geomPublishInFather(initLog.debug, Partition_1, Face_1, 'Face_1' )
geomPublishInFather(initLog.debug, Partition_1, Face_2, 'Face_2' )
geomPublish(initLog.debug, Vertex_2, 'Vertex_2' )
geomPublish(initLog.debug, Vertex_3, 'Vertex_3' )
geomPublish(initLog.debug, Rotation_1, 'Rotation_1' )
geomPublish(initLog.debug, Plane_1, 'Plane_1' )

###
### SMESH component
###
smesh = smeshBuilder.New()
Mesh_1 = smesh.Mesh(objetSain)
putName(Mesh_1.GetMesh(), 'Mesh_1')

Regular_1D = Mesh_1.Segment()
Nb_Segments_1 = Regular_1D.NumberOfSegments(15,[],[  ])
Nb_Segments_1.SetDistrType( 0 )
Quadrangle_2D = Mesh_1.Quadrangle(algo=smeshBuilder.QUADRANGLE)
Hexa_3D = Mesh_1.Hexahedron(algo=smeshBuilder.Hexa)

## set object names
#smesh.SetName(Regular_1D.GetAlgorithm(), 'Regular_1D')
#smesh.SetName(Quadrangle_2D.GetAlgorithm(), 'Quadrangle_2D')
#smesh.SetName(Hexa_3D.GetAlgorithm(), 'Hexa_3D')
putName(Nb_Segments_1, 'Nb. Segments_1', i_pref='fissuregauche2')

is_done = Mesh_1.Compute()
text = "Mesh_1.Compute"
if is_done:
  logging.info(text+" OK")
else:
  text = "Erreur au calcul du maillage.\n" + text
  logging.info(text)
  raise Exception(text)

Mesh_1.ExportMED(os.path.join(gmu.pathBloc, "materielCasTests", "boiteSaine.med"))

if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()
