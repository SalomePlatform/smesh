# Add Edge

import SMESH_mechanic

mesh = SMESH_mechanic.mesh
print ""

# add node
n1 = mesh.AddNode(50, 10, 0)
if n1 == 0: print "KO node addition." 

# add edge
e1 = mesh.AddEdge([n1, 38])
if e1 == 0: print "KO edge addition."
else:       print "New Edge has been added with ID ", e1
