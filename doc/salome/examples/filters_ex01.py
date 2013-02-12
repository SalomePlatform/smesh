# Aspect ratio

# create mesh
from SMESH_mechanic import *
# get faces with aspect ratio > 6.5
filter = smesh.GetFilter(smesh.FACE, smesh.FT_AspectRatio, smesh.FT_MoreThan, 6.5)
ids = mesh.GetIdsFromFilter(filter)
print "Number of faces with aspect ratio > 6.5:", len(ids)
