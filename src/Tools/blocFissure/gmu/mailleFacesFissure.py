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
"""maillage faces de fissure"""

import logging

import SMESH
from salome.smesh import smeshBuilder

from .geomsmesh import smesh

from .putName import putName

def mailleFacesFissure(faceFissureExterne, \
                       edgesPipeFissureExterneC, edgesPeauFissureExterneC, \
                       meshPipeGroups, areteFaceFissure, rayonPipe, nbsegRad, \
                       mailleur="MeshGems", nro_cas=None):
  """maillage faces de fissure"""
  logging.info('start')
  logging.info("Maillage avec %s pour le cas n°%d", mailleur, nro_cas)

  meshFaceFiss = smesh.Mesh(faceFissureExterne)
  logging.info("Maillage avec %s", mailleur)
  if ( mailleur == "MeshGems"):
    algo2d = meshFaceFiss.Triangle(algo=smeshBuilder.MG_CADSurf)
    hypo2d = algo2d.Parameters()
    hypo2d.SetPhySize( areteFaceFissure )
    hypo2d.SetMinSize( rayonPipe/float(nbsegRad) )
    hypo2d.SetMaxSize( areteFaceFissure*3. )
    hypo2d.SetChordalError( areteFaceFissure*0.25 )
    hypo2d.SetVerbosity( 0 )
  else:
    algo2d = meshFaceFiss.Triangle(algo=smeshBuilder.NETGEN_1D2D)
    hypo2d = algo2d.Parameters()
    hypo2d.SetMaxSize( areteFaceFissure )
    hypo2d.SetSecondOrder( 0 )
    hypo2d.SetOptimize( 1 )
    hypo2d.SetFineness( 2 )
    hypo2d.SetMinSize( rayonPipe/float(nbsegRad) )
    hypo2d.SetQuadAllowed( 0 )
  putName(algo2d.GetSubMesh(), "faceFiss", i_pref=nro_cas)
  putName(algo2d, "algo2d_faceFiss", i_pref=nro_cas)
  putName(hypo2d, "hypo2d_faceFiss", i_pref=nro_cas)

  texte = "Récupération des arêtes de '{}'".format(edgesPipeFissureExterneC.GetName())
  logging.info(texte)
  algo1d = meshFaceFiss.UseExisting1DElements(geom=edgesPipeFissureExterneC)
  hypo1d = algo1d.SourceEdges([ meshPipeGroups['edgeFaceFissGroup'] ],0,0)
  putName(algo1d.GetSubMesh(), "edgeFissPeau", i_pref=nro_cas)
  putName(algo1d, "algo1d_edgeFissPeau", i_pref=nro_cas)
  putName(hypo1d, "hypo1d_edgeFissPeau", i_pref=nro_cas)

  grpFaceFissureExterne = meshFaceFiss.GroupOnGeom(faceFissureExterne, "fisOutPi", SMESH.FACE)
  grpEdgesPeauFissureExterne = meshFaceFiss.GroupOnGeom(edgesPeauFissureExterneC,'edgesPeauFissureExterne',SMESH.EDGE)
  grpEdgesPipeFissureExterne = meshFaceFiss.GroupOnGeom(edgesPipeFissureExterneC,'edgesPipeFissureExterne',SMESH.EDGE)

  is_done = meshFaceFiss.Compute()
  text = "meshFaceFiss.Compute"
  if is_done:
    logging.info(text+" OK")
  else:
    text = "Erreur au calcul du maillage.\n" + text
    logging.info(text)
    raise Exception(text)

  return (meshFaceFiss, grpFaceFissureExterne, grpEdgesPeauFissureExterne, grpEdgesPipeFissureExterne)
