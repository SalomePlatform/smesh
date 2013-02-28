# Duplicate nodes

import salome
import geompy
import smesh

# Create a box

box = geompy.MakeBox(0., 0., 0., 100., 200., 300.)

# Define hexa mesh on a box
mesh = smesh.Mesh(box, "Mesh")
mesh.Segment().NumberOfSegments(7)
mesh.Quadrangle()
mesh.Hexahedron()

# Compute mesh
mesh.Compute()

# Without the duplication of border elements

# Nodes to duplicate
nodes1 = mesh.CreateEmptyGroup( smesh.NODE, 'nodes1' )
nodes1.Add( [ 119, 125, 131, 137 ] )

# Group of faces to replace nodes with new ones 
faces1 = mesh.CreateEmptyGroup( smesh.FACE, 'faces1' )
faces1.Add( [ 144, 151, 158 ] )

# Duplicate nodes
print "\nMesh before the first nodes duplication:"
print "Nodes       : ", mesh.NbNodes()
print "Edges       : ", mesh.NbEdges()
print "Quadrangles : ", mesh.NbQuadrangles()

groupOfCreatedNodes = mesh.DoubleNodeGroup(nodes1, faces1, theMakeGroup=True)
print "New nodes:", groupOfCreatedNodes.GetIDs()

print "\nMesh after the first nodes duplication:"
print "Nodes       : ", mesh.NbNodes()
print "Edges       : ", mesh.NbEdges()
print "Quadrangles : ", mesh.NbQuadrangles()

# With the duplication of border elements

# Edges to duplicate
edges = mesh.CreateEmptyGroup( smesh.EDGE, 'edges' )
edges.Add( [ 32, 33, 34 ] )

# Nodes not to duplicate
nodes2 = mesh.CreateEmptyGroup( smesh.NODE, 'nodes2' )
nodes2.Add( [ 35, 38 ] )

# Group of faces to replace nodes with new ones 
faces2 = mesh.CreateEmptyGroup( smesh.FACE, 'faces2' )
faces2.Add( [ 141, 148, 155 ] )

# Duplicate nodes
print "\nMesh before the second nodes duplication:"
print "Nodes       : ", mesh.NbNodes()
print "Edges       : ", mesh.NbEdges()
print "Quadrangles : ", mesh.NbQuadrangles()

groupOfNewEdges = mesh.DoubleNodeElemGroup( edges, nodes2, faces2, theMakeGroup=True )
print "New edges:", groupOfNewEdges.GetIDs()

print "\nMesh after the second nodes duplication:"
print "Nodes       : ", mesh.NbNodes()
print "Edges       : ", mesh.NbEdges()
print "Quadrangles : ", mesh.NbQuadrangles()

# Update object browser
if salome.sg.hasDesktop():
    salome.sg.updateObjBrowser(0)
