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
#  File        : SMESH_testExtrusion3D.py
#  Module      : SMESH
#  Description : Create meshes to test extrusion of mesh elements along path

import salome
import geompy
import SMESH
import StdMeshers

# get smesh engine
smesh = salome.lcc.FindOrLoadComponent("FactoryServer", "SMESH")
smesh.SetCurrentStudy(salome.myStudy)

# create points to build two circles
p1 = geompy.MakeVertex(0,  100,  0)
p2 = geompy.MakeVertex(100,  0,  0)
p3 = geompy.MakeVertex(0, -100,  0)
p4 = geompy.MakeVertex(0,   70,  0)
p5 = geompy.MakeVertex(0,  100, 30)
p6 = geompy.MakeVertex(0,  130,  0)

# create two circles
circle = geompy.MakeCircleThreePnt(p1, p2, p3)
cf     = geompy.MakeCircleThreePnt(p4, p5, p6)

# make circular face
wire = geompy.MakeWire([cf])
face = geompy.MakeFace(wire, 1)

# publish circular face and second circle
idcircle = geompy.addToStudy(circle, "Circle")
idface   = geompy.addToStudy(face,   "Circular face")

# get SMESH GUI
smeshgui = salome.ImportComponentGUI("SMESH")
smeshgui.Init(salome.myStudyId)

# create hypoteses
hypNbSeg = smesh.CreateHypothesis("NumberOfSegments", "libStdMeshersEngine.so")
hypNbSeg.SetNumberOfSegments(12)
idseg = salome.ObjectToID(hypNbSeg) 
smeshgui.SetName(idseg, "NumberOfSegments_10");

hypArea = smesh.CreateHypothesis("MaxElementArea", "libStdMeshersEngine.so")
hypArea.SetMaxElementArea(30)
idarea = salome.ObjectToID(hypArea)
smeshgui.SetName(idarea, "MaxElementArea_20");

# create algorithmes
algoReg = smesh.CreateHypothesis("Regular_1D", "libStdMeshersEngine.so")
idreg = salome.ObjectToID(algoReg)
smeshgui.SetName(idreg, "Regular_1D");

algoMef = smesh.CreateHypothesis("MEFISTO_2D", "libStdMeshersEngine.so")
idmef = salome.ObjectToID(algoMef)
smeshgui.SetName(idmef, "MEFISTO_2D");

# init a Mesh with the circular face
mesh1 = smesh.CreateMesh(face)
idmesh1 = salome.ObjectToID(mesh1)
smeshgui.SetName(idmesh1, "Mesh on circular face");

# set hypotheses and algos
mesh1.AddHypothesis(face,algoReg)
mesh1.AddHypothesis(face,hypNbSeg)
mesh1.AddHypothesis(face,algoMef)
mesh1.AddHypothesis(face,hypArea)

# init a Mesh with the second circle
mesh2 = smesh.CreateMesh(circle)
idmesh2 = salome.ObjectToID(mesh2)
smeshgui.SetName(idmesh2, "Mesh on circular edge");

# set hypotheses and algos
mesh2.AddHypothesis(circle,algoReg)
mesh2.AddHypothesis(circle,hypNbSeg)

# compute meshes
smesh.Compute(mesh1,face)
smesh.Compute(mesh2,circle)

# ---- udate object browser
salome.sg.updateObjBrowser(1);
