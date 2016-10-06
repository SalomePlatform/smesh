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
O_1 = geompy.MakeVertex(0, 0, 0)
OX_1 = geompy.MakeVectorDXDYDZ(1, 0, 0)
OY_1 = geompy.MakeVectorDXDYDZ(0, 1, 0)
OZ_1 = geompy.MakeVectorDXDYDZ(0, 0, 1)
Vertex_1 = geompy.MakeVertex(0, 0, 500)
Vertex_2 = geompy.MakeVertex(100, 0, 500)
Vertex_3 = geompy.MakeVertex(110, 0, 500)
Vertex_4 = geompy.MakeVertex(117.071068, -2.928932, 500)
Vertex_5 = geompy.MakeVertex(120, -10, 500)
Vertex_6 = geompy.MakeVertex(120, -30, 500)
Vertex_7 = geompy.MakeVertex(122.928932, -37.071068, 500)
Vertex_8 = geompy.MakeVertex(130, -40, 500)
Vertex_9 = geompy.MakeVertex(135, -40, 500)
Vertex_10 = geompy.MakeVertex(160, -40, 500)
Plane_1 = geompy.MakePlaneLCS(None, 2000, 2)
Mirror_1_1 = geompy.MakeMirrorByPlane(Vertex_2, Plane_1)
Mirror_1_2 = geompy.MakeMirrorByPlane(Vertex_3, Plane_1)
Mirror_1_3 = geompy.MakeMirrorByPlane(Vertex_4, Plane_1)
Mirror_1_4 = geompy.MakeMirrorByPlane(Vertex_5, Plane_1)
Mirror_1_5 = geompy.MakeMirrorByPlane(Vertex_6, Plane_1)
Mirror_1_6 = geompy.MakeMirrorByPlane(Vertex_7, Plane_1)
Mirror_1_7 = geompy.MakeMirrorByPlane(Vertex_8, Plane_1)
Mirror_1_8 = geompy.MakeMirrorByPlane(Vertex_9, Plane_1)
Mirror_1_9 = geompy.MakeMirrorByPlane(Vertex_10, Plane_1)
Curve_2 = geompy.MakeInterpol([Mirror_1_9, Mirror_1_8, Mirror_1_7, Mirror_1_6, Mirror_1_5, Mirror_1_4, Mirror_1_3, Mirror_1_2, Mirror_1_1, Vertex_1, Vertex_2, Vertex_3, Vertex_4, Vertex_5, Vertex_6, Vertex_7, Vertex_8, Vertex_9, Vertex_10], False, False)
Circle_1 = geompy.MakeCircle(Vertex_1, None, 145)
Vertex_11 = geompy.MakeVertex(0, -165, 500)
Curve_2_vertex_2 = geompy.GetSubShape(Curve_2, [2])
Curve_2_vertex_3 = geompy.GetSubShape(Curve_2, [3])
Arc_1 = geompy.MakeArc(Curve_2_vertex_2, Vertex_11, Curve_2_vertex_3)
FissInCylindre = geompy.MakeFaceWires([Curve_2, Arc_1], 1)
Divided_Cylinder_1 = geompy.MakeDividedCylinder(145, 800, GEOM.SQUARE)
CylindreSain = geompy.MakeRotation(Divided_Cylinder_1, OZ, 45*math.pi/180.0)
[Compound_1, vertical, radial, Compound_4] = geompy.Propagate(CylindreSain)
geompy.ExportBREP(FissInCylindre, os.path.join(gmu.pathBloc, "materielCasTests/FissInCylindre.brep"))
Vertex_12 = geompy.MakeVertex(0, -145, 500)
Circle_2 = geompy.MakeCircle(Vertex_12, None, 145)
Face_1 = geompy.MakeFaceWires([Circle_2], 1)
Vertex_13 = geompy.MakeVertex(0, 0, 500)
Disk_1 = geompy.MakeDiskPntVecR(Vertex_13, OZ_1, 170)
FissInCylindre2 = geompy.MakeCommon(Face_1, Disk_1)
geompy.ExportBREP(FissInCylindre2, os.path.join(gmu.pathBloc, "materielCasTests/FissInCylindre2.brep"))
geompy.addToStudy( O, 'O' )
geompy.addToStudy( OX, 'OX' )
geompy.addToStudy( OY, 'OY' )
geompy.addToStudy( OZ, 'OZ' )
geompy.addToStudy( O_1, 'O' )
geompy.addToStudy( OX_1, 'OX' )
geompy.addToStudy( OY_1, 'OY' )
geompy.addToStudy( OZ_1, 'OZ' )
geompy.addToStudy( Vertex_1, 'Vertex_1' )
geompy.addToStudy( Vertex_2, 'Vertex_2' )
geompy.addToStudy( Vertex_3, 'Vertex_3' )
geompy.addToStudy( Vertex_4, 'Vertex_4' )
geompy.addToStudy( Vertex_5, 'Vertex_5' )
geompy.addToStudy( Vertex_6, 'Vertex_6' )
geompy.addToStudy( Vertex_7, 'Vertex_7' )
geompy.addToStudy( Vertex_8, 'Vertex_8' )
geompy.addToStudy( Vertex_9, 'Vertex_9' )
geompy.addToStudy( Vertex_10, 'Vertex_10' )
geompy.addToStudy( Plane_1, 'Plane_1' )
geompy.addToStudy( Mirror_1_1, 'Mirror_1_1' )
geompy.addToStudy( Mirror_1_2, 'Mirror_1_2' )
geompy.addToStudy( Mirror_1_3, 'Mirror_1_3' )
geompy.addToStudy( Mirror_1_4, 'Mirror_1_4' )
geompy.addToStudy( Mirror_1_5, 'Mirror_1_5' )
geompy.addToStudy( Mirror_1_6, 'Mirror_1_6' )
geompy.addToStudy( Mirror_1_7, 'Mirror_1_7' )
geompy.addToStudy( Mirror_1_8, 'Mirror_1_8' )
geompy.addToStudy( Mirror_1_9, 'Mirror_1_9' )
geompy.addToStudy( Curve_2, 'Curve_2' )
geompy.addToStudy( Circle_1, 'Circle_1' )
geompy.addToStudy( Vertex_11, 'Vertex_11' )
geompy.addToStudyInFather( Curve_2, Curve_2_vertex_2, 'Curve_2:vertex_2' )
geompy.addToStudyInFather( Curve_2, Curve_2_vertex_3, 'Curve_2:vertex_3' )
geompy.addToStudy( Arc_1, 'Arc_1' )
geompy.addToStudy( FissInCylindre, 'FissInCylindre' )
geompy.addToStudy( Divided_Cylinder_1, 'Divided Cylinder_1' )
geompy.addToStudy( CylindreSain, 'CylindreSain' )
geompy.addToStudyInFather( CylindreSain, Compound_1, 'Compound_1' )
geompy.addToStudyInFather( CylindreSain, vertical, 'vertical' )
geompy.addToStudyInFather( CylindreSain, radial, 'radial' )
geompy.addToStudyInFather( CylindreSain, Compound_4, 'Compound_4' )
geompy.addToStudy( Vertex_12, 'Vertex_12' )
geompy.addToStudy( Circle_2, 'Circle_2' )
geompy.addToStudy( Face_1, 'Face_1' )
geompy.addToStudy( Vertex_13, 'Vertex_13' )
geompy.addToStudy( Disk_1, 'Disk_1' )
geompy.addToStudy( FissInCylindre2, 'FissInCylindre2' )

