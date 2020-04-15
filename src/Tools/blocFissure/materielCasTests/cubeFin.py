# -*- coding: utf-8 -*-
# Copyright (C) 2014-2020  EDF R&D
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

import sys
import salome

salome.salome_init()

import salome_notebook

import os
from blocFissure import gmu

###
### GEOM component
###

import GEOM
from salome.geom import geomBuilder
import math
import SALOMEDS


geompy = geomBuilder.New()

O = geompy.MakeVertex(0, 0, 0)
OX = geompy.MakeVectorDXDYDZ(1, 0, 0)
OY = geompy.MakeVectorDXDYDZ(0, 1, 0)
OZ = geompy.MakeVectorDXDYDZ(0, 0, 1)
cubeFin = geompy.MakeBoxDXDYDZ(200, 200, 200)
[DEPL,ENCASTR] = geompy.SubShapes(cubeFin, [27, 23])
origCoin = geompy.MakeVertex(0, 160, 200)
Disk_1 = geompy.MakeDiskPntVecR(origCoin, OY, 50)
Translation_1 = geompy.MakeTranslation(origCoin, 0, 10, 0)
Vector_1 = geompy.MakeVector(origCoin, Translation_1)
Rotation_1 = geompy.MakeRotation(Disk_1, Vector_1, 90*math.pi/180.0)
origMilieu = geompy.MakeVertex(0, 160, 100)
Translation_2 = geompy.MakeTranslationTwoPoints(Rotation_1, origCoin, origMilieu)
Scale_1 = geompy.MakeScaleAlongAxes(Translation_2, origMilieu, 0.984, 1, 1.2)
Vertex_1 = geompy.MakeVertex(-10, 160, 210)
Vertex_2 = geompy.MakeVertex(50, 160, 210)
Vertex_3 = geompy.MakeVertex(50, 160, -10)
Vertex_4 = geompy.MakeVertex(-10, 160, -10)
Line_1 = geompy.MakeLineTwoPnt(Vertex_1, Vertex_2)
Line_2 = geompy.MakeLineTwoPnt(Vertex_2, Vertex_3)
Line_3 = geompy.MakeLineTwoPnt(Vertex_3, Vertex_4)
Line_4 = geompy.MakeLineTwoPnt(Vertex_4, Vertex_1)
cubeFin_Transverse = geompy.MakeFaceWires([Line_1, Line_2, Line_3, Line_4], 1)
Vertex_5 = geompy.MakeVertex(55, 160, 100)
Line_1_vertex_3 = geompy.GetSubShape(Line_1, [3])
Line_3_vertex_2 = geompy.GetSubShape(Line_3, [2])
Arc_1 = geompy.MakeArc(Line_1_vertex_3, Vertex_5, Line_3_vertex_2)
Face_1 = geompy.MakeFaceWires([Line_1, Line_3, Line_4, Arc_1], 1)
cubeFin_Coin = geompy.MakeCommonList([Rotation_1, Face_1], True)
cubeFin_Milieu = geompy.MakeCommonList([Scale_1, cubeFin_Transverse], True)
O_1 = geompy.MakeVertex(0, 0, 0)
OX_1 = geompy.MakeVectorDXDYDZ(1, 0, 0)
OY_1 = geompy.MakeVectorDXDYDZ(0, 1, 0)
OZ_1 = geompy.MakeVectorDXDYDZ(0, 0, 1)
geompy.addToStudy( O, 'O' )
geompy.addToStudy( OX, 'OX' )
geompy.addToStudy( OY, 'OY' )
geompy.addToStudy( OZ, 'OZ' )
geompy.addToStudy( cubeFin, 'cubeFin' )
geompy.addToStudyInFather( cubeFin, DEPL, 'DEPL' )
geompy.addToStudyInFather( cubeFin, ENCASTR, 'ENCASTR' )
geompy.addToStudy( origCoin, 'origCoin' )
geompy.addToStudy( Disk_1, 'Disk_1' )
geompy.addToStudy( Translation_1, 'Translation_1' )
geompy.addToStudy( Vector_1, 'Vector_1' )
geompy.addToStudy( Rotation_1, 'Rotation_1' )
geompy.addToStudy( origMilieu, 'origMilieu' )
geompy.addToStudy( Translation_2, 'Translation_2' )
geompy.addToStudy( Scale_1, 'Scale_1' )
geompy.addToStudy( Vertex_1, 'Vertex_1' )
geompy.addToStudy( Vertex_2, 'Vertex_2' )
geompy.addToStudy( Vertex_3, 'Vertex_3' )
geompy.addToStudy( Vertex_4, 'Vertex_4' )
geompy.addToStudy( Line_1, 'Line_1' )
geompy.addToStudy( Line_2, 'Line_2' )
geompy.addToStudy( Line_3, 'Line_3' )
geompy.addToStudy( Line_4, 'Line_4' )
geompy.addToStudy( cubeFin_Transverse, 'cubeFin_Transverse' )
geompy.addToStudyInFather( Line_1, Line_1_vertex_3, 'Line_1:vertex_3' )
geompy.addToStudy( Vertex_5, 'Vertex_5' )
geompy.addToStudyInFather( Line_3, Line_3_vertex_2, 'Line_3:vertex_2' )
geompy.addToStudy( Arc_1, 'Arc_1' )
geompy.addToStudy( Face_1, 'Face_1' )
geompy.addToStudy( cubeFin_Coin, 'cubeFin_Coin' )
geompy.addToStudy( cubeFin_Milieu, 'cubeFin_Milieu' )
geompy.addToStudy( O_1, 'O' )
geompy.addToStudy( OX_1, 'OX' )
geompy.addToStudy( OY_1, 'OY' )
geompy.addToStudy( OZ_1, 'OZ' )
geompy.ExportBREP(cubeFin_Transverse, os.path.join(gmu.pathBloc, "materielCasTests/cubeFin_Transverse.brep"))
geompy.ExportBREP(cubeFin_Coin, os.path.join(gmu.pathBloc, "materielCasTests/cubeFin_Coin.brep"))
geompy.ExportBREP(cubeFin_Milieu, os.path.join(gmu.pathBloc, "materielCasTests/cubeFin_Milieu.brep"))

