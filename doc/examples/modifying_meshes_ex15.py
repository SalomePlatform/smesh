# Moving Nodes

from salome.kernel import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

box = geom_builder.MakeBoxDXDYDZ(200, 200, 200)

mesh = smesh_builder.Mesh( box )
mesh.Segment().AutomaticLength(0.1)
mesh.Quadrangle()
if not mesh.Compute(): raise Exception("Error when computing Mesh")

# find node at (0,0,0) which is located on a geom vertex
node000 = None
for vId in geom_builder.SubShapeAllIDs( box, geom_builder.ShapeType["VERTEX"]):
    if node000: break
    nodeIds = mesh.GetSubMeshNodesId( vId, True )
    for node in nodeIds:
        xyz = mesh.GetNodeXYZ( node )
        if xyz[0] == 0 and xyz[1] == 0 and xyz[2] == 0 :
            node000 = node
            pass
        pass
    pass

if not node000:
    raise Exception("node000 not found")

# find node000 using a dedicated function 
n = mesh.FindNodeClosestTo( -1,-1,-1 )
if not n == node000:
    raise Exception("FindNodeClosestTo() returns " + str( n ) + " != " + str( node000 ))

# move node000 to a new location
x,y,z = -10, -10, -10
n = mesh.MoveNode( n,x,y,z )
if not n:
    raise Exception("MoveNode() returns " + n)

# check the coordinates of the node000
xyz = mesh.GetNodeXYZ( node000 )
if not ( xyz[0] == x and xyz[1] == y and xyz[2] == z) :
    raise Exception("Wrong coordinates: " + str( xyz ) + " != " + str( [x,y,z] ))
