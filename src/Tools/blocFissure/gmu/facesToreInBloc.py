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
"""Identification des faces tore et fissure dans le solide hors tore du bloc partitionné"""

import logging

from . import initLog

from .geomsmesh import geompy
from .geomsmesh import geomPublishInFather

def facesToreInBloc(blocp, facefissoutore, facetore1, facetore2):
  """Identification des faces tore et fissure dans le bloc partitionné : sous shapes du bloc

  @param blocp : bloc partitionné
  @param facefissoutore : la face de fissure externe au tore
  @param facetore1 : face du tore selon la génératrice
  @param facetore2 : face du tore selon la génératrice
  @return (blocFaceFiss, blocFaceTore1, blocFaceTore2) sous shapes reperées
  """
  logging.info('start')

  blocFaceFiss = geompy.GetInPlaceByHistory(blocp, facefissoutore)
  blocFaceTore1 = geompy.GetInPlaceByHistory(blocp, facetore1)
  blocFaceTore2 = geompy.GetInPlaceByHistory(blocp, facetore2)

  geomPublishInFather(initLog.debug, blocp, blocFaceFiss,'blocFaceFiss')
  geomPublishInFather(initLog.debug, blocp, blocFaceTore1,'blocFaceTore1')
  geomPublishInFather(initLog.debug, blocp, blocFaceTore2,'blocFaceTore2')

  return blocFaceFiss, blocFaceTore1, blocFaceTore2
