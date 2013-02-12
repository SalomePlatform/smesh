# Length 2D

# create mesh
from SMESH_mechanic import *
# get all faces that have edges with length > 14
filter = smesh.GetFilter(smesh.FACE, smesh.FT_Length2D, smesh.FT_MoreThan, 14)
ids = mesh.GetIdsFromFilter(filter)
print "Number of faces with maximum edge length > 14:", len(ids)
