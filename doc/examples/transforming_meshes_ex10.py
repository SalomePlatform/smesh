# Sew Side Elements

import salome
salome.salome_init_without_session()

import SMESH
from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# create two boxes
box1 = geom_builder.MakeBox(0.,  0., 0., 10., 10., 10.)
box2 = geom_builder.MakeBox(0., 15., 0., 20., 25., 10.)

EdgesList = geom_builder.SubShapeAll(box2, geom_builder.ShapeType["EDGE"])

aComp = geom_builder.MakeCompound([box1, box2])
geom_builder.addToStudy(aComp, "Two boxes")

# create a mesh on two boxes
mesh = smesh_builder.Mesh(aComp, "Sew Side Elements")

algo1D = mesh.Segment()
algo1D.NumberOfSegments(2)
algo2D = mesh.Quadrangle()

algo_local = mesh.Segment(EdgesList[8])
algo_local.NumberOfSegments(4)
algo_local.Propagation()

if not mesh.Compute(): raise Exception("Error when computing Mesh")

# sew side elements

# find elements to sew
face1 = geom_builder.GetFaceNearPoint( aComp, geom_builder.MakeVertex( 5, 10, 5 ))
IDsOfSide1Elements = mesh.GetSubMeshElementsId( face1 )
print("side faces 1:",IDsOfSide1Elements)

face1Translated = geom_builder.MakeTranslation( face1, 0,5,0 )
faceFilter = smesh_builder.GetFilter( SMESH.FACE, SMESH.FT_BelongToGeom,'=', face1Translated )
IDsOfSide2Elements = mesh.GetIdsFromFilter( faceFilter )
print("side faces 2:",IDsOfSide2Elements)

# find corresponding nodes on sides
edge1 = geom_builder.GetEdgeNearPoint( aComp, geom_builder.MakeVertex( 0, 10, 5 ))
segs1 = mesh.GetSubMeshElementsId( edge1 ) # mesh segments generated on edge1
NodeID1OfSide1ToMerge = mesh.GetElemNode( segs1[0], 0 )
NodeID2OfSide1ToMerge = mesh.GetElemNode( segs1[0], 1 )
print("nodes of side1:", [NodeID1OfSide1ToMerge,NodeID2OfSide1ToMerge])

edge2 = geom_builder.GetEdgeNearPoint( aComp, geom_builder.MakeVertex( 0, 15, 5 ))
segs2 = mesh.GetSubMeshElementsId( edge2 ) # mesh segments generated on edge2
NodeID1OfSide2ToMerge = mesh.GetElemNode( segs2[0], 0 )
NodeID2OfSide2ToMerge = mesh.GetElemNode( segs2[0], 1 )
print("nodes of side2:", [NodeID1OfSide2ToMerge,NodeID2OfSide2ToMerge])

res = mesh.SewSideElements(IDsOfSide1Elements, IDsOfSide2Elements,
                           NodeID1OfSide1ToMerge, NodeID1OfSide2ToMerge,
                           NodeID2OfSide1ToMerge, NodeID2OfSide2ToMerge)
print(res)
