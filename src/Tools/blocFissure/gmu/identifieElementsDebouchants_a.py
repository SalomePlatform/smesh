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
"""Eléments débouchants (intersection pipe et peau), indexés selon les edges du fond de fissure (edgesFondIn)"""

import logging
import math

from . import initLog

from .geomsmesh import geompy
from .geomsmesh import geomPublishInFather

from .produitMixte import produitMixte

def identifieElementsDebouchants_a(iedf,\
                                   partitionPeauFissFond, edgesFondFiss, wireFondFiss, \
                                   verticesPipePeau, rayonPipe, edge):
  """Eléments débouchants (intersection pipe et peau), indexés selon les edges du fond de fissure (edgesFondIn)"""

  logging.info('start')

  name = "edgeFondIn{}".format(iedf)
  geomPublishInFather(initLog.debug, partitionPeauFissFond, edge, name)
  dist = [ geompy.MinDistance(pt, edge) for pt in verticesPipePeau]
  ptPeau = verticesPipePeau[dist.index(min(dist))] # le point de verticesPipePeau a distance minimale de l'edge
  [parametre, PointOnEdge, EdgeInWireIndex]  = geompy.MakeProjectionOnWire(ptPeau, wireFondFiss)
  logging.debug("parametre:%s, EdgeInWireIndex: %s, len(edgesFondFiss): %s", parametre, EdgeInWireIndex, len(edgesFondFiss))

  localEdgeInFondFiss = edgesFondFiss[EdgeInWireIndex]
  centre = PointOnEdge
  centre2 = geompy.MakeVertexOnCurve(localEdgeInFondFiss, parametre)
  geomPublishInFather(initLog.debug, partitionPeauFissFond, centre2, "centre2_{}".format(iedf))
  name = "verticeEdgesFondIn{}".format(iedf)
  geomPublishInFather(initLog.debug, partitionPeauFissFond, centre, name)
  norm = geompy.MakeTangentOnCurve(localEdgeInFondFiss, parametre)
  geomPublishInFather(initLog.debug, partitionPeauFissFond, centre, "norm{}".format(iedf))
  cercle = geompy.MakeCircle(centre, norm, rayonPipe)
  geomPublishInFather(initLog.debug, partitionPeauFissFond, cercle, "cerclorig{}".format(iedf))
  [vertex] = geompy.ExtractShapes(cercle, geompy.ShapeType["VERTEX"], False)
  vec1 = geompy.MakeVector(centre, vertex)
  vec2 = geompy.MakeVector(centre, ptPeau)
  angle = geompy.GetAngleRadians(vec1, vec2)
  # cas général : on reconstitue une portion de pipe, avec l'arête de couture qui coincide
  #   avec la face de fissure, au niveau du débouché sur la face externe
  # cas dégénéré : le pipe débouche perpendiculairement à une surface plane à l'origine.
  #   La partition filling / pipe reconstruit échoue.
  #   - Si on partitionne le filling avec un simple pipe obtenu par extrusion droite du cercle,
  #     cela donne un point en trop sur le cercle.
  #   - Si on prend une vraie surface plane (pas un filling), on peut faire la partition avec
  #     les pipes reconstruits
  logging.debug("angle=%s", angle)

  #if abs(angle) > 1.e-7:
  sommetAxe = geompy.MakeTranslationVector(centre, norm)
  if ( produitMixte(centre, vertex, ptPeau, sommetAxe) > 0 ):  # ajout de pi a (-)angle pour éviter des points confondus (partition échoue) dans les cas dégénérés
    cercle = geompy.MakeRotation(cercle, norm, angle + math.pi)
  else:
    cercle = geompy.MakeRotation(cercle, norm, -angle + math.pi)
  name = "cercle{}".format(iedf)
  geomPublishInFather(initLog.debug,partitionPeauFissFond, cercle, name)

  return ptPeau, centre, norm, localEdgeInFondFiss, localEdgeInFondFiss, cercle
