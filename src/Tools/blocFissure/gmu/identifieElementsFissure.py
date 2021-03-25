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

import logging

from .geomsmesh import geompy
from .geomsmesh import geomPublish
from .geomsmesh import geomPublishInFather
from . import initLog

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
    [edgesInside, edgesOutside, edgesOnside] = extractionOrientee(fillingFaceExterne, partitionPeauFissFond, centreFondFiss, "EDGE", 1.e-3)
    [facesInside, facesOutside, facesOnside] = extractionOrientee(fillingFaceExterne, partitionPeauFissFond, centreFondFiss, "FACE", 1.e-3)
  else:
    [edgesInside, edgesOutside, edgesOnside] = extractionOrienteeMulti(facesDefaut, ifil, partitionPeauFissFond, centreFondFiss, "EDGE", 1.e-3)
    [facesInside, facesOutside, facesOnside] = extractionOrienteeMulti(facesDefaut, ifil, partitionPeauFissFond, centreFondFiss, "FACE", 1.e-3)
    
  edgesPipeIn = geompy.GetSharedShapesMulti([edgesPipeC, geompy.MakeCompound(edgesInside)], geompy.ShapeType["EDGE"])
  verticesPipePeau = []

  for i, edge in enumerate(edgesPipeIn):
    try:
      vertices = geompy.GetSharedShapesMulti([edge, geompy.MakeCompound(facesOnside)], geompy.ShapeType["VERTEX"])
      verticesPipePeau.append(vertices[0])
      name = "edgePipeIn%d"%i
      geomPublishInFather(initLog.debug, partitionPeauFissFond, edge, name)
      name = "verticePipePeau%d"%i
      geomPublishInFather(initLog.debug, partitionPeauFissFond, vertices[0], name)
      logging.debug("edgePipeIn%s coupe les faces OnSide", i)
    except:
      logging.debug("edgePipeIn%s ne coupe pas les faces OnSide", i)

  edgesFondIn =[]
  if len(verticesPipePeau) > 0: # au moins une extrémité du pipe sur cette face de peau  
    #tmp = geompy.GetSharedShapesMulti([edgesFondC, geompy.MakeCompound(edgesOutside)], geompy.ShapeType["EDGE"])
    #edgesFondOut = [ ed for ed in tmp if geompy.MinDistance(ed, geompy.MakeCompound(facesOnside)) < 1.e-3] 
    tmp = geompy.GetSharedShapesMulti([edgesFondC, geompy.MakeCompound(edgesInside)], geompy.ShapeType["EDGE"])
    edgesFondIn = [ ed for ed in tmp if geompy.MinDistance(ed, geompy.MakeCompound(facesOnside)) < 1.e-3]

  return (edgesPipeIn, verticesPipePeau, edgesFondIn, facesInside, facesOnside) 