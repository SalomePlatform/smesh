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


#-----------------------------GEOM----------------------------------------

#----------Vertexes------------
p1 = geompy.MakeVertex(20.0,30.0,40.0)
p2 = geompy.MakeVertex(90.0,80.0,0.0)
p3 = geompy.MakeVertex(30.0,80.0,200.0)

#----------Edges---------------
e1 = geompy.MakeEdge(p1,p2)
e2 = geompy.MakeEdge(p2,p3)
e3 = geompy.MakeEdge(p3,p1)

#----------Wire----------------
ListOfEdges = []
ListOfEdges.append(e3)
ListOfEdges.append(e2)
ListOfEdges.append(e1)
wire1 = geompy.MakeWire(ListOfEdges)


#----------Face----------------
WantPlanarFace = 1
face1 = geompy.MakeFace(wire1,WantPlanarFace)

Id_face1 = geompy.addToStudy(face1,"Face1")



#-----------------------------SMESH-------------------------------------------

# -- Init mesh --
plane_mesh = salome.IDToObject( Id_face1)

mesh = smesh.Mesh(plane_mesh, "Mesh_1")

print"---------------------Hypothesis and Algorithms"

#---------------- NumberOfSegments

numberOfSegment = 9

algoWireDes = mesh.Segment()
listHyp = algoWireDes.GetCompatibleHypothesis()
print algoWireDes.GetName()
algoWireDes.SetName("Ware descritisation")

hypNbSeg = algoWireDes.NumberOfSegments(numberOfSegment)
print hypNbSeg.GetName()
print hypNbSeg.GetNumberOfSegments()
smesh.SetName(hypNbSeg, "Nb. Segments")


#--------------------------Max. Element Area
maxElementArea = 200

algoMef = mesh.Triangle()
listHyp = algoMef.GetCompatibleHypothesis()
print algoMef.GetName()
algoMef.SetName("Triangle (Mefisto)")

hypArea200 = algoMef.MaxElementArea(maxElementArea)
print hypArea200.GetName()
print hypArea200.GetMaxElementArea()
smesh.SetName(hypArea200, "Max. Element Area")


print "---------------------Compute the mesh"

ret = mesh.Compute()
print ret

salome.sg.updateObjBrowser(True)

