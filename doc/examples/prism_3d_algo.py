# Usage of Extrusion 3D meshing algorithm

import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

OX = geom_builder.MakeVectorDXDYDZ(1,0,0)
OY = geom_builder.MakeVectorDXDYDZ(0,1,0)
OZ = geom_builder.MakeVectorDXDYDZ(0,0,1)

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

quadBig   = geom_builder.MakeFaceHW( 20,20, 1 )
quadBig   = geom_builder.MakeTranslation( quadBig, 15,15,0 )
quadSmall = geom_builder.MakeFaceHW( 10,10, 1 )
smallQuads1 = geom_builder.MakeMultiTranslation1D( quadSmall, OX, 10, 3 )
smallQuads2 = geom_builder.MakeMultiTranslation1D( quadSmall, OY, 10, 3 )
smallQuads2 = geom_builder.SubShapeAllSortedCentres( smallQuads2, geom_builder.ShapeType["FACE"])[1:]

base = geom_builder.MakeCompound( smallQuads2 + [smallQuads1, quadBig])
axis = geom_builder.MakeLine( geom_builder.MakeVertex( 25,25,0), OZ )
base = geom_builder.MultiRotate1DNbTimes( base, axis, 4)
base = geom_builder.MakePartition( [base], theName="base")
path = geom_builder.MakeSketcher("Sketcher:F 0 0:TT 0 100:R 0:C -90 180:T 0 -150",[0,0,0, 0,-1,0, 1,0,0])

# Make the pipe, each quadrangle of the base turns into a prism with composite wall faces
pipe   = geom_builder.MakePipe( base, path )
prisms = geom_builder.MakePartition( [pipe], theName="prisms")

# get base faces of the prism to define sub-mesh on them
smallQuad = geom_builder.GetFaceNearPoint( prisms, geom_builder.MakeVertex( 0,0,0 ), "smallQuad")
bigQuad   = geom_builder.GetFaceNearPoint( prisms, geom_builder.MakeVertex( 15,15,0 ), "bigQuad")

mesh = smesh_builder.Mesh( prisms )

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
if not mesh.Compute(): raise Exception("Error when computing Mesh")
