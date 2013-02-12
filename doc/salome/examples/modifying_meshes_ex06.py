# Add Quadrangle

import SMESH_mechanic

mesh = SMESH_mechanic.mesh
print ""

# add node
n1 = mesh.AddNode(50, 10, 0)
if n1 == 0: print "KO node addition."

n2 = mesh.AddNode(40, 20, 0)
if n2 == 0: print "KO node addition."

# add quadrangle
q1 = mesh.AddFace([n2, n1, 38, 39])
if q1 == 0: print "KO quadrangle addition."
else:       print "New Quadrangle has been added with ID ", q1
