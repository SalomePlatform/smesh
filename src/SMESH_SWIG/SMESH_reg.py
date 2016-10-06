#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
#
# Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
# CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

#  File   : SMESH_reg.py
#  Module : SMESH
#
import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

from salome.StdMeshers import StdMeshersBuilder


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

salome.sg.updateObjBrowser(True)

# ---- launch SMESH
smeshgui = salome.ImportComponentGUI("SMESH")
smeshgui.Init(salome.myStudyId)
smesh.SetCurrentStudy(salome.myStudy)

# ---- Creating meshes

box = salome.IDToObject(idbox)
names = [ "MeshBoxReg", "MeshBoxScale", "MeshBoxTable", "MeshBoxExpr" ]


print "-------------------------- Create ", names[0], " mesh"
mesh = smesh.Mesh(box, names[0])
algo = mesh.Segment()
hyp = algo.NumberOfSegments(7)
hyp.SetDistrType(0)
smesh.SetName(hyp, "NumberOfSegmentsReg")
algo = mesh.Triangle()
algo.MaxElementArea(2500)

print "-------------------------- Create ", names[1], " mesh"
mesh = smesh.Mesh(box, names[1])
algo = mesh.Segment()
hyp = algo.NumberOfSegments(7)
hyp.SetDistrType(1)
hyp.SetScaleFactor(2)
smesh.SetName(hyp, "NumberOfSegmentsScale")
algo = mesh.Triangle()
algo.MaxElementArea(2500)

print "-------------------------- Create ", names[2], " mesh"
mesh = smesh.Mesh(box,names[2])
algo = mesh.Segment()
hyp = algo.NumberOfSegments(7)
hyp.SetDistrType(2)
hyp.SetTableFunction( [0, 0.1, 0.5, 1.0, 1.0, 0.1] )
hyp.SetConversionMode(0)
smesh.SetName(hyp, "NumberOfSegmentsTable")
algo = mesh.Triangle()
algo.MaxElementArea(2500)

print "-------------------------- Create ", names[3], " mesh"
mesh = smesh.Mesh(box, names[3])
algo = mesh.Segment()
hyp = algo.NumberOfSegments(10)
hyp.SetDistrType(3)
hyp.SetExpressionFunction("sin(3*t)")
hyp.SetConversionMode(1)
smesh.SetName(hyp, "NumberOfSegmentsExpr")
algo = mesh.Triangle()
algo.MaxElementArea(2500)


salome.sg.updateObjBrowser(True)

