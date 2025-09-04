# Bare border faces

from salome.kernel import salome
salome.salome_init_without_session()

from salome.kernel import SMESH
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

box = geom_builder.MakeBoxDXDYDZ(100, 100, 100)
geom_builder.addToStudy( box, "box" )

mesh = smesh_builder.Mesh(box)
mesh.Segment().NumberOfSegments(3)
mesh.Quadrangle()
if not mesh.Compute(): raise Exception("Error when computing Mesh")

# remove 2 faces
allFaces = mesh.GetElementsByType(SMESH.FACE)
mesh.RemoveElements( allFaces[0:2])

bareGroup = mesh.MakeGroup("bare faces", SMESH.FACE, SMESH.FT_BareBorderFace)
assert(bareGroup.Size() == 3)
