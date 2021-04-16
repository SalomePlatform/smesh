# -*- coding: utf-8 -*-
# Copyright (C) 2014-2020  EDF R&D
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
"""Insertion de fissure longue - maillage face de fissure"""

import logging
import math

import SMESH
from salome.smesh import smeshBuilder

from .geomsmesh import smesh

from .putName import putName

def insereFissureLongue_e (faceFiss, edgePeauFiss, groupEdgesPeauFiss, group_generFiss, groupEdgesFaceFissPipe, \
                           profondeur, rayonPipe, \
                           mailleur="MeshGems", nro_cas=None):
  """maillage face de fissure"""
  logging.info('start')
  logging.info("Maillage avec %s pour le cas n°%s", mailleur, nro_cas)

  meshFaceFiss = smesh.Mesh(faceFiss)
  putName(meshFaceFiss, "faceFiss", i_pref=nro_cas)

  mesh_size = (profondeur - rayonPipe)/math.sqrt(3.0) # pour avoir deux couches de triangles équilatéraux partout sur la fissure
  if ( mailleur == "MeshGems"):
    algo2d = meshFaceFiss.Triangle(algo=smeshBuilder.MG_CADSurf)
    hypo2d = algo2d.Parameters()
    hypo2d.SetPhySize( mesh_size )
    hypo2d.SetMinSize( mesh_size/10. )
    hypo2d.SetMaxSize( mesh_size*3. )
    hypo2d.SetChordalError( mesh_size*0.25 )
    hypo2d.SetVerbosity( 0 )
  else:
    algo2d = meshFaceFiss.Triangle(algo=smeshBuilder.NETGEN_2D)
    hypo2d = algo2d.Parameters()
    hypo2d.SetMaxSize( mesh_size )
    hypo2d.SetOptimize( 1 )
    hypo2d.SetFineness( 2 )
    hypo2d.SetMinSize( 2 )
    hypo2d.SetChordalErrorEnabled (True)
    hypo2d.SetChordalError( mesh_size*0.25 )
    hypo2d.SetUseSurfaceCurvature (True)
    hypo2d.SetQuadAllowed( 0 )
  putName(algo2d.GetSubMesh(), "faceFiss", i_pref=nro_cas)
  putName(hypo2d, "faceFiss", i_pref=nro_cas)
  #
  algo1d = meshFaceFiss.UseExisting1DElements(geom=edgePeauFiss)
  hypo1d = algo1d.SourceEdges([ groupEdgesPeauFiss ],0,0)
  putName(algo1d.GetSubMesh(), "edgeFissPeau", i_pref=nro_cas)
  putName(hypo1d, "edgeFissPeau", i_pref=nro_cas)
  #
  algo1d = meshFaceFiss.UseExisting1DElements(geom=groupEdgesFaceFissPipe)
  hypo1d = algo1d.SourceEdges([ group_generFiss ],0,0)
  putName(algo1d.GetSubMesh(), "edgeFissPeau", i_pref=nro_cas)
  putName(hypo1d, "edgeFissPeau", i_pref=nro_cas)

  _ = meshFaceFiss.GroupOnGeom(faceFiss, "fisOutPi", SMESH.FACE)

  is_done = meshFaceFiss.Compute()
  text = "meshFaceFiss.Compute"
  if is_done:
    logging.info(text+" OK")
  else:
    text = "Erreur au calcul du maillage.\n" + text
    logging.info(text)
    raise Exception(text)

  return meshFaceFiss
