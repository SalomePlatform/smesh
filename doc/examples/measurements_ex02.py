# Bounding Box

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

# compute bounding box for mesh1
mesh1.BoundingBox()

# compute bounding box for list of nodes of mesh2
mesh2.BoundingBox([363, 364, 370, 371, 372, 373, 379, 380, 381])

# compute bounding box for list of elements of mesh2
mesh2.BoundingBox([363, 364, 370, 371, 372, 373, 379, 380, 381], isElem=True)

# compute common bounding box of mesh1 and mesh2
smesh_builder.BoundingBox([mesh1, mesh2])

# etc...
