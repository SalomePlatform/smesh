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

box   = geompy.MakeBox(0., 0., 0., 100., 200., 300.)
idbox = geompy.addToStudy(box,"box")

subShapeList = geompy.SubShapeAll(box,ShapeTypeFace)
face   = subShapeList[0]
name   = geompy.SubShapeName( face._get_Name(), box._get_Name() )
idface = geompy.addToStudyInFather(box,face,name)

box  = salome.IDToObject(idbox)
face = salome.IDToObject(idface)

hyp1 = smesh.CreateHypothesis("NumberOfSegments", "libStdMeshersEngine.so")
hyp1.SetNumberOfSegments(10)
hyp2 = smesh.CreateHypothesis("MaxElementArea", "libStdMeshersEngine.so")
hyp2.SetMaxElementArea(10)
hyp3 = smesh.CreateHypothesis("MaxElementArea", "libStdMeshersEngine.so")
hyp3.SetMaxElementArea(100)

algo1 = smesh.CreateHypothesis("Regular_1D", "libStdMeshersEngine.so")
algo2 = smesh.CreateHypothesis("MEFISTO_2D", "libStdMeshersEngine.so")

mesh = smesh.CreateMesh(box)
mesh.AddHypothesis(box,hyp1)
mesh.AddHypothesis(box,hyp2)
mesh.AddHypothesis(box,algo1)
mesh.AddHypothesis(box,algo2)

submesh = mesh.GetSubMesh(face, "SubMeshFace")
mesh.AddHypothesis(face,hyp1)
mesh.AddHypothesis(face,hyp3)
mesh.AddHypothesis(face,algo1)
mesh.AddHypothesis(face,algo2)

smesh.Compute(mesh,box)

faces = submesh.GetElementsByType(SMESH.FACE)
if len(faces) > 1:
    print len(faces), len(faces)/2
    group1 = mesh.CreateGroup(SMESH.FACE,"Group of faces")
    group2 = mesh.CreateGroup(SMESH.FACE,"Another group of faces")
    group1.Add(faces[:int(len(faces)/2)])
    group2.Add(faces[int(len(faces)/2):])

salome.sg.updateObjBrowser(1)
