# -*- coding: utf-8 -*-

import logging

# -----------------------------------------------------------------------------
# --- calcul de distance carree entre deux points, jeux de coordonnees x,y,z

def distance2(xyz1, xyz2):
  """
  carré de la distance entre deux points donnés par des triplets [x,y,z]
  """
  #logging.info("start")
  
  d2 = 0
  for i in range(3):
    d2 += (xyz1[i]-xyz2[i])*(xyz1[i]-xyz2[i])
  logging.debug('d2=%s', d2)
  return d2

# -----------------------------------------------------------------------------
# --- test unitaire

import unittest
class Test_distance2(unittest.TestCase):

  def setUp(self):
    self.a=[0, 0, 0]
    self.b=[3, 4, 5]
    self.c=[-5,-4,-3]
    
  def test_calcul(self):
    self.assertEqual(distance2(self.a, self.b), distance2(self.b, self.a))
    self.assertEqual(distance2(self.a, self.b), distance2(self.a, self.c))
    self.assertEqual(distance2(self.b, self.b), 0)
    self.assertEqual(distance2(self.a, self.b), 50)
        