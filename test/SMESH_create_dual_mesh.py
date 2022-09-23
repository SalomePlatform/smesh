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
Sphere_1 = geompy.MakeSphereR(100)
[geomObj_1] = geompy.ExtractShapes(Sphere_1, geompy.ShapeType["FACE"], True)
geompy.addToStudy( O, 'O' )
geompy.addToStudy( OX, 'OX' )
geompy.addToStudy( OY, 'OY' )
geompy.addToStudy( OZ, 'OZ' )
geompy.addToStudy( Sphere_1, 'Sphere_1' )

###
### SMESH component
###

import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New()

NETGEN_3D_Parameters_1 = smesh.CreateHypothesisByAverageLength( 'NETGEN_Parameters', 'NETGENEngine', 34.641, 0 )
Mesh_1 = smesh.Mesh(Sphere_1,'Mesh_1')
status = Mesh_1.AddHypothesis( Sphere_1, NETGEN_3D_Parameters_1 )
NETGEN_1D_2D_3D = Mesh_1.Tetrahedron(algo=smeshBuilder.NETGEN_1D2D3D)
isDone = Mesh_1.Compute()
dual_Mesh_1 = smesh.CreateDualMesh( Mesh_1, 'dual_Mesh_1')


assert(dual_Mesh_1.NbPolyhedrons() > 0)
assert(dual_Mesh_1.NbTetras() == 0)


if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()
