import SMESH
import smeshpy
import salome
from salome import sg
import math
#import SMESH_BasicHypothesis_idl

import geompy

geom = salome.lcc.FindOrLoadComponent("FactoryServer", "Geometry")
myBuilder = salome.myStudy.NewBuilder()
from geompy import gg

smeshgui = salome.ImportComponentGUI("SMESH")
smeshgui.Init(salome.myStudyId);

ShapeTypeCompSolid = 1
ShapeTypeSolid = 2
ShapeTypeShell = 3
ShapeTypeFace = 4
ShapeTypeWire = 5
ShapeTypeEdge = 6
ShapeTypeVertex = 7

# ---- define a box

box = geompy.MakeBox(0., 0., 0., 100., 200., 300.)
idbox = geompy.addToStudy(box,"box")

# ---- add first face of box in study

subShapeList=geompy.SubShapeAll(box,ShapeTypeFace)
face=subShapeList[0]
name = geompy.SubShapeName( face._get_Name(), box._get_Name() )
print name
idface=geompy.addToStudyInFather(box,face,name)

# ---- add shell from box  in study

subShellList=geompy.SubShapeAll(box,ShapeTypeShell)
shell = subShellList[0]
name = geompy.SubShapeName( shell._get_Name(), box._get_Name() )
print name
idshell=geompy.addToStudyInFather(box,shell,name)

# ---- add first edge of face in study

edgeList = geompy.SubShapeAll(face,ShapeTypeEdge)
edge=edgeList[0];
name = geompy.SubShapeName( edge._get_Name(), face._get_Name() )
print name
idedge=geompy.addToStudyInFather(face,edge,name)

# ---- launch SMESH, init a Mesh with the box
gen=smeshpy.smeshpy()
mesh=gen.Init(idbox)

idmesh = smeshgui.AddNewMesh( salome.orb.object_to_string(mesh) )
smeshgui.SetName(idmesh, "Meshbox");
smeshgui.SetShape(idbox, idmesh);

# ---- create Hypothesis

print "-------------------------- create Hypothesis"
print "-------------------------- LocalLength"
hyp1=gen.CreateHypothesis("LocalLength")
hypLen1 = hyp1._narrow(SMESH.SMESH_LocalLength)
hypLen1.SetLength(100)
print hypLen1.GetName()
print hypLen1.GetId()
print hypLen1.GetLength()

idlength = smeshgui.AddNewHypothesis( salome.orb.object_to_string(hypLen1) );
smeshgui.SetName(idlength, "Local_Length_100");

print "-------------------------- NumberOfSegments"
hyp2=gen.CreateHypothesis("NumberOfSegments")
hypNbSeg1=hyp2._narrow(SMESH.SMESH_NumberOfSegments)
hypNbSeg1.SetNumberOfSegments(7)
print hypNbSeg1.GetName()
print hypNbSeg1.GetId()
print hypNbSeg1.GetNumberOfSegments()

idseg = smeshgui.AddNewHypothesis( salome.orb.object_to_string(hypNbSeg1) );
smeshgui.SetName(idseg, "NumberOfSegments_7");

print "-------------------------- MaxElementArea"
hyp3=gen.CreateHypothesis("MaxElementArea")
hypArea1=hyp3._narrow(SMESH.SMESH_MaxElementArea)
hypArea1.SetMaxElementArea(2500)
print hypArea1.GetName()
print hypArea1.GetId()
print hypArea1.GetMaxElementArea()

idarea1 = smeshgui.AddNewHypothesis( salome.orb.object_to_string(hypArea1) );
smeshgui.SetName(idarea1, "MaxElementArea_2500");

print "-------------------------- MaxElementArea"
hyp3=gen.CreateHypothesis("MaxElementArea")
hypArea2=hyp3._narrow(SMESH.SMESH_MaxElementArea)
hypArea2.SetMaxElementArea(500)
print hypArea2.GetName()
print hypArea2.GetId()
print hypArea2.GetMaxElementArea()

idarea2 = smeshgui.AddNewHypothesis( salome.orb.object_to_string(hypArea2) );
smeshgui.SetName(idarea2, "MaxElementArea_500");

print "-------------------------- Regular_1D"
alg1=gen.CreateHypothesis("Regular_1D")
algo1=alg1._narrow(SMESH.SMESH_Algo)
listHyp=algo1.GetCompatibleHypothesis()
for hyp in listHyp:
    print hyp
algoReg=alg1._narrow(SMESH.SMESH_Regular_1D)
print algoReg.GetName()
print algoReg.GetId()

idreg = smeshgui.AddNewAlgorithms( salome.orb.object_to_string(algoReg) );
smeshgui.SetName(idreg, "Regular_1D");

print "-------------------------- MEFISTO_2D"
alg2=gen.CreateHypothesis("MEFISTO_2D")
algo2=alg2._narrow(SMESH.SMESH_Algo)
listHyp=algo2.GetCompatibleHypothesis()
for hyp in listHyp:
    print hyp
algoMef=alg2._narrow(SMESH.SMESH_MEFISTO_2D)
print algoMef.GetName()
print algoMef.GetId()

idmef = smeshgui.AddNewAlgorithms( salome.orb.object_to_string(algoMef) );
smeshgui.SetName(idmef, "MEFISTO_2D");

# ---- add hypothesis to edge

print "-------------------------- add hypothesis to edge"
edge=salome.IDToObject(idedge)
submesh=mesh.GetElementsOnShape(edge)
ret=mesh.AddHypothesis(edge,algoReg)
print ret
ret=mesh.AddHypothesis(edge,hypLen1)
print ret

idsm1 = smeshgui.AddSubMeshOnShape( idmesh,
                                    idedge,
                                    salome.orb.object_to_string(submesh),
                                    ShapeTypeEdge )
smeshgui.SetName(idsm1, "SubMeshEdge")
smeshgui.SetAlgorithms( idsm1, idreg );
smeshgui.SetHypothesis( idsm1, idlength );

print "-------------------------- add hypothesis to face"
face=salome.IDToObject(idface)
submesh=mesh.GetElementsOnShape(face)
ret=mesh.AddHypothesis(face,hypArea2)
print ret

idsm2 = smeshgui.AddSubMeshOnShape( idmesh,
                                    idface,
                                    salome.orb.object_to_string(submesh),
                                    ShapeTypeFace )
smeshgui.SetName(idsm2, "SubMeshFace")
smeshgui.SetHypothesis( idsm2, idarea2 );

# ---- add hypothesis to box

print "-------------------------- add hypothesis to box"
box=salome.IDToObject(idbox)
submesh=mesh.GetElementsOnShape(box)
ret=mesh.AddHypothesis(box,algoReg)
print ret
ret=mesh.AddHypothesis(box,hypNbSeg1)
print ret
ret=mesh.AddHypothesis(box,algoMef)
print ret
ret=mesh.AddHypothesis(box,hypArea1)
print ret

smeshgui.SetAlgorithms( idmesh, idreg );
smeshgui.SetHypothesis( idmesh, idseg );
smeshgui.SetAlgorithms( idmesh, idmef );
smeshgui.SetHypothesis( idmesh, idarea1 );

sg.updateObjBrowser(1);
