# Badly oriented volume

# create mesh with volumes
from mechanic import *

# get all badly oriented volumes
filter = smesh_builder.GetFilter(SMESH.VOLUME, SMESH.FT_BadOrientedVolume)
ids = mesh.GetIdsFromFilter(filter)
print("Number of badly oriented volumes:", len(ids))
