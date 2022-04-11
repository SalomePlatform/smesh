# Free edges

import salome
salome.salome_init_without_session()

import SMESH
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# create mesh
face = geom_builder.MakeFaceHW(100, 100, 1)
geom_builder.addToStudy( face, "quadrangle" )
mesh = smesh_builder.Mesh(face)
mesh.Segment().NumberOfSegments(10)
mesh.Triangle().MaxElementArea(25)
mesh.Compute()

# get all faces with free edges
filter = smesh_builder.GetFilter(SMESH.FACE, SMESH.FT_FreeEdges)
ids = mesh.GetIdsFromFilter(filter)
print("Number of faces with free edges:", len(ids))
