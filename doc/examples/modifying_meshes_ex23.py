# Extrusion along a Path

import math

import salome
salome.salome_init_without_session()

import SMESH
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# 1. Create points
points = [[0, 0], [50, 30], [50, 110], [0, 150], [-80, 150], [-130, 70], [-130, -20]]

iv = 1
vertices = []
for point in points:
    vert = geom_builder.MakeVertex(point[0], point[1], 0)
    vertices.append(vert)
    iv += 1

# 2. Create edges and wires
Edge_straight = geom_builder.MakeEdge(vertices[0], vertices[4])
Edge_bezierrr = geom_builder.MakeBezier(vertices)
Wire_polyline = geom_builder.MakePolyline(vertices)
Edge_Circle   = geom_builder.MakeCircleThreePnt(vertices[0], vertices[1], vertices[2])

geom_builder.addToStudy(Edge_straight, "Edge_straight")
geom_builder.addToStudy(Edge_bezierrr, "Edge_bezierrr")
geom_builder.addToStudy(Wire_polyline, "Wire_polyline")
geom_builder.addToStudy(Edge_Circle  , "Edge_Circle")

# 3. Explode wire on edges, as they will be used for mesh extrusion
Wire_polyline_edges = geom_builder.SubShapeAll(Wire_polyline, geom_builder.ShapeType["EDGE"])
for ii in range(len(Wire_polyline_edges)):
    geom_builder.addToStudyInFather(Wire_polyline, Wire_polyline_edges[ii], "Edge_" + repr(ii + 1))

# Mesh

# Mesh the given shape with the given 1d hypothesis
def Mesh1D(shape1d, nbSeg, name):
  mesh1d_tool = smesh_builder.Mesh(shape1d, name)
  algo = mesh1d_tool.Segment()
  hyp  = algo.NumberOfSegments(nbSeg)
  isDone = mesh1d_tool.Compute()
  if not isDone: raise Exception('Mesh %s: computation failed' % name)
  return mesh1d_tool

# Create a mesh with six nodes, seven edges and two quadrangle faces
def MakeQuadMesh2(mesh_name):
  quad_1 = smesh_builder.Mesh(name = mesh_name)
  
  # six nodes
  n1 = quad_1.AddNode(0, 20, 10)
  n2 = quad_1.AddNode(0, 40, 10)
  n3 = quad_1.AddNode(0, 40, 30)
  n4 = quad_1.AddNode(0, 20, 30)
  n5 = quad_1.AddNode(0,  0, 30)
  n6 = quad_1.AddNode(0,  0, 10)

  # seven edges
  quad_1.AddEdge([n1, n2]) # 1
  quad_1.AddEdge([n2, n3]) # 2
  quad_1.AddEdge([n3, n4]) # 3
  quad_1.AddEdge([n4, n1]) # 4
  quad_1.AddEdge([n4, n5]) # 5
  quad_1.AddEdge([n5, n6]) # 6
  quad_1.AddEdge([n6, n1]) # 7

  # two quadrangle faces
  quad_1.AddFace([n1, n2, n3, n4]) # 8
  quad_1.AddFace([n1, n4, n5, n6]) # 9
  return [quad_1, [1,2,3,4,5,6,7], [8,9]]

# Path meshes
Edge_straight_mesh = Mesh1D(Edge_straight, 7, "Edge_straight")
Edge_bezierrr_mesh = Mesh1D(Edge_bezierrr, 7, "Edge_bezierrr")
Wire_polyline_mesh = Mesh1D(Wire_polyline, 3, "Wire_polyline")
Edge_Circle_mesh   = Mesh1D(Edge_Circle  , 8, "Edge_Circle")

# Initial meshes (to be extruded)
[quad_1, ee_1, ff_1] = MakeQuadMesh2("quad_1")
[quad_2, ee_2, ff_2] = MakeQuadMesh2("quad_2")
[quad_3, ee_3, ff_3] = MakeQuadMesh2("quad_3")
[quad_4, ee_4, ff_4] = MakeQuadMesh2("quad_4")
[quad_5, ee_5, ff_5] = MakeQuadMesh2("quad_5")
[quad_6, ee_6, ff_6] = MakeQuadMesh2("quad_6")
[quad_7, ee_7, ff_7] = MakeQuadMesh2("quad_7")

# ExtrusionAlongPath
# IDsOfElements, PathMesh, PathShape, NodeStart,
# HasAngles, Angles, HasRefPoint, RefPoint
refPoint = SMESH.PointStruct(0, 0, 0)
a10 = math.radians( 10.0 )
a45 = math.radians( 45.0 )

