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
"""construction de la fissure générale - le point interne"""

import logging

# -----------------------------------------------------------------------------

def construitFissureGenerale_a (geompy, shapeFissureParams):
  """construction de la fissure générale - le point interne"""
  logging.info('start')

  point_in_x = 0.0
  point_in_y = 0.0
  point_in_z = 0.0
  is_point_interne = False
  if 'pointIn_x' in shapeFissureParams:
    point_in_x = shapeFissureParams['pointIn_x']
    is_point_interne = True
  if 'pointIn_y' in shapeFissureParams:
    point_in_y = shapeFissureParams['pointIn_y']
    is_point_interne = True
  if 'pointIn_z' in shapeFissureParams:
    point_in_z = shapeFissureParams['pointIn_z']
    is_point_interne = True

  if is_point_interne:
    pointInterne = geompy.MakeVertex(point_in_x, point_in_y, point_in_z)
  else:
    pointInterne = None

  return pointInterne
