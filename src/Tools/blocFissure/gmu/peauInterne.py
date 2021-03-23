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
from .geomsmesh import smesh
import SMESH
import traceback
from .fissError import fissError

from .listOfExtraFunctions import lookForCorner
from .fusionMaillageAttributionDefaut import fusionMaillageDefaut

# -----------------------------------------------------------------------------
# --- peau interne du defaut dans le maillage sain

def peauInterne(fichierMaillage, shapeDefaut, nomZones):
  """
  Retrouve les groupes de défaut dans le maillage sain modifié par CreateHoleSkin (CreeZoneDefautMaillage)
  On récupère le volume et la peau de la zone de défaut, les éventuelles faces et arêtes internes de cette zone.
  """
  logging.info("start")
  ([maillageSain], status) = smesh.CreateMeshesFromMED(fichierMaillage)

  groups = maillageSain.GetGroups()
  zoneDefaut = None
  for grp in groups:
    logging.debug("groupe %s",grp.GetName())
    if grp.GetName() == nomZones + "_vol":
      zoneDefaut = grp
      break
  zoneDefaut_skin = None
  for grp in groups:
    if grp.GetName() == nomZones + "_skin":
      zoneDefaut_skin = grp
      break
  zoneDefaut_internalFaces = None
  for grp in groups:
    if grp.GetName() == nomZones + "_internalFaces":
      zoneDefaut_internalFaces = grp
      break
  zoneDefaut_internalEdges = None
  for grp in groups:
    if grp.GetName() == nomZones + "_internalEdges":
      zoneDefaut_internalEdges = grp
      break
    
  # --- Le groupe ZoneDefaut ne doit contenir que des Hexaèdres"
  
  info=maillageSain.GetMeshInfo(zoneDefaut)
  keys = list(info.keys()); keys.sort()
  nbelem=0
  nbhexa=0
  for i in keys:
    #print "  %s  :  %d" % ( i, info[i] )
    nbelem+=info[i]
    if "Entity_Hexa" in str(i):
      nbhexa+=info[i]
  if (nbelem == 0) or (nbhexa < nbelem) :
    texte = "La zone a remailler est incorrecte.<br>"
    texte += "Causes possibles :<ul>"
    texte += "<li>Les mailles à enlever dans le maillage sain n'ont pas été détectées.</li>"
    texte += "<li>Certaines faces du maillage sain sont à l'envers : les normales aux faces en paroi de volume doivent être sortantes.</li>"
    texte += "<li>Il n'y a pas que des Hexaèdres réglés linéaires dans la zone à remailler (notamment mailles quadratiques, tetraèdres non traités)</li></ul>"
    raise fissError(traceback.extract_stack(),texte)

  nbAdded, maillageSain, DefautBoundary = maillageSain.MakeBoundaryElements( SMESH.BND_2DFROM3D, 'DefBound', '', 0, [ zoneDefaut ])
  internal = maillageSain.GetMesh().CutListOfGroups( [ DefautBoundary ], [ zoneDefaut_skin ], 'internal' )
  internalBoundary = smesh.CopyMesh( internal, 'internalBoundary', 0, 0)
  
  maillageDefautCible = smesh.CopyMesh(zoneDefaut_skin, 'maillageCible', 0, 0)
  listOfCorner = lookForCorner(maillageDefautCible)
  logging.debug("listOfCorner = %s", listOfCorner)
  if len(listOfCorner) > 0:
      logging.info("présence de coins à la surface externe de la zone à reconstruire")
      zoneDefaut_skin, internalBoundary = fusionMaillageDefaut(maillageSain, maillageDefautCible, internalBoundary, zoneDefaut_skin, shapeDefaut, listOfCorner)

  return maillageSain, internalBoundary, zoneDefaut, zoneDefaut_skin, zoneDefaut_internalFaces, zoneDefaut_internalEdges

