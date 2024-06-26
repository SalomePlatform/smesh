# Element Diameter 3D

# create mesh with volumes
from mechanic import *

# get all volumes that have elements with length > 10
filter = smesh_builder.GetFilter(SMESH.VOLUME, SMESH.FT_MaxElementLength3D, SMESH.FT_MoreThan, 10)
ids = mesh.GetIdsFromFilter(filter)
print("Number of volumes with maximum element length > 10:", len(ids))
