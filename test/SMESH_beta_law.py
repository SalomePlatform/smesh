#!/usr/bin/env python

from salome.kernel import salome

salome.salome_init()

###
### GEOM component
###

from salome.geom import geomBuilder

geompy = geomBuilder.New()

p1 = geompy.MakeVertex(0, 0, 0, theName="p1")
p2 = geompy.MakeVertex(.1, 0, 0, theName="p2")
p3 = geompy.MakeVertex(.1, .3, 0, theName="p3")
p4 = geompy.MakeVertex(0, .3, 0, theName="p4")

p5 = geompy.MakeVertex(-0.05, 0.1, 0, theName="p5")
p6 = geompy.MakeVertex(-0.05, 0.05, 0, theName="p6")

points = [p1, p2, p3, p4, p5, p6, p1]

polyline = geompy.MakePolyline(points, theName="polyline")

face = geompy.MakeFace(polyline, 1, theName="face")

edges_by_name = {}
for i in range(len(points)-1):
  name = "edge_%i"%(i+1)
  edge = geompy.GetEdge(face, points[i], points[i+1], theName=name)
  edges_by_name[name] = edge

###
### SMESH component
###

from salome.kernel import SMESH
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New()

Mesh_1 = smesh.Mesh(face,'Mesh_1')

main_algo1D = Mesh_1.Segment()
main_algo1D.NumberOfSegments(5)

sub_algo1D = Mesh_1.Segment(edges_by_name["edge_1"])
nb_seg_hyp = sub_algo1D.NumberOfSegments(29)
nb_seg_hyp.SetDistrType( 4 )
nb_seg_hyp.SetBeta( 1.001 )


sub_algo1D = Mesh_1.Segment(edges_by_name["edge_2"])
nb_seg_hyp = sub_algo1D.NumberOfSegments(19)

sub_algo1D = Mesh_1.Segment(edges_by_name["edge_3"])
nb_seg_hyp = sub_algo1D.NumberOfSegments(29)
nb_seg_hyp.SetDistrType( 4 )
nb_seg_hyp.SetBeta( -1.001 )

sub_algo1D = Mesh_1.Segment(edges_by_name["edge_4"])
nb_seg_hyp = sub_algo1D.NumberOfSegments(9)

Quadrangle_2D = Mesh_1.Quadrangle()

isDone = Mesh_1.Compute()
Mesh_1.CheckCompute()

gr_edges_by_name = {}
for name, edge in edges_by_name.items():
  gr_edge = Mesh_1.Group(edge)
  gr_edges_by_name[name] = gr_edge

assert Mesh_1.NbEdges() == 5+5+29*2+19+9
assert Mesh_1.NbQuadrangles() == 551

expected_mini = 2.9948449467664952e-05
expected_maxi = 0.013044372341778604
mini_1, maxi_1 = Mesh_1.GetMinMax(SMESH.FT_Length, meshPart=gr_edges_by_name["edge_1"])
mini_3, maxi_3 = Mesh_1.GetMinMax(SMESH.FT_Length, meshPart=gr_edges_by_name["edge_3"])

assert abs(expected_mini-mini_1)/mini_1 < 1e-12
assert abs(expected_mini-mini_3)/mini_3 < 1e-12

assert abs(expected_maxi-maxi_1)/maxi_1 < 1e-12
assert abs(expected_maxi-maxi_3)/maxi_3 < 1e-12


if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()
