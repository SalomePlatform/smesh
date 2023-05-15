# Warping 3D

# create mesh
from mechanic import *

# get faces with warping angle = 2.0e-13 with tolerance 5.0e-14
filter = smesh_builder.GetFilter(SMESH.VOLUME, SMESH.FT_Warping3D, "=", 2.0e-13, Tolerance=5.0e-14)
ids = mesh.GetIdsFromFilter(filter)
print("Number of volumes with warping = 2.0e-13 (tolerance 5.0e-14):", len(ids))
