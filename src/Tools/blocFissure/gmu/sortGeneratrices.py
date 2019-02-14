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
from .geomsmesh import geomPublish
from .geomsmesh import geomPublishInFather
from . import initLog

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
