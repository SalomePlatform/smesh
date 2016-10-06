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

#  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
#  File   : SMESH_test.py
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
idb = geompy.addToStudy(box, "box")

# ---- add first face of box in study

subShapeList = geompy.SubShapeAll(box, geompy.ShapeType["FACE"])
face = subShapeList[0]
name = geompy.SubShapeName(face, box)
idf = geompy.addToStudyInFather(box, face, name)

# ---- add shell from box  in study

subShellList = geompy.SubShapeAll(box, geompy.ShapeType["SHELL"])
shell = subShellList[0]
name = geompy.SubShapeName(shell, box)
ids = geompy.addToStudyInFather(box, shell, name)

# ---- add first edge of face in study

edgeList = geompy.SubShapeAll(face, geompy.ShapeType["EDGE"])
edge = edgeList[0];
name = geompy.SubShapeName(edge, face)
ide = geompy.addToStudyInFather(face, edge, name)

# ---- SMESH

smesh.SetCurrentStudy(salome.myStudy)
box = salome.IDToObject(idb)
mesh = smesh.Mesh(box, "Meshbox")

print "-------------------------- add hypothesis to box"

algo_1 = mesh.Segment(box)
hyp = algo_1.LocalLength(100)
print hyp.GetName()
print hyp.GetId()
print hyp.GetLength()

algo_2 = mesh.Triangle(smeshBuilder.MEFISTO, box)
hyp = algo_2.MaxElementArea(5000)
print hyp.GetName()
print hyp.GetId()
print hyp.GetMaxElementArea()

smesh.SetName(algo_2.GetSubMesh(), "SubMeshBox")


print "-------------------------- add hypothesis to edge"

edge = salome.IDToObject(ide)

algo_3 = mesh.Segment(edge)
hyp = algo_3.LocalLength(100)
print hyp.GetName()
print hyp.GetId()
print hyp.GetLength()

smesh.SetName(algo_3.GetSubMesh(), "SubMeshEdge")


print "-------------------------- compute face"

face = salome.IDToObject(idf)

ret = mesh.Compute(face)
print ret
log = mesh.GetLog(0) # 0 - GetLog without ClearLog after, else if 1 - ClearLog after
for a in log:
    print "-------"
    ii = 0
    ir = 0
    comType = a.commandType
    if comType == 0:
        for i in range(a.number):
            ind = a.indexes[ii]
            ii = ii+1
            r1 = a.coords[ir]
            ir = ir+1
            r2 = a.coords[ir]
            ir = ir+1
            r3 = a.coords[ir]
            ir = ir+1
            print "AddNode %i - %g %g %g" % (ind, r1, r2, r3)
    elif comType == 1:
        for i in range(a.number):
            ind = a.indexes[ii]
            ii = ii+1
            i1 = a.indexes[ii]
            ii = ii+1
            i2 = a.indexes[ii]
            ii = ii+1
            print "AddEdge %i - %i %i" % (ind, i1, i2)
    elif comType == 2:
        for i in range(a.number):
            ind = a.indexes[ii]
	    print ind
            ii = ii+1
	    print ii
            i1 = a.indexes[ii]
            ii = ii+1
            i2 = a.indexes[ii]
	    print i2
            ii = ii+1
	    print "ii", ii
            i3 = a.indexes[ii]
	    print i3
            #ii = ii+1
            ii = ii+1
            print "AddTriangle %i - %i %i %i" % (ind, i1, i2, i3)

salome.sg.updateObjBrowser(True)
