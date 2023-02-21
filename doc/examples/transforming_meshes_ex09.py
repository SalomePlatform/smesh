# Sew Free Borders

import salome
salome.salome_init_without_session()

from salome.geom import geomBuilder
from salome.smesh import smeshBuilder

geom_builder = geomBuilder.New()
smesh_builder = smeshBuilder.New()

# make two not sewed quadranges
OY0 = geom_builder.MakeVectorDXDYDZ(0, 1, 0)
OY1 = geom_builder.MakeTranslation( OY0, 1, 0, 0, theName="OY1" )
OY2 = geom_builder.MakeTranslation( OY0, 1.01, 0, 0, theName="OY2" )
OY3 = geom_builder.MakeTranslation( OY0, 2, 0, 0 )
q1  = geom_builder.MakeQuad2Edges( OY0, OY1 )
q2  = geom_builder.MakeQuad2Edges( OY2, OY3 )

shape = geom_builder.MakeCompound( [q1,q2], theName='shape' )

# make a non-uniform quadrangle mesh on two faces
mesh = smesh_builder.Mesh(shape, "Two faces : quadrangle mesh")
mesh.Segment().Arithmetic1D( 0.1, 0.4 )
mesh.Segment(q1).NumberOfSegments( 5 )
mesh.Quadrangle()
if not mesh.Compute(): raise Exception("Error when computing Mesh")

# sew free borders

segs1 = mesh.GetSubMeshElementsId( OY1 ) # mesh segments generated on borders
segs2 = mesh.GetSubMeshElementsId( OY2 )

FirstNodeID1  = mesh.GetElemNode( segs1[0], 0 )
SecondNodeID1 = mesh.GetElemNode( segs1[0], 1 )
LastNodeID1   = mesh.GetElemNode( segs1[-1], 1 )
FirstNodeID2  = mesh.GetElemNode( segs2[0], 0 )
SecondNodeID2 = mesh.GetElemNode( segs2[0], 1 )
LastNodeID2   = mesh.GetElemNode( segs2[-1], 1 )
CreatePolygons = True
CreatePolyedrs = False

res = mesh.SewFreeBorders(FirstNodeID1, SecondNodeID1, LastNodeID1,
                          FirstNodeID2, SecondNodeID2, LastNodeID2,
                          CreatePolygons, CreatePolyedrs )
print(res)
print("nb polygons:", mesh.NbPolygons())
