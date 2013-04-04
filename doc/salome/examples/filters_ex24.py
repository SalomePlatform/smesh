# Bare border volumes

# create mesh
from SMESH_mechanic import *
mesh.Tetrahedron()
mesh.Compute()
# remove some volumes to have volumes with bare borders
mesh.RemoveElements( mesh.GetElementsByType(VOLUME)[0:5] )
# get all volumes with bare borders
filter = smesh.GetFilter(SMESH.VOLUME, SMESH.FT_BareBorderVolume)
ids = mesh.GetIdsFromFilter(filter)
print "Volumes with bare borders:", ids
