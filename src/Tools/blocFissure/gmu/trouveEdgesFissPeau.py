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
"""Pour les faces de peau sans extremité débouchante de fissure, il faut recenser les edges de fissure sur la face de peau"""

import logging

from .geomsmesh import geompy
from .geomsmesh import geomPublishInFather

from . import initLog

def trouveEdgesFissPeau(facesInside, facesOnside, edgesPipeIn, edgesFondIn, partitionPeauFissFond, edgesFissExtPeau):
  """Pour les faces de peau sans extremité débouchante de fissure, il faut recenser les edges de fissure sur la face de peau"""
  logging.info('start')

  i_aux = 0
  for face in facesInside:

    edgesPeauFis = list()
    edgesPipeFis = list()
    edgesPipeFnd = list()
    try:
      edgesPeauFis = geompy.GetSharedShapesMulti([geompy.MakeCompound(facesOnside), face], geompy.ShapeType["EDGE"])
      edgesPipeFis = geompy.GetSharedShapesMulti([geompy.MakeCompound(edgesPipeIn), face], geompy.ShapeType["EDGE"])
      edgesPipeFnd = geompy.GetSharedShapesMulti([geompy.MakeCompound(edgesFondIn), face], geompy.ShapeType["EDGE"])
    except:
      pass

    if ( edgesPeauFis and edgesPipeFis and ( not edgesPipeFnd ) ):
      edgesFissExtPeau.append(edgesPeauFis[0])
      name="edgesFissExtPeau{}".format(i_aux)
      geomPublishInFather(initLog.debug,partitionPeauFissFond, edgesPeauFis[0], name)
      i_aux += 1

  return edgesFissExtPeau