###
### SMESH component
###

import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New(theStudy)
from salome.StdMeshers import StdMeshersBuilder
smeshObj_1 = smesh.CreateHypothesis('NumberOfSegments')
smeshObj_1.SetNumberOfSegments( 5 )
smeshObj_1.SetDistrType( 0 )
CylindreSain_1 = smesh.Mesh(CylindreSain)
Regular_1D = CylindreSain_1.Segment()
Nb_Segments_1 = Regular_1D.NumberOfSegments(15,[],[  ])
Nb_Segments_1.SetDistrType( 0 )
Quadrangle_2D = CylindreSain_1.Quadrangle(algo=smeshBuilder.QUADRANGLE)
Hexa_3D = CylindreSain_1.Hexahedron(algo=smeshBuilder.Hexa)
Regular_1D_1 = CylindreSain_1.Segment(geom=vertical)
Nb_Segments_2 = Regular_1D_1.NumberOfSegments(30,[],[  ])
Nb_Segments_2.SetDistrType( 0 )
Regular_1D_2 = CylindreSain_1.Segment(geom=radial)
Nb_Segments_3 = Regular_1D_2.NumberOfSegments(6,[],[  ])
Nb_Segments_3.SetDistrType( 0 )
isDone = CylindreSain_1.Compute()
smesh.SetName(CylindreSain_1, 'CylindreSain')
CylindreSain_1.ExportMED( os.path.join(gmu.pathBloc, "materielCasTests//CylindreSain.med"), 0, SMESH.MED_V2_2, 1 )
SubMesh_1 = Regular_1D_1.GetSubMesh()
SubMesh_2 = Regular_1D_2.GetSubMesh()

## some objects were removed
aStudyBuilder = theStudy.NewBuilder()
SO = theStudy.FindObjectIOR(theStudy.ConvertObjectToIOR(smeshObj_1))
if SO is not None: aStudyBuilder.RemoveObjectWithChildren(SO)
## set object names
smesh.SetName(CylindreSain_1.GetMesh(), 'CylindreSain')
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
