# Over-constrained faces
# create mesh
from SMESH_mechanic import *
# get all over-constrained faces
filter = smesh.GetFilter(SMESH.FACE, SMESH.FT_OverConstrainedFace)
ids = mesh.GetIdsFromFilter(filter)
print "Over-constrained faces:", ids
