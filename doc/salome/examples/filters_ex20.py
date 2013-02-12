# Length

# create mesh
from SMESH_mechanic import *
# get edges with length > 14
filter = smesh.GetFilter(smesh.EDGE, smesh.FT_Length, smesh.FT_MoreThan, 14)
ids = mesh.GetIdsFromFilter(filter)
print "Number of edges with length > 14:", len(ids)
