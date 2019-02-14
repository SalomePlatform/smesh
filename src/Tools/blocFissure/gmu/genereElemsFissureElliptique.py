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

from .toreFissure import toreFissure
from .ellipsoideDefaut import ellipsoideDefaut
from .rotTrans import rotTrans
from .genereMeshCalculZoneDefaut import genereMeshCalculZoneDefaut

# -----------------------------------------------------------------------------
# --- création élements géométriques fissure elliptique

def genereElemsFissureElliptique(shapeFissureParams):
  """
  TODO: a completer
  """
  logging.info('start')
  
  centreDefaut  = shapeFissureParams['centreDefaut']
  vecteurDefaut = shapeFissureParams['vecteurDefaut']
  demiGrandAxe  = shapeFissureParams['demiGrandAxe']
  demiPetitAxe  = shapeFissureParams['demiPetitAxe']
  orientation   = shapeFissureParams['orientation']
  tailleDefaut  = shapeFissureParams['taille']

  # --- ellipse incomplete : generatrice

  allonge = demiGrandAxe/demiPetitAxe
  rayonTore = demiPetitAxe/5.0
  generatrice, FaceGenFiss, Pipe_1, FaceFissure, Plane_1, Pipe1Part = toreFissure(demiPetitAxe, allonge, rayonTore)
  ellipsoide = ellipsoideDefaut(demiPetitAxe, allonge, rayonTore)

  # --- positionnement sur le bloc defaut de generatrice, tore et plan fissure

  pipe0 = rotTrans(Pipe_1, orientation, centreDefaut, vecteurDefaut)
  gener1 = rotTrans(generatrice, orientation, centreDefaut, vecteurDefaut)
  pipe1 = rotTrans(Pipe1Part, orientation, centreDefaut, vecteurDefaut)
  facefis1 = rotTrans(FaceFissure, orientation, centreDefaut, vecteurDefaut)
  plane1 = rotTrans(Plane_1, orientation, centreDefaut, vecteurDefaut)
  ellipsoide1 = rotTrans(ellipsoide, orientation, centreDefaut, vecteurDefaut)

  geomPublish(initLog.debug,  pipe0, 'pipe0' )
  geomPublish(initLog.debug,  gener1, 'gener1' )
  geomPublish(initLog.debug,  pipe1, 'pipe1' )
  geomPublish(initLog.debug,  facefis1, 'facefis1' )
  geomPublish(initLog.debug,  plane1, 'plane1' )
  geomPublish(initLog.debug,  ellipsoide1, 'ellipsoide1' )

  shapeDefaut = facefis1
  xyz_defaut = geompy.PointCoordinates(centreDefaut)
  coordsNoeudsFissure = genereMeshCalculZoneDefaut(facefis1, demiPetitAxe/10.0, demiPetitAxe/5.0)

  return shapeDefaut, xyz_defaut, tailleDefaut, coordsNoeudsFissure, pipe0, gener1, pipe1, facefis1, plane1, ellipsoide1
