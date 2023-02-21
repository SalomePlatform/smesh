# Basic Properties

import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# create a box
box = geom_builder.MakeBoxDXDYDZ(100,100,100)
face = geom_builder.SubShapeAllSorted(box, geom_builder.ShapeType['FACE'])[0]

# mesh a box
mesh = smesh_builder.Mesh(box)
submesh_1d = mesh.Segment().NumberOfSegments(5)
submesh_2d = mesh.Quadrangle()
submesh_3d = mesh.Hexahedron()
submesh_2d_face = mesh.Triangle(face)
if not mesh.Compute(): raise Exception("Error when computing Mesh")

# create a group

group_2d = mesh.Group(face)

# compute basic properties

print("Get basic properties: approach 1 (via measurements tool) ----")

measure = smesh_builder.CreateMeasurements()

print("* for mesh:")
print("  length:", measure.Length(mesh.mesh))
print("  area:",   measure.Area(mesh.mesh))
print("  volume:", measure.Volume(mesh.mesh))

print("* for group (2d):")
print("  length:", measure.Length(group_2d))
print("  area:",   measure.Area(group_2d))
print("  volume:", measure.Volume(group_2d))

print("* for submesh (2d):")
print("  length:", measure.Length(submesh_2d_face.GetSubMesh()))
print("  area:",   measure.Area(submesh_2d_face.GetSubMesh()))
print("  volume:", measure.Volume(submesh_2d_face.GetSubMesh()))

measure.UnRegister()

print("Get basic properties: approach 2 (via smeshBuilder) ----")

print("* for mesh:")
print("  length:", smesh_builder.GetLength(mesh))
print("  area:",   smesh_builder.GetArea(mesh))
print("  volume:", smesh_builder.GetVolume(mesh))

print("* for group (2d):")
print("  length:", smesh_builder.GetLength(group_2d))
print("  area:",   smesh_builder.GetArea(group_2d))
print("  volume:", smesh_builder.GetVolume(group_2d))

print("* for submesh (2d):")
print("  length:", smesh_builder.GetLength(submesh_2d_face))
print("  area:",   smesh_builder.GetArea(submesh_2d_face))
print("  volume:", smesh_builder.GetVolume(submesh_2d_face))

print("Get basic properties: approach 3 (via smeshBuilder.Mesh) ----")

print("* for mesh:")
print("  length:", mesh.GetLength())
print("  area:",   mesh.GetArea())
print("  volume:", mesh.GetVolume())

print("* for group (2d): unsupported")

print("* for submesh (2d): unsupported")
