# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy
from geomsmesh import geomPublish
from geomsmesh import geomPublishInFather
import initLog

# -----------------------------------------------------------------------------
# --- tri par longueur des 3 generatrices

def sortGeneratrices(tore, geners):
  """
  tri des 3 edges 'génératrices' selon leur longueur.
  @param tore
  @param les edges 'generatrices'
  @return (genext, genint, gencnt) les 3 edges, de la plus grande à la plus petite
  """
  logging.info("start")

  genx = geompy.ExtractShapes(geners[0], geompy.ShapeType["EDGE"], True)

  lenx = []
  for i in range(len(genx)):
    props = geompy.BasicProperties(genx[i])
    lenx.append(props[0])
    pass
  minlen = min(lenx)
  maxlen = max(lenx)
  genext=None
  gencnt=None
  genint=None
  for i in range(len(genx)):
    if lenx[i] == minlen:
      genint = genx[i]
    elif lenx[i] == maxlen:
      genext = genx[i]
    else:
      gencnt= genx[i]
    pass

  geomPublishInFather(initLog.debug, tore, genext, 'genext' )
  geomPublishInFather(initLog.debug, tore, genint, 'genint' )
  geomPublishInFather(initLog.debug, tore, gencnt, 'gencnt' )

  return genext, genint, gencnt
