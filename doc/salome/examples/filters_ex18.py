# Borders at multi-connection

# create mesh
from SMESH_mechanic import *
# get border edges with number of connected faces = 5
filter = smesh.GetFilter(smesh.EDGE, smesh.FT_MultiConnection, 5)
ids = mesh.GetIdsFromFilter(filter)
print "Number of border edges with 5 faces connected:", len(ids)
