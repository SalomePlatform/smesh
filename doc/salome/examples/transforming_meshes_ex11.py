# Duplicate nodes

import salome
import smesh
import SMESH_test1

mesh = SMESH_test1.mesh

# Compute mesh
mesh.Compute()

# Without the duplication of border elements

# Nodes to duplicate
nodes1 = mesh.CreateEmptyGroup( smesh.NODE, 'nodes1' )
nodes1.Add( [ 289, 278, 302, 285 ] )

# Group of faces to replace nodes with new ones 
faces1 = mesh.CreateEmptyGroup( smesh.FACE, 'faces1' )
faces1.Add( [ 519, 556, 557 ] )

# Duplicate nodes
print "\nMesh before the first nodes duplication:"
print "Nodes      : ", mesh.NbNodes()
print "Edges      : ", mesh.NbEdges()
print "Triangles  : ", mesh.NbTriangles()

groupOfCreatedNodes = mesh.DoubleNodeGroup(nodes1, faces1, theMakeGroup=True)
print "New nodes:", groupOfCreatedNodes.GetIDs()

print "\nMesh after the first nodes duplication:"
print "Nodes      : ", mesh.NbNodes()
print "Edges      : ", mesh.NbEdges()
print "Triangles  : ", mesh.NbTriangles()

# With the duplication of border elements

# Edges to duplicate
edges = mesh.CreateEmptyGroup( smesh.EDGE, 'edges' )
edges.Add( [ 29, 30, 31 ] )

# Nodes not to duplicate
nodes2 = mesh.CreateEmptyGroup( smesh.NODE, 'nodes2' )
nodes2.Add( [ 32, 5 ] )

# Group of faces to replace nodes with new ones 
faces2 = mesh.CreateEmptyGroup( smesh.FACE, 'faces2' )
faces2.Add( [ 576, 578, 580 ] )

# Duplicate nodes
print "\nMesh before the second nodes duplication:"
print "Nodes      : ", mesh.NbNodes()
print "Edges      : ", mesh.NbEdges()
print "Triangles  : ", mesh.NbTriangles()

groupOfNewEdges = mesh.DoubleNodeElemGroup( edges, nodes2, faces2, theMakeGroup=True )
print "New edges:", groupOfNewEdges.GetIDs()

print "\nMesh after the second nodes duplication:"
print "Nodes      : ", mesh.NbNodes()
print "Edges      : ", mesh.NbEdges()
print "Triangles  : ", mesh.NbTriangles()

# Update object browser
if salome.sg.hasDesktop():
    salome.sg.updateObjBrowser(0)
