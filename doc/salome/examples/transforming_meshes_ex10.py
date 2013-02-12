# Sew Side Elements

import geompy
import smesh

# create two boxes
box1 = geompy.MakeBox(0.,  0., 0., 10., 10., 10.)
box2 = geompy.MakeBox(0., 15., 0., 20., 25., 10.)

EdgesList = geompy.SubShapeAll(box2, geompy.ShapeType["EDGE"])

aComp = geompy.MakeCompound([box1, box2])
geompy.addToStudy(aComp, "Two boxes")

# create a mesh on two boxes
mesh = smesh.Mesh(aComp, "Two faces : quadrangle mesh")

algo1D = mesh.Segment()
algo1D.NumberOfSegments(2)
algo2D = mesh.Quadrangle()

algo_local = mesh.Segment(EdgesList[8])
algo_local.NumberOfSegments(4)
algo_local.Propagation()

mesh.Compute()

# sew side elements
# IDsOfSide1Elements, IDsOfSide2Elements,
# NodeID1OfSide1ToMerge, NodeID1OfSide2ToMerge, NodeID2OfSide1ToMerge, NodeID2OfSide2ToMerge
mesh.SewSideElements([69, 70, 71, 72], [91, 92, 89, 90], 8, 38, 23, 58)
