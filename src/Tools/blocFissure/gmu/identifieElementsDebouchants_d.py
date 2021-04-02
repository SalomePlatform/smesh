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
"""Recherche edges communes entre une face inside et (faces onside, edges pipe et fond débouchante)"""

import logging

from . import initLog

from .geomsmesh import geompy
from .geomsmesh import geomPublishInFather

def identifieElementsDebouchants_d(iedf,\
                                   partitionPeauFissFond, edgesFondIn, edgesPipeIn, \
                                   facesInside, facesOnside, \
                                   ptPeau, facesFissExt, edgesFissExtPeau, edgesFissExtPipe):
  """Recherche edges communes entre une face inside et (faces onside, edges pipe et fond débouchante)"""

  for ifa, face in enumerate(facesInside):
    logging.debug("recherche edges communes entre une face inside et (faces onside, edges pipe et fond débouchante)")
    edgesPeauFis = list()
    edgesPipeFis = list()
    edgesPipeFnd = list()
    try:
      edgesPeauFis = geompy.GetSharedShapesMulti([geompy.MakeCompound(facesOnside), face], geompy.ShapeType["EDGE"])
      logging.debug("    faces onside %s",edgesPeauFis)
      edgesPipeFis = geompy.GetSharedShapesMulti([geompy.MakeCompound(edgesPipeIn), face], geompy.ShapeType["EDGE"])
      logging.debug("    edgesPipeIn %s", edgesPipeFis)
      edgesPipeFnd = geompy.GetSharedShapesMulti([geompy.MakeCompound(edgesFondIn), face], geompy.ShapeType["EDGE"])
      logging.debug("    edgesFondIn %s ", edgesPipeFnd)
    except:
      logging.debug("  pb edges communes %s %s %s",edgesPeauFis, edgesPipeFis, edgesPipeFnd)
    if (len(edgesPeauFis) > 0) and (len(edgesPipeFis) > 0) and (len(edgesPipeFnd) == 0):
      dist = geompy.MinDistance(geompy.MakeCompound(edgesPeauFis), ptPeau)
      logging.debug("    test distance extrémité reference %s", dist)
      if dist < 1.e-3: # c'est la face de fissure externe associée
        logging.debug("    face %s inside ajoutée", ifa)
        facesFissExt.append(face)
        name="faceFissExt%d"%iedf
        geomPublishInFather(initLog.debug, partitionPeauFissFond, face, name)
        dist = 1.
        for edpe in edgesPeauFis:
          for edpi in edgesPipeFis:
            dist = geompy.MinDistance(edpe, edpi)
            if dist < 1.e-3:
              edgesFissExtPeau.append(edpe)
              name="edgesFissExtPeau%d"%iedf
              geomPublishInFather(initLog.debug, partitionPeauFissFond, edpe, name)
              edgesFissExtPipe.append(edpi)
              name="edgesFissExtPipe%d"%iedf
              geomPublishInFather(initLog.debug, partitionPeauFissFond, edpi, name)
              break
          if dist < 1.e-3:
            break

  return
