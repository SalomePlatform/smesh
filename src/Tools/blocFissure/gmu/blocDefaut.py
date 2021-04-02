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
"""bloc defaut"""

import logging
from .geomsmesh import geompy

def blocDefaut(blocDim):
  """ Le bloc contenant la fissure est un cube construit centre a l'origine, dont on donne la demi arete.

  @param blocdim : demi arete
  @return  cube (geomObject)
  """
  logging.info("start")
  point_1 = geompy.MakeVertex(-blocDim, -blocDim, -blocDim)
  point_2 = geompy.MakeVertex( blocDim,  blocDim,  blocDim)
  le_cube = geompy.MakeBoxTwoPnt(point_1, point_2)

  return le_cube
