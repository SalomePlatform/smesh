import salome
from geompy import gg
import geompy
import SMESH

import StdMeshers

ShapeTypeCompSolid = 1
ShapeTypeSolid = 2
ShapeTypeShell = 3
ShapeTypeFace = 4
ShapeTypeWire = 5
ShapeTypeEdge = 6
ShapeTypeVertex = 7

geom  = salome.lcc.FindOrLoadComponent("FactoryServer", "GEOM")
smesh = salome.lcc.FindOrLoadComponent("FactoryServer", "SMESH")

geom.GetCurrentStudy(salome.myStudy._get_StudyId())
smesh.SetCurrentStudy(salome.myStudy)

# Create box without one plane

box = geompy.MakeBox(0., 0., 0., 10., 20., 30.)
subShapeList = geompy.SubShapeAll(box,ShapeTypeFace)

FaceList  = []
for i in range( 5 ):
  FaceList.append( subShapeList[ i ]._get_Name() )

aBox = geompy.MakeSewing( FaceList, 1. )
idbox = geompy.addToStudy( aBox, "box" )
  
aBox  = salome.IDToObject( idbox )

# Create mesh

hyp1 = smesh.CreateHypothesis("NumberOfSegments", "libStdMeshersEngine.so")
hyp1.SetNumberOfSegments(5)
hyp2 = smesh.CreateHypothesis("MaxElementArea", "libStdMeshersEngine.so")
hyp2.SetMaxElementArea(20)
hyp3 = smesh.CreateHypothesis("MaxElementArea", "libStdMeshersEngine.so")
hyp3.SetMaxElementArea(50)

algo1 = smesh.CreateHypothesis("Regular_1D", "libStdMeshersEngine.so")
algo2 = smesh.CreateHypothesis("MEFISTO_2D", "libStdMeshersEngine.so")

mesh = smesh.CreateMesh(aBox)
mesh.AddHypothesis(aBox,hyp1)
mesh.AddHypothesis(aBox,hyp2)
mesh.AddHypothesis(aBox,algo1)
mesh.AddHypothesis(aBox,algo2)

smesh.Compute(mesh,aBox)

smeshgui = salome.ImportComponentGUI("SMESH")
smeshgui.Init(salome.myStudyId);
smeshgui.SetName( salome.ObjectToID( mesh ), "Mesh_freebord" );

# Criterion : Free edges
aFilterMgr = smesh.CreateFilterManager()
aPredicate = aFilterMgr.CreateFreeBorders()
aFilter = aFilterMgr.CreateFilter()
aFilter.SetPredicate( aPredicate )

anIds = aFilter.GetElementsId( mesh )

# print result
print "Criterion: Free edges Nb = ", len( anIds )
for i in range( len( anIds ) ):
  print anIds[ i ]

# create group
aGroup = mesh.CreateGroup( SMESH.EDGE, "Free edges" )
aGroup.Add( anIds )


salome.sg.updateObjBrowser(1)
