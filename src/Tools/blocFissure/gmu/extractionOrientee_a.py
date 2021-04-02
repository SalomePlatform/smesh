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
"""Détermine le bon côté"""

import logging
from . import initLog

from .geomsmesh import geomPublishInFather

def extractionOrientee_a(obj, shape, side, side_ref, \
                         shapesInside, shapesOutside, shapesOnside, \
                         i_aux, j_aux, k_aux, \
                         trace=True, prefix=""):
  """Détermine le bon côté"""
  #logging.info('start')

  if side == side_ref:
    shapesInside.append(shape)
    if trace:
      name = prefix + "_Inside%d"%i_aux
      geomPublishInFather(initLog.debug, obj, shape, name)
    i_aux += 1

  elif side == -side_ref:
    shapesOutside.append(shape)
    if trace:
      name = prefix + "_Outside%d"%j_aux
      geomPublishInFather(initLog.debug, obj, shape, name)
    j_aux += 1

  elif side == 0:
    shapesOnside.append(shape)
    if trace:
      name = prefix + "_Onside%d"%k_aux
      geomPublishInFather(initLog.debug, obj, shape, name)
    k_aux += 1

  logging.debug("--- shape was %s", name)

  return i_aux, j_aux, k_aux
