#  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
#
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
#  File   : SMESH_test.py
#  Module : SMESH

import salome
import geompy
import smeshpy

import SMESH
import StdMeshers

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

# ---- launch SMESH, init a Mesh with the box

gen = smeshpy.smeshpy()
mesh = gen.CreateMesh(idb)

print "-------------------------- create Hypothesis"

print "-------------------------- LocalLength"

hypo1 = gen.CreateHypothesis("LocalLength", "libStdMeshersEngine.so")
print hypo1.GetName()
print hypo1.GetId()
print hypo1.GetLength()
hypo1.SetLength(100)
print hypo1.GetLength()

print "-------------------------- bidon"

hyp3 = gen.CreateHypothesis("bidon", "")

print "-------------------------- NumberOfSegments"

hypo3 = gen.CreateHypothesis("NumberOfSegments", "libStdMeshersEngine.so")
hypo3.SetNumberOfSegments(7)
print hypo3.GetName()
print hypo3.GetNumberOfSegments()
print hypo3.GetId()

print "-------------------------- MaxElementArea"

hypo4 = gen.CreateHypothesis("MaxElementArea", "libStdMeshersEngine.so")
hypo4.SetMaxElementArea(5000)
print hypo4.GetName()
print hypo4.GetMaxElementArea()
print hypo4.GetId()

print "-------------------------- Regular_1D"

algo_1 = gen.CreateHypothesis("Regular_1D", "libStdMeshersEngine.so")
print algo_1.GetName()
print algo_1.GetId()
listHyp = algo_1.GetCompatibleHypothesis()
for hyp in listHyp:
    print hyp
print algo_1.GetId()

print "-------------------------- MEFISTO_2D"

algo_2 = gen.CreateHypothesis("MEFISTO_2D", "libStdMeshersEngine.so")
print algo_2.GetName()
print algo_2.GetId()
listHyp = algo_2.GetCompatibleHypothesis()
for hyp in listHyp:
    print hyp
print algo_2.GetId()

print "-------------------------- add hypothesis to edge"

edge = salome.IDToObject(ide)
submesh = mesh.GetSubMesh(edge, "SubMeshEdge")
ret = mesh.AddHypothesis(edge,algo_1)
print ret
ret = mesh.AddHypothesis(edge,hypo1)
print ret

##print "-------------------------- compute edge"
##ret=gen.Compute(mesh,ide)
##print ret
##log=mesh.GetLog(1);
##for a in log:
##    print a

print "-------------------------- add hypothesis to box"

box = salome.IDToObject(idb)
submesh = mesh.GetSubMesh(box, "SubMeshBox")
ret = mesh.AddHypothesis(box,algo_1)
print ret
ret = mesh.AddHypothesis(box,hypo1)
print ret
ret = mesh.AddHypothesis(box,algo_2)
print ret
ret = mesh.AddHypothesis(box,hypo4)
print ret

print "-------------------------- compute face"

ret = gen.Compute(mesh,idf)
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
            ii = ii+1
            i1 = a.indexes[ii]
            ii = ii+1
            i2 = a.indexes[ii]
            ii = ii+1
            i3 = a.indexes[ii]
            ii = ii+1
            print "AddTriangle %i - %i %i %i" % (ind, i1, i2, i3)

##print "-------------------------- compute box"
##ret=gen.Compute(mesh,idb)
##print ret
##log=mesh.GetLog(1);
##print log

##shell=salome.IDToObject(ids)
##submesh=mesh.GetElementsOnShape(shell)
##ret=mesh.AddHypothesis(shell,algo_1)
##print ret
##ret=mesh.AddHypothesis(shell,hypo1)
##print ret
##ret=gen.Compute(mesh,ids)
##print ret
