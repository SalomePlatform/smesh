# -*- coding: utf-8 -*-
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

import math

def GetNewNodes(mesh,Elems,OldNodes):
    """
    Auxilary function, which return list of nodes from
    given Elems avoided nodes from OldNodes
    """
    newnodes = []
    for i in Elems:
        nbn = mesh.GetElemNbNodes(i)
        for j in range(0,nbn):
            nn = mesh.GetElemNode(i,j)
            isold = 0
            for k in range(0,len(newnodes)):
                if nn==newnodes[k]:
                    isold = 1
                    break
                pass
            if isold: continue
            for k in range(0,len(OldNodes)):
                if nn==OldNodes[k]:
                    isold = 1
                    break
                pass
            if isold: continue
            newnodes.append(nn)
            pass
        pass
    return newnodes
            

# create empty mesh
mesh = smesh.Mesh()

tol = 0.001

# create a cross from quadrangle faces
# 1. create first edge and make extrusion along 0x
n1 = mesh.AddNode(55,-5,0)
n2 = mesh.AddNode(55,5,0)
e1 = mesh.AddEdge([n1,n2])
dir1 = SMESH.DirStruct(SMESH.PointStruct(-10,0,0))
mesh.ExtrusionSweep([e1],dir1,11)
# 2. create second edge and make extrusion along 0y
n3 = mesh.AddNode(-5,-55,0)
n4 = mesh.AddNode(5,-55,0)
e2 = mesh.AddEdge([n3,n4])
dir2 = SMESH.DirStruct(SMESH.PointStruct(0,10,0))
mesh.ExtrusionSweep([e2],dir2,11)

# since result has coincident nodes and faces
# we have to make merge
nodes = mesh.FindCoincidentNodes(0.001)
mesh.MergeNodes(nodes)
mesh.MergeEqualElements()

# make extrusion faces along 0z
faces = mesh.GetElementsByType(SMESH.FACE)
nbf = len(faces)
maxang = 2.0
zstep = 5
nbzsteps = 50
dir3 = SMESH.DirStruct(SMESH.PointStruct(0,0,zstep))
newfaces = [] # list for keeping created top faces
              # during extrusion

for i in range(0,nbzsteps):
    mesh.ExtrusionSweep(faces,dir3,1)
    # find top faces after each extrusion and keep them
    res = mesh.GetLastCreatedElems()
    nbr = len(res)
    nfaces = []
    for j in res:
        nbn = mesh.GetElemNbNodes(j)
        if nbn!=4: continue
        nn1 = mesh.GetElemNode(j,0)
        xyz1 = mesh.GetNodeXYZ(nn1)
        nn2 = mesh.GetElemNode(j,1)
        xyz2 = mesh.GetNodeXYZ(nn2)
        nn3 = mesh.GetElemNode(j,2)
        xyz3 = mesh.GetNodeXYZ(nn3)
        if abs(xyz1[2]-xyz2[2])<tol and abs(xyz1[2]-xyz3[2])<tol :
            # this face is a top face
            nfaces.append(j)
            pass
        pass
    if len(nfaces)!=nbf:
        print "len(nfaces)!=nbf"
        break
    newfaces.append(nfaces)
    # update faces for before next step of extrusion
    faces = nfaces
    pass
    
# rotate faces from newfaces
axisr1 = SMESH.AxisStruct(0,0,0,0,0,1)
for i in range(0,nbzsteps):
    ang = maxang*(1-math.cos((i+1)*math.pi/nbzsteps))
    mesh.Rotate(newfaces[i],axisr1,ang,0)


# create circles
# create two edges and rotate them for creation
# full circle
n5 = mesh.AddNode(65,0,0)
n6 = mesh.AddNode(67.5,0,0)
n7 = mesh.AddNode(70,0,0)
e56 = mesh.AddEdge([n5,n6])
e67 = mesh.AddEdge([n6,n7])
axisr2 = SMESH.AxisStruct(65,0,0,0,1,0)
mesh.RotationSweep([e56,e67],axisr2, math.pi/6, 12, tol)
res = mesh.GetLastCreatedElems()
faces1 = []
for i in res:
    nbn = mesh.GetElemNbNodes(i)
    if nbn>2: faces1.append(i)
    pass
nbf1 = len(faces1)

# create other two edges and rotate them for creation
# other full circle
n8 = mesh.AddNode(-65,0,0)
n9 = mesh.AddNode(-67.5,0,0)
n10 = mesh.AddNode(-70,0,0)
e8 = mesh.AddEdge([n8,n9])
e9 = mesh.AddEdge([n9,n10])
axisr3 = SMESH.AxisStruct(-65,0,0,0,-1,0)
mesh.RotationSweep([e8,e9],axisr3, math.pi/6, 12, tol)
res = mesh.GetLastCreatedElems()
faces2 = []
for i in res:
    nbn = mesh.GetElemNbNodes(i)
    if nbn>2: faces2.append(i)
    pass
nbf2 = len(faces2)

# there are coincident nodes after rotation
# therefore we have to merge nodes
nodes = mesh.FindCoincidentNodes(0.001)
mesh.MergeNodes(nodes)

nbcircs = 2
nbrsteps = 24
nbrs = nbcircs*nbrsteps
dz = nbzsteps*zstep/nbrs

# create first spiral
oldnodes = []
newnodes = GetNewNodes(mesh,faces1,oldnodes)
oldnodes = newnodes

nodes = []
mesh.RotationSweep(faces1,axisr1, math.pi*2/nbrsteps, nbrs, tol)
res = mesh.GetLastCreatedElems()

for i in range(0,nbrs):
    volumes = []
    for j in range(0,nbf1): volumes.append(res[i+j*nbrs])
    newnodes = GetNewNodes(mesh,volumes,oldnodes)
    for j in newnodes:
        xyz = mesh.GetNodeXYZ(j)
        mesh.MoveNode(j,xyz[0],xyz[1],xyz[2]+dz*(i+1))
        pass
    oldnodes = newnodes
    pass

# create second spiral
oldnodes = []
newnodes = GetNewNodes(mesh,faces2,oldnodes)
oldnodes = newnodes

nodes = []
mesh.RotationSweep(faces2,axisr1, math.pi*2/nbrsteps, nbrs, tol)
res = mesh.GetLastCreatedElems()

for i in range(0,nbrs):
    volumes = []
    for j in range(0,nbf2): volumes.append(res[i+j*nbrs])
    newnodes = GetNewNodes(mesh,volumes,oldnodes)
    for j in newnodes:
        xyz = mesh.GetNodeXYZ(j)
        mesh.MoveNode(j,xyz[0],xyz[1],xyz[2]+dz*(i+1))
        pass
    oldnodes = newnodes
    pass

salome.sg.updateObjBrowser(True)
