# Bare border volumes

from salome.kernel import salome
salome.salome_init_without_session()

from salome.kernel import SMESH
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

box = geom_builder.MakeBoxDXDYDZ(100, 30, 10)
# the smallest face of the box
face = geom_builder.SubShapeAllSorted( box, geom_builder.ShapeType["FACE"])[0]

geom_builder.addToStudy( box, "box" )
geom_builder.addToStudyInFather( box, face, "face" )

mesh = smesh_builder.Mesh(box)
mesh.AutomaticHexahedralization();

# remove half of mesh faces from the smallest face
faceFaces = mesh.GetSubMeshElementsId(face)
faceToRemove = faceFaces[: len(faceFaces) // 2]
mesh.RemoveElements( faceToRemove )

# make a group of volumes missing the removed faces
bareGroup = mesh.MakeGroup("bare volumes", SMESH.VOLUME, SMESH.FT_BareBorderVolume)
assert(bareGroup.Size() == len( faceToRemove))
