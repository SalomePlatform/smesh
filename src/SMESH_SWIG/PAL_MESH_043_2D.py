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

#  File        : SMESH_testExtrusion2D.py
#  Module      : SMESH
#  Description : Create meshes to test extrusion of mesh elements along path
#
import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)


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

# create the path mesh
mesh1 = smesh.Mesh(ellipse1, "Path Mesh")

algoReg1 = mesh1.Segment()
algoReg1.SetName("Regular_1D")
hypNbSeg1 = algoReg1.NumberOfSegments(18)
smesh.SetName(hypNbSeg1, "NumberOfSegments 1")

# create the tool mesh
mesh2 = smesh.Mesh(ellipse2, "Tool Mesh")
algoReg2 = mesh2.Segment()
algoReg2.SetName("Regular_1D")
hypNbSeg2 = algoReg2.NumberOfSegments(34)
smesh.SetName(hypNbSeg2, "NumberOfSegments 2")

# compute meshes
mesh1.Compute()
mesh2.Compute()


# ---- udate object browser
salome.sg.updateObjBrowser(True)
