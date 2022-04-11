# Length

# create mesh
from mechanic import *

# get edges with length > 14
filter = smesh_builder.GetFilter(SMESH.EDGE, SMESH.FT_Length, SMESH.FT_MoreThan, 14)
ids = mesh.GetIdsFromFilter(filter)
print("Number of edges with length > 14:", len(ids))
