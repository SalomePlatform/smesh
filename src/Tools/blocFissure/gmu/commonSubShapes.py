# -*- coding: utf-8 -*-
# Copyright (C) 2014-2020  EDF R&D
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

# -----------------------------------------------------------------------------
# --- subShapes communes à deux listes

def commonSubShapes(obj, sub1, sub2):
  """
  liste de subshapes communes
  """
  logging.info("start")
  idsub1 = {}
  subList = []
  for s in sub1:
    idsub1[geompy.GetSubShapeID(obj, s)] = s
  for s in sub2:
    idsub = geompy.GetSubShapeID(obj, s)
    if idsub in list(idsub1.keys()):
      subList.append(s)
  logging.debug("subList=%s", subList)
  return subList
