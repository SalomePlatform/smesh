
import sys
import salome
import geompy
import math
import SALOMEDS
import SMESH
import smesh

salome.salome_init()
aStudyId = salome.myStudy._get_StudyId()

geompy.init_geom(salome.myStudy)
global Face_1
Face_1 = geompy.MakeFaceHW(100, 100, 1)
geompy.addToStudy( Face_1, "Face_1" )

#smesh.smesh.SetCurrentStudy(aStudyId)
import StdMeshers
pattern = smesh.GetPattern()
Mesh_1 = smesh.Mesh(Face_1)
Regular_1D = Mesh_1.Segment()
Nb_Segments_1 = Regular_1D.NumberOfSegments(10)
Nb_Segments_1.SetDistrType( 0 )
Quadrangle_2D = Mesh_1.Quadrangle()
isDone = Mesh_1.Compute()

# groups creation

aListOfElems = [ 52, 53, 54, 55, 56, 57,
                 62, 63, 64, 65, 66, 67,
                 72, 73, 74, 75, 76, 77,
                 82, 83, 84, 85, 86, 87 ]
                 
aRedGroup = Mesh_1.GetMesh().CreateGroup( smesh.FACE, "Red" )
aRedGroup.Add( aListOfElems );
aRedGroup.SetColor( SALOMEDS.Color( 1, 0, 0 ) )

aListOfElems = [ 55, 56, 57, 58, 59,
                 65, 66, 67, 68, 69,
                 75, 76, 77, 78, 79,
                 85, 86, 87, 88, 89,
                 95, 96, 97, 98, 99,
                 105, 106, 107, 108, 109,
                 115, 116, 117, 118, 119,
                 125, 126, 127, 128, 129 ]
                 
aGreenGroup = Mesh_1.GetMesh().CreateGroup( smesh.FACE, "Green" )
aGreenGroup.Add( aListOfElems );
aGreenGroup.SetColor( SALOMEDS.Color( 0, 1, 0 ) )

aListOfElems = [ 63, 64, 65, 66, 67, 68, 
                 73, 74, 75, 76, 77, 78,
                 83, 84, 85, 86, 87, 88, 
                 93, 94, 95, 96, 97, 98, 
                 103, 104, 105, 106, 107, 108, 
                 113, 114, 115, 116, 117, 118 ]
                 
aBlueGroup = Mesh_1.GetMesh().CreateGroup( smesh.FACE, "Blue" )
aBlueGroup.Add( aListOfElems );
aBlueGroup.SetColor( SALOMEDS.Color( 0, 0, 1 ) )

# UnionListOfGroups()
aUnGrp = Mesh_1.UnionListOfGroups([aRedGroup, aGreenGroup, aBlueGroup], "UnionGrp" )

# IntersectListOfGroups()
aIntGrp=Mesh_1.IntersectListOfGroups([aRedGroup, aGreenGroup, aBlueGroup], "IntGrp" )

# CutListOfGroups()
aCutGrp=Mesh_1.CutListOfGroups([aRedGroup],[aGreenGroup,aBlueGroup],"CutGrp")

salome.sg.updateObjBrowser( 1 )

