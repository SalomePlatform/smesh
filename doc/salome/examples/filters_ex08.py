# Volume

# create mesh with volumes
from SMESH_mechanic import *
mesh.Tetrahedron()
mesh.Compute()
# get volumes faces with volume > 100
filter = smesh.GetFilter(SMESH.VOLUME, SMESH.FT_Volume3D, SMESH.FT_MoreThan, 100)
ids = mesh.GetIdsFromFilter(filter)
print "Number of volumes with volume > 100:", len(ids)
