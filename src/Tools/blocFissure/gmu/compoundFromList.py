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

import logging
from .geomsmesh import geompy
from .geomsmesh import geomPublish
from .geomsmesh import geomPublishInFather
from . import initLog

def compoundFromList(elements, nom=None):
  """
  
  """
  logging.debug('start')
  
  shapeList = []
  for a in elements:
    if not isinstance(a, list):
      shapeList.append(a)
    else:
      if a[0] is not None:
        shapeList.append(a[0])
      
  if nom is not None:
    for i,a in enumerate(shapeList):
      nomi = nom +"%d"%i
      logging.debug('nom: %s',nomi)
      geomPublish(initLog.debug, a, nomi)

  shapeCompound = None
  if len(shapeList) > 0:
    shapeCompound =geompy.MakeCompound(shapeList)
    nomc = "compound_%s"%nom
    geomPublish(initLog.debug, shapeCompound, nomc)
  return shapeCompound
    
