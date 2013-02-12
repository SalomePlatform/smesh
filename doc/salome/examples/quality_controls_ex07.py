# Bare border faces

from smesh import *
SetCurrentStudy(salome.myStudy)

box = geompy.MakeBoxDXDYDZ(100, 100, 100)
geompy.addToStudy( box, "box" )

mesh = smesh.Mesh(box)
mesh.Segment().NumberOfSegments(3)
mesh.Quadrangle()
mesh.Compute()

# remove 2 faces
allFaces = mesh.GetElementsByType(FACE)
mesh.RemoveElements( allFaces[0:2])

bareGroup = mesh.MakeGroup("bare faces", FACE, FT_BareBorderFace)
assert(bareGroup.Size() == 3)
