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
"""Géométrie et maillage de base nécessaire au cas-test :
. vis_1
"""

import os
import math

import logging

import salome
from salome.smesh import smeshBuilder
from salome.StdMeshers import StdMeshersBuilder
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

geomObj_1 = geompy.MakeMarker(0, 0, 0, 1, 0, 0, 0, 1, 0)
geomObj_2 = geompy.MakeMarker(0, 0, 0, 1, 0, 0, 0, 1, 0)
sk = geompy.Sketcher2D()
sk.addPoint(0.000000, 0.000000)
sk.addSegmentAbsolute(10.000000, 0.000000)
sk.addSegmentAbsolute(10.000000, 98.750000)
sk.addArcAngleRadiusLength(0, -1.250000, 90.000000)
sk.addSegmentAbsolute(15.000000, 100.000000)
sk.addSegmentAbsolute(15.000000, 120.000000)
sk.addSegmentAbsolute(9.945000, 120.000000)
sk.addSegmentAbsolute(9.945000, 108.000000)
sk.addSegmentAbsolute(0.000000, 105.000000)
sk.close()
Sketch_1 = sk.wire(geomObj_2)
Face_1 = geompy.MakeFaceWires([Sketch_1], 1)
Vertex_1 = geompy.MakeVertex(15, 108, 0)
Vertex_2 = geompy.MakeVertex(0, 94, 0)
Vertex_3 = geompy.MakeVertex(10, 94, 0)
Face_1_vertex_17 = geompy.GetSubShape(Face_1, [17])
Line_1 = geompy.MakeLineTwoPnt(Vertex_1, Face_1_vertex_17)
Face_1_vertex_9 = geompy.GetSubShape(Face_1, [9])
Line_2 = geompy.MakeLineTwoPnt(Face_1_vertex_17, Face_1_vertex_9)
Face_1_vertex_19 = geompy.GetSubShape(Face_1, [19])
Face_1_vertex_7 = geompy.GetSubShape(Face_1, [7])
Line_3 = geompy.MakeLineTwoPnt(Face_1_vertex_19, Face_1_vertex_7)
Line_4 = geompy.MakeLineTwoPnt(Vertex_2, Vertex_3)
coupe_vis = geompy.MakePartition([Face_1], [Line_1, Line_2, Line_3, Line_4], [], [], geompy.ShapeType["FACE"], 0, [], 0)
[tige, section, tige_haute, rond, tete, section_tete] = geompy.Propagate(coupe_vis)
conge = geompy.CreateGroup(coupe_vis, geompy.ShapeType["EDGE"])
geompy.UnionIDs(conge, [25])
appui = geompy.CreateGroup(coupe_vis, geompy.ShapeType["EDGE"])
geompy.UnionIDs(appui, [39])
p_imp = geompy.CreateGroup(coupe_vis, geompy.ShapeType["EDGE"])
geompy.UnionIDs(p_imp, [11])

geomPublish(initLog.debug, Sketch_1, 'Sketch_1' )
geomPublish(initLog.debug, Face_1, 'Face_1' )
geomPublish(initLog.debug, Vertex_1, 'Vertex_1' )
geomPublish(initLog.debug, Vertex_2, 'Vertex_2' )
geomPublish(initLog.debug, Vertex_3, 'Vertex_3' )
geomPublishInFather(initLog.debug, Face_1, Face_1_vertex_17, 'Face_1:vertex_17' )
geomPublish(initLog.debug, Line_1, 'Line_1' )
geomPublishInFather(initLog.debug, Face_1, Face_1_vertex_9, 'Face_1:vertex_9' )
geomPublish(initLog.debug, Line_2, 'Line_2' )
geomPublishInFather(initLog.debug, Face_1, Face_1_vertex_19, 'Face_1:vertex_19' )
geomPublishInFather(initLog.debug, Face_1, Face_1_vertex_7, 'Face_1:vertex_7' )
geomPublish(initLog.debug, Line_3, 'Line_3' )
geomPublish(initLog.debug, Line_4, 'Line_4' )

geompy.addToStudy( coupe_vis, 'coupe_vis' )
geompy.addToStudyInFather( coupe_vis, tige, 'tige' )
geompy.addToStudyInFather( coupe_vis, section, 'section' )
geompy.addToStudyInFather( coupe_vis, tige_haute, 'tige_haute' )
geompy.addToStudyInFather( coupe_vis, rond, 'rond' )
geompy.addToStudyInFather( coupe_vis, tete, 'tete' )
geompy.addToStudyInFather( coupe_vis, section_tete, 'section_tete' )
geompy.addToStudyInFather( coupe_vis, conge, 'conge' )
geompy.addToStudyInFather( coupe_vis, appui, 'appui' )
geompy.addToStudyInFather( coupe_vis, p_imp, 'p_imp' )

