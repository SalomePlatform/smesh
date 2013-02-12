# Bounding Box

import smesh
from SMESH_mechanic import mesh as mesh1
from SMESH_test1 import mesh as mesh2

mesh1.Compute()
mesh2.Compute()

# compute bounding box for mesh1
mesh1.BoundingBox()

# compute bounding box for list of nodes of mesh1
mesh1.BoundingBox([363, 364, 370, 371, 372, 373, 379, 380, 381])

# compute bounding box for list of elements of mesh1
mesh1.BoundingBox([363, 364, 370, 371, 372, 373, 379, 380, 381], isElem=True)

# compute common bounding box of mesh1 and mesh2
smesh.BoundingBox([mesh1, mesh2])

# etc...