###
### SMESH component
###

import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New()
cubeFin_1 = smesh.Mesh(cubeFin)
Regular_1D = cubeFin_1.Segment()
Nb_Segments_1 = Regular_1D.NumberOfSegments(20)
Nb_Segments_1.SetDistrType( 0 )
Quadrangle_2D = cubeFin_1.Quadrangle(algo=smeshBuilder.QUADRANGLE)
Hexa_3D = cubeFin_1.Hexahedron(algo=smeshBuilder.Hexa)
isDone = cubeFin_1.Compute()
DEPL_1 = cubeFin_1.GroupOnGeom(DEPL,'DEPL',SMESH.FACE)
ENCASTR_1 = cubeFin_1.GroupOnGeom(ENCASTR,'ENCASTR',SMESH.FACE)

## Set names of Mesh objects
smesh.SetName(Regular_1D.GetAlgorithm(), 'Regular_1D')
smesh.SetName(Quadrangle_2D.GetAlgorithm(), 'Quadrangle_2D')
smesh.SetName(Hexa_3D.GetAlgorithm(), 'Hexa_3D')
smesh.SetName(DEPL_1, 'DEPL')
smesh.SetName(ENCASTR_1, 'ENCASTR')
smesh.SetName(cubeFin_1.GetMesh(), 'cubeFin')
smesh.SetName(Nb_Segments_1, 'Nb. Segments_1')

cubeFin_1.ExportMED(os.path.join(gmu.pathBloc, "materielCasTests/cubeFin.med"))

if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()
