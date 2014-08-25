# -*- coding: utf-8 -*-

import logging
from geomsmesh import smesh
import SMESH

from listOfExtraFunctions import lookForCorner
from fusionMaillageAttributionDefaut import fusionMaillageDefaut

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
    #print " ",grp.GetName()
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

  nbAdded, maillageSain, DefautBoundary = maillageSain.MakeBoundaryElements( SMESH.BND_2DFROM3D, 'DefBound', '', 0, [ zoneDefaut ])
  internal = maillageSain.GetMesh().CutListOfGroups( [ DefautBoundary ], [ zoneDefaut_skin ], 'internal' )
  internalBoundary = smesh.CopyMesh( internal, 'internalBoundary', 0, 0)
  
  maillageDefautCible = smesh.CopyMesh(zoneDefaut_skin, 'maillageCible', 0, 0)
  listOfCorner = lookForCorner(maillageDefautCible)
  print "listOfCorner = ", listOfCorner
  if len(listOfCorner) > 0:
      print " /!\ SUITE DU SCRIPT EN CONSTRUCTION /!\\"
      zoneDefaut_skin, internalBoundary = fusionMaillageDefaut(maillageSain, maillageDefautCible, internalBoundary, zoneDefaut_skin, shapeDefaut, listOfCorner)

  return maillageSain, internalBoundary, zoneDefaut, zoneDefaut_skin, zoneDefaut_internalFaces, zoneDefaut_internalEdges

