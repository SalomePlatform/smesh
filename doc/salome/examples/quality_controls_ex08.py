# Bare border volumes


import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)
import salome_notebook


box = geompy.MakeBoxDXDYDZ(100, 30, 10)
# the smallest face of the box
face = geompy.SubShapeAllSorted( box, geompy.ShapeType["FACE"])[0]

geompy.addToStudy( box, "box" )
geompy.addToStudyInFather( box, face, "face" )

mesh = smesh.Mesh(box)
mesh.AutomaticHexahedralization();

# remove half of mesh faces from the smallest face
faceFaces = mesh.GetSubMeshElementsId(face)
faceToRemove = faceFaces[: len(faceFaces)/2]
mesh.RemoveElements( faceToRemove )

# make a group of volumes missing the removed faces
bareGroup = mesh.MakeGroup("bare volumes", SMESH.VOLUME, SMESH.FT_BareBorderVolume)
assert(bareGroup.Size() == len( faceToRemove))
