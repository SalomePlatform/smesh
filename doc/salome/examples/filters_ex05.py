# Taper

# create mesh
from SMESH_mechanic import *
# get faces with taper < 1.e-15
filter = smesh.GetFilter(smesh.FACE, smesh.FT_Taper, smesh.FT_LessThan, 1.e-15)
ids = mesh.GetIdsFromFilter(filter)
print "Number of faces with taper < 1.e-15:", len(ids)
