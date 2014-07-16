import salome, math
salome.salome_init()
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)


box   = geompy.MakeBoxDXDYDZ( 100, 100, 100 )
tool  = geompy.MakeTranslation( box, 50, 0, 10 )
axis  = geompy.MakeVector( geompy.MakeVertex( 100, 0, 100 ),geompy.MakeVertex( 100, 10, 100 ),)
tool  = geompy.Rotate( tool, axis, math.pi * 25 / 180. )
shape = geompy.MakeCut( box, tool )
cyl   = geompy.MakeCylinder( geompy.MakeVertex( -10,5, 95 ), geompy.MakeVectorDXDYDZ(1,0,0), 2, 90)
shape = geompy.MakeCut( shape, cyl )
tool  = geompy.MakeBoxTwoPnt( geompy.MakeVertex( -10, 2, 15 ), geompy.MakeVertex( 90, 5, 16 ))
shape = geompy.MakeCut( shape, tool, theName="shape" )

# Parameters of Adaptive hypothesis. minSize and maxSize are such that they do not limit
# size of segments because size of geometrical features lies within [2.-100.] range, hence
# size of segments is defined by deflection parameter and size of geometrical features only.
minSize = 0.1
maxSize = 200
deflection = 0.05

mesh = smesh.Mesh( shape )
mesh.Segment().Adaptive( minSize, maxSize, deflection )
mesh.Triangle().MaxElementArea( 300 )
mesh.Compute()

