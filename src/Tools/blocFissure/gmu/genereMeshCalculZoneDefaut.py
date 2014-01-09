# -*- coding: utf-8 -*-

import logging
from geomsmesh import smesh
from salome.smesh import smeshBuilder

# -----------------------------------------------------------------------------
# --- maillage face de fissure pour identification zone de defaut

def genereMeshCalculZoneDefaut(facefiss, minSize, maxSize):
  """
  TODO: a completer
  """
  logging.info('start')

  meshFissure = smesh.Mesh(facefiss)
  algo2d = meshFissure.Triangle(algo=smeshBuilder.NETGEN_1D2D)
  hypo2d = algo2d.Parameters()
  hypo2d.SetMaxSize( maxSize )
  hypo2d.SetSecondOrder( 0 )
  hypo2d.SetOptimize( 1 )
  hypo2d.SetFineness( 2 )
  hypo2d.SetMinSize( minSize )
  hypo2d.SetQuadAllowed( 0 )
  isDone = meshFissure.Compute()
  smesh.SetName(algo2d, "algo2d_zoneFiss")
  smesh.SetName(hypo2d, "hypo1d_zoneFiss")

  coordsNoeudsFissure = []
  nodeIds = meshFissure.GetNodesId()
  for id in nodeIds:
    coords = meshFissure.GetNodeXYZ(id)
    coordsNoeudsFissure.append(coords[0])
    coordsNoeudsFissure.append(coords[1])
    coordsNoeudsFissure.append(coords[2])
  return coordsNoeudsFissure
