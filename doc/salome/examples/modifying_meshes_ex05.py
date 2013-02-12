# Add Triangle

import SMESH_mechanic

mesh = SMESH_mechanic.mesh
print ""

# add node
n1 = mesh.AddNode(50, 10, 0)
if n1 == 0: print "KO node addition."

# add triangle
t1 = mesh.AddFace([n1, 38, 39])
if t1 == 0: print "KO triangle addition."
else:       print "New Triangle has been added with ID ", t1
