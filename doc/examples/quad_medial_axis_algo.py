# Usage of Medial Axis Projection algorithm

# for meshing a ring face with quadrangles

import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# create a ring face
circleEdge1 = geom_builder.MakeCircleR( 3 )
circleEdge2 = geom_builder.MakeCircleR( 7 )
ring = geom_builder.MakeFaceWires( [ circleEdge1, circleEdge2 ], True, theName='Ring' )
circleLen1  = geom_builder.BasicProperties( circleEdge1 )[0]
circleLen2  = geom_builder.BasicProperties( circleEdge2 )[0]

# make a mesh

mesh = smesh_builder.Mesh( ring )

circNbSeg = 60
algo1d = mesh.Segment()
algo1d.NumberOfSegments( circNbSeg ) # division of circle edges

algo2d = mesh.Quadrangle( smeshBuilder.QUAD_MA_PROJ )
algo2d.StartEndLength( circleLen2 / circNbSeg, circleLen1 / circNbSeg ) # radial division

if not mesh.Compute(): raise Exception("Error when computing Mesh")
