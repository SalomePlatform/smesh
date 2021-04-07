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
. cylindre
. cylindre_2
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

Vertex_1 = geompy.MakeVertex(0, 0, 500)
Vertex_2 = geompy.MakeVertex(100, 0, 500)
Vertex_3 = geompy.MakeVertex(110, 0, 500)
Vertex_4 = geompy.MakeVertex(117.071068, -2.928932, 500)
Vertex_5 = geompy.MakeVertex(120, -10, 500)
Vertex_6 = geompy.MakeVertex(120, -30, 500)
Vertex_7 = geompy.MakeVertex(122.928932, -37.071068, 500)
Vertex_8 = geompy.MakeVertex(130, -40, 500)
Vertex_9 = geompy.MakeVertex(135, -40, 500)
Vertex_10 = geompy.MakeVertex(160, -40, 500)
Plane_1 = geompy.MakePlaneLCS(None, 2000, 2)
Mirror_1_1 = geompy.MakeMirrorByPlane(Vertex_2, Plane_1)
Mirror_1_2 = geompy.MakeMirrorByPlane(Vertex_3, Plane_1)
Mirror_1_3 = geompy.MakeMirrorByPlane(Vertex_4, Plane_1)
Mirror_1_4 = geompy.MakeMirrorByPlane(Vertex_5, Plane_1)
Mirror_1_5 = geompy.MakeMirrorByPlane(Vertex_6, Plane_1)
Mirror_1_6 = geompy.MakeMirrorByPlane(Vertex_7, Plane_1)
Mirror_1_7 = geompy.MakeMirrorByPlane(Vertex_8, Plane_1)
Mirror_1_8 = geompy.MakeMirrorByPlane(Vertex_9, Plane_1)
Mirror_1_9 = geompy.MakeMirrorByPlane(Vertex_10, Plane_1)
Curve_2 = geompy.MakeInterpol([Mirror_1_9, Mirror_1_8, Mirror_1_7, Mirror_1_6, Mirror_1_5, Mirror_1_4, Mirror_1_3, Mirror_1_2, Mirror_1_1, Vertex_1, Vertex_2, Vertex_3, Vertex_4, Vertex_5, Vertex_6, Vertex_7, Vertex_8, Vertex_9, Vertex_10], False, False)
Divided_Cylinder_1 = geompy.MakeDividedCylinder(145, 800, GEOM.SQUARE)
CylindreSain = geompy.MakeRotation(Divided_Cylinder_1, OZ, 45*math.pi/180.0)
[Compound_1, vertical, radial, Compound_4] = geompy.Propagate(CylindreSain)
geompy.addToStudy( CylindreSain, 'CylindreSain' )
geomPublishInFather(initLog.debug,CylindreSain, Compound_1, 'Compound_1' )
geompy.addToStudyInFather(CylindreSain, vertical, 'vertical' )
geompy.addToStudyInFather(CylindreSain, radial, 'radial' )
geompy.addToStudyInFather(CylindreSain, Compound_4, 'Compound_4' )

Circle_1 = geompy.MakeCircle(Vertex_1, None, 145)
Vertex_11 = geompy.MakeVertex(0, -165, 500)
Curve_2_vertex_2 = geompy.GetSubShape(Curve_2, [2])
Curve_2_vertex_3 = geompy.GetSubShape(Curve_2, [3])
Arc_1 = geompy.MakeArc(Curve_2_vertex_2, Vertex_11, Curve_2_vertex_3)
FissInCylindre = geompy.MakeFaceWires([Curve_2, Arc_1], 1)
geompy.addToStudy( FissInCylindre, 'cylindre_fissure' )

geompy.ExportBREP(FissInCylindre, os.path.join(gmu.pathBloc, "materielCasTests", "FissInCylindre.brep"))

Vertex_12 = geompy.MakeVertex(0, -145, 500)
Circle_2 = geompy.MakeCircle(Vertex_12, None, 145)
Face_1 = geompy.MakeFaceWires([Circle_2], 1)
Vertex_13 = geompy.MakeVertex(0, 0, 500)
Disk_1 = geompy.MakeDiskPntVecR(Vertex_13, OZ, 170)
FissInCylindre2 = geompy.MakeCommon(Face_1, Disk_1)
geompy.addToStudy( FissInCylindre2, 'cylindre_2_fissure' )

geompy.ExportBREP(FissInCylindre2, os.path.join(gmu.pathBloc, "materielCasTests", "FissInCylindre2.brep"))

