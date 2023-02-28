# Change priority of sub-meshes in Mesh

import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

box = geom_builder.MakeBoxDXDYDZ(200, 200, 200)
[Face_1, Face_2, Face_3, Face_4, Face_5, Face_6] = geom_builder.SubShapeAllSorted(box, geom_builder.ShapeType["FACE"])

# create Mesh object on Box shape
mesh = smesh_builder.Mesh(box)

# assign mesh algorithms and hypotheses
mesh.Segment().NumberOfSegments(20)
mesh.Triangle().MaxElementArea(1200)
mesh.Tetrahedron().MaxElementVolume(40000)

# create sub-mesh and assign algorithms on Face_1
mesh.Segment(geom=Face_1).NumberOfSegments(4)
mesh.Triangle(geom=Face_1)

# create sub-mesh and assign algorithms on Face_2
mesh.Segment(geom=Face_2).NumberOfSegments(8)
mesh.Triangle(geom=Face_2)

# create sub-mesh and assign algorithms on Face_3
mesh.Segment(geom=Face_3).NumberOfSegments(12)
mesh.Triangle(geom=Face_3)

# get existing sub-mesh priority order: F1 -> F2 -> F3
[[SubMesh_F1, SubMesh_F3, SubMesh_F2]] = mesh.GetMeshOrder()
isDone = mesh.Compute()
if not isDone: raise Exception("Error when computing Mesh")
print("Nb elements at initial order of sub-meshes:", mesh.NbElements())

# set new sub-mesh order: F2 -> F1 -> F3
isDone = mesh.SetMeshOrder([[SubMesh_F2, SubMesh_F1, SubMesh_F3]])
isDone = mesh.Compute()
if not isDone: raise Exception("Error when computing Mesh")
print("Nb elements at new order of sub-meshes:", mesh.NbElements())

# compute with other sub-mesh order: F3 -> F2 -> F1
isDone = mesh.SetMeshOrder([[SubMesh_F3, SubMesh_F2, SubMesh_F1]])
isDone = mesh.Compute()
if not isDone: raise Exception("Error when computing Mesh")
print("Nb elements at another order of sub-meshes:", mesh.NbElements())
