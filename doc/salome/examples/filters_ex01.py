# Aspect ratio

# create mesh
from SMESH_mechanic import *
# get faces with aspect ratio > 6.5
filter = smesh.GetFilter(SMESH.FACE, SMESH.FT_AspectRatio, SMESH.FT_MoreThan, 6.5)
ids = mesh.GetIdsFromFilter(filter)
print "Number of faces with aspect ratio > 6.5:", len(ids)