geomPublish(initLog.debug, Vertex_1, 'Vertex_1' )
geomPublish(initLog.debug, Vertex_2, 'Vertex_2' )
geomPublish(initLog.debug, Vertex_3, 'Vertex_3' )
geomPublish(initLog.debug, Vertex_4, 'Vertex_4' )
geomPublish(initLog.debug, Vertex_5, 'Vertex_5' )
geomPublish(initLog.debug, Vertex_6, 'Vertex_6' )
geomPublish(initLog.debug, Vertex_7, 'Vertex_7' )
geomPublish(initLog.debug, Vertex_8, 'Vertex_8' )
geomPublish(initLog.debug, Vertex_9, 'Vertex_9' )
geomPublish(initLog.debug, Vertex_10, 'Vertex_10' )
geomPublish(initLog.debug, Plane_1, 'Plane_1' )
geomPublish(initLog.debug, Mirror_1_1, 'Mirror_1_1' )
geomPublish(initLog.debug, Mirror_1_2, 'Mirror_1_2' )
geomPublish(initLog.debug, Mirror_1_3, 'Mirror_1_3' )
geomPublish(initLog.debug, Mirror_1_4, 'Mirror_1_4' )
geomPublish(initLog.debug, Mirror_1_5, 'Mirror_1_5' )
geomPublish(initLog.debug, Mirror_1_6, 'Mirror_1_6' )
geomPublish(initLog.debug, Mirror_1_7, 'Mirror_1_7' )
geomPublish(initLog.debug, Mirror_1_8, 'Mirror_1_8' )
geomPublish(initLog.debug, Mirror_1_9, 'Mirror_1_9' )
geomPublish(initLog.debug, Curve_2, 'Curve_2' )
geomPublish(initLog.debug, Circle_1, 'Circle_1' )
geomPublish(initLog.debug, Vertex_11, 'Vertex_11' )
geomPublishInFather(initLog.debug, Curve_2, Curve_2_vertex_2, 'Curve_2:vertex_2' )
geomPublishInFather(initLog.debug, Curve_2, Curve_2_vertex_3, 'Curve_2:vertex_3' )
geomPublish(initLog.debug, Arc_1, 'Arc_1' )
geomPublish(initLog.debug, FissInCylindre, 'FissInCylindre' )
geomPublish(initLog.debug, Divided_Cylinder_1, 'Divided Cylinder_1' )
geomPublish(initLog.debug, Vertex_12, 'Vertex_12' )
geomPublish(initLog.debug, Circle_2, 'Circle_2' )
geomPublish(initLog.debug, Face_1, 'Face_1' )
geomPublish(initLog.debug, Vertex_13, 'Vertex_13' )
geomPublish(initLog.debug, Disk_1, 'Disk_1' )
geomPublish(initLog.debug, FissInCylindre2, 'FissInCylindre2' )

###
### SMESH component
###

smesh = smeshBuilder.New()
CylindreSain_1 = smesh.Mesh(CylindreSain)
putName(CylindreSain_1.GetMesh(), 'CylindreSain')

Regular_1D = CylindreSain_1.Segment()
Nb_Segments_1 = Regular_1D.NumberOfSegments(15,[],[  ])
Nb_Segments_1.SetDistrType( 0 )
Quadrangle_2D = CylindreSain_1.Quadrangle(algo=smeshBuilder.QUADRANGLE)
Hexa_3D = CylindreSain_1.Hexahedron(algo=smeshBuilder.Hexa)
Regular_1D_1 = CylindreSain_1.Segment(geom=vertical)
Nb_Segments_2 = Regular_1D_1.NumberOfSegments(30,[],[  ])
Nb_Segments_2.SetDistrType( 0 )
Regular_1D_2 = CylindreSain_1.Segment(geom=radial)
Nb_Segments_3 = Regular_1D_2.NumberOfSegments(6,[],[  ])
Nb_Segments_3.SetDistrType( 0 )

## set object names
#smesh.SetName(Regular_1D.GetAlgorithm(), 'Regular_1D')
#smesh.SetName(Quadrangle_2D.GetAlgorithm(), 'Quadrangle_2D')
#smesh.SetName(Hexa_3D.GetAlgorithm(), 'Hexa_3D')
putName(Nb_Segments_1, 'Nb. Segments_1', i_pref='cylindre')
putName(Nb_Segments_2, 'Nb. Segments_2', i_pref='cylindre')
putName(Nb_Segments_3, 'Nb. Segments_3', i_pref='cylindre')

is_done = CylindreSain_1.Compute()
text = "CylindreSain_1.Compute"
if is_done:
  logging.info(text+" OK")
else:
  text = "Erreur au calcul du maillage.\n" + text
  logging.info(text)
  raise Exception(text)

CylindreSain_1.ExportMED(os.path.join(gmu.pathBloc, "materielCasTests", "CylindreSain.med"))

if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()
