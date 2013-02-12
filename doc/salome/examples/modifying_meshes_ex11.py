# Removing Nodes

import SMESH_mechanic

mesh = SMESH_mechanic.mesh

# remove nodes #246 and #255
res = mesh.RemoveNodes([246, 255])
if res == 1: print "Nodes removing is OK!"
else:        print "KO nodes removing."
