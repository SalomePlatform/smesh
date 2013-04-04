# Over-constrained volumes

# create mesh
from SMESH_mechanic import *
mesh.Tetrahedron()
mesh.Compute()
# get all over-constrained volumes
filter = smesh.GetFilter(SMESH.VOLUME, SMESH.FT_OverConstrainedVolume)
ids = mesh.GetIdsFromFilter(filter)
print "Over-constrained volumes:", ids
