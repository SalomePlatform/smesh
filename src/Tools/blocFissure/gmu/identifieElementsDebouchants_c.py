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
"""Position des points extremite du pipe sur l'edge debouchante"""

import logging

import traceback

from . import initLog

from .geomsmesh import geompy
from .geomsmesh import geomPublishInFather

from .fissError import fissError

def identifieElementsDebouchants_c(iedf,\
                                   partitionPeauFissFond, wireFondFiss, \
                                   centre, localEdgeInFondFiss, cercle, loc_pt0, lgp, pipexts):
  """Position des points extremite du pipe sur l'edge debouchante"""

  logging.info('start')

  # ---
  #     il faut la distance curviligne ofp du point central par rapport à une extrémité de l'edge débouchante
  locEdgePart = geompy.MakePartition([localEdgeInFondFiss],[centre], list(), list(), geompy.ShapeType["EDGE"], 0, list(), 0)
  edgesLoc = geompy.ExtractShapes(locEdgePart, geompy.ShapeType["EDGE"], False)
  edgesLocSorted =[(geompy.MinDistance(edge, loc_pt0), kk, edge) for kk, edge in enumerate(edgesLoc)]
  edgesLocSorted.sort()
  try:
    ofp = geompy.BasicProperties(edgesLocSorted[0][2])[0] # distance curviligne centre loc_pt0
  except:
    texte = "Identification des éléments au débouché du pipe sur la face externe impossible. "
    texte += "Cause possible : la ligne de fond de fissure comprend un point géométrique coincidant avec la face externe. "
    texte += "La ligne de fond de fissure doit déboucher franchement de la face externe, et ne doit pas être coupée au niveau de la face."
    raise fissError(traceback.extract_stack(),texte)
  logging.debug("distance curviligne centre extremite0: %s", ofp)
  point_1 = geompy.MakeVertexOnCurveByLength(localEdgeInFondFiss, ofp +lgp, loc_pt0)
  point_2 = geompy.MakeVertexOnCurveByLength(localEdgeInFondFiss, ofp -lgp, loc_pt0)
  geomPublishInFather(initLog.debug, wireFondFiss, point_1, "point_1_{}".format(iedf))
  geomPublishInFather(initLog.debug, wireFondFiss, point_2, "point_2_{}".format(iedf))

  edgePart = geompy.MakePartition([localEdgeInFondFiss], [point_1,point_2], list(), list(), geompy.ShapeType["EDGE"], 0, list(), 0)
  edps = geompy.ExtractShapes(edgePart, geompy.ShapeType["EDGE"], True)
  for edp in edps:
    if geompy.MinDistance(centre, edp) < 1.e-3:
      pipext = geompy.MakePipe(cercle, edp)
      name = "pipeExt{}".format(iedf)
      geomPublishInFather(initLog.debug, partitionPeauFissFond, pipext, name)
      pipexts.append(pipext)

  return
