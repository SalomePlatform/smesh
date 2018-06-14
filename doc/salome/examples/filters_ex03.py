# Warping angle

# create mesh
from SMESH_mechanic import *
# get faces with warping angle = 2.0e-13 with tolerance 5.0e-14
filter = smesh.GetFilter(SMESH.FACE, SMESH.FT_Warping, "=", 2.0e-13, Tolerance=5.0e-14)
ids = mesh.GetIdsFromFilter(filter)
print("Number of faces with warping angle = 2.0e-13 (tolerance 5.0e-14):", len(ids))
