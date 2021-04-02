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
"""Identification edges fond de fissure, edges pipe sur la face de fissure,
edges prolongées
edges internes communes pipe et fissure, points communs edges fissure peau et edges circulaires
"""

import logging

from . import initLog

from .geomsmesh import geompy
from .geomsmesh import geomPublishInFather

from .extractionOrientee import extractionOrientee
from .extractionOrienteeMulti import extractionOrienteeMulti

def identifieElementsFissure(ifil, facesDefaut, partitionPeauFissFond,
                             edgesPipeFiss, edgesFondFiss, aretesVivesC,
                             fillingFaceExterne, centreFondFiss):
  """
  # -----------------------------------------------------------------------
  # --- identification edges fond de fissure, edges pipe sur la face de fissure,
  #     edges prolongées
  #     edges internes communes pipe et fissure, points communs edges fissure peau et edges circulaires
  """

  logging.info('start')

  edgesPipeC = geompy.GetInPlace(partitionPeauFissFond, geompy.MakeCompound(edgesPipeFiss))
  geomPublishInFather(initLog.debug, partitionPeauFissFond, edgesPipeC, "edgesPipeFiss")
  edgesFondC = geompy.GetInPlace(partitionPeauFissFond, geompy.MakeCompound(edgesFondFiss))
  geomPublishInFather(initLog.debug, partitionPeauFissFond, edgesFondC, "edgesFondFiss")

  if aretesVivesC is None:
    [edgesInside, _, _] = extractionOrientee(fillingFaceExterne, partitionPeauFissFond, centreFondFiss, "EDGE", 1.e-3)
    [facesInside, _, facesOnside] = extractionOrientee(fillingFaceExterne, partitionPeauFissFond, centreFondFiss, "FACE", 1.e-3)
  else:
    [edgesInside, _, _] = extractionOrienteeMulti(facesDefaut, ifil, partitionPeauFissFond, centreFondFiss, "EDGE", 1.e-3)
    [facesInside, _, facesOnside] = extractionOrienteeMulti(facesDefaut, ifil, partitionPeauFissFond, centreFondFiss, "FACE", 1.e-3)

  edgesPipeIn = geompy.GetSharedShapesMulti([edgesPipeC, geompy.MakeCompound(edgesInside)], geompy.ShapeType["EDGE"])
  verticesPipePeau = list()

  for i_aux, edge in enumerate(edgesPipeIn):
    try:
      vertices = geompy.GetSharedShapesMulti([edge, geompy.MakeCompound(facesOnside)], geompy.ShapeType["VERTEX"])
      verticesPipePeau.append(vertices[0])
      name = "edgePipeIn{}".format(i_aux)
      geomPublishInFather(initLog.debug, partitionPeauFissFond, edge, name)
      name = "verticePipePeau{}".format(i_aux)
      geomPublishInFather(initLog.debug, partitionPeauFissFond, vertices[0], name)
      logging.debug("edgePipeIn%s coupe les faces OnSide", i_aux)
    except:
      logging.debug("edgePipeIn%s ne coupe pas les faces OnSide", i_aux)

  edgesFondIn = list()
  if verticesPipePeau: # au moins une extrémité du pipe sur cette face de peau
    tmp = geompy.GetSharedShapesMulti([edgesFondC, geompy.MakeCompound(edgesInside)], geompy.ShapeType["EDGE"])
    edgesFondIn = [ ed for ed in tmp if geompy.MinDistance(ed, geompy.MakeCompound(facesOnside)) < 1.e-3]

  return (edgesPipeIn, verticesPipePeau, edgesFondIn, facesInside, facesOnside)
