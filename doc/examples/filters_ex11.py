# Free nodes

# create mesh
from mechanic import *

# add node
mesh.AddNode(0,0,0)

# get all free nodes
filter = smesh_builder.GetFilter(SMESH.NODE, SMESH.FT_FreeNodes)
ids = mesh.GetIdsFromFilter(filter)
print("Number of free nodes:", len(ids))
