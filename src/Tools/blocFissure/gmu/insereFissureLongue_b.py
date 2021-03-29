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
"""Insertion de fissure longue - maillage face de peau"""

import logging
import salome
from .geomsmesh import geompy
from .geomsmesh import smesh
from salome.smesh import smeshBuilder
import SMESH

from .putName import putName

def insereFissureLongue_b (facePeau, edgePeauFiss, groupEdgesBordPeau, bordsLibres, \
                           groupsDemiCerclesPeau, groups_demiCercles, verticesOutCercles, \
                           nbSegGenLong, nbSegGenBout, profondeur, \
                           mailleur="MeshGems"):
  """maillage face de peau"""
  logging.info('start')

  meshFacePeau = smesh.Mesh(facePeau)
  logging.info("Maillage avec %s", mailleur)
  if ( mailleur == "MeshGems"):
    algo2d = meshFacePeau.Triangle(algo=smeshBuilder.MG_CADSurf)
    hypo2d = algo2d.Parameters()
    hypo2d.SetPhySize( 1000 )
    hypo2d.SetMinSize( 100 )
    hypo2d.SetMaxSize( 3000. )
    hypo2d.SetChordalError( 250. )
    hypo2d.SetVerbosity( 0 )
  else:
    algo2d = meshFacePeau.Triangle(algo=smeshBuilder.NETGEN_2D)
    hypo2d = algo2d.Parameters()
    hypo2d.SetMaxSize( 1000 )
    hypo2d.SetOptimize( 1 )
    hypo2d.SetFineness( 2 )
    hypo2d.SetMinSize( 2 )
    hypo2d.SetQuadAllowed( 0 )
  putName(algo2d.GetSubMesh(), "facePeau")
  putName(algo2d, "algo2d_facePeau")
  putName(hypo2d, "hypo2d_facePeau")
  #
  lenEdgePeauFiss = geompy.BasicProperties(edgePeauFiss)[0]
  frac = profondeur/lenEdgePeauFiss
  nbSeg = nbSegGenLong +2*nbSegGenBout
  ratio = (nbSegGenBout/float(profondeur)) / (nbSegGenLong/lenEdgePeauFiss)
  logging.info("lenEdgePeauFiss %s, profondeur %s, nbSegGenLong %s, nbSegGenBout %s, frac %s, ratio %s", lenEdgePeauFiss, profondeur, nbSegGenLong, nbSegGenBout, frac, ratio)

  algo1d = meshFacePeau.Segment(geom=edgePeauFiss)
  hypo1d = algo1d.NumberOfSegments(nbSeg,list(),[  ])
  hypo1d.SetDistrType( 2 )
  hypo1d.SetConversionMode( 1 )
  hypo1d.SetTableFunction( [ 0, ratio, frac, 1, (1.-frac), 1, 1, ratio ] )
  putName(algo1d.GetSubMesh(), "edgePeauFiss")
  putName(algo1d, "algo1d_edgePeauFiss")
  putName(hypo1d, "hypo1d_edgePeauFiss")
  #
  algo1d = meshFacePeau.UseExisting1DElements(geom=groupEdgesBordPeau)
  hypo1d = algo1d.SourceEdges([ bordsLibres ],0,0)
  putName(algo1d.GetSubMesh(), "bordsLibres")
  putName(algo1d, "algo1d_bordsLibres")
  putName(hypo1d, "hypo1d_bordsLibres")
  #
  for i in range(2):
    algo1d = meshFacePeau.UseExisting1DElements(geom=groupsDemiCerclesPeau[i])
    hypo1d = algo1d.SourceEdges([ groups_demiCercles[i] ],0,0)
    putName(algo1d.GetSubMesh(), "DemiCercles", i)
    putName(algo1d, "algo1d_groupDemiCercles", i)
    putName(hypo1d, "hypo1d_groupDemiCercles", i)

  _ = meshFacePeau.GroupOnGeom(verticesOutCercles[0], "THOR", SMESH.NODE)
  _ = meshFacePeau.GroupOnGeom(verticesOutCercles[1], "THEX", SMESH.NODE)

  groupEdgesPeauFiss = meshFacePeau.GroupOnGeom(edgePeauFiss, "PeauFis", SMESH.EDGE)

  is_done = meshFacePeau.Compute()
  text = "meshFacePeau.Compute"
  if is_done:
    logging.info(text+" OK")
  else:
    text = "Erreur au calcul du maillage.\n" + text
    logging.info(text)
    raise Exception(text)

  peauext_face = meshFacePeau.CreateEmptyGroup( SMESH.FACE, 'PEAUEXT' )
  _ = peauext_face.AddFrom( meshFacePeau.GetMesh() )

  return meshFacePeau, groupEdgesPeauFiss
