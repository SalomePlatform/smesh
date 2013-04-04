# Belong to Cylinder

# create mesh
from SMESH_mechanic import *
# get all faces which lie on the cylindrical face \a sub_face1
filter = smesh.GetFilter(SMESH.FACE, SMESH.FT_BelongToCylinder, sub_face1)
ids = mesh.GetIdsFromFilter(filter)
print "Number of faces which lie on the cylindrical surface sub_face1:", len(ids)
