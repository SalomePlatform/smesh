
"""
Most basic test of GEOM/SMESH usecase, but it can be tested without any session launched.
"""

import sys
from salome.kernel import salome

import os
salome.standalone() # <- key point of test is here
salome.salome_init()
from salome.kernel import salome_notebook
notebook = salome_notebook.NoteBook()

###
### GEOM component
###

from salome.kernel import GEOM
from salome.geom import geomBuilder
import math
from salome.kernel import SALOMEDS

geompy = geomBuilder.New()
O = geompy.MakeVertex(0, 0, 0)
OX = geompy.MakeVectorDXDYDZ(1, 0, 0)
OY = geompy.MakeVectorDXDYDZ(0, 1, 0)
OZ = geompy.MakeVectorDXDYDZ(0, 0, 1)
Box_1 = geompy.MakeBoxDXDYDZ(200, 200, 200)
geompy.addToStudy( O, 'O' )
geompy.addToStudy( OX, 'OX' )
geompy.addToStudy( OY, 'OY' )
geompy.addToStudy( OZ, 'OZ' )
geompy.addToStudy( Box_1, 'Box_1' )
###
### SMESH component
###
from salome.kernel import SMESH
from salome.kernel import  SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New()
smesh.SetEnablePublish( True ) # Set to False to avoid publish in study if not needed or in some particular situations:
                                 # multiples meshes built in parallel, complex and numerous mesh edition (performance)
Mesh_1 = smesh.Mesh(Box_1)
NETGEN_1D_2D_3D = Mesh_1.Tetrahedron(algo=smeshBuilder.NETGEN_1D2D3D)
isDone = Mesh_1.Compute()
if not isDone:
    raise Exception("Error when computing Mesh")

## Set names of Mesh objects
smesh.SetName(NETGEN_1D_2D_3D.GetAlgorithm(), 'NETGEN 1D-2D-3D')
smesh.SetName(Mesh_1.GetMesh(), 'Mesh_1')

nbOfTetraExp = 5

assert(Mesh_1.GetMesh().NbTetras()>=nbOfTetraExp)

#Mesh_1.ExportMED("toto.med")

import medcoupling as mc

mv_mm = Mesh_1.ExportMEDCoupling()
assert(mc.MEDCoupling1SGTUMesh(mv_mm[0]).getNumberOfCells()>=nbOfTetraExp)
