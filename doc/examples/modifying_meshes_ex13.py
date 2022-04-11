# Removing Orphan Nodes

from mechanic import *

# add orphan nodes
mesh.AddNode(0,0,0)
mesh.AddNode(1,1,1)
# remove just created orphan nodes
res = mesh.RemoveOrphanNodes()
if res == 1: print("Removed %d nodes!" % res)
else:        print("KO nodes removing.")
