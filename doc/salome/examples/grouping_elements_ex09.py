# Creating groups of faces separated by sharp edges

import salome
salome.salome_init_without_session()
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder
geompy = geomBuilder.New()
smesh =  smeshBuilder.New()

# create a mesh on a box
box = geompy.MakeBoxDXDYDZ( 10,10,10, theName="Box" )
mesh = smesh.Mesh(box,"Mesh")
mesh.AutomaticHexahedralization()

# create groups of faces of each side of the box
groups = mesh.FaceGroupsSeparatedByEdges( 89 )
