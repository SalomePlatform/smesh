# Minimum angle

# create mesh
from mechanic import *

# get faces with minimum angle > 75
filter = smesh_builder.GetFilter(SMESH.FACE, SMESH.FT_MinimumAngle,">", 75)
ids = mesh.GetIdsFromFilter(filter)
print("Number of faces with minimum angle > 75:", len(ids))
