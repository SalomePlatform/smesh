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
"""Recherche et classement des edges du tore par propagate"""


import logging

from .geomsmesh import geompy
from .geomsmesh import geomPublishInFather

from . import initLog

def propagateTore(tore):
  """Classement des edges du tore par une operation 'propagate'

  @param tore partionné et coupé
  @return (diams, circles, geners) edges dans le plan de fissure, edges demi circulaires,
  edges selon la generatrice (liste de compounds)
  """
  logging.info("start")

  lencomp = list()
  compounds = geompy.Propagate(tore)
  for objet in compounds:
    #geomPublishInFather(initLog.debug, tore, objet, 'edges' )
    props = geompy.BasicProperties(objet)
    lencomp.append(props[0])
# Remarque : on passe par min et max plutôt que faire lencomp.sort() pour garder l'ordre dans le tri suivant
  minlen = min(lencomp)
  maxlen = max(lencomp)

  diams = list()
  geners = list()
  circles = list()
  for i_aux, longueur in enumerate(lencomp):
    if ( (longueur- minlen)/minlen < 0.01 ):
      diams.append(compounds[i_aux])
    elif ( (maxlen - longueur)/longueur < 0.2 ):
      geners.append(compounds[i_aux])
    else:
      circles.append(compounds[i_aux])

  geomPublishInFather(initLog.debug, tore, diams[0], 'diams0' )
  geomPublishInFather(initLog.debug, tore, diams[1], 'diams1' )
  geomPublishInFather(initLog.debug, tore, circles[0], 'circles0' )
  geomPublishInFather(initLog.debug, tore, circles[1], 'circles1' )
  geomPublishInFather(initLog.debug, tore, geners[0], 'geners' )

  return diams, circles, geners
