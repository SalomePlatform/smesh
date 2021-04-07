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
"""Inventaire des faces de peau"""

import logging

from . import initLog

from .geomsmesh import geompy
from .geomsmesh import geomPublish

from .sortFaces import sortFaces
from .extractionOrientee import extractionOrientee

def identifieFacesPeau(ifil, verticesPipePeau, facesOnside, wireFondFiss, \
                       verticesEdgesFondIn, pipexts, cercles, \
                       fillingFaceExterne, centreFondFiss, \
                       nro_cas=None):
  """Inventaire des faces de peau : face de peau percée du pipe, extrémités du pipe

  La partition avec le pipe peut créer un vertex (et un edge) de trop sur le cercle projeté,
  quand le cercle est très proche de la face.
  dans ce cas, la projection du cercle sur la face suivie d'une partition permet
  d'éviter le point en trop
  """
  logging.info('start')

  facesAndFond = facesOnside
  facesAndFond.append(wireFondFiss)
  try:
    partitionPeauByPipe = geompy.MakePartition(facesAndFond, pipexts, [], [], geompy.ShapeType["FACE"], 0, [], 1)
  except:
    logging.debug("probleme partition face pipe, contournement avec MakeSection")
    sections = list()
    for pipext in pipexts:
      sections.append(geompy.MakeSection(facesOnside[0], pipext))
    partitionPeauByPipe = geompy.MakePartition(facesAndFond, sections, [], [], geompy.ShapeType["FACE"], 0, [], 1)

  # contrôle edge en trop sur edges circulaires
  if len(verticesPipePeau) > 0: # --- au moins une extrémité du pipe sur cette face de peau
    edgeEnTrop = list()
    outilPart = pipexts
    facesPeau = geompy.ExtractShapes(partitionPeauByPipe, geompy.ShapeType["FACE"], False)
    facesPeauSorted, _, _ = sortFaces(facesPeau)
    for face in facesPeauSorted[:-1]: # on ne teste que la ou les petites faces "circulaires"
      nbv = geompy.NumberOfEdges(face)
      logging.debug("nombre d'edges sur face circulaire: %s", nbv)
      edgeEnTrop.append(bool(nbv > 3)) # TODO : distinguer les cas avec deux faces circulaires dont l'une est correcte
    refaire = sum(edgeEnTrop)
    if refaire > 0:
      l_aux = [(geompy.MinDistance(verticesEdgesFondIn[0], fac), i_aux)  for i_aux, fac in enumerate(facesPeauSorted[:-1])]
      l_aux.sort()
      logging.debug("l_aux sorted: %s", l_aux)
      direct = bool(l_aux[0][1] == 0) # l_aux[0][1] = indice de facesPeauSorted qui correspond à verticesEdgesFondIn[0], donc 0 pour cercles
      for i_aux, bad in enumerate(edgeEnTrop):
        if direct:
          j_aux = i_aux
        else:
          j_aux = 1-i_aux
        if bad:
          outilPart[j_aux] = geompy.MakeProjection(cercles[j_aux],facesOnside[0])
      partitionPeauByPipe = geompy.MakePartition(facesAndFond, outilPart, [], [], geompy.ShapeType["FACE"], 0, [], 1)

  name="partitionPeauByPipe_{}".format(ifil)
  geomPublish(initLog.debug, partitionPeauByPipe, name)
  [edgesPeauFondIn, _, _] = extractionOrientee(fillingFaceExterne, partitionPeauByPipe, centreFondFiss, "EDGE", 1.e-3)
  [_, _, facesPeauFondOn] = extractionOrientee(fillingFaceExterne, partitionPeauByPipe, centreFondFiss, "FACE", 1.e-3)

  if verticesPipePeau: # --- au moins une extrémité du pipe sur cette face de peau
    facesPeauSorted, _, _ = sortFaces(facesPeauFondOn)
    facePeau = facesPeauSorted[-1] # la plus grande face
  else:
    facePeau =geompy.MakePartition(facesPeauFondOn, [], [], [], geompy.ShapeType["FACE"], 0, [], 1)
    facesPeauSorted = [facePeau]

  name="facePeau_{}".format(ifil)
  geomPublish(initLog.always, facePeau, name, nro_cas)

  return (facePeau, facesPeauSorted, edgesPeauFondIn)
