# Element Diameter 2D

# create mesh
from mechanic import *

# get all faces that have elements with length > 10
filter = smesh_builder.GetFilter(SMESH.FACE, SMESH.FT_MaxElementLength2D, SMESH.FT_MoreThan, 10)
ids = mesh.GetIdsFromFilter(filter)
print("Number of faces with maximum element length > 10:", len(ids))
