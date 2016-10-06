# -*- coding: utf-8 -*-

import sys
import salome

salome.salome_init()
theStudy = salome.myStudy

import salome_notebook
notebook = salome_notebook.NoteBook(theStudy)

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
eprouvetteDroite = geompy.MakeBoxDXDYDZ(100, 200, 500)
[Compound_z, Compound_y, Compound_x] = geompy.Propagate(eprouvetteDroite)

Vertex_1 = geompy.MakeVertex(-10, -10, 200)
geomObj_1 = geompy.MakeMarker(0, 0, 0, 1, 0, 0, 0, 1, 0)
Plane_1 = geompy.MakePlane(Vertex_1, OZ, 2000)
geomObj_2 = geompy.MakeMarker(0, 0, 0, 1, 0, 0, 0, 1, 0)
Sketch_1 = geompy.MakeSketcherOnPlane("Sketcher:F 0.000000 0.000000:TT 120.000000 0.000000:T 0.000000 100.000000:T -120.000000 0.000000:WW", Plane_1 )
sectionDroite = geompy.MakeFaceWires([Sketch_1], 1)
Box_1_vertex_7 = geompy.GetSubShape(eprouvetteDroite, [7])
Box_1_vertex_16 = geompy.GetSubShape(eprouvetteDroite, [16])
Line_1 = geompy.MakeLineTwoPnt(Box_1_vertex_7, Box_1_vertex_16)
geomObj_3 = geompy.MakeMarker(0, 0, 0, 1, 0, 0, 0, 1, 0)
Plane_2 = geompy.MakePlane(Vertex_1, Line_1, 2000)
geomObj_4 = geompy.MakeMarker(0, 0, 0, 1, 0, 0, 0, 1, 0)
Sketch_2 = geompy.MakeSketcherOnPlane("Sketcher:F 0.000000 0.000000:T 120.000000 0.000000:T 0.000000 100.000000:T -120.000000 20.000000:WW", Plane_2 )
SectionInclinee = geompy.MakeFaceWires([Sketch_2], 1)
geompy.ExportBREP(sectionDroite, os.path.join(gmu.pathBloc, "materielCasTests/EprouvetteDroiteFissPb1.brep"))
geompy.ExportBREP(SectionInclinee, os.path.join(gmu.pathBloc, "materielCasTests/EprouvetteDroiteFiss2.brep"))
Vertex_2 = geompy.MakeVertex(110, -10, 200)
Vertex_3 = geompy.MakeVertex(110, 80, 200)
Vertex_4 = geompy.MakeVertex(-10, 80, 200)
Line_2 = geompy.MakeLineTwoPnt(Vertex_1, Vertex_2)
Line_3 = geompy.MakeLineTwoPnt(Vertex_2, Vertex_3)
Line_4 = geompy.MakeLineTwoPnt(Vertex_3, Vertex_4)
Line_5 = geompy.MakeLineTwoPnt(Vertex_4, Vertex_1)
Face_1 = geompy.MakeFaceWires([Line_2, Line_3, Line_4, Line_5], 1)
Vertex_5 = geompy.MakeVertex(110, -10, 180)
Vertex_6 = geompy.MakeVertex(110, 70, 180)
Face_1_vertex_4 = geompy.GetSubShape(Face_1, [4])
Line_6 = geompy.MakeLineTwoPnt(Face_1_vertex_4, Vertex_5)
Line_7 = geompy.MakeLineTwoPnt(Vertex_5, Vertex_6)
Face_1_vertex_9 = geompy.GetSubShape(Face_1, [9])
Line_8 = geompy.MakeLineTwoPnt(Vertex_6, Face_1_vertex_9)
Face_2 = geompy.MakeFaceWires([Line_5, Line_6, Line_7, Line_8], 1)
geompy.ExportBREP(Face_1, os.path.join(gmu.pathBloc, "materielCasTests/EprouvetteDroiteFiss_1.brep"))
geompy.ExportBREP(Face_2, os.path.join(gmu.pathBloc, "materielCasTests/EprouvetteDroiteFiss_2.brep"))
geompy.addToStudy( O, 'O' )
geompy.addToStudy( OX, 'OX' )
geompy.addToStudy( OY, 'OY' )
geompy.addToStudy( OZ, 'OZ' )
geompy.addToStudy( eprouvetteDroite, 'eprouvetteDroite' )
geompy.addToStudyInFather( eprouvetteDroite, Compound_z, 'Compound_z' )
geompy.addToStudyInFather( eprouvetteDroite, Compound_y, 'Compound_y' )
geompy.addToStudyInFather( eprouvetteDroite, Compound_x, 'Compound_x' )
geompy.addToStudy( Vertex_1, 'Vertex_1' )
geompy.addToStudy( Plane_1, 'Plane_1' )
geompy.addToStudy( Sketch_1, 'Sketch_1' )
geompy.addToStudy( sectionDroite, 'sectionDroite' )
geompy.addToStudyInFather( eprouvetteDroite, Box_1_vertex_7, 'Box_1:vertex_7' )
geompy.addToStudyInFather( eprouvetteDroite, Box_1_vertex_16, 'Box_1:vertex_16' )
geompy.addToStudy( Line_1, 'Line_1' )
geompy.addToStudy( Plane_2, 'Plane_2' )
geompy.addToStudy( Sketch_2, 'Sketch_2' )
geompy.addToStudy( SectionInclinee, 'SectionInclinee' )
geompy.addToStudy( Vertex_2, 'Vertex_2' )
geompy.addToStudy( Vertex_3, 'Vertex_3' )
geompy.addToStudy( Vertex_4, 'Vertex_4' )
geompy.addToStudy( Line_2, 'Line_2' )
geompy.addToStudy( Line_3, 'Line_3' )
geompy.addToStudy( Line_4, 'Line_4' )
geompy.addToStudy( Line_5, 'Line_5' )
geompy.addToStudy( Face_1, 'Face_1' )
geompy.addToStudy( Vertex_5, 'Vertex_5' )
geompy.addToStudy( Vertex_6, 'Vertex_6' )
geompy.addToStudyInFather( Face_1, Face_1_vertex_4, 'Face_1:vertex_4' )
geompy.addToStudy( Line_6, 'Line_6' )
geompy.addToStudy( Line_7, 'Line_7' )
geompy.addToStudyInFather( Face_1, Face_1_vertex_9, 'Face_1:vertex_9' )
geompy.addToStudy( Line_8, 'Line_8' )
geompy.addToStudy( Face_2, 'Face_2' )

