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
"""Maillage face de fissure pour identification zone de défaut"""

import logging

from salome.smesh import smeshBuilder

from .geomsmesh import smesh

from .putName import putName

def genereMeshCalculZoneDefaut(facefiss, minSize, maxSize, \
                               mailleur="MeshGems", nro_cas=None):
  """Maillage de l'objet géométrique 'facefiss'

. Avec l'algorithme MG_CADSurf :
      -SetMaxSize     = dimension max d'un élément (maxSize)
      -SetSecondOrder = élément quadratique (Y=1, N=0)
      -SetOptimize    = élément régulier (Y=1, N=0)
      -SetFineness    = finesse du maillage

. Avec l'algorithme NETGEN_1D2D :
      -SetMaxSize     = dimension max d'un élément (maxSize)
      -SetSecondOrder = élément quadratique (Y=1, N=0)
      -SetOptimize    = élément régulier (Y=1, N=0)
      -SetFineness    = finesse du maillage
       [very_coarse, coarse, moderate, fine, very_fine, custom]
       [0,           1,      2,        3,    4,         5     ]
      -SetMinSize     = dimension min d'un élément (minSize)
      -SetQuadAllowed = permission quadrangle dans maillage triangle

-On récupère les coordonnées de chaque noeud de la fissure qu'on stocke
   dans une liste sous la forme : [X0, Y0, Z0, ..., Xn, Yn, Zn]
  """

  logging.info('start')
  logging.info("Maillage avec %s pour le cas n°%s", mailleur, nro_cas)

  meshFissure = smesh.Mesh(facefiss)
  putName(meshFissure, "facefiss", i_pref=nro_cas)

  text = "Maillage de '{}'".format(facefiss.GetName())
  logging.info(text)
  if ( mailleur == "MeshGems"):
    algo2d = meshFissure.Triangle(algo=smeshBuilder.MG_CADSurf)
    hypo2d = algo2d.Parameters()
    hypo2d.SetPhySize( maxSize )
    hypo2d.SetMinSize( maxSize/4. )
    hypo2d.SetMaxSize( maxSize*2. )
    hypo2d.SetChordalError( maxSize*0.25 )
    hypo2d.SetVerbosity( 0 )
  else:
    algo2d = meshFissure.Triangle(algo=smeshBuilder.NETGEN_1D2D)
    hypo2d = algo2d.Parameters()
    hypo2d.SetMaxSize( maxSize )
    hypo2d.SetSecondOrder( 0 )
    hypo2d.SetOptimize( 1 )
    hypo2d.SetFineness( 2 )
    hypo2d.SetMinSize( minSize )
    hypo2d.SetChordalErrorEnabled (True)
    hypo2d.SetChordalError( maxSize*0.25 )
    hypo2d.SetUseSurfaceCurvature (True)
    hypo2d.SetQuadAllowed( 0 )
  putName(hypo2d, "zoneFiss", i_pref=nro_cas)

  is_done = meshFissure.Compute()
  text = "meshFissure.Compute"
  if is_done:
    logging.info(text+" OK")
  else:
    text = "Erreur au calcul du maillage.\n" + text
    logging.info(text)
    raise Exception(text)

  coordsNoeudsFissure = list()
  nodeIds = meshFissure.GetNodesId()
  for indice in nodeIds:
    coords = meshFissure.GetNodeXYZ(indice)
    coordsNoeudsFissure.append(coords[0])
    coordsNoeudsFissure.append(coords[1])
    coordsNoeudsFissure.append(coords[2])

  logging.info('end')

  return coordsNoeudsFissure
