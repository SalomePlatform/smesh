# Length 2D

# create mesh
from mechanic import *

# get all faces that have edges with length > 14
filter = smesh_builder.GetFilter(SMESH.FACE, SMESH.FT_MaxElementLength2D, SMESH.FT_MoreThan, 14)
ids = mesh.GetIdsFromFilter(filter)
print("Number of faces with maximum edge length > 14:", len(ids))
