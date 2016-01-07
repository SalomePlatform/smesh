# Sew Side Elements

import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

# create two boxes
box1 = geompy.MakeBox(0.,  0., 0., 10., 10., 10.)
box2 = geompy.MakeBox(0., 15., 0., 20., 25., 10.)

EdgesList = geompy.SubShapeAll(box2, geompy.ShapeType["EDGE"])

aComp = geompy.MakeCompound([box1, box2])
geompy.addToStudy(aComp, "Two boxes")

# create a mesh on two boxes
mesh = smesh.Mesh(aComp, "Sew Side Elements")

algo1D = mesh.Segment()
algo1D.NumberOfSegments(2)
algo2D = mesh.Quadrangle()

algo_local = mesh.Segment(EdgesList[8])
algo_local.NumberOfSegments(4)
algo_local.Propagation()

mesh.Compute()

# sew side elements

# find elements to sew
face1 = geompy.GetFaceNearPoint( aComp, geompy.MakeVertex( 5, 10, 5 ))
IDsOfSide1Elements = mesh.GetSubMeshElementsId( face1 )
print "side faces 1:",IDsOfSide1Elements

face1Translated = geompy.MakeTranslation( face1, 0,5,0 )
faceFilter = smesh.GetFilter( SMESH.FACE, SMESH.FT_BelongToGeom,'=', face1Translated )
IDsOfSide2Elements = mesh.GetIdsFromFilter( faceFilter )
print "side faces 2:",IDsOfSide2Elements

# find corresponding nodes on sides
edge1 = geompy.GetEdgeNearPoint( aComp, geompy.MakeVertex( 0, 10, 5 ))
segs1 = mesh.GetSubMeshElementsId( edge1 ) # mesh segments generated on edge1
NodeID1OfSide1ToMerge = mesh.GetElemNode( segs1[0], 0 )
NodeID2OfSide1ToMerge = mesh.GetElemNode( segs1[0], 1 )
print "nodes of side1:", [NodeID1OfSide1ToMerge,NodeID2OfSide1ToMerge]

edge2 = geompy.GetEdgeNearPoint( aComp, geompy.MakeVertex( 0, 15, 5 ))
segs2 = mesh.GetSubMeshElementsId( edge2 ) # mesh segments generated on edge2
NodeID1OfSide2ToMerge = mesh.GetElemNode( segs2[0], 0 )
NodeID2OfSide2ToMerge = mesh.GetElemNode( segs2[0], 1 )
print "nodes of side2:", [NodeID1OfSide2ToMerge,NodeID2OfSide2ToMerge]

res = mesh.SewSideElements(IDsOfSide1Elements, IDsOfSide2Elements,
                           NodeID1OfSide1ToMerge, NodeID1OfSide2ToMerge,
                           NodeID2OfSide1ToMerge, NodeID2OfSide2ToMerge)
print res
