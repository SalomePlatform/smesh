#!/usr/bin/env python

###
### This file is generated automatically by SALOME v9.9.0 with dump python functionality
###

import sys
import salome

salome.salome_init()
import salome_notebook
notebook = salome_notebook.NoteBook()
sys.path.insert(0, r'/home/B61570/work_in_progress/dual_mesh')

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
Cylinder_1 = geompy.MakeCylinderRH(100, 400)
Sphere_1 = geompy.MakeSpherePntR(O, 100)
Fuse_1 = geompy.MakeFuseList([Cylinder_1, Sphere_1], True, True)

[geomObj_1,geomObj_2,geomObj_3] = geompy.ExtractShapes(Fuse_1, geompy.ShapeType["FACE"], True)

top = geompy.CreateGroup(Fuse_1, geompy.ShapeType["FACE"])
geompy.UnionIDs(top, geomObj_1.GetSubShapeIndices())

middle = geompy.CreateGroup(Fuse_1, geompy.ShapeType["FACE"])
geompy.UnionIDs(middle, geomObj_2.GetSubShapeIndices())

bottom = geompy.CreateGroup(Fuse_1, geompy.ShapeType["FACE"])
geompy.UnionIDs(bottom, geomObj_3.GetSubShapeIndices())

#[top, middle, bottom] = geompy.GetExistingSubObjects(Fuse_1, False)

geompy.addToStudy( O, 'O' )
geompy.addToStudy( OX, 'OX' )
geompy.addToStudy( OY, 'OY' )
geompy.addToStudy( OZ, 'OZ' )
geompy.addToStudy( Cylinder_1, 'Cylinder_1' )
geompy.addToStudy( Sphere_1, 'Sphere_1' )
geompy.addToStudy( Fuse_1, 'Fuse_1' )
geompy.addToStudyInFather( Fuse_1, top, 'top' )
geompy.addToStudyInFather( Fuse_1, middle, 'middle' )
geompy.addToStudyInFather( Fuse_1, bottom, 'bottom' )

###
### SMESH component
###

import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New()

NETGEN_3D_Parameters_1 = smesh.CreateHypothesisByAverageLength( 'NETGEN_Parameters', 'NETGENEngine', 50, 0 )
Mesh_1 = smesh.Mesh(Fuse_1,'Mesh_1')
status = Mesh_1.AddHypothesis( Fuse_1, NETGEN_3D_Parameters_1 )
NETGEN_1D_2D_3D = Mesh_1.Tetrahedron(algo=smeshBuilder.NETGEN_1D2D3D)
top_1 = Mesh_1.GroupOnGeom(top,'top',SMESH.FACE)
middle_1 = Mesh_1.GroupOnGeom(middle,'middle',SMESH.FACE)
bottom_1 = Mesh_1.GroupOnGeom(bottom,'bottom',SMESH.FACE)
isDone = Mesh_1.Compute()

[ top_1, middle_1, bottom_1 ] = Mesh_1.GetGroups()


dual_Mesh_raw_1 = smesh.CreateDualMesh(Mesh_1, 'dual_Mesh_raw_1', False)
dual_Mesh_1 = smesh.CreateDualMesh(Mesh_1, 'dual_Mesh_1', True)

[ top_2, middle_2, bottom_2 ] = dual_Mesh_1.GetGroups()

#Comparing volumes
dual_volume = dual_Mesh_1.GetVolume()
dual_raw_volume = dual_Mesh_raw_1.GetVolume()
print("dual_volume: ", dual_volume)
print("dual_raw_volume: ", dual_raw_volume)

assert (dual_volume >= dual_raw_volume)

if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()
