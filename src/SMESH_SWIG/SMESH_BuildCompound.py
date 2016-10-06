#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

#  File   : SMESH_BuildCompound.py
#  Author : Alexander KOVALEV
#  Module : SMESH
#  $Header$
# ! Please, if you edit this example file, update also
# ! SMESH_SRC/doc/salome/gui/SMESH/input/tui_creating_meshes.doc
# ! as some sequences of symbols from this example are used during
# ! documentation generation to identify certain places of this file
#
import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

## create a bottom box
Box_inf = geompy.MakeBox(0., 0., 0., 200., 200., 50.)

# get a top face
Psup1=geompy.MakeVertex(100., 100., 50.)
Fsup1=geompy.GetFaceNearPoint(Box_inf, Psup1)
# get a bottom face
Pinf1=geompy.MakeVertex(100., 100., 0.)
Finf1=geompy.GetFaceNearPoint(Box_inf, Pinf1)

## create a top box
Box_sup = geompy.MakeBox(100., 100., 50., 200., 200., 100.)

# get a top face
Psup2=geompy.MakeVertex(150., 150., 100.)
Fsup2=geompy.GetFaceNearPoint(Box_sup, Psup2)
# get a bottom face
Pinf2=geompy.MakeVertex(150., 150., 50.)
Finf2=geompy.GetFaceNearPoint(Box_sup, Pinf2)

## Publish in the study
geompy.addToStudy(Box_inf, "Box_inf")
geompy.addToStudyInFather(Box_inf, Fsup1, "Fsup")
geompy.addToStudyInFather(Box_inf, Finf1, "Finf")

geompy.addToStudy(Box_sup, "Box_sup")
geompy.addToStudyInFather(Box_sup, Fsup2, "Fsup")
geompy.addToStudyInFather(Box_sup, Finf2, "Finf")


## create a bottom mesh
Mesh_inf = smesh.Mesh(Box_inf, "Mesh_inf")
algo1D_1=Mesh_inf.Segment()
algo1D_1.NumberOfSegments(10)
algo2D_1=Mesh_inf.Quadrangle()
algo3D_1=Mesh_inf.Hexahedron()
Mesh_inf.Compute()

# create a group on the top face
Gsup1=Mesh_inf.Group(Fsup1, "Sup")
# create a group on the bottom face
Ginf1=Mesh_inf.Group(Finf1, "Inf")

## create a top mesh
Mesh_sup = smesh.Mesh(Box_sup, "Mesh_sup")
algo1D_2=Mesh_sup.Segment()
algo1D_2.NumberOfSegments(5)
algo2D_2=Mesh_sup.Quadrangle()
algo3D_2=Mesh_sup.Hexahedron()
Mesh_sup.Compute()

# create a group on the top face
Gsup2=Mesh_sup.Group(Fsup2, "Sup")
# create a group on the bottom face
Ginf2=Mesh_sup.Group(Finf2, "Inf")

## create compounds
# create a compound of two meshes with renaming groups with the same names and
# merging of elements with the given tolerance
Compound1 = smesh.Concatenate([Mesh_inf.GetMesh(), Mesh_sup.GetMesh()], 0, 1, 1e-05)
smesh.SetName(Compound1, 'Compound_with_RenamedGrps_and_MergeElems')
# create a compound of two meshes with uniting groups with the same names and
# creating groups of all elements
Compound2 = smesh.Concatenate([Mesh_inf.GetMesh(), Mesh_sup.GetMesh()], 1, 0, 1e-05, True)
smesh.SetName(Compound2, 'Compound_with_UniteGrps_and_GrpsOfAllElems')
#end

salome.sg.updateObjBrowser(True)
