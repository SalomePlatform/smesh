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
"""tore et plan de fissure"""

import logging
import math

from .geomsmesh import geompy
from .geomsmesh import geomPublish

from . import initLog

from .triedreBase import triedreBase

O, OX, OY, OZ = triedreBase()

def toreFissure(minRad,allonge,rayTore):
  """
  Construction de la geometrie du tore elliptique autour du front de fissure.
  L'ellipse est construite dans le plan xoy, axe oy.
  @param minRad :petit rayon
  @param allonge :rapport grand rayon / petit rayon
  @param rayTore :rayon du tore construit autour de la generatrice de l'ellipse
  @return (generatrice, face_gen_fiss, pipe, face_fissure, plane, pipe_part) : ellipse, section du tore,
  tore plein, face plane de le fissure, plan de la fissure, tore partitioné par le plan de fissure.
  """
  logging.info("start %s %s %s", minRad, allonge, rayTore)

  vertex_1 = geompy.MakeVertex( minRad, 0., 0.)
  vertex_2 = geompy.MakeVertex(-minRad, 0., 0.)
  vertex_3 = geompy.Makerotation(vertex_1, OZ,  45.*math.pi/180.0)
  arc = geompy.MakeArc(vertex_1, vertex_2, vertex_3)
  generatrice = geompy.MakeScaleAlongAxes(arc, O, 1, allonge, 1)

  #geomPublish(initLog.debug,  vertex_1, 'vertex_1' )
  #geomPublish(initLog.debug,  vertex_2, 'vertex_2' )
  #geomPublish(initLog.debug,  vertex_3, 'vertex_3' )
  #geomPublish(initLog.debug,  arc, 'arc' )
  #geomPublish(initLog.debug,  generatrice, 'generatrice' )

  # --- face circulaire sur la generatrice, pour extrusion

  circle = geompy.MakeCircle(O, OY, rayTore)
  rotation = geompy.MakeRotation(circle, OY, -90.*math.pi/180.0)
  translation = geompy.MakeTranslation(rotation, minRad, 0., 0.)
  face_gen_fiss = geompy.MakeFaceWires([translation], 1)

  #geomPublish(initLog.debug,  circle, 'circle' )
  #geomPublish(initLog.debug,  rotation, 'rotation' )
  #geomPublish(initLog.debug,  translation, 'translation' )
  #geomPublish(initLog.debug,  face_gen_fiss, 'face_gen_fiss' )

  # --- tore extrude

  pipe = geompy.MakePipe(face_gen_fiss, generatrice)

  # --- plan fissure, delimite par la generatrice

  scale_vertex_3 = geompy.GetSubShape(generatrice, [3])
  line = geompy.MakeLineTwoPnt(vertex_1, scale_vertex_3)
  face_fissure = geompy.MakeFaceWires([generatrice, line], 1)

  #geomPublishInFather(initLog.debug, generatrice, scale_vertex_3, 'scale_1:vertex_3' )
  #geomPublish(initLog.debug,  line, 'line' )
  #geomPublish(initLog.debug,  face_fissure, 'face_fissure' )

  # --- tore coupe en 2 demi tore de section 1/2 disque

  plane = geompy.MakePlane(O, OZ, 2000)
  pipe_part = geompy.MakePartition([pipe], [plane], [], [], geompy.ShapeType["SOLID"], 0, [], 1)
  geomPublish(initLog.debug, pipe_part , 'pipe_part' )

  return generatrice, face_gen_fiss, pipe, face_fissure, plane, pipe_part