# 1. Extrusion of two mesh edges along a straight path
error = quad_1.ExtrusionAlongPath([1,2], Edge_straight_mesh, Edge_straight, 1,
                                  0, [], 0, refPoint)

# 2. Extrusion of one mesh edge along a curved path
error = quad_2.ExtrusionAlongPath([2], Edge_bezierrr_mesh, Edge_bezierrr, 1,
                                  0, [], 0, refPoint)

# 3. Extrusion of one mesh edge along a curved path with usage of angles
error = quad_3.ExtrusionAlongPath([2], Edge_bezierrr_mesh, Edge_bezierrr, 1,
                                  1, [a45, a45, a45, 0, -a45, -a45, -a45], 0, refPoint)

# 4. Extrusion of one mesh edge along the path, which is a part of a meshed wire
error = quad_4.ExtrusionAlongPath([4], Wire_polyline_mesh, Wire_polyline_edges[0], 1,
                                  1, [a10, a10, a10], 0, refPoint)

# 5. Extrusion of two mesh faces along the path, which is a part of a meshed wire
error = quad_5.ExtrusionAlongPath(ff_5 , Wire_polyline_mesh, Wire_polyline_edges[2], 4,
                                  0, [], 0, refPoint)

# 6. Extrusion of two mesh faces along a closed path
error = quad_6.ExtrusionAlongPath(ff_6 , Edge_Circle_mesh, Edge_Circle, 1,
                                  0, [], 0, refPoint)

# 7. Extrusion of two mesh faces along a closed path with usage of angles
error = quad_7.ExtrusionAlongPath(ff_7, Edge_Circle_mesh, Edge_Circle, 1,
                                  1, [a45, -a45, a45, -a45, a45, -a45, a45, -a45], 0, refPoint)



# Make the same meshes using a fully functional method ExtrusionAlongPathObjects() having
# the following arguments:
#   Nodes, Edges, Faces, PathObject, PathShape=None,
#   NodeStart=1, HasAngles=False, Angles=[], LinearVariation=False,
#   HasRefPoint=False, RefPoint=[0,0,0], MakeGroups=False,
#   ScaleFactors=[], ScalesVariation=False

quad_1 = MakeQuadMesh2("quad_1")[0]
quad_2 = MakeQuadMesh2("quad_2")[0]
quad_3 = MakeQuadMesh2("quad_3")[0]
quad_4 = MakeQuadMesh2("quad_4")[0]
quad_5 = MakeQuadMesh2("quad_5")[0]
quad_6 = MakeQuadMesh2("quad_6")[0]
quad_7 = MakeQuadMesh2("quad_7")[0]

# 1. Extrusion of two mesh edges along a straight path
nn, ee, ff = [], [1,2], []
error = quad_1.ExtrusionAlongPathObjects( nn, ee, ff, Edge_straight_mesh )

# 2. Extrusion of one mesh edge along a curved path
nn, ee, ff = [], [2], []
error = quad_2.ExtrusionAlongPathObjects( nn, ee, ff, Edge_bezierrr_mesh )

# 3. Extrusion of one mesh edge along a curved path with usage of angles
error = quad_3.ExtrusionAlongPathObjects( nn, ee, ff, Edge_bezierrr_mesh,
                                          Angles=[a45, a45, a45, 0, -a45, -a45, -a45])

# 4. Extrusion of one mesh edge along the path, which is a part of a meshed wire
nn, ee, ff = [], [4], []
error = quad_4.ExtrusionAlongPathObjects( nn, ee, ff, Wire_polyline_mesh, Wire_polyline_edges[0],
                                          Angles=[a10, a10, a10])

# 5. Extrusion of two mesh faces along the path, which is a part of a meshed wire
nn, ee, ff = [], [], quad_5
error = quad_5.ExtrusionAlongPathObjects( nn, ee, ff, Wire_polyline_mesh, Wire_polyline_edges[2],
                                          NodeStart=4 )

# 6. Extrusion of two mesh faces along a closed path
nn, ee, ff = [], [], quad_6
error = quad_6.ExtrusionAlongPathObjects( nn, ee, ff, Edge_Circle_mesh )

# 7. Extrusion of two mesh faces along a closed path with usage of angles
nn, ee, ff = [], [], quad_7
error = quad_7.ExtrusionAlongPathObjects( nn, ee, ff, Edge_Circle_mesh, Edge_Circle,
                                          Angles=[a45, -a45, a45, -a45, a45, -a45, a45, -a45])
