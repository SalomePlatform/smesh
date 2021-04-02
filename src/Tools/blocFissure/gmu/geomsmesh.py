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
"""Publications dans salome"""

import salome
salome.salome_init()

from salome.geom import geomBuilder
geompy = geomBuilder.New()

from salome.smesh import smeshBuilder
smesh = smeshBuilder.New()

from . import initLog

def geomPublish(level,aShape, aName):
  """Publication d'un objet"""
  if initLog.getLogLevel() <= level:
    geompy.addToStudy(aShape, aName)

def geomPublishInFather(level, aFather, aShape, aName):
  """Publication d'un objet sous son ascendant"""
  if initLog.getLogLevel() <= level:
    geompy.addToStudyInFather(aFather, aShape, aName)
