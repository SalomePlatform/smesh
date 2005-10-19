#####################################################################
#Created                :17/02/2005
#Auhtor                 :MASLOV Eugeny, KOVALTCHUK Alexey 
#####################################################################

import geompy
import salome
import os
import math
import StdMeshers
import SMESH

#Sketcher_1 creation
print "Sketcher creation..."
Sketcher_1 = geompy.MakeSketcher("Sketcher:F 100 -57.7:TT 100 57.7:TT 0 115.47:TT -100 57.7:TT -100 -57.7:TT 0 -115.47:WW") 
geompy.addToStudy(Sketcher_1, "Sketcher_1")
Face_1 = geompy.MakeFace(Sketcher_1, 1)
geompy.addToStudy(Face_1, "Face_1")

#Line creation
print "Line  creation..."
Line_1 = geompy.MakeLineTwoPnt(geompy.MakeVertex(0,0,0), geompy.MakeVertex(0,0,100))
geompy.addToStudy(Line_1, "Line_1")

#Prism creation
print "Prism creation..."
Prism_1 = geompy.MakePrismVecH(Face_1, Line_1, 100)
geompy.addToStudy(Prism_1, "Prism_1")

#Sketcher_2 creation
print "Sketcher creation..."
Sketcher_2 = geompy.MakeSketcher("Sketcher:F 50 0:TT 80 0:TT 112 13:TT 112 48:TT 80 63:TT 80 90:TT 50 90:WW", [0,0,0, 1,0,0, 0,1,0]) 
geompy.addToStudy(Sketcher_2, "Sketcher_2")
Face_2 = geompy.MakeFace(Sketcher_2, 1)
geompy.addToStudy(Face_2, "Face_2")

#Revolution creation
print "Revolution creation..."
Revolution_1 = geompy.MakeRevolution(Face_2, Line_1, 2*math.pi)
geompy.addToStudy(Revolution_1, "Revolution_1")

#Common applying
print "Common of Revolution and Prism..."
Common_1 = geompy.MakeBoolean(Revolution_1, Prism_1, 1)
geompy.addToStudy(Common_1, "Common_1")

#Explode Common_1 on edges
CommonExplodedListEdges = geompy.SubShapeAll(Common_1, geompy.ShapeType["EDGE"])
for i in range(0, len(CommonExplodedListEdges)):
    name = "Edge_"+str(i+1)
    geompy.addToStudyInFather(Common_1, CommonExplodedListEdges[i], name)

#Fillet applying
print "Fillet creation..."
Fillet_1 = geompy.MakeFillet(Common_1, 10, geompy.ShapeType["EDGE"], [6])
geompy.addToStudy(Fillet_1, "Fillet_1")

#Chamfer applying
print "Chamfer creation..."
Chamfer_1 = geompy.MakeChamferEdge(Fillet_1, 10, 10, 16, 50 )
geompy.addToStudy(Chamfer_1, "Chamfer_1")
Chamfer_2 = geompy.MakeChamferEdge(Chamfer_1, 10, 10, 21, 31 )
geompy.addToStudy(Chamfer_2, "Chamfer_2")

#Import of the shape from "slots.brep"
print "Import multi-rotation from the DATA_DIR/Shapes/Brep/slots.brep"
thePath = os.getenv("DATA_DIR")
theFileName = thePath + "/Shapes/Brep/slots.brep"
theShapeForCut = geompy.ImportBREP(theFileName)
geompy.addToStudy(theShapeForCut, "slot.brep_1")

#Cut applying
print "Cut..."
Cut_1 = geompy.MakeBoolean(Chamfer_2, theShapeForCut, 2)
Cut_1_ID = geompy.addToStudy(Cut_1, "Cut_1")

#Mesh creation
smesh = salome.lcc.FindOrLoadComponent("FactoryServer", "SMESH")

# -- Init --
shape_mesh = salome.IDToObject( Cut_1_ID )
smesh.SetCurrentStudy(salome.myStudy)
mesh = smesh.CreateMesh(shape_mesh)
smeshgui = salome.ImportComponentGUI("SMESH")
smeshgui.Init(salome.myStudyId)
idmesh = salome.ObjectToID(mesh)
smeshgui.SetName( idmesh, "Nut" )

#HYPOTHESIS CREATION
print "-------------------------- Average length"
theAverageLength = 5
hAvLength = smesh.CreateHypothesis( "LocalLength", "libStdMeshersEngine.so" )
hAvLength.SetLength( theAverageLength )
print hAvLength.GetName()
print hAvLength.GetId()
smeshgui.SetName(salome.ObjectToID(hAvLength), "AverageLength_5")

print "-------------------------- MaxElementArea"
theMaxElementArea = 20
hArea20 = smesh.CreateHypothesis( "MaxElementArea", "libStdMeshersEngine.so" )
hArea20.SetMaxElementArea( theMaxElementArea )
print hArea20.GetName()
print hArea20.GetId()
print hArea20.GetMaxElementArea()
smeshgui.SetName(salome.ObjectToID(hArea20), "MaxElementArea_20")

print "-------------------------- MaxElementVolume"
theMaxElementVolume = 150
hVolume150 = smesh.CreateHypothesis( "MaxElementVolume", "libStdMeshersEngine.so" )
hVolume150.SetMaxElementVolume( theMaxElementVolume )
print hVolume150.GetName()
print hVolume150.GetId()
print hVolume150.GetMaxElementVolume()
smeshgui.SetName(salome.ObjectToID(hVolume150), "MaxElementVolume_150")

mesh.AddHypothesis(shape_mesh, hAvLength)
mesh.AddHypothesis(shape_mesh, hArea20)
mesh.AddHypothesis(shape_mesh, hVolume150)

print "-------------------------- Regular_1D"

algoReg1D = smesh.CreateHypothesis( "Regular_1D", "libStdMeshersEngine.so" )
listHyp = algoReg1D.GetCompatibleHypothesis()
for hyp in listHyp:
    print hyp
print algoReg1D.GetName()
print algoReg1D.GetId()
smeshgui.SetName(salome.ObjectToID(algoReg1D), "Wire discretisation")

print "-------------------------- MEFISTO_2D"
algoMef = smesh.CreateHypothesis( "MEFISTO_2D", "libStdMeshersEngine.so" )
listHyp = algoMef.GetCompatibleHypothesis()
for hyp in listHyp:
    print hyp
print algoMef.GetName()
print algoMef.GetId()
smeshgui.SetName(salome.ObjectToID(algoMef), "Triangle (Mefisto)")

print "-------------------------- NETGEN_3D"

algoNg = smesh.CreateHypothesis( "NETGEN_3D", "libNETGENEngine.so" )
print algoNg.GetName()
print algoNg.GetId()
smeshgui.SetName(salome.ObjectToID(algoNg), "Tetrahedron (NETGEN)")
mesh.AddHypothesis(shape_mesh, algoReg1D)
mesh.AddHypothesis(shape_mesh, algoMef)
mesh.AddHypothesis(shape_mesh, algoNg)

print "-------------------------- compute the mesh of the mechanic piece"
smesh.Compute(mesh,shape_mesh)

print "Information about the Nut:"
print "Number of nodes       : ", mesh.NbNodes()
print "Number of edges       : ", mesh.NbEdges()
print "Number of faces       : ", mesh.NbFaces()
print "Number of triangles   : ", mesh.NbTriangles()
print "Number of quadrangles : ", mesh.NbQuadrangles()
print "Number of volumes     : ", mesh.NbVolumes()
print "Number of tetrahedrons: ", mesh.NbTetras()

salome.sg.updateObjBrowser(1)
