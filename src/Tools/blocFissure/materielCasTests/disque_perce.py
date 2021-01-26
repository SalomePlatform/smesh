# -*- coding: iso-8859-1 -*-
# Copyright (C) 2014-2020  EDF R&D
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
Cylinder_1 = geompy.MakeCylinderRH(100, 300)
Cylinder_2 = geompy.MakeCylinderRH(600, 200)
Cut_1 = geompy.MakeCut(Cylinder_2, Cylinder_1)
Face_1 = geompy.MakeFaceHW(500, 1500, 3)
Disque = geompy.MakePartition([Cut_1], [Face_1], [], [], geompy.ShapeType["SOLID"], 0, [], 0)
[Compound_1, Compound_2, Compound_3, Compound_4] = geompy.Propagate(Disque)
geompy.addToStudy( O, 'O' )
geompy.addToStudy( OX, 'OX' )
geompy.addToStudy( OY, 'OY' )
geompy.addToStudy( OZ, 'OZ' )
geompy.addToStudy( Cylinder_1, 'Cylinder_1' )
geompy.addToStudy( Cylinder_2, 'Cylinder_2' )
geompy.addToStudy( Cut_1, 'Cut_1' )
geompy.addToStudy( Face_1, 'Face_1' )
geompy.addToStudy( Disque, 'Disque' )
geompy.addToStudyInFather( Disque, Compound_1, 'Compound_1' )
geompy.addToStudyInFather( Disque, Compound_2, 'Compound_2' )
geompy.addToStudyInFather( Disque, Compound_3, 'Compound_3' )
geompy.addToStudyInFather( Disque, Compound_4, 'Compound_4' )

###
### SMESH component
###

import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New()
from salome.StdMeshers import StdMeshersBuilder
Disque_1 = smesh.Mesh(Disque)
smesh.SetName(Disque_1, 'Disque')
Regular_1D = Disque_1.Segment()
Nb_Segments_1 = Regular_1D.NumberOfSegments(10)
Nb_Segments_1.SetDistrType( 0 )
Hexa_3D = Disque_1.Hexahedron(algo=smeshBuilder.Hexa)
Regular_1D_1 = Disque_1.Segment(geom=Compound_3)
Nb_Segments_2 = Regular_1D_1.NumberOfSegments(20)
Nb_Segments_2.SetDistrType( 0 )
Regular_1D_2 = Disque_1.Segment(geom=Compound_4)
status = Disque_1.AddHypothesis(Nb_Segments_2,Compound_4)
Quadrangle_2D = Disque_1.Quadrangle(algo=smeshBuilder.QUADRANGLE)
is_done = Disque_1.Compute()
text = "Disque_1.Compute"
if is_done:
  logging.info(text+" OK")
else:
  text = "Erreur au calcul du maillage.\n" + text
  logging.info(text)
  raise Exception(text)
Disque_1.ExportMED(os.path.join(gmu.pathBloc, "materielCasTests", "disque.med"))
SubMesh_1 = Regular_1D_1.GetSubMesh()
SubMesh_2 = Regular_1D_2.GetSubMesh()

## set object names
smesh.SetName(Disque_1.GetMesh(), 'Disque')
smesh.SetName(Regular_1D.GetAlgorithm(), 'Regular_1D')
smesh.SetName(Nb_Segments_1, 'Nb. Segments_1')
smesh.SetName(Hexa_3D.GetAlgorithm(), 'Hexa_3D')
smesh.SetName(Nb_Segments_2, 'Nb. Segments_2')
smesh.SetName(Quadrangle_2D.GetAlgorithm(), 'Quadrangle_2D')
smesh.SetName(SubMesh_1, 'SubMesh_1')
smesh.SetName(SubMesh_2, 'SubMesh_2')

if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()
