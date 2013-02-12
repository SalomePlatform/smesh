# Merging Nodes

import SMESH_mechanic
mesh = SMESH_mechanic.mesh

# merge nodes
Tolerance = 25.0

GroupsOfNodes =  mesh.FindCoincidentNodes(Tolerance)
mesh.MergeNodes(GroupsOfNodes)  
