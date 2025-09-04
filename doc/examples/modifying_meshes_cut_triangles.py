# Cutting Triangles

from salome.kernel import salome
salome.salome_init_without_session()

from salome.smesh import smeshBuilder

smesh_builder = smeshBuilder.New()

# create 3 triangles and 1 segment all sharing edge 1-2
mesh = smesh_builder.Mesh()
n1 = mesh.AddNode( 0, 0, 0)
n2 = mesh.AddNode( 0, 0, -10)
n3 = mesh.AddNode( 10, 0, 0)
n4 = mesh.AddNode( 0, 10, 0)
n5 = mesh.AddNode( 0, -10, 0)
mesh.AddFace([ n1, n2, n3])
mesh.AddFace([ n1, n2, n4])
mesh.AddFace([ n1, n2, n5])
mesh.AddEdge([ n1, n2] )

# ===========================================================================
# cut all the triangles and the segment by setting a new node on the segment
# ===========================================================================

mesh.AddNodeOnSegment( n1, n2, 0.6 )
assert mesh.NbNodes() == 6     # one new node created
assert mesh.NbTriangles() == 6 # each of the 3 triangles is split into two
assert mesh.NbEdges() == 2     # a segment is split into two

# ===============================================================
# cut a triangle into three by adding a new node on the triangle
# ===============================================================

triangleID = 1
mesh.AddNodeOnFace( triangleID, 2, 0, -6 )
assert mesh.NbNodes() == 7     # one new node created
assert mesh.NbTriangles() == 8 # the triangles is split into three
