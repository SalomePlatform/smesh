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
"""tri par volume de solides"""

import logging

from .geomsmesh import geompy

def sortSolids(solidsToSort):
  """tri des solides par volume"""
  logging.info('start')

  volSolids = [(geompy.BasicProperties(solid)[2], i, solid) for i, solid in enumerate(solidsToSort)]
  volSolids.sort()
  solidsSorted = [solid for _, i, solid in volSolids]

  return solidsSorted, volSolids[0][0], volSolids[-1][0]
