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

""" FissureLauncher:
    Permet de lancer le script 'casStandard'
    en fonction des paramètres contenus dans 'dicoParams'. """

#import sys
import salome

#import os
#from blocFissure import gmu
from blocFissure.gmu import initLog
initLog.setDebug()
#initLog.setVerbose()

from blocFissure.casStandard import casStandard

mesh = 'maillageSainTuyauFEM.med'
crack = 'fissureQuartEllipse.brep'

dicoParams = dict(nomCas            = 'fissTuyau',
                  maillageSain      = '/local00/home/I48174/Bureau/supportMaillage/{0}'.format(mesh),
                  CAOFaceFissure    = '/local00/home/I48174/Bureau/supportMaillage/{0}'.format(crack),
                  edgeFiss          = [9],
                  lgInfluence       = 250,
                  meshBrep          = (10, 50),
                  rayonPipe         = 35,
                  lenSegPipe        = 7, #9,
                  nbSegRad          = 6,
                  nbSegCercle       = 30,
                  areteFaceFissure  = 20)

execInstance = casStandard(dicoParams)

if salome.sg.hasDesktop():
    salome.sg.updateObjBrowser()
