# -*- coding: iso-8859-1 -*-

import sys
import salome

salome.salome_init()
theStudy = salome.myStudy

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


geompy = geomBuilder.New(theStudy)

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

smesh = smeshBuilder.New(theStudy)
from salome.StdMeshers import StdMeshersBuilder
Disque_1 = smesh.Mesh(Disque)
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
isDone = Disque_1.Compute()
smesh.SetName(Disque_1, 'Disque')
Disque_1.ExportMED( os.path.join(gmu.pathBloc, "materielCasTests/disque.med"), 0, SMESH.MED_V2_2, 1 )
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
  salome.sg.updateObjBrowser(True)