Vertex_4 = geompy.MakeVertex(11.25, 98.75, 0)
Vertex_5 = geompy.MakeVertexWithRef(Vertex_4, -0.5, 0.5, 0)
Vertex_6 = geompy.MakeVertexWithRef(Vertex_4, -5, 5, 0)
Line_5 = geompy.MakeLineTwoPnt(Vertex_5, Vertex_6)
Partition_1 = geompy.MakePartition([Line_5], [conge], [], [], geompy.ShapeType["EDGE"], 0, [], 1)
[Vertex_7] = geompy.SubShapes(Partition_1, [4])
Vertex_8 = geompy.MakeVertexWithRef(Vertex_7, -1.1, 1.1, 0)
generatrice = geompy.MakeLineTwoPnt(Vertex_5, Vertex_8)
Revolution_1 = geompy.MakeRevolution2Ways(generatrice, OY, 60*math.pi/180.0)
Partition_2 = geompy.MakePartition([Revolution_1], [conge], [], [], geompy.ShapeType["FACE"], 0, [], 1)
Partition_2_vertex_11 = geompy.GetSubShape(Partition_2, [11])
Plane_1 = geompy.MakePlane(Partition_2_vertex_11, OY, 2000)
Partition_3 = geompy.MakePartition([Revolution_1], [Plane_1], [], [], geompy.ShapeType["FACE"], 0, [], 0)
Vertex_9 = geompy.MakeVertex(0, 99.633883, 1.977320000000001)
Vertex_10 = geompy.MakeVertex(0, 99.633883, -8.977320000000001)
Vertex_11 = geompy.MakeVertexWithRef(Vertex_9, 0, 0, -1)
Vertex11x = geompy.MakeVertexWithRef(Vertex_11, 1, 0, 0)
Line_11x = geompy.MakeLineTwoPnt(Vertex_11, Vertex11x)
Vertex_12 = geompy.MakeVertexWithRef(Vertex_10, 0, 0, 1)
Vertex12x = geompy.MakeVertexWithRef(Vertex_12, 1, 0, 0)
Line_12x = geompy.MakeLineTwoPnt(Vertex_12, Vertex12x)
Vertex_13 = geompy.MakeVertexWithRef(Vertex_11, 0, 1, 0)
Vertex_14 = geompy.MakeVertexWithRef(Vertex_12, 0, 1, 0)
Vertex_15 = geompy.MakeRotation(Vertex_9, Line_11x, 30*math.pi/180.0)
Vertex_16 = geompy.MakeRotation(Vertex_10, Line_12x, -30*math.pi/180.0)
Arc_1 = geompy.MakeArc(Vertex_15, Vertex_9, Vertex_13,False)
Arc_2 = geompy.MakeArc(Vertex_14, Vertex_10, Vertex_16,False)
Line_6 = geompy.MakeLineTwoPnt(Vertex_13, Vertex_14)
Line_8 = geompy.MakeLineTwoPnt(Vertex_16, Vertex_15)
Wire_1 = geompy.MakeWire([Arc_1, Arc_2, Line_6, Line_8], 1e-07)
Face_2 = geompy.MakeFaceWires([Wire_1], 1)
Extrusion_1 = geompy.MakePrismVecH(Face_2, OX, 15)
Revolution_2 = geompy.MakeRevolution2Ways(generatrice, OY, 65*math.pi/180.0)
Fissure = geompy.MakeCommonList([Extrusion_1, Revolution_2], True)
fondFiss = geompy.CreateGroup(Fissure, geompy.ShapeType["EDGE"])
geompy.UnionIDs(fondFiss, [6, 8])

geomPublish(initLog.debug, Vertex_4, 'Vertex_4' )
geomPublish(initLog.debug, Vertex_6, 'Vertex_6' )
geomPublish(initLog.debug, Vertex_5, 'Vertex_5' )
geomPublish(initLog.debug, Line_5, 'Line_5' )
geomPublish(initLog.debug, Partition_1, 'Partition_1' )
geomPublishInFather(initLog.debug, Partition_1, Vertex_7, 'Vertex_7' )
geomPublish(initLog.debug, Vertex_8, 'Vertex_8' )
geomPublish(initLog.debug, generatrice, 'generatrice' )
geomPublish(initLog.debug, Revolution_1, 'Revolution_1' )
geomPublish(initLog.debug, Partition_2, 'Partition_2' )
geomPublishInFather(initLog.debug, Partition_2, Partition_2_vertex_11, 'Partition_2:vertex_11' )
geomPublish(initLog.debug, Plane_1, 'Plane_1' )
geomPublish(initLog.debug, Partition_3, 'Partition_3' )
geomPublish(initLog.debug, Vertex_9, 'Vertex_9' )
geomPublish(initLog.debug, Vertex_10, 'Vertex_10' )
geomPublish(initLog.debug, Vertex_11, 'Vertex_11' )
geomPublish(initLog.debug, Vertex_12, 'Vertex_12' )
geomPublish(initLog.debug, Vertex_13, 'Vertex_13' )
geomPublish(initLog.debug, Vertex_14, 'Vertex_14' )
geomPublish(initLog.debug, Vertex_15, 'Vertex_15' )
geomPublish(initLog.debug, Vertex_16, 'Vertex_16' )
geomPublish(initLog.debug, Arc_1, 'Arc_1' )
geomPublish(initLog.debug, Arc_2, 'Arc_2' )
geomPublish(initLog.debug, Line_6, 'Line_6' )
geomPublish(initLog.debug, Line_8, 'Line_8' )
geomPublish(initLog.debug, Wire_1, 'Wire_1' )
geomPublish(initLog.debug, Face_2, 'Face_2' )
geomPublish(initLog.debug, Extrusion_1, 'Extrusion_1' )
geomPublish(initLog.debug, Revolution_2, 'Revolution_2' )
geompy.addToStudy( Fissure, 'vis_Fissure' )
geompy.addToStudyInFather( Fissure, fondFiss, 'fondFiss' )

