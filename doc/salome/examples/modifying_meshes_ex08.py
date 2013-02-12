# Add Hexahedron

import SMESH_mechanic

mesh = SMESH_mechanic.mesh
print ""

# add nodes
nId1 = mesh.AddNode(50, 10, 0)
nId2 = mesh.AddNode(47, 12, 0)
nId3 = mesh.AddNode(50, 10, 10)
nId4 = mesh.AddNode(47, 12, 10)

if nId1 == 0 or nId2 == 0 or nId3 == 0 or nId4 == 0: print "KO node addition."

# add hexahedron
vId = mesh.AddVolume([nId2, nId1, 38, 39, nId4, nId3, 245, 246])
if vId == 0: print "KO Hexahedron addition."
else:        print "New Hexahedron has been added with ID ", vId
