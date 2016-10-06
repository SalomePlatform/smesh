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
geomObj_1 = geompy.MakeCylinderRH(1000, 3000)
Cylinder_1 = geompy.MakeRotation(geomObj_1, OZ, 180*math.pi/180.0)
geomObj_2 = geompy.MakeCylinder(O, OX, 2000, 5000)
Cylinder_2 = geompy.MakeRotation(geomObj_2, OX, 180*math.pi/180.0)
Translation_1 = geompy.MakeTranslation(Cylinder_2, -2000, 0, 0)
Fuse_1 = geompy.MakeFuse(Cylinder_1, Translation_1)
Fillet_1 = geompy.MakeFillet(Fuse_1, 800, geompy.ShapeType["EDGE"], [11])
Vertex_1 = geompy.MakeVertex(0, -3000, -3000)
Vertex_2 = geompy.MakeVertex(2500, 3000, 3000)
Box_1 = geompy.MakeBoxTwoPnt(Vertex_2, Vertex_1)
Partition_1 = geompy.MakePartition([Box_1], [Fillet_1], [], [], geompy.ShapeType["SOLID"], 0, [], 0)
geomObj_3 = geompy.MakeCylinderRH(1450, 8000)
Cylinder_3 = geompy.MakeRotation(geomObj_3, OZ, 180*math.pi/180.0)
Cut_1 = geompy.MakeCut(Partition_1, Cylinder_3)
geompy.addToStudy( Cut_1, 'Cut_1' )
[faceFiss1] = geompy.SubShapes(Cut_1, [61])
geompy.addToStudyInFather( Cut_1, faceFiss1, 'faceFiss1' )
[Vertex_3,geomObj_4] = geompy.SubShapes(faceFiss1, [4, 5])
Cylinder_4 = geompy.MakeCylinderRH(2000, 4000)
Cylinder_5 = geompy.MakeCylinderRH(1500, 4000)
Cut_2 = geompy.MakeCut(Cylinder_4, Cylinder_5)
Plane_1 = geompy.MakePlaneLCS(None, 10000, 3)
Vertex_5 = geompy.MakeVertex(0, 0, 100)
Plane_2 = geompy.MakePlaneThreePnt(O, Vertex_5, Vertex_3, 10000)
Plane_3 = geompy.MakePlaneThreePnt(O, Vertex_5, geomObj_4, 10000)
Vertex_6 = geompy.MakeVertex(0, -5000, -5000)
Vertex_7 = geompy.MakeVertex(5000, 5000, 5000)
Box_2 = geompy.MakeBoxTwoPnt(Vertex_7, Vertex_6)
Common_1 = geompy.MakeCommon(Box_2, Cut_2)
objetSain = geompy.MakePartition([Common_1], [Plane_1, Plane_2, Plane_3], [], [], geompy.ShapeType["SOLID"], 0, [], 0)
[hauteurs, epaisseurs, Compound_3, Compound_4, Compound_5, Compound_6] = geompy.Propagate(objetSain)
geompy.ExportBREP(faceFiss1, os.path.join(gmu.pathBloc, "materielCasTests/faceGaucheFiss.brep"))
geompy.addToStudy( O, 'O' )
geompy.addToStudy( OX, 'OX' )
geompy.addToStudy( OY, 'OY' )
geompy.addToStudy( OZ, 'OZ' )
geompy.addToStudy( Cylinder_1, 'Cylinder_1' )
geompy.addToStudy( Cylinder_2, 'Cylinder_2' )
geompy.addToStudy( Translation_1, 'Translation_1' )
geompy.addToStudy( Fuse_1, 'Fuse_1' )
geompy.addToStudy( Fillet_1, 'Fillet_1' )
geompy.addToStudy( Vertex_1, 'Vertex_1' )
geompy.addToStudy( Vertex_2, 'Vertex_2' )
geompy.addToStudy( Box_1, 'Box_1' )
geompy.addToStudy( Partition_1, 'Partition_1' )
geompy.addToStudy( Cylinder_3, 'Cylinder_3' )
geompy.addToStudyInFather( faceFiss1, Vertex_3, 'Vertex_3' )
geompy.addToStudy( Cylinder_4, 'Cylinder_4' )
geompy.addToStudy( Cylinder_5, 'Cylinder_5' )
geompy.addToStudy( Cut_2, 'Cut_2' )
geompy.addToStudy( Plane_1, 'Plane_1' )
geompy.addToStudy( Vertex_5, 'Vertex_5' )
geompy.addToStudy( Plane_2, 'Plane_2' )
geompy.addToStudy( Plane_3, 'Plane_3' )
geompy.addToStudy( Vertex_6, 'Vertex_6' )
geompy.addToStudy( Vertex_7, 'Vertex_7' )
geompy.addToStudy( Box_2, 'Box_2' )
geompy.addToStudy( Common_1, 'Common_1' )
geompy.addToStudy( objetSain, 'objetSain' )
geompy.addToStudyInFather( objetSain, hauteurs, 'hauteurs' )
geompy.addToStudyInFather( objetSain, epaisseurs, 'epaisseurs' )
geompy.addToStudyInFather( objetSain, Compound_3, 'Compound_3' )
geompy.addToStudyInFather( objetSain, Compound_4, 'Compound_4' )
geompy.addToStudyInFather( objetSain, Compound_5, 'Compound_5' )
geompy.addToStudyInFather( objetSain, Compound_6, 'Compound_6' )

###
### SMESH component
###

import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New(theStudy)
from salome.StdMeshers import StdMeshersBuilder
objetSain_1 = smesh.Mesh(objetSain)
Regular_1D = objetSain_1.Segment()
Nb_Segments_1 = Regular_1D.NumberOfSegments(10,[],[  ])
Nb_Segments_1.SetDistrType( 0 )
Quadrangle_2D = objetSain_1.Quadrangle(algo=smeshBuilder.QUADRANGLE)
Hexa_3D = objetSain_1.Hexahedron(algo=smeshBuilder.Hexa)
Regular_1D_1 = objetSain_1.Segment(geom=hauteurs)
Nb_Segments_2 = Regular_1D_1.NumberOfSegments(15,[],[  ])
Nb_Segments_2.SetDistrType( 0 )
Regular_1D_2 = objetSain_1.Segment(geom=epaisseurs)
Nb_Segments_3 = Regular_1D_2.NumberOfSegments(5,[],[  ])
Nb_Segments_3.SetDistrType( 0 )
isDone = objetSain_1.Compute()
smesh.SetName(objetSain_1, 'objetSain')
objetSain_1.ExportMED( os.path.join(gmu.pathBloc, "materielCasTests/faceGaucheSain.med"), 0, SMESH.MED_V2_2, 1 )
SubMesh_1 = Regular_1D_1.GetSubMesh()
SubMesh_2 = Regular_1D_2.GetSubMesh()

## set object names
smesh.SetName(objetSain_1.GetMesh(), 'objetSain')
smesh.SetName(Regular_1D.GetAlgorithm(), 'Regular_1D')
smesh.SetName(Nb_Segments_1, 'Nb. Segments_1')
smesh.SetName(Quadrangle_2D.GetAlgorithm(), 'Quadrangle_2D')
smesh.SetName(Hexa_3D.GetAlgorithm(), 'Hexa_3D')
smesh.SetName(Nb_Segments_2, 'Nb. Segments_2')
smesh.SetName(Nb_Segments_3, 'Nb. Segments_3')
smesh.SetName(SubMesh_1, 'SubMesh_1')
smesh.SetName(SubMesh_2, 'SubMesh_2')

if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser(True)
