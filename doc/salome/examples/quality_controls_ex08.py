# Bare border volumes

from smesh import *
SetCurrentStudy(salome.myStudy)

box = geompy.MakeBoxDXDYDZ(100, 30, 10)
# the smallest face of the box
face = geompy.SubShapeAllSorted( box, geompy.ShapeType["FACE"])[0]

geompy.addToStudy( box, "box" )
geompy.addToStudyInFather( box, face, "face" )

mesh = Mesh(box)
mesh.AutomaticHexahedralization();

# remove half of mesh faces from the smallest face
faceFaces = mesh.GetSubMeshElementsId(face)
faceToRemove = faceFaces[: len(faceFaces)/2]
mesh.RemoveElements( faceToRemove )

# make a group of volumes missing the removed faces
bareGroup = mesh.MakeGroup("bare volumes", VOLUME, FT_BareBorderVolume)
assert(bareGroup.Size() == len( faceToRemove))
