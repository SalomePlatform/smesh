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
"""Calcul de produit mixte pour orientation"""

import logging

from .geomsmesh import geompy

def produitMixte(origine, point_1, point_2, point_3):
  """produit mixte de 3 vecteurs a partir d'une origine et 3 points"""
  coordo = geompy.PointCoordinates(origine)
  coord_1 = geompy.PointCoordinates(point_1)
  coord_2 = geompy.PointCoordinates(point_2)
  coord_3 = geompy.PointCoordinates(point_3)

  o_1 = [coord_1[0] - coordo[0], coord_1[1] - coordo[1], coord_1[2] - coordo[2]]
  o_2 = [coord_2[0] - coordo[0], coord_2[1] - coordo[1], coord_2[2] - coordo[2]]
  o_3 = [coord_3[0] - coordo[0], coord_3[1] - coordo[1], coord_3[2] - coordo[2]]

  prmi =  ( o_1[0]*o_2[1]*o_3[2] + o_2[0]*o_3[1]*o_1[2] + o_3[0]*o_1[1]*o_2[2] ) \
        - ( o_1[0]*o_3[1]*o_2[2] + o_2[0]*o_1[1]*o_3[2] + o_3[0]*o_2[1]*o_1[2] )

  logging.debug('prmi=%s', prmi)

  return prmi
