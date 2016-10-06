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

import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)


# Create box without one plane

box = geompy.MakeBox(0., 0., 0., 10., 20., 30.)
subShapeList = geompy.SubShapeAll(box, geompy.ShapeType["FACE"])

FaceList  = []
for i in range( 5 ):
  FaceList.append( subShapeList[ i ] )

aComp = geompy.MakeCompound( FaceList )
aBox = geompy.Sew( aComp, 1. )
idbox = geompy.addToStudy( aBox, "box" )

aBox  = salome.IDToObject( idbox )

# Create mesh
smesh.SetCurrentStudy(salome.myStudy)

mesh = smesh.Mesh(aBox, "Mesh_freebord")

algoReg = mesh.Segment()
hypNbSeg = algoReg.NumberOfSegments(5)

algoMef = mesh.Triangle()
hypArea = algoMef.MaxElementArea(20)


mesh.Compute()


# Criterion : Free edges. Create group.

aCriterion = smesh.GetCriterion(SMESH.EDGE, SMESH.FT_FreeEdges)

aGroup = mesh.MakeGroupByCriterion("Free edges", aCriterion)

anIds = aGroup.GetIDs()

# print result
print "Criterion: Free edges Nb = ", len( anIds )
for i in range( len( anIds ) ):
  print anIds[ i ]

salome.sg.updateObjBrowser(True)
