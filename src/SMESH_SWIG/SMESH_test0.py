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

#  File   : SMESH_test0.py
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

salome.sg.updateObjBrowser(True)
