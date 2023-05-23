# Scaled Jacobian

# create mesh with volumes
from mechanic import *

# get volumes with scaled jacobian > 0.75
filter = smesh_builder.GetFilter(SMESH.VOLUME, SMESH.FT_ScaledJacobian, SMESH.FT_MoreThan, 0.75 )
ids = mesh.GetIdsFromFilter(filter)
print("Number of volumes with scaled jacobian > 0.75:", len(ids))
