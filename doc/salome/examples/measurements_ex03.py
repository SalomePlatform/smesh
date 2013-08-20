# Basic Properties


import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

# create a box
box = geompy.MakeBoxDXDYDZ(100,100,100)
face = geompy.SubShapeAllSorted(box, geompy.ShapeType['FACE'])[0]

# mesh a box
mesh = smesh.Mesh(box)
submesh_1d = mesh.Segment().NumberOfSegments(5)
submesh_2d = mesh.Quadrangle()
submesh_3d = mesh.Hexahedron()
submesh_2d_face = mesh.Triangle(face)
mesh.Compute()

# create a group

group_2d = mesh.Group(face)

# compute basic properties

print "Get basic properties: approach 1 (via measurements tool) ----"

measure = smesh.CreateMeasurements()

print "* for mesh:"
print "  length:", measure.Length(mesh.mesh)
print "  area:",   measure.Area(mesh.mesh)
print "  volume:", measure.Volume(mesh.mesh)

print "* for group (2d):"
print "  length:", measure.Length(group_2d)
print "  area:",   measure.Area(group_2d)
print "  volume:", measure.Volume(group_2d)

print "* for submesh (2d):"
print "  length:", measure.Length(submesh_2d_face.GetSubMesh())
print "  area:",   measure.Area(submesh_2d_face.GetSubMesh())
print "  volume:", measure.Volume(submesh_2d_face.GetSubMesh())

measure.UnRegister()

print "Get basic properties: approach 2 (via smeshBuilder) ----"

print "* for mesh:"
print "  length:", smesh.GetLength(mesh)
print "  area:",   smesh.GetArea(mesh)
print "  volume:", smesh.GetVolume(mesh)

print "* for group (2d):"
print "  length:", smesh.GetLength(group_2d)
print "  area:",   smesh.GetArea(group_2d)
print "  volume:", smesh.GetVolume(group_2d)

print "* for submesh (2d):"
print "  length:", smesh.GetLength(submesh_2d_face)
print "  area:",   smesh.GetArea(submesh_2d_face)
print "  volume:", smesh.GetVolume(submesh_2d_face)

print "Get basic properties: approach 3 (via smeshBuilder.Mesh) ----"

print "* for mesh:"
print "  length:", mesh.GetLength()
print "  area:",   mesh.GetArea()
print "  volume:", mesh.GetVolume()

print "* for group (2d): unsupported"

print "* for submesh (2d): unsupported"
