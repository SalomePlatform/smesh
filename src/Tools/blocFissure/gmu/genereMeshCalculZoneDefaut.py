# -*- coding: utf-8 -*-

import logging
from geomsmesh import smesh
from salome.smesh import smeshBuilder

# -----------------------------------------------------------------------------
# --- maillage face de fissure pour identification zone de defaut

def genereMeshCalculZoneDefaut(facefiss, minSize, maxSize):
  """
  -Permet de générer un maillage sur l'objet géométrique 'facefiss' via
   l'algorithme NETGEN_1D2D :
      -SetMaxSize     = dimension max d'un élément (maxSize)
      -SetSecondOrder = élément quadratique (Y=1, N=0)
      -SetOptimize    = élément régulier (Y=1, N=0)
      -SetFineness    = finesse du maillage
       [very_coarse, coarse, moderate, fine, very_fine, custom]
       [0,           1,      2,        3,    4,         5     ]
      -SetMinSize     = dimension min d'un élément (minSize)
      -SetQuadAllowed = permission quadrangle dans maillage triangle
  -On récupère les coordonnées de chaque noeud de la fissure qu'on stocke
   dans une liste sous la forme : [X0, Y0, Z0, ..., Xn, Yn, Zn]"""
   
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
