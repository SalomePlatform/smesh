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
"""calcul du carré de la distance entre deux points"""

import logging

# -----------------------------------------------------------------------------
# ---

def distance2(xyz1, xyz2):
  """
  carré de la distance entre deux points donnés par des triplets [x,y,z]
  """
  #logging.info("start")

  dcarre = 0
  for ijk in range(3):
    dcarre += (xyz1[ijk]-xyz2[ijk])*(xyz1[ijk]-xyz2[ijk])
  logging.debug('dcarre=%s', dcarre)

  return dcarre

## -----------------------------------------------------------------------------
## --- test unitaire

#import unittest
#class Test_distance2(unittest.TestCase):
  #"""test de la fonction"""

  #def setUp(self):
    #self.a=[0, 0, 0]
    #self.b=[3, 4, 5]
    #self.c=[-5,-4,-3]

  #def test_calcul(self):
    #self.assertEqual(distance2(self.a, self.b), distance2(self.b, self.a))
    #self.assertEqual(distance2(self.a, self.b), distance2(self.a, self.c))
    #self.assertEqual(distance2(self.b, self.b), 0)
    #self.assertEqual(distance2(self.a, self.b), 50)
