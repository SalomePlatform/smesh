# Moving Nodes

from geompy import *
from smesh import *

box = MakeBoxDXDYDZ(200, 200, 200)

mesh = Mesh( box )
mesh.Segment().AutomaticLength(0.1)
mesh.Quadrangle()
mesh.Compute()

# find node at (0,0,0)
node000 = None
for vId in SubShapeAllIDs( box, ShapeType["VERTEX"]):
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
    raise "node000 not found"

# find node000 using the tested function 
n = mesh.FindNodeClosestTo( -1,-1,-1 )
if not n == node000:
    raise "FindNodeClosestTo() returns " + str( n ) + " != " + str( node000 )

# move node000 to a new location
x,y,z = -10, -10, -10
n = mesh.MoveNode( n,x,y,z )
if not n:
    raise "MoveNode() returns " + n

# check the coordinates of the node000
xyz = mesh.GetNodeXYZ( node000 )
if not ( xyz[0] == x and xyz[1] == y and xyz[2] == z) :
    raise "Wrong coordinates: " + str( xyz ) + " != " + str( [x,y,z] )
