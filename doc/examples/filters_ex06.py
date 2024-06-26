# Skew

# create mesh
from mechanic import *

# get faces with skew > 50
filter = smesh_builder.GetFilter(SMESH.FACE, SMESH.FT_Skew, SMESH.FT_MoreThan, 50)
ids = mesh.GetIdsFromFilter(filter)
print("Number of faces with skew > 50:", len(ids))
