# Taper

# create mesh
from mechanic import *

# get faces with taper < 1.e-15
filter = smesh_builder.GetFilter(SMESH.FACE, SMESH.FT_Taper, SMESH.FT_LessThan, 1.e-15)
ids = mesh.GetIdsFromFilter(filter)
print("Number of faces with taper < 1.e-15:", len(ids))
