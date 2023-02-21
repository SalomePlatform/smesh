# Minimum Distance

import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# create and mesh two boxes

p1 = geom_builder.MakeVertex(10.0, 10.0, 10.0)
p2 = geom_builder.MakeVertex(20.0, 20.0, 20.0)
p3 = geom_builder.MakeVertex(30.0, 40.0, 50.0)
p4 = geom_builder.MakeVertex(70.0, 100.0, 200.0)

box1 = geom_builder.MakeBoxTwoPnt(p1, p2)
box2 = geom_builder.MakeBoxTwoPnt(p3, p4)

mesh1 = smesh_builder.Mesh(box1, 'box1')
mesh1.Segment().NumberOfSegments(2)
mesh1.Triangle().MaxElementArea(10)

mesh2 = smesh_builder.Mesh(box2, 'box2')
mesh2.Segment().NumberOfSegments(5)
mesh2.Triangle().MaxElementArea(20)

if not mesh1.Compute(): raise Exception("Error when computing Mesh")
if not mesh2.Compute(): raise Exception("Error when computing Mesh")

# compute min distance from grp1 to the origin (not available yet)
smesh_builder.MinDistance(mesh1)

# compute min distance from node 10 of mesh1 to the origin
smesh_builder.MinDistance(mesh1, id1=10)
# ... or
mesh1.MinDistance(10)

# compute min distance between nodes 10 and 20 of mesh1
smesh_builder.MinDistance(mesh1, id1=10, id2=20)
# ... or
mesh1.MinDistance(10, 20)

# compute min distance from element 100 of mesh1 to the origin (not available yet)
smesh_builder.MinDistance(mesh1, id1=100, isElem1=True)
# ... or
mesh1.MinDistance(100, isElem1=True)

# compute min distance between elements 100 and 200 of mesh1 (not available yet)
smesh_builder.MinDistance(mesh1, id1=100, id2=200, isElem1=True, isElem2=True)
# ... or
mesh1.MinDistance(100, 200, True, True)

# compute min distance from element 100 to node 20 of mesh1 (not available yet)
smesh_builder.MinDistance(mesh1, id1=100, id2=20, isElem1=True)
# ... or
mesh1.MinDistance(100, 20, True)

# compute min distance from mesh1 to mesh2 (not available yet)
smesh_builder.MinDistance(mesh1, mesh2)

# compute min distance from node 10 of mesh1 to node 20 of mesh2
smesh_builder.MinDistance(mesh1, mesh2, 10, 20)

# compute min distance from node 10 of mesh1 to element 200 of mesh2 (not available yet)
smesh_builder.MinDistance(mesh1, mesh2, 10, 200, isElem2=True)

# etc...
