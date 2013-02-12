# Free nodes

# create mesh
from SMESH_mechanic import *
# add node
mesh.AddNode(0,0,0)
# get all free nodes
filter = smesh.GetFilter(smesh.NODE, smesh.FT_FreeNodes)
ids = mesh.GetIdsFromFilter(filter)
print "Number of free nodes:", len(ids)
