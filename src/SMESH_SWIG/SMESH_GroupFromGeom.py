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

#  File   : SMESH_GroupFromGeom.py
#  Module : SMESH
#
from SMESH_test1 import *

# Compute the mesh created in SMESH_test1

mesh.Compute()

# Create geometry groups on plane:
aGeomGroup1 = geompy.CreateGroup(face , geompy.ShapeType["FACE"])
geompy.AddObject(aGeomGroup1, 1)

aGeomGroup2 = geompy.CreateGroup(face , geompy.ShapeType["EDGE"])

geompy.AddObject(aGeomGroup2, 3)
geompy.AddObject(aGeomGroup2, 6)
geompy.AddObject(aGeomGroup2, 8)
geompy.AddObject(aGeomGroup2, 10)

geompy.addToStudy(aGeomGroup1, "Group on Faces")
geompy.addToStudy(aGeomGroup2, "Group on Edges")

aSmeshGroup1 = mesh.GroupOnGeom(aGeomGroup1, "SMESHGroup1", SMESH.FACE)
aSmeshGroup2 = mesh.GroupOnGeom(aGeomGroup2, "SMESHGroup2", SMESH.EDGE)

salome.sg.updateObjBrowser(True)
