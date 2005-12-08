#  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
#  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
# 
#  This library is free software; you can redistribute it and/or 
#  modify it under the terms of the GNU Lesser General Public 
#  License as published by the Free Software Foundation; either 
#  version 2.1 of the License. 
# 
#  This library is distributed in the hope that it will be useful, 
#  but WITHOUT ANY WARRANTY; without even the implied warranty of 
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
#  Lesser General Public License for more details. 
# 
#  You should have received a copy of the GNU Lesser General Public 
#  License along with this library; if not, write to the Free Software 
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA 
# 
#  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
#
#
#
#  File   : SMESH_reg.py
#  Module : SMESH

import salome
import geompy

import StdMeshers

smesh = salome.lcc.FindOrLoadComponent("FactoryServer", "SMESH")
smesh.SetCurrentStudy(salome.myStudy)

# ---- define a box
print "Define box"
box = geompy.MakeBox(0., 0., 0., 100., 200., 300.)
idbox = geompy.addToStudy(box, "box")

# ---- add faces of box to study
print "Add faces to study"
idface = []
subShapeList = geompy.SubShapeAll(box, geompy.ShapeType["FACE"])
for f in subShapeList:
  name = geompy.SubShapeName(f, box)
  print name
  idface.append( geompy.addToStudyInFather(box, f, name) )

# ---- add edges of box to study
print "Add edges to study"
idedge = []
subShapeList = geompy.SubShapeAll(box, geompy.ShapeType["EDGE"])
for f in subShapeList:
  name = geompy.SubShapeName(f, box)
  print name
  idedge.append( geompy.addToStudyInFather(box, f, name) )

salome.sg.updateObjBrowser(1);

# ---- launch SMESH
smeshgui = salome.ImportComponentGUI("SMESH")
smeshgui.Init(salome.myStudyId)


print "-------------------------- create Hypothesis"

print "-------------------------- LocalLength"
hypLen1 = smesh.CreateHypothesis("LocalLength", "libStdMeshersEngine.so")
hypLen1.SetLength(100)
print hypLen1.GetName()
print hypLen1.GetId()
print hypLen1.GetLength()

idlength = salome.ObjectToID(hypLen1)
smeshgui.SetName(idlength, "Local_Length_100");

hypNbSeg = []
print "-------------------------- NumberOfSegments"
hypNbSeg1 = smesh.CreateHypothesis("NumberOfSegments", "libStdMeshersEngine.so")
hypNbSeg1.SetDistrType(0)
hypNbSeg1.SetNumberOfSegments(7)
print hypNbSeg1.GetName()
print hypNbSeg1.GetId()
print hypNbSeg1.GetNumberOfSegments()
idseg1 = salome.ObjectToID(hypNbSeg1)
smeshgui.SetName(idseg1, "NumberOfSegmentsReg");
hypNbSeg.append(hypNbSeg1)

hypNbSeg2 = smesh.CreateHypothesis("NumberOfSegments", "libStdMeshersEngine.so")
hypNbSeg2.SetDistrType(1)
hypNbSeg2.SetNumberOfSegments(7)
hypNbSeg2.SetScaleFactor(2)
print hypNbSeg2.GetName()
print hypNbSeg2.GetId()
print hypNbSeg2.GetNumberOfSegments()
idseg2 = salome.ObjectToID(hypNbSeg2)
smeshgui.SetName(idseg2, "NumberOfSegmentsScale");
hypNbSeg.append(hypNbSeg2)

hypNbSeg3 = smesh.CreateHypothesis("NumberOfSegments", "libStdMeshersEngine.so")
hypNbSeg3.SetDistrType(2)
hypNbSeg3.SetNumberOfSegments(7)
hypNbSeg3.SetTableFunction( [0, 0.1, 0.5, 1.0, 1.0, 0.1] )
hypNbSeg3.SetConversionMode(0)
print hypNbSeg3.GetName()
print hypNbSeg3.GetId()
print hypNbSeg3.GetNumberOfSegments()
idseg3 = salome.ObjectToID(hypNbSeg3)
smeshgui.SetName(idseg3, "NumberOfSegmentsTable");
hypNbSeg.append(hypNbSeg3)

hypNbSeg4 = smesh.CreateHypothesis("NumberOfSegments", "libStdMeshersEngine.so")
hypNbSeg4.SetDistrType(3)
hypNbSeg4.SetNumberOfSegments(10)
hypNbSeg4.SetExpressionFunction("sin(3*t)")
hypNbSeg4.SetConversionMode(1)
print hypNbSeg4.GetName()
print hypNbSeg4.GetId()
print hypNbSeg4.GetNumberOfSegments()
idseg4 = salome.ObjectToID(hypNbSeg4)
smeshgui.SetName(idseg4, "NumberOfSegmentsExpr");
hypNbSeg.append(hypNbSeg4)

print "-------------------------- MaxElementArea"
hypArea1 = smesh.CreateHypothesis("MaxElementArea", "libStdMeshersEngine.so")
hypArea1.SetMaxElementArea(2500)
print hypArea1.GetName()
print hypArea1.GetId()
print hypArea1.GetMaxElementArea()

idarea1 = salome.ObjectToID(hypArea1)
smeshgui.SetName(idarea1, "MaxElementArea_2500");

print "-------------------------- MaxElementArea"
hypArea2 = smesh.CreateHypothesis("MaxElementArea", "libStdMeshersEngine.so")
hypArea2.SetMaxElementArea(500)
print hypArea2.GetName()
print hypArea2.GetId()
print hypArea2.GetMaxElementArea()

idarea2 = salome.ObjectToID(hypArea2)
smeshgui.SetName(idarea2, "MaxElementArea_500");

print "-------------------------- Regular_1D"
algoReg = smesh.CreateHypothesis("Regular_1D", "libStdMeshersEngine.so")
listHyp = algoReg.GetCompatibleHypothesis()
for hyp in listHyp:
    print hyp
print algoReg.GetName()
print algoReg.GetId()

idreg = salome.ObjectToID(algoReg)
smeshgui.SetName(idreg, "Regular_1D");

print "-------------------------- MEFISTO_2D"
algoMef = smesh.CreateHypothesis("MEFISTO_2D", "libStdMeshersEngine.so")
listHyp = algoMef.GetCompatibleHypothesis()
for hyp in listHyp:
    print hyp
print algoMef.GetName()
print algoMef.GetId()

idmef = salome.ObjectToID(algoMef)
smeshgui.SetName(idmef, "MEFISTO_2D");

salome.sg.updateObjBrowser(1);

# ---- Init a Mesh with the box

box = salome.IDToObject(idbox)
names = [ "MeshBoxReg", "MeshBoxScale", "MeshBoxTable", "MeshBoxExpr" ];
j = 0
for i in range(4):
  mesh = smesh.CreateMesh(box)
  idmesh = salome.ObjectToID(mesh)
  smeshgui.SetName(idmesh, names[j]);
  print "-------------------------- add hypothesis to box"
  mesh.AddHypothesis(box,algoReg)
  mesh.AddHypothesis(box,hypNbSeg[j])
  mesh.AddHypothesis(box,algoMef)
  mesh.AddHypothesis(box,hypArea1)
  j=j+1

salome.sg.updateObjBrowser(1);

