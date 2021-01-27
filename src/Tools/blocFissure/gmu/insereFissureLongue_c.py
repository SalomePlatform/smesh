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
import salome
from .geomsmesh import smesh
from salome.smesh import smeshBuilder
import SMESH

from .putName import putName

import math

def insereFissureLongue_c (faceFiss, edgePeauFiss, groupEdgesPeauFiss, group_generFiss, groupEdgesFaceFissPipe, \
                           profondeur, rayonPipe):
  """maillage face de fissure"""
  logging.info('start')

  meshFaceFiss = smesh.Mesh(faceFiss)
  algo2d = meshFaceFiss.Triangle(algo=smeshBuilder.NETGEN_2D)
  hypo2d = algo2d.Parameters()
  hypo2d.SetMaxSize( (profondeur - rayonPipe)/math.sqrt(3.0) ) # pour avoir deux couches de triangles equilateraux partout sur la fissure
  hypo2d.SetOptimize( 1 )
  hypo2d.SetFineness( 2 )
  hypo2d.SetMinSize( 2 )
  hypo2d.SetQuadAllowed( 0 )
  putName(algo2d.GetSubMesh(), "faceFiss")
  putName(algo2d, "algo2d_faceFiss")
  putName(hypo2d, "hypo2d_faceFiss")
  #
  algo1d = meshFaceFiss.UseExisting1DElements(geom=edgePeauFiss)
  hypo1d = algo1d.SourceEdges([ groupEdgesPeauFiss ],0,0)
  putName(algo1d.GetSubMesh(), "edgeFissPeau")
  putName(algo1d, "algo1d_edgeFissPeau")
  putName(hypo1d, "hypo1d_edgeFissPeau")
  #
  algo1d = meshFaceFiss.UseExisting1DElements(geom=groupEdgesFaceFissPipe)
  hypo1d = algo1d.SourceEdges([ group_generFiss ],0,0)
  putName(algo1d.GetSubMesh(), "edgeFissPeau")
  putName(algo1d, "algo1d_edgeFissPeau")
  putName(hypo1d, "hypo1d_edgeFissPeau")

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
