# Usage of Extrusion 3D meshing algorithm

import salome
salome.salome_init()

from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

OX = geompy.MakeVectorDXDYDZ(1,0,0)
OY = geompy.MakeVectorDXDYDZ(0,1,0)
OZ = geompy.MakeVectorDXDYDZ(0,0,1)

#  Y ^       Make geometry of a "pipe" with the following base (cross section).
#    |       Big central quadrangles will be meshed with triangles, walls
#                         of the pipe will be meshed with quadrilaterals
#   +--+--+--+--+--+--+
#   |  |  |  |  |  |  |
#   +--+--+--+--+--+--+
#   |  |     |     |  |
#   +--+     |     +--+
#   |  |     |     |  |
#   +--+-----+-----+--+
#   |  |     |     |  |
#   +--+     |     +--+
#   |  |     |     |  |
#   +--+--+--+--+--+--+
#   |  |  |  |  |  |  |  -->
#   +--+--+--+--+--+--+   X

quadBig   = geompy.MakeFaceHW( 20,20, 1 )
quadBig   = geompy.MakeTranslation( quadBig, 15,15,0 )
quadSmall = geompy.MakeFaceHW( 10,10, 1 )
smallQuads1 = geompy.MakeMultiTranslation1D( quadSmall, OX, 10, 3 )
smallQuads2 = geompy.MakeMultiTranslation1D( quadSmall, OY, 10, 3 )
smallQuads2 = geompy.SubShapeAllSortedCentres( smallQuads2, geompy.ShapeType["FACE"])[1:]

base = geompy.MakeCompound( smallQuads2 + [smallQuads1, quadBig])
axis = geompy.MakeLine( geompy.MakeVertex( 25,25,0), OZ )
base = geompy.MultiRotate1DNbTimes( base, axis, 4)
base = geompy.MakePartition( [base], theName="base")
path = geompy.MakeSketcher("Sketcher:F 0 0:TT 0 100:R 0:C -90 180:T 0 -150",[0,0,0, 0,-1,0, 1,0,0])

# Make the pipe, each quadrangle of the base turns into a prism with composite wall faces
pipe   = geompy.MakePipe( base, path )
prisms = geompy.MakePartition( [pipe], theName="prisms")


# get base faces of the prism to define sub-mesh on them
smallQuad = geompy.GetFaceNearPoint( prisms, geompy.MakeVertex( 0,0,0 ), "smallQuad")
bigQuad   = geompy.GetFaceNearPoint( prisms, geompy.MakeVertex( 15,15,0 ), "bigQuad")


mesh = smesh.Mesh( prisms )

# assign Global hypotheses

# 1D algorithm and hypothesis for division along the pipe
mesh.Segment().NumberOfSegments(15)

# Extrusion 3D algo
mesh.Prism()

# assign Local hypotheses

# 1D and 2D algos and hyps to mesh smallQuad with quadrilaterals
mesh.Segment(smallQuad).LocalLength( 3 )
mesh.Quadrangle(smallQuad)

# 1D and 2D algos and hyps to mesh bigQuad with triangles
mesh.Segment(bigQuad).LocalLength( 3 )
mesh.Triangle(bigQuad)

# compute the mesh
mesh.Compute()
