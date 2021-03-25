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

import os
from blocFissure import gmu
from blocFissure.gmu import initLog
initLog.setDebug()
#initLog.setVerbose()

from blocFissure.gmu.casStandard import casStandard

dicoParams = dict(nomCas            = "angleCube",
                  maillageSain      = os.path.join(gmu.pathBloc, "materielCasTests", "CubeAngle.med"),
                  CAOFaceFissure    = os.path.join(gmu.pathBloc, "materielCasTests", "CubeAngleFiss.brep"),
                  edgeFiss          = [4],
                  lgInfluence       = 20,
                  meshBrep          = (5,10),
                  rayonPipe         = 5,
                  lenSegPipe        = 2.5,
                  nbSegRad          = 5,
                  nbSegCercle       = 32,
                  areteFaceFissure  = 10)

execInstance = casStandard(dicoParams)
