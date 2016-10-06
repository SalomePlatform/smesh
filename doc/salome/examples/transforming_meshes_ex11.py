# Duplicate nodes or/and elements


import salome
salome.salome_init()

import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

# Create a box

box = geompy.MakeBox(0., 0., 0., 100., 200., 300.)

# Define hexa mesh on a box
mesh = smesh.Mesh(box, "Mesh")
mesh.Segment().NumberOfSegments(7)
mesh.Quadrangle()
mesh.Hexahedron()

# Compute mesh
mesh.Compute()

# Duplicate nodes only

# Nodes to duplicate
nodes1 = mesh.CreateEmptyGroup( SMESH.NODE, 'nodes1' )
nodes1.Add( [ 119, 125, 131, 137 ] )

# Group of faces to replace nodes with new ones 
faces1 = mesh.CreateEmptyGroup( SMESH.FACE, 'faces1' )
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

# Duplicate nodes and border elements

# Edges to duplicate
edges = mesh.CreateEmptyGroup( SMESH.EDGE, 'edges' )
edges.Add( [ 32, 33, 34 ] )

# Nodes not to duplicate
nodes2 = mesh.CreateEmptyGroup( SMESH.NODE, 'nodes2' )
nodes2.Add( [ 35, 38 ] )

# Group of faces to replace nodes with new ones 
faces2 = mesh.CreateEmptyGroup( SMESH.FACE, 'faces2' )
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


# Duplicate elements only

# Duplicate all faces and make a group of new faces.
# If a mesh is given to DoubleElements(), all elements of the greatest dimension are duplicated
newFacesGroup = mesh.DoubleElements( mesh, "newFacesGroup" )

# Duplicate edges contained in the group "edges" and add new edges to this group
mesh.DoubleElements( edges, edges.GetName() )

# Duplicate two first edges of the mesh
mesh.DoubleElements([ 1, 2 ])

# Update object browser
if salome.sg.hasDesktop():
    salome.sg.updateObjBrowser(False)
