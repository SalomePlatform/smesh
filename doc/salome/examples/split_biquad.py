# Split bi-quadratic to linear

import salome
salome.salome_init()

from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

from salome.smesh import smeshBuilder
smesh = smeshBuilder.New(salome.myStudy)

# make a shape consisting of two quadranges
OY  = geompy.MakeVectorDXDYDZ(0, 1, 0)
OY1 = geompy.MakeTranslation( OY, 1, 0, 0 )
OY2 = geompy.MakeTranslation( OY, 2, 0, 0 )
q1  = geompy.MakeQuad2Edges( OY, OY1 )
q2  = geompy.MakeQuad2Edges( OY1, OY2 )

shape = geompy.Partition( [q1,q2], theName='shape' )
ff    = geompy.SubShapeAll( shape, geompy.ShapeType["FACE"], theName="quad" )

# mesh one quadrange with quadrangless and the other with triangles
mesh = smesh.Mesh( shape )
mesh.Segment().NumberOfSegments(1)
mesh.Quadrangle()
mesh.Triangle( ff[1] )
mesh.Compute()

# make group of quadrangles and extrude them into a hexahedron
quadGroup = mesh.Group( ff[0], "quads")
mesh.ExtrusionSweepObject2D( quadGroup, [0,0,1], 1 )

# make the mesh bi-quadratic
mesh.ConvertToQuadratic( theToBiQuad=True )

# split all elements into linear ones
mesh.SplitBiQuadraticIntoLinear()
