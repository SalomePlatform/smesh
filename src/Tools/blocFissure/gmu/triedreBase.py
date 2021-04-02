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
"""Origine et vecteurs de base"""

import logging

from .geomsmesh import geompy
from .geomsmesh import geomPublish

from . import initLog

O = None
OX = None
OY = None
OZ = None

def triedreBase():
  """
  definitions globales du triedre de reference,
  objets partages par plusieurs methodes
  """
  global O, OX, OY, OZ

  if ( O is None ):
    logging.info("start")
    O = geompy.MakeVertex(0, 0, 0)
    OX = geompy.MakeVectorDXDYDZ(1, 0, 0)
    OY = geompy.MakeVectorDXDYDZ(0, 1, 0)
    OZ = geompy.MakeVectorDXDYDZ(0, 0, 1)

    if not geompy.myStudy.FindObjectByName( 'OX', geompy.ComponentDataType() ):
      geomPublish(initLog.debug,  O, 'O' )
      geomPublish(initLog.debug,  OX, 'OX' )
      geomPublish(initLog.debug,  OY, 'OY' )
      geomPublish(initLog.debug,  OZ, 'OZ' )

  return O, OX, OY, OZ
