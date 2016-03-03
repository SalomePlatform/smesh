# Bare border faces

# create mesh
from SMESH_mechanic import *
# remove some faces to have faces with bare borders
mesh.RemoveElements( mesh.GetElementsByType(SMESH.FACE)[0:5] )
# get all faces with bare borders
filter = smesh.GetFilter(SMESH.FACE, SMESH.FT_BareBorderFace)
ids = mesh.GetIdsFromFilter(filter)
print "Faces with bare borders:", ids
