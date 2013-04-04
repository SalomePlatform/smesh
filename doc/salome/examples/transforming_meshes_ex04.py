# Symmetry

import math

import SMESH_mechanic
import SMESH

smesh = SMESH_mechanic.smesh
mesh = SMESH_mechanic.mesh 

# create a symmetrical copy of the mesh mirrored through a point
axis = SMESH.AxisStruct(0, 0, 0, 0, 0, 0)

mesh.Mirror([], axis, smesh.POINT, 1)
