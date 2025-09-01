# Creating groups of faces separated by sharp edges

from salome.kernel import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# create a mesh on a box
box = geom_builder.MakeBoxDXDYDZ( 10,10,10, theName="Box" )
mesh = smesh_builder.Mesh(box,"Mesh")
mesh.AutomaticHexahedralization()

# create groups of faces of each side of the box
groups = mesh.FaceGroupsSeparatedByEdges( 89 )
