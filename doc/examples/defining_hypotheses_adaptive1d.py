# Usage of Adaptive 1D hypothesis

import math

import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

box   = geom_builder.MakeBoxDXDYDZ( 100, 100, 100 )
tool  = geom_builder.MakeTranslation( box, 50, 0, 10 )
axis  = geom_builder.MakeVector( geom_builder.MakeVertex( 100, 0, 100 ),geom_builder.MakeVertex( 100, 10, 100 ),)
tool  = geom_builder.Rotate( tool, axis, math.pi * 25 / 180. )
shape = geom_builder.MakeCut( box, tool )
cyl   = geom_builder.MakeCylinder( geom_builder.MakeVertex( -10,5, 95 ), geom_builder.MakeVectorDXDYDZ(1,0,0), 2, 90)
shape = geom_builder.MakeCut( shape, cyl )
tool  = geom_builder.MakeBoxTwoPnt( geom_builder.MakeVertex( -10, 2, 15 ), geom_builder.MakeVertex( 90, 5, 16 ))
shape = geom_builder.MakeCut( shape, tool, theName="shape" )

# Parameters of Adaptive hypothesis. minSize and maxSize are such that they do not limit
# size of segments because size of geometrical features lies within [2.-100.] range, hence
# size of segments is defined by deflection parameter and size of geometrical features only.
minSize = 0.1
maxSize = 200
deflection = 0.05

mesh = smesh_builder.Mesh( shape )
mesh.Segment().Adaptive( minSize, maxSize, deflection )
mesh.Triangle().MaxElementArea( 300 )
mesh.Compute()
