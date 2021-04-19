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
"""Maillage selon le rayon de courbure du fond de fissure"""

import logging
import math

from .geomsmesh import geompy
from .geomsmesh import smesh

from .putName import putName

def calculePointsAxiauxPipe_a(facesDefaut, centreFondFiss, wireFondFiss, \
                              lenSegPipe, \
                              nro_cas=None):
  """Maillage selon le rayon de courbure du fond de fissure"""

  logging.info('start')

  # Rayon de courbure maximal
  disfond = list()
  for filling in facesDefaut:
    disfond.append(geompy.MinDistance(centreFondFiss, filling))
  disfond.sort()

  texte = "rcourb: {}, lenSegPipe: {}".format(disfond[0], lenSegPipe)
  logging.info(texte)

  # Maillage 1D
  lgmin = lenSegPipe*0.25
  lgmax = lenSegPipe*1.5
  # la déflexion ets la distance maximale entre une arête du maillage et la courbe support
  nbSegQuart = 5 # on veut 5 segments min sur un quart de cercle
  alpha = math.pi/(4*nbSegQuart)
  deflexion = disfond[0]*(1.0 -math.cos(alpha))
  texte = "==> lgmin: {}, lgmax: {}, deflexion: {}".format(deflexion, lgmin, lgmax)
  logging.info(texte)

  meshFondFiss = smesh.Mesh(wireFondFiss)
  putName(meshFondFiss, "wireFondFiss", i_pref=nro_cas)
  algo1d = meshFondFiss.Segment()
  putName(algo1d.GetSubMesh(), "wireFondFiss", i_pref=nro_cas)
  hypo1d = algo1d.Adaptive(lgmin, lgmax, deflexion) # a ajuster selon la profondeur de la fissure
  putName(hypo1d, "Adaptive_wireFondFiss", i_pref=nro_cas)

  is_done = meshFondFiss.Compute()
  text = "calculePointsAxiauxPipe meshFondFiss.Compute"
  if is_done:
    logging.info(text)
  else:
    text = "Erreur au calcul du maillage.\n" + text
    logging.info(text)
    raise Exception(text)

  return meshFondFiss
