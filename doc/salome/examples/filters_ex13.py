# Bare border faces

# create mesh
from SMESH_mechanic import *
# remove some faces to have faces with bare borders
mesh.RemoveElements( mesh.GetElementsByType(smesh.FACE)[0:5] )
# get all faces bare borders
filter = smesh.GetFilter(smesh.FACE, smesh.FT_BareBorderFace)
ids = mesh.GetIdsFromFilter(filter)
print "Faces with bare borders:", ids
