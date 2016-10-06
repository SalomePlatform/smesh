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

# Triangulation of the skin of the geometry from a Brep representing a plane
# Hypothesis and algorithms for the mesh generation are global
#
import os
import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)


# ---------------------------- GEOM --------------------------------------

# import a BRep
#before running this script, please be sure about
#the path the file fileName

filePath = os.environ["DATA_DIR"]
filePath = filePath + "/Shapes/Brep/"

filename = "flight_solid.brep"
filename = filePath + filename

shape = geompy.Import(filename, "BREP")
idShape = geompy.addToStudy(shape, "flight")

print "Analysis of the geometry flight :"
subShellList = geompy.SubShapeAll(shape, geompy.ShapeType["SHELL"])
subFaceList  = geompy.SubShapeAll(shape, geompy.ShapeType["FACE"])
subEdgeList  = geompy.SubShapeAll(shape, geompy.ShapeType["EDGE"])

print "number of Shells in flight : ", len(subShellList)
print "number of Faces  in flight : ", len(subFaceList)
print "number of Edges  in flight : ", len(subEdgeList)


### ---------------------------- SMESH --------------------------------------
smesh.SetCurrentStudy(salome.myStudy)

# ---- init a Mesh with the shell
shape_mesh = salome.IDToObject( idShape )

mesh = smesh.Mesh(shape_mesh, "MeshFlight")


# ---- set Hypothesis and Algorithm

print "-------------------------- LocalLength"

lengthOfSegments = 0.3

regular1D = mesh.Segment()
hypLength = regular1D.LocalLength(lengthOfSegments)
print hypLength.GetName()
print hypLength.GetId()
print hypLength.GetLength()
smesh.SetName(hypLength, "LocalLength_" + str(lengthOfSegments))

print "-------------------------- LengthFromEdges"

mefisto2D = mesh.Triangle()
hypLengthFromEdge = mefisto2D.LengthFromEdges()
print hypLengthFromEdge.GetName()
print hypLengthFromEdge.GetId()
smesh.SetName(hypLengthFromEdge,"LengthFromEdge")

print "-------------------------- compute the skin flight"
ret = mesh.Compute()
print ret
if ret != 0:
    log = mesh.GetLog(0) # no erase trace
    for linelog in log:
        print linelog
    print "Information about the Mesh_mechanic_tetra:"
    print "Number of nodes      : ", mesh.NbNodes()
    print "Number of edges      : ", mesh.NbEdges()
    print "Number of faces      : ", mesh.NbFaces()
    print "Number of triangles  : ", mesh.NbTriangles()
    print "Number of volumes    : ", mesh.NbVolumes()
else:
    print "probleme when computing the mesh"

salome.sg.updateObjBrowser(True)
