# Over-constrained faces

# create mesh
from mechanic import *

# get all over-constrained faces
filter = smesh_builder.GetFilter(SMESH.FACE, SMESH.FT_OverConstrainedFace)
ids = mesh.GetIdsFromFilter(filter)
print("Over-constrained faces:", ids)
