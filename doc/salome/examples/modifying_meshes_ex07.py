# Add Tetrahedron

import SMESH_mechanic

mesh = SMESH_mechanic.mesh
print ""

# add node
n1 = mesh.AddNode(50, 10, 0)
if n1 == 0: print "KO node addition."

# add tetrahedron
t1 = mesh.AddVolume([n1, 38, 39, 246])
if t1 == 0: print "KO tetrahedron addition."
else:       print "New Tetrahedron has been added with ID ", t1
