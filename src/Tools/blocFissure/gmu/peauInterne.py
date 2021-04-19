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
"""Peau interne du defaut dans le maillage sain"""

import logging
import traceback

import SMESH

from .geomsmesh import smesh

from .fissError import fissError
from .listOfExtraFunctions import lookForCorner
from .fusionMaillageAttributionDefaut import fusionMaillageDefaut
from .putName import putName

def peauInterne(fichierMaillage, shapeDefaut, nomZones, \
                nro_cas=None):
  """Retrouve les groupes de défaut dans le maillage sain modifié par CreateHoleSkin (CreeZoneDefautMaillage)

  On récupère le volume et la peau de la zone de défaut, les éventuelles faces et arêtes internes de cette zone.
  Remarque : intérêt du passage par fichierMaillage plutôt que par maillageSain ?
  """
  logging.info("start")
  ([maillageSain], _) = smesh.CreateMeshesFromMED(fichierMaillage)

  groups = maillageSain.GetGroups()
  #print ("groupes :")
  #for grp in groups:
    #print ("\t{}".format(grp.GetName()))
  zoneDefaut = None
  zoneDefaut_skin = None
  zoneDefaut_internalFaces = None
  zoneDefaut_internalEdges = None
  for grp in groups:
    nom = grp.GetName()
    logging.debug("groupe %s",nom)
    if ( nom == nomZones + "_vol" ):
      zoneDefaut = grp
    elif ( nom == nomZones + "_skin" ):
      zoneDefaut_skin = grp
    elif ( nom == nomZones + "_internalFaces" ):
      zoneDefaut_internalFaces = grp
    elif ( nom == nomZones + "_internalEdges" ):
      zoneDefaut_internalEdges = grp
  # --- Le groupe ZoneDefaut ne doit contenir que des Hexaèdres"

  info = maillageSain.GetMeshInfo(zoneDefaut)
  #print ("info = {}".format(info))
  nbelem = 0
  nbhexa = 0
  for entity_type in info:
    #print (". {} : {})".format(entity_type, info[entity_type]))
    nbelem += info[entity_type]
    if ( str(entity_type) == "Entity_Hexa" ):
      nbhexa += info[entity_type]
      nbhexa += info[entity_type]
  #print ("==> nbelem = {}, nbhexa = {}".format(nbelem,nbhexa))

  if ( (nbelem == 0) or (nbhexa < nbelem) ):
    print ("==> nbelem = {}, nbhexa = {}".format(nbelem,nbhexa))
    texte = "La zone a remailler est incorrecte.<br>"
    texte += "Causes possibles :<ul>"
    texte += "<li>Les mailles à enlever dans le maillage sain n'ont pas été détectées.</li>"
    texte += "<li>Certaines faces du maillage sain sont à l'envers : les normales aux faces en paroi de volume doivent être sortantes.</li>"
    texte += "<li>Il n'y a pas que des hexaèdres réglés linéaires dans la zone à remailler (notamment mailles quadratiques, tetraèdres non traités)</li></ul>"
    raise fissError(traceback.extract_stack(),texte)

  _, maillageSain, DefautBoundary = maillageSain.MakeBoundaryElements( SMESH.BND_2DFROM3D, 'DefBound', '', 0, [ zoneDefaut ])
  internal = maillageSain.GetMesh().CutListOfGroups( [ DefautBoundary ], [ zoneDefaut_skin ], 'internal' )
  internalBoundary = smesh.CopyMesh( internal, 'internalBoundary', 0, 0)
  putName(internalBoundary, "internalBoundary", i_pref=nro_cas)

  maillageDefautCible = smesh.CopyMesh(zoneDefaut_skin, 'maillageCible', 0, 0)
  putName(maillageDefautCible, "maillageCible", i_pref=nro_cas)
  listOfCorner = lookForCorner(maillageDefautCible)
  texte = "listOfCorner = {}".format(listOfCorner)
  logging.debug(texte)
  if listOfCorner:
    logging.info("présence de coins à la surface externe de la zone à reconstruire")
    zoneDefaut_skin, internalBoundary = \
          fusionMaillageDefaut(maillageSain, maillageDefautCible, internalBoundary, zoneDefaut_skin, shapeDefaut, listOfCorner, \
          nro_cas)

  return maillageSain, internalBoundary, zoneDefaut, zoneDefaut_skin, zoneDefaut_internalFaces, zoneDefaut_internalEdges
