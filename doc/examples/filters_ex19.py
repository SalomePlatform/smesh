# Borders at multi-connection 2D

# create mesh
from mechanic import *

# get faces which consist of edges belonging to 2 mesh elements
filter = smesh_builder.GetFilter(SMESH.FACE, SMESH.FT_MultiConnection2D, 2)
ids = mesh.GetIdsFromFilter(filter)
print("Number of faces consisting of edges belonging to 2 faces:", len(ids))