ficcao = os.path.join(gmu.pathBloc, "materielCasTests", "visFiss.brep")
text = ".. Exportation de la géométrie de la fissure dans le fichier '{}'".format(ficcao)
logging.info(text)
geompy.ExportBREP(Fissure, ficcao)

###
### SMESH component
###

logging.info("Maillage de %s", coupe_vis.GetName())

smesh = smeshBuilder.New()
coupe_vis_1 = smesh.Mesh(coupe_vis)
putName(coupe_vis_1.GetMesh(), 'coupe_vis')

Regular_1D = coupe_vis_1.Segment()
Nb_Segments_1 = Regular_1D.NumberOfSegments(10)
Quadrangle_2D = coupe_vis_1.Quadrangle(algo=smeshBuilder.QUADRANGLE)
Regular_1D_1 = coupe_vis_1.Segment(geom=tige)
Nb_Segments_2 = Regular_1D_1.NumberOfSegments(30)
Regular_1D_2 = coupe_vis_1.Segment(geom=section)
Nb_Segments_3 = Regular_1D_2.NumberOfSegments(10,1,[ 7, 11, 16, 23 ])

_ = coupe_vis_1.GroupOnGeom(tige,'tige',SMESH.EDGE)
_ = coupe_vis_1.GroupOnGeom(section,'section',SMESH.EDGE)
_ = coupe_vis_1.GroupOnGeom(tige_haute,'tige_haute',SMESH.EDGE)
_ = coupe_vis_1.GroupOnGeom(rond,'rond',SMESH.EDGE)
_ = coupe_vis_1.GroupOnGeom(tete,'tete',SMESH.EDGE)
_ = coupe_vis_1.GroupOnGeom(section_tete,'section_tete',SMESH.EDGE)
_ = coupe_vis_1.GroupOnGeom(conge,'conge',SMESH.EDGE)
_ = coupe_vis_1.GroupOnGeom(appui,'appui',SMESH.EDGE)
_ = coupe_vis_1.GroupOnGeom(p_imp,'p_imp',SMESH.EDGE)

## set object names
#smesh.SetName(Regular_1D.GetAlgorithm(), 'Regular_1D')
#smesh.SetName(Quadrangle_2D.GetAlgorithm(), 'Quadrangle_2D')
putName(Nb_Segments_1, 'Nb. Segments_1', i_pref='vis')
putName(Nb_Segments_2, 'Nb. Segments_2', i_pref='vis')
putName(Nb_Segments_3, 'Nb. Segments_3', i_pref='vis')

is_done = coupe_vis_1.Compute()
text = "coupe_vis_1.Compute"
if is_done:
  logging.debug(text+" : OK")
else:
  text = "Erreur au calcul du maillage.\n" + text
  logging.info(text)
  raise Exception(text)

# Découpage en 2 triangles du quadrangle dans l'angle
cg_x=0.972772
cg_y=104.835
cg_z=0.
l_ids = coupe_vis_1.FindElementsByPoint( cg_x,cg_y,cg_z, SMESH.FACE )
if ( len(l_ids) != 1 ):
  text = "Maillage {}.\nImpossible de trouver l'élément proche de ({},{},{}).".format(coupe_vis.GetName(),cg_x,cg_y,cg_z)
  raise Exception(text)
isDone = coupe_vis_1.SplitQuad( l_ids, 1 )
text = "SplitQuad de l'élément n° {} du maillage de {}".format(l_ids[0],coupe_vis.GetName())
if isDone:
  logging.debug(text+" : OK")
else:
  text = "Erreur.\n" + text
  logging.info(text)
  raise Exception(text)

visHex80 = smesh.CopyMesh( coupe_vis_1, 'visHex80', 1, 0)
putName(visHex80.GetMesh(), 'visHex80')

_ = visHex80.RotationSweepObjects( [ visHex80 ], [ visHex80 ], [ visHex80 ], SMESH.AxisStruct( 0, 0, 0, 0, 10, 0 ), 0.0785398, 40, 1e-05, 1 )

ficmed = os.path.join(gmu.pathBloc, "materielCasTests","visSain.med")
text = ".. Archivage du maillage dans le fichier '{}'".format(ficmed)
logging.info(text)
visHex80.ExportMED(ficmed)

## Set names of Mesh objects

if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()
