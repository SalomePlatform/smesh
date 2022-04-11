# Bare border volumes

# create mesh
from mechanic import *

# remove some volumes to have volumes with bare borders
mesh.RemoveElements(mesh.GetElementsByType(SMESH.VOLUME)[0:5])
# get all volumes with bare borders
filter = smesh_builder.GetFilter(SMESH.VOLUME, SMESH.FT_BareBorderVolume)
ids = mesh.GetIdsFromFilter(filter)
print("Volumes with bare borders:", ids)
