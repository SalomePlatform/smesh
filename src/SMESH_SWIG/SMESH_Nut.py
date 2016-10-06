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

#####################################################################
#Created                :17/02/2005
#Auhtor                 :MASLOV Eugeny, KOVALTCHUK Alexey 
#####################################################################
#
import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

import os
import math

#Sketcher_1 creation
print "Sketcher creation..."
Sketcher_1 = geompy.MakeSketcher("Sketcher:F 100 -57.7:TT 100 57.7:TT 0 115.47:TT -100 57.7:TT -100 -57.7:TT 0 -115.47:WW") 
geompy.addToStudy(Sketcher_1, "Sketcher_1")
Face_1 = geompy.MakeFace(Sketcher_1, 1)
geompy.addToStudy(Face_1, "Face_1")

#Line creation
print "Line  creation..."
Line_1 = geompy.MakeLineTwoPnt(geompy.MakeVertex(0,0,0), geompy.MakeVertex(0,0,100))
geompy.addToStudy(Line_1, "Line_1")

#Prism creation
print "Prism creation..."
Prism_1 = geompy.MakePrismVecH(Face_1, Line_1, 100)
geompy.addToStudy(Prism_1, "Prism_1")

#Sketcher_2 creation
print "Sketcher creation..."
Sketcher_2 = geompy.MakeSketcher("Sketcher:F 50 0:TT 80 0:TT 112 13:TT 112 48:TT 80 63:TT 80 90:TT 50 90:WW", [0,0,0, 1,0,0, 0,1,0]) 
geompy.addToStudy(Sketcher_2, "Sketcher_2")
Face_2 = geompy.MakeFace(Sketcher_2, 1)
geompy.addToStudy(Face_2, "Face_2")

#Revolution creation
print "Revolution creation..."
Revolution_1 = geompy.MakeRevolution(Face_2, Line_1, 2*math.pi)
geompy.addToStudy(Revolution_1, "Revolution_1")

#Common applying
print "Common of Revolution and Prism..."
Common_1 = geompy.MakeBoolean(Revolution_1, Prism_1, 1)
geompy.addToStudy(Common_1, "Common_1")

#Explode Common_1 on edges
CommonExplodedListEdges = geompy.SubShapeAll(Common_1, geompy.ShapeType["EDGE"])
for i in range(0, len(CommonExplodedListEdges)):
    name = "Edge_"+str(i+1)
    geompy.addToStudyInFather(Common_1, CommonExplodedListEdges[i], name)

#Fillet applying
print "Fillet creation..."
Fillet_1 = geompy.MakeFillet(Common_1, 10, geompy.ShapeType["EDGE"], [5])
geompy.addToStudy(Fillet_1, "Fillet_1")

#Chamfer applying
print "Chamfer creation..."
cyl_face = geompy.GetFaceNearPoint( Fillet_1, geompy.MakeVertex( 50, 0, 45 ), theName='cyl_face')
cyl_face_id = geompy.GetSubShapeID( Fillet_1, cyl_face )
top_face = geompy.GetFaceNearPoint( Fillet_1, geompy.MakeVertex( 60, 0, 90 ), theName='top_face')
top_face_id = geompy.GetSubShapeID( Fillet_1, top_face )
Chamfer_1 = geompy.MakeChamferEdge(Fillet_1, 10, 10, cyl_face_id, top_face_id, theName='Chamfer_1' )

cyl_face = geompy.GetFaceNearPoint( Chamfer_1, geompy.MakeVertex( 80, 0, 85 ), theName='cyl_face')
cyl_face_id = geompy.GetSubShapeID( Chamfer_1, cyl_face )
top_face = geompy.GetFaceNearPoint( Chamfer_1, geompy.MakeVertex( 65, 0, 90 ), theName='top_face')
top_face_id = geompy.GetSubShapeID( Chamfer_1, top_face )
Chamfer_2 = geompy.MakeChamferEdge(Chamfer_1, 10, 10, cyl_face_id, top_face_id, theName='Chamfer_2' )

#Import of the shape from "slots.brep"
print "Import multi-rotation from the DATA_DIR/Shapes/Brep/slots.brep"
thePath = os.getenv("DATA_DIR")
theFileName = os.path.join( thePath,"Shapes","Brep","slots.brep")
theShapeForCut = geompy.ImportBREP(theFileName)
geompy.addToStudy(theShapeForCut, "slot.brep_1")

#Cut applying
print "Cut..."
Cut_1 = geompy.MakeBoolean(Chamfer_2, theShapeForCut, 2)
Cut_1_ID = geompy.addToStudy(Cut_1, "Cut_1")

#Mesh creation

# -- Init --
shape_mesh = salome.IDToObject( Cut_1_ID )

mesh = smesh.Mesh(shape_mesh, "Nut")

#HYPOTHESIS CREATION
print "-------------------------- Average length"
theAverageLength = 5
algoReg1D = mesh.Segment()
hAvLength = algoReg1D.LocalLength(theAverageLength)
print hAvLength.GetName()
print hAvLength.GetId()
print hAvLength.GetLength()
smesh.SetName(hAvLength, "AverageLength_"+str(theAverageLength))

print "-------------------------- MaxElementArea"
theMaxElementArea = 20
algoMef = mesh.Triangle(smeshBuilder.MEFISTO)
hArea = algoMef.MaxElementArea( theMaxElementArea )
print hArea.GetName()
print hArea.GetId()
print hArea.GetMaxElementArea()
smesh.SetName(hArea, "MaxElementArea_"+str(theMaxElementArea))

print "-------------------------- MaxElementVolume"
theMaxElementVolume = 150
algoNg = mesh.Tetrahedron(smeshBuilder.NETGEN)
hVolume = algoNg.MaxElementVolume( theMaxElementVolume )
print hVolume.GetName()
print hVolume.GetId()
print hVolume.GetMaxElementVolume()
smesh.SetName(hVolume, "MaxElementVolume_"+str(theMaxElementVolume))


print "-------------------------- compute the mesh of the mechanic piece"
mesh.Compute()

print "Information about the Nut:"
print "Number of nodes       : ", mesh.NbNodes()
print "Number of edges       : ", mesh.NbEdges()
print "Number of faces       : ", mesh.NbFaces()
print "Number of triangles   : ", mesh.NbTriangles()
print "Number of quadrangles : ", mesh.NbQuadrangles()
print "Number of volumes     : ", mesh.NbVolumes()
print "Number of tetrahedrons: ", mesh.NbTetras()

salome.sg.updateObjBrowser(True)
