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

from .calculePointsAxiauxPipe_a import calculePointsAxiauxPipe_a
from .calculePointsAxiauxPipe_b import calculePointsAxiauxPipe_b
from .calculePointsAxiauxPipe_c import calculePointsAxiauxPipe_c

def calculePointsAxiauxPipe(edgesFondFiss, edgesIdByOrientation, facesDefaut, \
                            centreFondFiss, wireFondFiss, wirePipeFiss, \
                            lenSegPipe, rayonPipe, nbsegCercle, nbsegRad, \
                            nro_cas=None):
  """Préparation maillage du pipe :

  - détections des points a respecter : jonction des edges/faces constituant la face de fissure externe au pipe
  - points sur les edges de fond de fissure et edges pipe/face fissure,
  - vecteurs tangents au fond de fissure (normal au disque maillé)
  """

  logging.info('start')
  logging.info("Pour le cas n°%s", nro_cas)

  # --- Maillage selon le rayon de courbure du fond de fissure

  meshFondFiss = calculePointsAxiauxPipe_a(facesDefaut, centreFondFiss, wireFondFiss, \
                                           lenSegPipe, \
                                           nro_cas)
  # --- points géométriques

  centres, origins, normals = calculePointsAxiauxPipe_b(meshFondFiss, \
                                                        edgesFondFiss, edgesIdByOrientation, \
                                                        wireFondFiss, wirePipeFiss, \
                                                        rayonPipe)

  gptsdisks, raydisks = calculePointsAxiauxPipe_c(centres, origins, normals, \
                                                  rayonPipe, nbsegCercle, nbsegRad)

  return (centres, gptsdisks, raydisks)
