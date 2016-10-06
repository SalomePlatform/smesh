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
Vertex_1 = geompy.MakeVertex(-2000, 0, 0)
Cylinder_1 = geompy.MakeCylinder(Vertex_1, OY, 1900, 300)
Cylinder_2 = geompy.MakeCylinder(Vertex_1, OY, 2100, 300)
Cut_1 = geompy.MakeCut(Cylinder_2, Cylinder_1)
Vertex_2 = geompy.MakeVertex(-500, -100, -300)
Vertex_3 = geompy.MakeVertex(500, 400, 300)
Box_1 = geompy.MakeBoxTwoPnt(Vertex_3, Vertex_2)
Common_1 = geompy.MakeCommon(Box_1, Cut_1)
Vertex_4 = geompy.MakeVertex(-300, -1000, 0)
Cylinder_3 = geompy.MakeCylinder(Vertex_4, OX, 1100, 600)
EprouvetteCourbe = geompy.MakeCut(Common_1, Cylinder_3)
[Compound_y, Compound_z, Compound_x] = geompy.Propagate(EprouvetteCourbe)
geomObj_1 = geompy.MakeMarker(0, 0, 0, 1, 0, 0, 0, 1, 0)
Sketch_1 = geompy.MakeSketcherOnPlane("Sketcher:F -110.000000 85.000000:T 220.000000 0.000000:T 0.000000 75.000000:T -220.000000 0.000000:WW", geomObj_1 )
SectionDroite = geompy.MakeFaceWires([Sketch_1], 1)
geompy.ExportBREP(SectionDroite, os.path.join(gmu.pathBloc, "materielCasTests/EprouvetteCourbeFiss.brep"))
geompy.addToStudy( O, 'O' )
geompy.addToStudy( OX, 'OX' )
geompy.addToStudy( OY, 'OY' )
geompy.addToStudy( OZ, 'OZ' )
geompy.addToStudy( Vertex_1, 'Vertex_1' )
geompy.addToStudy( Cylinder_1, 'Cylinder_1' )
geompy.addToStudy( Cylinder_2, 'Cylinder_2' )
geompy.addToStudy( Cut_1, 'Cut_1' )
geompy.addToStudy( Vertex_2, 'Vertex_2' )
geompy.addToStudy( Vertex_3, 'Vertex_3' )
geompy.addToStudy( Box_1, 'Box_1' )
geompy.addToStudy( Common_1, 'Common_1' )
geompy.addToStudy( Cylinder_3, 'Cylinder_3' )
geompy.addToStudy( Vertex_4, 'Vertex_4' )
geompy.addToStudy( EprouvetteCourbe, 'EprouvetteCourbe' )
geompy.addToStudy( Sketch_1, 'Sketch_1' )
geompy.addToStudy( SectionDroite, 'SectionDroite' )
geompy.addToStudyInFather( EprouvetteCourbe, Compound_y, 'Compound_y' )
geompy.addToStudyInFather( EprouvetteCourbe, Compound_z, 'Compound_z' )
geompy.addToStudyInFather( EprouvetteCourbe, Compound_x, 'Compound_x' )

###
### SMESH component
###

import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New(theStudy)
from salome.StdMeshers import StdMeshersBuilder
EprouvetteCourbe_1 = smesh.Mesh(EprouvetteCourbe)
Regular_1D = EprouvetteCourbe_1.Segment()
Nb_Segments_1 = Regular_1D.NumberOfSegments(50)
Nb_Segments_1.SetDistrType( 0 )
Quadrangle_2D = EprouvetteCourbe_1.Quadrangle(algo=smeshBuilder.QUADRANGLE)
Hexa_3D = EprouvetteCourbe_1.Hexahedron(algo=smeshBuilder.Hexa)
Regular_1D_1 = EprouvetteCourbe_1.Segment(geom=Compound_x)
Nb_Segments_2 = Regular_1D_1.NumberOfSegments(15)
Nb_Segments_2.SetDistrType( 0 )
Regular_1D_2 = EprouvetteCourbe_1.Segment(geom=Compound_y)
Nb_Segments_3 = Regular_1D_2.NumberOfSegments(25)
Nb_Segments_3.SetDistrType( 0 )
isDone = EprouvetteCourbe_1.Compute()
smesh.SetName(EprouvetteCourbe_1, 'EprouvetteCourbe')
EprouvetteCourbe_1.ExportMED( os.path.join(gmu.pathBloc, "materielCasTests/EprouvetteCourbe.med"), 0, SMESH.MED_V2_2, 1 )
SubMesh_1 = Regular_1D_1.GetSubMesh()
SubMesh_2 = Regular_1D_2.GetSubMesh()

## set object names
smesh.SetName(EprouvetteCourbe_1.GetMesh(), 'EprouvetteCourbe')
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
