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
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#
#
#
#  File   : SMESH_test1.py
#  Module : SMESH

import salome
import geompy

import StdMeshers

smesh = salome.lcc.FindOrLoadComponent("FactoryServer", "SMESH")
smesh.SetCurrentStudy(salome.myStudy)

# ---- define a box

box = geompy.MakeBox(0., 0., 0., 100., 200., 300.)
idbox = geompy.addToStudy(box, "box")

# ---- add first face of box in study

subShapeList = geompy.SubShapeAll(box, geompy.ShapeType["FACE"])
face = subShapeList[0]
name = geompy.SubShapeName(face, box)
print name
idface = geompy.addToStudyInFather(box, face, name)

# ---- add shell from box  in study

subShellList = geompy.SubShapeAll(box, geompy.ShapeType["SHELL"])
shell = subShellList[0]
name = geompy.SubShapeName(shell, box)
print name
idshell = geompy.addToStudyInFather(box, shell, name)

# ---- add first edge of face in study

edgeList = geompy.SubShapeAll(face, geompy.ShapeType["EDGE"])
edge = edgeList[0]
name = geompy.SubShapeName(edge, face)
print name
idedge = geompy.addToStudyInFather(face, edge, name)

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

print "-------------------------- NumberOfSegments"
hypNbSeg1 = smesh.CreateHypothesis("NumberOfSegments", "libStdMeshersEngine.so")
hypNbSeg1.SetNumberOfSegments(7)
print hypNbSeg1.GetName()
print hypNbSeg1.GetId()
print hypNbSeg1.GetNumberOfSegments()

idseg = salome.ObjectToID(hypNbSeg1) 
smeshgui.SetName(idseg, "NumberOfSegments_7");

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

# ---- Init a Mesh with the box

box = salome.IDToObject(idbox)
mesh = smesh.CreateMesh(box)
idmesh = salome.ObjectToID(mesh)
smeshgui.SetName(idmesh, "Meshbox");

print "-------------------------- add hypothesis to box"
mesh.AddHypothesis(box,algoReg)
mesh.AddHypothesis(box,hypNbSeg1)
mesh.AddHypothesis(box,algoMef)
mesh.AddHypothesis(box,hypArea1)

# ---- add hypothesis to edge

print "-------------------------- add hypothesis to edge"
edge = salome.IDToObject(idedge)
submesh = mesh.GetSubMesh(edge, "SubMeshEdge")
mesh.AddHypothesis(edge, algoReg)
mesh.AddHypothesis(edge, hypLen1)

print "-------------------------- add hypothesis to face"
face = salome.IDToObject(idface)
submesh = mesh.GetSubMesh(face, "SubMeshFace")
mesh.AddHypothesis(face, hypArea2)

salome.sg.updateObjBrowser(1);
