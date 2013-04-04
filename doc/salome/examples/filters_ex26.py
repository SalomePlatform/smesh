# Belong to Geom

# create mesh
from SMESH_mechanic import *
# get all faces which nodes lie on the face sub_face3
filter = smesh.GetFilter(SMESH.FACE, SMESH.FT_BelongToGeom, sub_face3)
ids = mesh.GetIdsFromFilter(filter)
print "Number of faces which nodes lie on sub_face3:", len(ids)
