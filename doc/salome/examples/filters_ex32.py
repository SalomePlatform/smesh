# Badly oriented volume

# create mesh with volumes
from SMESH_mechanic import *
mesh.Tetrahedron()
mesh.Compute()
# get all badly oriented volumes
filter = smesh.GetFilter(SMESH.VOLUME, SMESH.FT_BadOrientedVolume)
ids = mesh.GetIdsFromFilter(filter)
print "Number of badly oriented volumes:", len(ids)
