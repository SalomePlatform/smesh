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
#  File        : SMESH_testExtrusion2D.py
#  Module      : SMESH
#  Description : Create meshes to test extrusion of mesh elements along path

import salome
import geompy
import SMESH
import StdMeshers

#----------------------------------GEOM

# create points
p1 = geompy.MakeVertex(100, 0, 0)
p2 = geompy.MakeVertex(100, 0, 100)
p3 = geompy.MakeVertex(0, 0, 0)
p4 = geompy.MakeVertex(0, 100, 0)


# create two vectors
vector1 = geompy.MakeVector(p1,p2)
vector2 = geompy.MakeVector(p3,p4)

# make two ellipses
ellipse1 = geompy.MakeEllipse(p1,vector1,50,25)
ellipse2 = geompy.MakeEllipse(p3,vector2,300,50)

# publish circular face and second circle
id_ellipse1 = geompy.addToStudy(ellipse1, "Ellips 1")
id_ellipse2 = geompy.addToStudy(ellipse2, "Ellips 2")


#---------------------------------SMESH
# get smesh engine
smesh = salome.lcc.FindOrLoadComponent("FactoryServer", "SMESH")
smesh.SetCurrentStudy(salome.myStudy)

# get SMESH GUI
smeshgui = salome.ImportComponentGUI("SMESH")
smeshgui.Init(salome.myStudyId)

# create hypoteses
hypNbSeg1 = smesh.CreateHypothesis("NumberOfSegments", "libStdMeshersEngine.so")
hypNbSeg1.SetNumberOfSegments(18)
id_hypNbSeg1 = salome.ObjectToID(hypNbSeg1) 
smeshgui.SetName(id_hypNbSeg1, "NumberOfSegments 1");

hypNbSeg2 = smesh.CreateHypothesis("NumberOfSegments", "libStdMeshersEngine.so")
hypNbSeg2.SetNumberOfSegments(34)
id_hypNbSeg2 = salome.ObjectToID(hypNbSeg2) 
smeshgui.SetName(id_hypNbSeg2, "NumberOfSegments 2");

# create algorithmes
algoReg = smesh.CreateHypothesis("Regular_1D", "libStdMeshersEngine.so")
id_algoReg = salome.ObjectToID(algoReg)
smeshgui.SetName(id_algoReg, "Regular_1D");

# create the path mesh
mesh1 = smesh.CreateMesh(ellipse1)
id_mesh1 = salome.ObjectToID(mesh1)
smeshgui.SetName(id_mesh1, "Path Mesh");

# set hypotheses and algos
mesh1.AddHypothesis(ellipse1,algoReg)
mesh1.AddHypothesis(ellipse1,hypNbSeg1)

# create the tool mesh
mesh2 = smesh.CreateMesh(ellipse2)
id_mesh2 = salome.ObjectToID(mesh2)
smeshgui.SetName(id_mesh2, "Tool Mesh");

# set hypotheses and algos
mesh2.AddHypothesis(ellipse2,algoReg)
mesh2.AddHypothesis(ellipse2,hypNbSeg2)

# compute meshes
smesh.Compute(mesh1,ellipse1)
smesh.Compute(mesh2,ellipse2)


# ---- udate object browser
salome.sg.updateObjBrowser(1);
