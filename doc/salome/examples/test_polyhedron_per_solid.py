#!/usr/bin/env python

import sys
import salome

salome.salome_init()

###
### GEOM component
###

import GEOM
from salome.geom import geomBuilder
import math
import SALOMEDS


geompy = geomBuilder.New()

## Creates a polygon given its centre, external radius and number of sides
def makePolygon(p_centre, radius, nb_sides, theName=""):
  points = []
  x, y, z = geompy.PointCoordinates(p_centre)
  for i in range(nb_sides):
    angle = i*2*math.pi/nb_sides
    p = geompy.MakeVertex(x+radius*math.cos(angle), y+radius*math.sin(angle), 0)
    points.append(p)
  wire = geompy.MakePolyline(points, True)
  face = geompy.MakeFace(wire, 1)
  if theName:
    geompy.addToStudy(face, theName)
  return face

## Creates a solid by adding a vertex on its top
def makeSummitSolid(face, height, theName=""):
  p_cdg = geompy.MakeCDG(face)
  p_top = geompy.MakeTranslation(p_cdg, 0, 0, height)
  edges = geompy.SubShapeAll(face, geompy.ShapeType["EDGE"])
  faces = [face]
  for edge in edges:
    p1, p2 = geompy.SubShapeAll(edge, geompy.ShapeType["VERTEX"])
    wire = geompy.MakePolyline([p1, p2, p_top], True)
    face = geompy.MakeFace(wire, 1)
    faces.append(face)
  shell = geompy.MakeShell(faces)
  solid = geompy.MakeSolid(shell)
  if theName:
    geompy.addToStudy(solid, theName)
  return solid

O = geompy.MakeVertex(0, 0, 0)
OX = geompy.MakeVectorDXDYDZ(1, 0, 0)
OY = geompy.MakeVectorDXDYDZ(0, 1, 0)
OZ = geompy.MakeVectorDXDYDZ(0, 0, 1)
geompy.addToStudy( O, 'O' )
geompy.addToStudy( OX, 'OX' )
geompy.addToStudy( OY, 'OY' )
geompy.addToStudy( OZ, 'OZ' )

height = 0.5

triangle = makePolygon(O, 1, 3, "triangle")

P1 = geompy.MakeVertex(2, 0, 0)
quadrangle = makePolygon(P1, 1, 4, "quadrangle")

P2 = geompy.MakeVertex(4, 0, 0)
pentagon = makePolygon(P2, 1, 5, "pentagon")

P3 = geompy.MakeVertex(6, 0, 0)
hexagon = makePolygon(P3, 1, 6, "hexagon")

P4 = geompy.MakeVertex(8, 0, 0)
heptagon = makePolygon(P4, 1, 7, "heptagon")

P5 = geompy.MakeVertex(10, 0, 0)
octagon = makePolygon(P5, 1, 7, "octagon")

polygons = [triangle, quadrangle, pentagon, hexagon, heptagon, octagon]
polyhedrons = []
for polygon in polygons:
  name = polygon.GetName()
  polygon_extruded = geompy.MakePrismVecH(polygon, OZ, height, theName="%s_prism"%name)
  polygon_summit = makeSummitSolid(polygon, -height, theName="%s_summit"%name)
  polyhedrons += [polygon_extruded, polygon_summit]

solids = geompy.MakeCompound(polyhedrons, theName="solids")


###
### SMESH component
###

import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New()

Mesh_1 = smesh.Mesh(solids)
Regular_1D = Mesh_1.Segment()
Number_of_Segments_1 = Regular_1D.NumberOfSegments(1)
PolygonPerFace_2D = Mesh_1.Polygon()
PolyhedronPerSolid_3D = Mesh_1.Polyhedron()

ok = Mesh_1.Compute()

if not ok:
  raise Exception("Error when computing Mesh")

# check volumes
vol_geom = geompy.BasicProperties(solids)[2]
vol_mesh = Mesh_1.GetVolume()

assert abs(vol_geom - vol_mesh) < 1e-12

assert Mesh_1.NbVolumes() == 12

# check type of elements
assert Mesh_1.NbTetras() == 1
assert Mesh_1.NbHexas() == 1
assert Mesh_1.NbPyramids() == 1
assert Mesh_1.NbPrisms() == 1
assert Mesh_1.NbHexagonalPrisms() == 1
assert Mesh_1.NbPolyhedrons() == 7


if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()
