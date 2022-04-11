# Double nodes

import salome
salome.salome_init_without_session()

import SMESH
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# make a mesh on a box
box = geom_builder.MakeBoxDXDYDZ(100,100,100)
mesh = smesh_builder.Mesh( box, "Box" )
mesh.Segment().NumberOfSegments(10)
mesh.Quadrangle()
mesh.Hexahedron()
mesh.Compute()
# copy all elements with translation
mesh.TranslateObject( mesh, [10,0,0], Copy=True )
# create  a filter to find nodes equal within tolerance of 1e-5
filter = smesh_builder.GetFilter(SMESH.NODE, SMESH.FT_EqualNodes, Tolerance=1e-5)
# get equal nodes
print("Number of equal nodes:", len( mesh.GetIdsFromFilter( filter )))
