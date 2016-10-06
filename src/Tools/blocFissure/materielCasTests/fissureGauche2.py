# -*- coding: utf-8 -*-

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
Circle_1 = geompy.MakeCircle(O, OX, 500)
Extrusion_1 = geompy.MakePrismVecH2Ways(Circle_1, OX, 500)
Vertex_1 = geompy.MakeVertex(500, 0, 0)
Circle_3 = geompy.MakeCircle(Vertex_1, OZ, 300)
Extrusion_2 = geompy.MakePrismVecH(Circle_3, OZ, 1000)
Partition_1 = geompy.MakePartition([Extrusion_1], [Extrusion_2], [], [], geompy.ShapeType["FACE"], 0, [], 0)
[Face_1,Face_2] = geompy.SubShapes(Partition_1, [18, 13])
FaceFissExt = geompy.MakeFuse(Face_2, Face_1)
geompy.ExportBREP(FaceFissExt, os.path.join(gmu.pathBloc, "materielCasTests/faceGauche2Fiss.brep"))
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
geompy.ExportBREP(FaceFissExtCoupe, os.path.join(gmu.pathBloc, "materielCasTests/faceGauche2FissCoupe.brep"))
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

smesh = smeshBuilder.New(theStudy)
from salome.StdMeshers import StdMeshersBuilder
Mesh_1 = smesh.Mesh(objetSain)
Regular_1D = Mesh_1.Segment()
Nb_Segments_1 = Regular_1D.NumberOfSegments(15,[],[  ])
Nb_Segments_1.SetDistrType( 0 )
Quadrangle_2D = Mesh_1.Quadrangle(algo=smeshBuilder.QUADRANGLE)
Hexa_3D = Mesh_1.Hexahedron(algo=smeshBuilder.Hexa)
isDone = Mesh_1.Compute()
smesh.SetName(Mesh_1, 'Mesh_1')
Mesh_1.ExportMED( os.path.join(gmu.pathBloc, "materielCasTests/boiteSaine.med"), 0, SMESH.MED_V2_2, 1 )

## set object names
smesh.SetName(Mesh_1.GetMesh(), 'Mesh_1')
smesh.SetName(Regular_1D.GetAlgorithm(), 'Regular_1D')
smesh.SetName(Nb_Segments_1, 'Nb. Segments_1')
smesh.SetName(Quadrangle_2D.GetAlgorithm(), 'Quadrangle_2D')
smesh.SetName(Hexa_3D.GetAlgorithm(), 'Hexa_3D')

if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser(True)
