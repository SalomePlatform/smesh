# Bare border faces


import salome
salome.salome_init_without_session()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New()

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New()
import salome_notebook


box = geompy.MakeBoxDXDYDZ(100, 100, 100)
geompy.addToStudy( box, "box" )

mesh = smesh.Mesh(box)
mesh.Segment().NumberOfSegments(3)
mesh.Quadrangle()
mesh.Compute()

# remove 2 faces
allFaces = mesh.GetElementsByType(SMESH.FACE)
mesh.RemoveElements( allFaces[0:2])

bareGroup = mesh.MakeGroup("bare faces", SMESH.FACE, SMESH.FT_BareBorderFace)
assert(bareGroup.Size() == 3)
