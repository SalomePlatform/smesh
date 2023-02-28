# Free borders

import salome
salome.salome_init_without_session()

import SMESH
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# create mesh
face = geom_builder.MakeFaceHW(100, 100, 1, theName="quadrangle")
mesh = smesh_builder.Mesh(face)
mesh.Segment().NumberOfSegments(10)
mesh.Triangle().MaxElementArea(25)
if not mesh.Compute(): raise Exception("Error when computing Mesh")

# get all free borders
filter = smesh_builder.GetFilter(SMESH.EDGE, SMESH.FT_FreeBorders)
ids = mesh.GetIdsFromFilter(filter)
print("Number of edges on free borders:", len(ids))
