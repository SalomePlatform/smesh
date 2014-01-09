# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy

# -----------------------------------------------------------------------------
# --- bloc defaut

def blocDefaut(blocDim):
  """
  Le bloc contenant la fissure est un cube construit centre a l'origine, dont on donne la demi arete.
  @param blocdim : demi arete
  @return  cube (geomObject)
  """
  logging.info("start")
  geomObj_1 = geompy.MakeVertex(-blocDim, -blocDim, -blocDim)
  geomObj_2 = geompy.MakeVertex( blocDim,  blocDim,  blocDim)
  Box = geompy.MakeBoxTwoPnt(geomObj_1, geomObj_2)
  #geompy.addToStudy( Box_1, 'Box_1' )
  return Box
