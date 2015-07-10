# Merging Nodes

import SMESH_mechanic, SMESH
mesh = SMESH_mechanic.mesh

# merge nodes
Tolerance = 4.0

# prevent nodes located on geom edges from removal during merge:
# create a group including all nodes on edges
allSegs = mesh.MakeGroup( "all segments", SMESH.EDGE, SMESH.FT_ElemGeomType,'=', SMESH.Geom_EDGE )

GroupsOfNodes =  mesh.FindCoincidentNodes(Tolerance)
mesh.MergeNodes(GroupsOfNodes, NodesToKeep=allSegs)
