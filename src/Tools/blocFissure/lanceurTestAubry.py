# -*- coding: iso-8859-1 -*-
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

import sys
import salome

import os
from blocFissure import gmu
from blocFissure.gmu import initLog
#initLog.setDebug()
initLog.setVerbose()

from blocFissure.casStandard import casStandard

mesh = 'huehue.med'
crack = 'hue.brep'

dicoParams = dict(nomCas            = 'testAubry',
                  maillageSain      = '/local00/home/I48174/Bureau/{0}'.format(mesh),
                  CAOFaceFissure    = '/local00/home/I48174/Bureau/{0}'.format(crack),
                  edgeFiss          = [8],
                  lgInfluence       = 0.01,
                  meshBrep          = (0.0002,0.003),
                  rayonPipe         = 0.005,
                  lenSegPipe        = 0.0015,
                  nbSegRad          = 8,
                  nbSegCercle       = 18,
                  areteFaceFissure  = 0.0015)

execInstance = casStandard(dicoParams)

if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()
