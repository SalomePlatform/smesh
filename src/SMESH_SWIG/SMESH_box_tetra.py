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

# Tetrahedrization of a simple box. Hypothesis and algorithms for
# the mesh generation are global
#
import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

# ---- define a boxe

box = geompy.MakeBox(0., 0., 0., 100., 200., 300.)

idbox = geompy.addToStudy(box, "box")

print "Analysis of the geometry box :"
subShellList = geompy.SubShapeAll(box, geompy.ShapeType["SHELL"])
subFaceList  = geompy.SubShapeAll(box, geompy.ShapeType["FACE"])
subEdgeList  = geompy.SubShapeAll(box, geompy.ShapeType["EDGE"])

print "number of Shells in box : ", len(subShellList)
print "number of Faces  in box : ", len(subFaceList)
print "number of Edges  in box : ", len(subEdgeList)


### ---------------------------- SMESH --------------------------------------

# ---- init a Mesh with the boxe

mesh = smesh.Mesh(box, "MeshBox")

# ---- set Hypothesis and Algorithm

print "-------------------------- NumberOfSegments"
numberOfSegments = 10

regular1D = mesh.Segment()
hypNbSeg = regular1D.NumberOfSegments(numberOfSegments)
print hypNbSeg.GetName()
print hypNbSeg.GetId()
print hypNbSeg.GetNumberOfSegments()
smesh.SetName(hypNbSeg, "NumberOfSegments_" + str(numberOfSegments))

print "-------------------------- MaxElementArea"

maxElementArea = 500

mefisto2D = mesh.Triangle()
hypArea = mefisto2D.MaxElementArea(maxElementArea)
print hypArea.GetName()
print hypArea.GetId()
print hypArea.GetMaxElementArea()
smesh.SetName(hypArea, "MaxElementArea_" + str(maxElementArea))

print "-------------------------- MaxElementVolume"

maxElementVolume = 500

netgen3D = mesh.Tetrahedron(smeshBuilder.NETGEN)
hypVolume = netgen3D.MaxElementVolume(maxElementVolume)
print hypVolume.GetName()
print hypVolume.GetId()
print hypVolume.GetMaxElementVolume()
smesh.SetName(hypVolume, "MaxElementVolume_" + str(maxElementVolume))

print "-------------------------- compute the mesh of the boxe"
ret = mesh.Compute()
print ret
if ret != 0:
    log = mesh.GetLog(0) # no erase trace
    for linelog in log:
        print linelog
    print "Information about the MeshBox:"
    print "Number of nodes       : ", mesh.NbNodes()
    print "Number of edges       : ", mesh.NbEdges()
    print "Number of faces       : ", mesh.NbFaces()
    print "Number of triangles   : ", mesh.NbTriangles()
    print "Number of volumes     : ", mesh.NbVolumes()
    print "Number of tetrahedrons: ", mesh.NbTetras()
else:
    print "probleme when computing the mesh"

salome.sg.updateObjBrowser(True)
