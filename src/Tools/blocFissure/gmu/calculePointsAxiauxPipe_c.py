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
"""Préparation maillage du pipe"""

import logging
import math

from .geomsmesh import geompy

def calculePointsAxiauxPipe_c(centres, origins, normals, \
                              rayonPipe, nbsegCercle, nbsegRad):
  """Préparation maillage du pipe :

  - détections des points a respecter : jonction des edges/faces constituant la face de fissure externe au pipe
  - points sur les edges de fond de fissure et edges pipe/face fissure,
  - vecteurs tangents au fond de fissure (normal au disque maillé)
  """

  logging.info('start')
  logging.debug("nbsegCercle = %d, nbsegRad = %d", nbsegCercle, nbsegRad)

  # -----------------------------------------------------------------------
  # --- points géométriques

  gptsdisks = list() # vertices géométrie de tous les disques
  raydisks = [list() for _ in range(nbsegCercle)]

# boucle sur les disques
  for indice, centres_i in enumerate(centres):
    gptdsk = list() # vertices géométrie d'un disque
    vertcx = centres_i
    vertpx = origins[indice]
    normal = normals[indice]
    vec1 = geompy.MakeVector(vertcx, vertpx)

    points = [vertcx] # les points du rayon de référence
    dist_0 = rayonPipe/float(nbsegRad)
    for j_aux in range(nbsegRad):
      point = geompy.MakeTranslationVectorDistance(vertcx, vec1, float(j_aux+1)*dist_0)
      points.append(point)
    gptdsk.append(points)
    point = geompy.MakeTranslationVectorDistance(vertcx, vec1, 1.5*rayonPipe)
    rayon = geompy.MakeLineTwoPnt(vertcx, point)
    raydisks[0].append(rayon)

    angle_0 = 2.*math.pi/float(nbsegCercle)
    for k_aux in range(nbsegCercle-1):
      angle = float(k_aux+1)*angle_0
      pts = [vertcx] # les points d'un rayon obtenu par rotation
      for j_aux in range(nbsegRad):
        point = geompy.MakeRotation(points[j_aux+1], normal, angle)
        pts.append(point)
      gptdsk.append(pts)
      ray = geompy.MakeRotation(rayon, normal, angle)
      raydisks[k_aux+1].append(ray)

    gptsdisks.append(gptdsk)

  return gptsdisks, raydisks
