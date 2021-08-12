# Usage of Medial Axis Projection algorithm

# for meshing a ring face with quadrangles

import salome
salome.salome_init_without_session()
from salome.geom import geomBuilder
geompy = geomBuilder.New()
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New()

# create a ring face
circleEdge1 = geompy.MakeCircleR( 3 )
circleEdge2 = geompy.MakeCircleR( 7 )
ring = geompy.MakeFaceWires( [ circleEdge1, circleEdge2 ], True, theName='Ring' )
circleLen1  = geompy.BasicProperties( circleEdge1 )[0]
circleLen2  = geompy.BasicProperties( circleEdge2 )[0]

# make a mesh

mesh = smesh.Mesh( ring )

circNbSeg = 60
algo1d = mesh.Segment()
algo1d.NumberOfSegments( circNbSeg ) # division of circle edges

algo2d = mesh.Quadrangle( smeshBuilder.QUAD_MA_PROJ )
algo2d.StartEndLength( circleLen2 / circNbSeg, circleLen1 / circNbSeg ) # radial division

mesh.Compute()
