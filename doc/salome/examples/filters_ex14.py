# Coplanar faces

# create mesh
from SMESH_mechanic import *
faceID = mesh.GetElementsByType(FACE)[0]
# get all faces co-planar to the first face with tolerance 5 degrees
filter = smesh.GetFilter(smesh.FACE, smesh.FT_CoplanarFaces,faceID,Tolerance=5.0)
ids = mesh.GetIdsFromFilter(filter)
print "Number of faces coplanar with the first one:", len(ids)
