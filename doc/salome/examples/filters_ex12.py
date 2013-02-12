# Free faces

# create mesh
from SMESH_mechanic import *
# get all free faces
filter = smesh.GetFilter(smesh.FACE, smesh.FT_FreeFaces)
ids = mesh.GetIdsFromFilter(filter)
print "Number of free faces:", len(ids)
