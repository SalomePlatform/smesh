# Split bi-quadratic to linear

import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# make a shape consisting of two quadranges
OY  = geom_builder.MakeVectorDXDYDZ(0, 1, 0)
OY1 = geom_builder.MakeTranslation( OY, 1, 0, 0 )
OY2 = geom_builder.MakeTranslation( OY, 2, 0, 0 )
q1  = geom_builder.MakeQuad2Edges( OY, OY1 )
q2  = geom_builder.MakeQuad2Edges( OY1, OY2 )

shape = geom_builder.Partition( [q1,q2], theName='shape' )
ff    = geom_builder.SubShapeAll( shape, geom_builder.ShapeType["FACE"], theName="quad" )

# mesh one quadrange with quadrangless and the other with triangles
mesh = smesh_builder.Mesh( shape )
mesh.Segment().NumberOfSegments(1)
mesh.Quadrangle()
mesh.Triangle( ff[1] )
if not mesh.Compute(): raise Exception("Error when computing Mesh")

# make group of quadrangles and extrude them into a hexahedron
quadGroup = mesh.Group( ff[0], "quads")
mesh.ExtrusionSweepObject2D( quadGroup, [0,0,1], 1 )

# make the mesh bi-quadratic
mesh.ConvertToQuadratic( theToBiQuad=True )

# split all elements into linear ones
mesh.SplitBiQuadraticIntoLinear()
