# -*- coding: utf-8 -*-

import logging

from geomsmesh import geompy
from geomsmesh import smesh
from salome.smesh import smeshBuilder
import SMESH

from putName import putName
  
def mailleFacesFissure(faceFissureExterne, edgesPipeFissureExterneC, edgesPeauFissureExterneC,
                        meshPipeGroups, areteFaceFissure, rayonPipe, nbsegRad):
  """
  maillage faces de fissure
  """
  logging.info('start')

  meshFaceFiss = smesh.Mesh(faceFissureExterne)
  algo2d = meshFaceFiss.Triangle(algo=smeshBuilder.NETGEN_1D2D)
  hypo2d = algo2d.Parameters()
  hypo2d.SetMaxSize( areteFaceFissure )
  hypo2d.SetSecondOrder( 0 )
  hypo2d.SetOptimize( 1 )
  hypo2d.SetFineness( 2 )
  hypo2d.SetMinSize( rayonPipe/float(nbsegRad) )
  hypo2d.SetQuadAllowed( 0 )
  putName(algo2d.GetSubMesh(), "faceFiss")
  putName(algo2d, "algo2d_faceFiss")
  putName(hypo2d, "hypo2d_faceFiss")
  
  algo1d = meshFaceFiss.UseExisting1DElements(geom=edgesPipeFissureExterneC)
  hypo1d = algo1d.SourceEdges([ meshPipeGroups['edgeFaceFissGroup'] ],0,0)
  putName(algo1d.GetSubMesh(), "edgeFissPeau")
  putName(algo1d, "algo1d_edgeFissPeau")
  putName(hypo1d, "hypo1d_edgeFissPeau")
  
  isDone = meshFaceFiss.Compute()
  logging.info("meshFaceFiss fini")

  grpFaceFissureExterne = meshFaceFiss.GroupOnGeom(faceFissureExterne, "fisOutPi", SMESH.FACE)
  grpEdgesPeauFissureExterne = meshFaceFiss.GroupOnGeom(edgesPeauFissureExterneC,'edgesPeauFissureExterne',SMESH.EDGE)
  grpEdgesPipeFissureExterne = meshFaceFiss.GroupOnGeom(edgesPipeFissureExterneC,'edgesPipeFissureExterne',SMESH.EDGE)

  return (meshFaceFiss, grpFaceFissureExterne, grpEdgesPeauFissureExterne, grpEdgesPipeFissureExterne)