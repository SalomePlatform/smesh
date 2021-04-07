# -*- coding: utf-8 -*-
# Copyright (C) 2014-2021  EDF R&D
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
"""Géométries et maillages de base nécessaires aux cas-tests :
. cubeFin_Transverse
. cubeFin_Coin
. cubeFin_Milieu
"""

import os
import math

import logging

import salome
from salome.smesh import smeshBuilder
import GEOM
import SMESH
import SALOMEDS

from blocFissure import gmu
from blocFissure.gmu.geomsmesh import geompy
from blocFissure.gmu.geomsmesh import geomPublish
from blocFissure.gmu.geomsmesh import geomPublishInFather

from blocFissure.gmu.triedreBase import triedreBase
from blocFissure.gmu.putName import putName
from blocFissure.gmu import initLog

###
### GEOM component
###

O, OX, OY, OZ = triedreBase()

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

geompy.addToStudy( cubeFin, 'cubeFin' )
geompy.addToStudyInFather( cubeFin, DEPL, 'DEPL' )
geompy.addToStudyInFather( cubeFin, ENCASTR, 'ENCASTR' )

geomPublish(initLog.debug, origCoin, 'origCoin' )
geomPublish(initLog.debug, Disk_1, 'Disk_1' )
geomPublish(initLog.debug, Translation_1, 'Translation_1' )
geomPublish(initLog.debug, Vector_1, 'Vector_1' )
geomPublish(initLog.debug, Rotation_1, 'Rotation_1' )
geomPublish(initLog.debug, origMilieu, 'origMilieu' )
geomPublish(initLog.debug, Translation_2, 'Translation_2' )
geomPublish(initLog.debug, Scale_1, 'Scale_1' )
geomPublish(initLog.debug, Vertex_1, 'Vertex_1' )
geomPublish(initLog.debug, Vertex_2, 'Vertex_2' )
geomPublish(initLog.debug, Vertex_3, 'Vertex_3' )
geomPublish(initLog.debug, Vertex_4, 'Vertex_4' )
geomPublish(initLog.debug, Line_1, 'Line_1' )
geomPublish(initLog.debug, Line_2, 'Line_2' )
geomPublish(initLog.debug, Line_3, 'Line_3' )
geomPublish(initLog.debug, Line_4, 'Line_4' )

geomPublishInFather(initLog.debug, Line_1, Line_1_vertex_3, 'Line_1:vertex_3' )
geomPublish(initLog.debug, Vertex_5, 'Vertex_5' )
geomPublishInFather(initLog.debug, Line_3, Line_3_vertex_2, 'Line_3:vertex_2' )
geomPublish(initLog.debug, Arc_1, 'Arc_1' )
geomPublish(initLog.debug, Face_1, 'Face_1' )

geompy.addToStudy( cubeFin_Transverse, 'cubeFin_Transverse_fissure' )
geompy.addToStudy( cubeFin_Coin, 'cubeFin_Coin_fissure' )
geompy.addToStudy( cubeFin_Milieu, 'cubeFin_Milieu_fissure' )

geompy.ExportBREP(cubeFin_Transverse, os.path.join(gmu.pathBloc, "materielCasTests", "cubeFin_Transverse.brep"))
geompy.ExportBREP(cubeFin_Coin, os.path.join(gmu.pathBloc, "materielCasTests", "cubeFin_Coin.brep"))
geompy.ExportBREP(cubeFin_Milieu, os.path.join(gmu.pathBloc, "materielCasTests", "cubeFin_Milieu.brep"))

###
### SMESH component
###

smesh = smeshBuilder.New()
cubeFin_1 = smesh.Mesh(cubeFin)
putName(cubeFin_1.GetMesh(), 'cubeFin')

Regular_1D = cubeFin_1.Segment()
Nb_Segments_1 = Regular_1D.NumberOfSegments(20)
Nb_Segments_1.SetDistrType( 0 )
Quadrangle_2D = cubeFin_1.Quadrangle(algo=smeshBuilder.QUADRANGLE)
Hexa_3D = cubeFin_1.Hexahedron(algo=smeshBuilder.Hexa)
_ = cubeFin_1.GroupOnGeom(DEPL,'DEPL',SMESH.FACE)
_ = cubeFin_1.GroupOnGeom(ENCASTR,'ENCASTR',SMESH.FACE)

## Set names of Mesh objects
#putName(Regular_1D.GetAlgorithm(), 'Regular_1D')
#putName(Quadrangle_2D.GetAlgorithm(), 'Quadrangle_2D')
#putName(Hexa_3D.GetAlgorithm(), 'Hexa_3D')
putName(Nb_Segments_1, 'Nb. Segments_1', i_pref='cubeFin')

is_done = cubeFin_1.Compute()
text = "cubeFin_1.Compute"
if is_done:
  logging.info(text+" OK")
else:
  text = "Erreur au calcul du maillage.\n" + text
  logging.info(text)
  raise Exception(text)

cubeFin_1.ExportMED(os.path.join(gmu.pathBloc, "materielCasTests", "cubeFin.med"))

if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()
