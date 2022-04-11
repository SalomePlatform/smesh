# Free faces

# create mesh
from mechanic import *

# get all free faces
filter = smesh_builder.GetFilter(SMESH.FACE, SMESH.FT_FreeFaces)
ids = mesh.GetIdsFromFilter(filter)
print("Number of free faces:", len(ids))
