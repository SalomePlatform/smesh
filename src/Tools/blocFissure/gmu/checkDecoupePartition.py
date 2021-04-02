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
"""Teste si l'opération de partition a produit une modification"""

import logging
from .geomsmesh import geompy


def checkDecoupePartition(shapes, part):
  """Teste si l'opération de partition a produit une découpe (plus de shapes dans la partition).

  Résultat non garanti si recouvrement des shapes d'origine.
  @param shapes : liste des shapes d'origine
  @param part : résultat de la partition
  @return True si la partition a découpé les shapes d'origine
  """
  logging.info('start')
  # TODO: ShapeInfo donne des résultats faux (deux faces au lieu de une)

  orig = dict()
  for shape in shapes:
    info = geompy.ShapeInfo(shape)
    logging.debug("shape info %s", info)
    for type_shape in ['VERTEX', 'EDGE', 'FACE', 'SOLID']:
      if type_shape in orig:
        orig[type_shape] += info[type_shape]
      else:
        orig[type_shape] = info[type_shape]
  logging.debug("original shapes info %s", orig)

  info = geompy.ShapeInfo(part)
  logging.debug("partition info %s", info)
  decoupe = False
  for type_shape in ['VERTEX', 'EDGE', 'FACE', 'SOLID']:
    if orig[type_shape] < info[type_shape]:
      decoupe = True
      break
  logging.debug("partition modifie l'original %s", decoupe)

  return decoupe
