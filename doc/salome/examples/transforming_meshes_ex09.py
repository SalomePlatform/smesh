# Sew Free Borders


import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

# make two not sewed quadranges
OY0 = geompy.MakeVectorDXDYDZ(0, 1, 0)
OY1 = geompy.MakeTranslation( OY0, 1, 0, 0, theName="OY1" )
OY2 = geompy.MakeTranslation( OY0, 1.01, 0, 0, theName="OY2" )
OY3 = geompy.MakeTranslation( OY0, 2, 0, 0 )
q1  = geompy.MakeQuad2Edges( OY0, OY1 )
q2  = geompy.MakeQuad2Edges( OY2, OY3 )

shape = geompy.MakeCompound( [q1,q2], theName='shape' )

# make a non-uniform quadrangle mesh on two faces
mesh = smesh.Mesh(shape, "Two faces : quadrangle mesh")
mesh.Segment().Arithmetic1D( 0.1, 0.4 )
mesh.Segment(q1).NumberOfSegments( 5 )
mesh.Quadrangle()
mesh.Compute()

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
print res
print "nb polygons:", mesh.NbPolygons()

