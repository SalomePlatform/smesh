# Symmetry

from mechanic import *

# create a symmetrical copy of the mesh mirrored through a point
axis = SMESH.AxisStruct(0, 0, 0, 0, 0, 0)

mesh.Mirror([], axis, smesh_builder.POINT, 1)
