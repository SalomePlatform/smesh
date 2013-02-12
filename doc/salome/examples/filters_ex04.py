# Minimum angle

# create mesh
from SMESH_mechanic import *
# get faces with minimum angle > 75
filter = smesh.GetFilter(smesh.FACE, smesh.FT_MinimumAngle,">", 75)
ids = mesh.GetIdsFromFilter(filter)
print "Number of faces with minimum angle > 75:", len(ids)
