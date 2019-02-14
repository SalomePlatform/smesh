# -*- coding: utf-8 -*-
# Copyright (C) 2014-2019  CEA/DEN, EDF R&D
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
# --- calcul de produit mixte pour orientation

def produitMixte(o, p1, p2, p3):
  """
  produit mixte de 3 vecteurs a partir d'une origine et 3 points
  """
  coordo = geompy.PointCoordinates(o)
  coordp1 = geompy.PointCoordinates(p1)
  coordp2 = geompy.PointCoordinates(p2)
  coordp3 = geompy.PointCoordinates(p3)
  u = [coordp1[0] - coordo[0], coordp1[1] - coordo[1], coordp1[2] - coordo[2]]
  v = [coordp2[0] - coordo[0], coordp2[1] - coordo[1], coordp2[2] - coordo[2]]
  w = [coordp3[0] - coordo[0], coordp3[1] - coordo[1], coordp3[2] - coordo[2]]
  pm = (u[0]*v[1]*w[2] + v[0]*w[1]*u[2] + w[0]*u[1]*v[2]) - (u[0]*w[1]*v[2] + v[0]*u[1]*w[2] + w[0]*v[1]*u[2])
  logging.debug('pm=%s', pm)
  return pm

