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
#  File   : SMESH_test0.py
#  Module : SMESH

import SMESH
import smeshpy
import salome
from salome import sg
import math
#import SMESH_BasicHypothesis_idl

import geompy

geom = salome.lcc.FindOrLoadComponent("FactoryServer", "GEOM")
myBuilder = salome.myStudy.NewBuilder()
from geompy import gg

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

sg.updateObjBrowser(1);