###
### SMESH component
###

import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New(theStudy)
from salome.StdMeshers import StdMeshersBuilder
eprouvetteDroite_1 = smesh.Mesh(eprouvetteDroite)
Regular_1D = eprouvetteDroite_1.Segment()
Nb_Segments_1 = Regular_1D.NumberOfSegments(50,[],[  ])
Nb_Segments_1.SetDistrType( 0 )
Quadrangle_2D = eprouvetteDroite_1.Quadrangle(algo=smeshBuilder.QUADRANGLE)
Hexa_3D = eprouvetteDroite_1.Hexahedron(algo=smeshBuilder.Hexa)
Regular_1D_1 = eprouvetteDroite_1.Segment(geom=Compound_y)
Nb_Segments_2 = Regular_1D_1.NumberOfSegments(20,[],[  ])
Nb_Segments_2.SetDistrType( 0 )
Regular_1D_2 = eprouvetteDroite_1.Segment(geom=Compound_x)
Nb_Segments_3 = Regular_1D_2.NumberOfSegments(10,[],[  ])
Nb_Segments_3.SetDistrType( 0 )
isDone = eprouvetteDroite_1.Compute()
smesh.SetName(eprouvetteDroite_1, 'eprouvetteDroite')
eprouvetteDroite_1.ExportMED( os.path.join(gmu.pathBloc, "materielCasTests/eprouvetteDroite.med"), 0, SMESH.MED_V2_2, 1 )
SubMesh_1 = Regular_1D_1.GetSubMesh()
SubMesh_2 = Regular_1D_2.GetSubMesh()

## set object names
smesh.SetName(eprouvetteDroite_1.GetMesh(), 'eprouvetteDroite')
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
