# Lying on Geom

# create mesh
from SMESH_mechanic import *
# get all faces at least one node of each lies on the face sub_face3
filter = smesh.GetFilter(SMESH.FACE, SMESH.FT_LyingOnGeom, sub_face3)
ids = mesh.GetIdsFromFilter(filter)
print "Number of faces at least one node of each lies on sub_face3:", len(ids)
