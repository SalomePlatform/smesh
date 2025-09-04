# Removing Nodes

from salome.kernel import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# create and mesh two boxes

box = geom_builder.MakeBoxDXDYDZ(10.0, 10.0, 10.0)
mesh = smesh_builder.Mesh(box, 'box')
mesh.Segment().NumberOfSegments(10)
mesh.Triangle().MaxElementArea(5)

if not mesh.Compute(): raise Exception("Error when computing Mesh")
print("After Compute(): %s nodes, %s faces" % ( mesh.NbNodes(), mesh.NbFaces()))

# remove nodes #246 and #255

res = mesh.RemoveNodes([246, 255])
print("After RemoveNodes(): %s nodes, %s faces" % ( mesh.NbNodes(), mesh.NbFaces()))

# removing node #100 with reconnection 

mesh.RemoveNodeWithReconnection( 100 )
print("After RemoveNodeWithReconnection(): %s nodes, %s faces" % ( mesh.NbNodes(), mesh.NbFaces()))
