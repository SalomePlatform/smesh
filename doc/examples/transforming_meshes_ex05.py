# Merging Nodes

from mechanic import *

# criterion of coincidence
Tolerance = 4.0

# find close nodes of triangle elements only
triangleFilter = smesh_builder.GetFilter( SMESH.FACE, SMESH.FT_ElemGeomType,'=', SMESH.Geom_TRIANGLE )
GroupsOfNodesOfTriangles = mesh.FindCoincidentNodesOnPart([triangleFilter],Tolerance)

# prevent nodes located on geom edges from removal during merge:
# create a group including all nodes on edges
allSegs = mesh.MakeGroup( "all segments", SMESH.EDGE, SMESH.FT_ElemGeomType,'=', SMESH.Geom_EDGE )

mesh.MergeNodes(GroupsOfNodesOfTriangles, NodesToKeep=allSegs)

# find close nodes in the whole mesh
GroupsOfNodes = mesh.FindCoincidentNodes(Tolerance)

mesh.MergeNodes(GroupsOfNodes, NodesToKeep=allSegs)
