# Over-constrained volumes

# create mesh
from mechanic import *

# get all over-constrained volumes
filter = smesh_builder.GetFilter(SMESH.VOLUME, SMESH.FT_OverConstrainedVolume)
ids = mesh.GetIdsFromFilter(filter)
print("Over-constrained volumes:", ids)
