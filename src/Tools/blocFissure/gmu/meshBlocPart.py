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
"""Maillage du bloc partitionné"""

import logging

import SMESH
from salome.smesh import smeshBuilder
from salome.StdMeshers import StdMeshersBuilder

from .geomsmesh import geompy
from .geomsmesh import smesh

from .putName import putName

# -----------------------------------------------------------------------------

def meshBlocPart(blocPartition, faceFissure, tore, centres, edges, diams, circles, faces, \
                gencnt, facefissoutore, edgeext, facesExternes, facesExtBloc, facesExtElli, \
                aretesInternes, internalBoundary, ellipsoidep, sharedFaces, sharedEdges, edgesBords, \
                nbsegExt, nbsegGen, nbsegRad, scaleRad, reverses, reverext, nbsegCercle, nbsegFis, dmoyen, lensegEllipsoide, \
                mailleur="MeshGems", nro_cas=None):
  """Maillage du bloc partitionné"""
  logging.info('start')
  logging.info("Maillage avec %s pour le cas n°%s", mailleur, nro_cas)

  # --- edges de bord à respecter

  _ = smesh.CreateFilterManager()
  _, internalBoundary, _NoneGroup = internalBoundary.MakeBoundaryElements( SMESH.BND_1DFROM2D, '', '', 0, [ ])
  criteres = list()
  unCritere = smesh.GetCriterion(SMESH.EDGE,SMESH.FT_FreeBorders,SMESH.FT_Undefined,0)
  criteres.append(unCritere)
  filtre = smesh.GetFilterFromCriteria(criteres)
  bordsLibres = internalBoundary.MakeGroupByFilter( 'bords', filtre )
  putName(bordsLibres, 'bordsLibres', i_pref=nro_cas)

  # --- maillage bloc

  bloc1 = smesh.Mesh(blocPartition)

  for i_aux, sharedFaces_i in enumerate(sharedFaces):
    algo2d = bloc1.Triangle(algo=smeshBuilder.NETGEN, geom=sharedFaces_i)
    putName(algo2d.GetSubMesh(), "sharedFaces", i_aux, nro_cas)
    hypo2d = algo2d.Parameters(which=smesh.SIMPLE)
    hypo2d.SetLocalLength(lensegEllipsoide)
    hypo2d.LengthFromEdges()
    hypo2d.SetAllowQuadrangles(0)
    putName(hypo2d, "sharedFaces", i_aux, nro_cas)

  for i_aux, sharedEdges_i in enumerate(sharedEdges):
    algo1d = bloc1.Segment(geom=sharedEdges_i)
    putName(algo1d.GetSubMesh(), "sharedEdges", i_aux, nro_cas)
    hypo1d = algo1d.LocalLength(lensegEllipsoide)
    putName(hypo1d, "sharedEdges={}".format(lensegEllipsoide), i_aux, nro_cas)

  declareAlgoEllipsoideFirst = False
  if declareAlgoEllipsoideFirst:
    algo3d = bloc1.Tetrahedron(algo=smeshBuilder.NETGEN,geom=ellipsoidep)
    putName(algo3d.GetSubMesh(), "ellipsoide", i_pref=nro_cas)
    hypo3d = algo3d.MaxElementVolume(1000.0)
    putName(hypo3d, "ellipsoide", i_pref=nro_cas)

  algo3d = bloc1.Prism(geom=tore)
  putName(algo3d.GetSubMesh(), "tore", i_pref=nro_cas)
  algo2d = bloc1.Quadrangle(geom=tore)
  algo1d = bloc1.Segment(geom=tore)
  hypo1d = algo1d.NumberOfSegments(nbsegGen)
  putName(hypo1d, "tore={}".format(nbsegGen), i_pref=nro_cas)

  for i_aux, faces_i in enumerate(faces):
    algo2d = bloc1.Quadrangle(geom=faces_i)
    putName(algo2d.GetSubMesh(), "faces", i_aux, nro_cas)
    hypo2d = smesh.CreateHypothesis('QuadrangleParams')
    hypo2d.SetTriaVertex( geompy.GetSubShapeID(blocPartition,centres[i_aux]) )
    hypo2d.SetQuadType( StdMeshersBuilder.QUAD_STANDARD )
    _ = bloc1.AddHypothesis(hypo2d,faces_i)
    putName(hypo2d, "faces", i_aux, nro_cas)

  for i_aux, edges_i in enumerate(edges):
    algo1d = bloc1.Segment(geom=edges_i)
    putName(algo1d.GetSubMesh(), "edges", i_aux, nro_cas)
    if reverses[i_aux] > 0:
      hypo1d = algo1d.NumberOfSegments(nbsegRad, scaleRad,[ geompy.GetSubShapeID(blocPartition,edges_i) ])
    else:
      hypo1d = algo1d.NumberOfSegments(nbsegRad, scaleRad,[ ])
    putName(hypo1d, "edges", i_aux, nro_cas)

  for i_aux, circles_i in enumerate(circles):
    algo1d = bloc1.Segment(geom=circles_i)
    putName(algo1d.GetSubMesh(), "circles", i_aux, nro_cas)
    hypo1d = algo1d.NumberOfSegments(nbsegCercle)
    putName(hypo1d, "circles={}".format(nbsegCercle), i_aux, nro_cas)

  if len(edgeext) == 1:
    densite = int(round(nbsegFis/2))
    algo1d = bloc1.Segment(geom=edgeext[0])
    putName(algo1d.GetSubMesh(), "edgeext", i_pref=nro_cas)
    hypo1d = algo1d.NumberOfSegments(nbsegFis)
    hypo1d.SetDistrType( 2 )
    hypo1d.SetConversionMode( 1 )
    hypo1d.SetTableFunction( [ 0, densite, 0.4, 1, 0.6, 1, 1, densite ] )
    putName(hypo1d, "edgeext", i_pref=nro_cas)
  else:
    longTotal = 0
    longEdgeExts = list()
    for edgeext_i in edgeext:
      props = geompy.BasicProperties(edgeext_i)
      longEdgeExts.append(props[0])
      longTotal += props[0]
    for i_aux, edgeext_i in enumerate(edgeext):
      nbLocal = int(round(nbsegFis*longEdgeExts[i_aux]/longTotal))
      densite = int(round(nbLocal/2))
      algo1d = bloc1.Segment(geom=edgeext_i)
      putName(algo1d.GetSubMesh(), "edgeext", i_aux, nro_cas)
      hypo1d = algo1d.NumberOfSegments(nbLocal)
      hypo1d.SetDistrType( 2 )
      hypo1d.SetConversionMode( 1 )
      hypo1d.SetTableFunction( [ 0, densite, 0.8, 1, 1, 1 ] )
      if reverext[i_aux]:
        hypo1d.SetReversedEdges([ geompy.GetSubShapeID(blocPartition, edgeext_i) ])
      putName(hypo1d, "edgeext", i_aux, nro_cas)

  algo2d = bloc1.Triangle(algo=smeshBuilder.NETGEN_2D, geom=facefissoutore)
  putName(algo2d.GetSubMesh(), "facefissoutore", i_pref=nro_cas)
  hypo2d = algo2d.LengthFromEdges()
  putName(hypo2d, "facefissoutore", i_pref=nro_cas)


  maxElemArea = 0.5*dmoyen*dmoyen
  logging.debug("dmoyen %s, maxElemArea %s", dmoyen, maxElemArea)

  for i_aux, facesExternes_i in enumerate(facesExternes):
    algo2d = bloc1.Triangle(algo=smeshBuilder.NETGEN_2D, geom=facesExternes_i)
    putName(algo2d.GetSubMesh(), "facesExternes", i_aux, nro_cas)
    hypo2d = algo2d.MaxElementArea(maxElemArea)
    putName(hypo2d, "facesExternes={}".format(maxElemArea), i_aux, nro_cas)
    if edgesBords is None:
      algo1d = bloc1.Segment(geom=facesExternes_i)
      hypo1d = algo1d.NumberOfSegments(1)
      putName(hypo1d, "facesExternes", i_aux, nro_cas)

  for i_aux, aretesInternes_i in enumerate(aretesInternes):
    algo1d = bloc1.Segment(geom=aretesInternes_i)
    putName(algo1d.GetSubMesh(), "aretesInternes", i_aux, nro_cas)
    hypo1d = algo1d.NumberOfSegments(nbsegExt)
    putName(hypo1d, "aretesInternes={}".format(nbsegExt), i_aux, nro_cas)

  if edgesBords is not None:
    algo1d = bloc1.UseExisting1DElements(geom=edgesBords)
    putName(algo1d.GetSubMesh(), "bordsLibres", i_pref=nro_cas)
    hypo1d = algo1d.SourceEdges([ bordsLibres ],0,0)
    putName(hypo1d, "bordsLibres", i_pref=nro_cas)

  if not declareAlgoEllipsoideFirst:
    algo3d = bloc1.Tetrahedron(algo=smeshBuilder.NETGEN,geom=ellipsoidep)
    putName(algo3d.GetSubMesh(), "ellipsoide", i_pref=nro_cas)
    hypo3d = algo3d.MaxElementVolume(1000.0)
    putName(hypo3d, "ellipsoide", i_pref=nro_cas)

  _ = bloc1.GroupOnGeom(faceFissure,'FACE1',SMESH.FACE)
  _ = bloc1.GroupOnGeom(gencnt,'nfondfis',SMESH.NODE)

  groups_faceCommuneEllipsoideBloc = list()
  for i_aux, sharedFaces_i in enumerate(sharedFaces):
    name = "faceCommuneEllipsoideBloc_{}".format(i_aux)
    groups_faceCommuneEllipsoideBloc.append(bloc1.GroupOnGeom(sharedFaces_i, name, SMESH.FACE))
  groups_faceExterneBloc = list()
  for i_aux, facesExtBloc_i in enumerate(facesExtBloc):
    name = "faceExterneBloc_{}".format(i_aux)
    groups_faceExterneBloc.append(bloc1.GroupOnGeom(facesExtBloc_i, name, SMESH.FACE))

  is_done = bloc1.Compute()
  text = "bloc1.Compute"
  if is_done:
    logging.info(text+" OK")
  else:
    text = "Erreur au calcul du maillage.\n" + text
    logging.info(text)
    raise Exception(text)

  _ = bloc1.RemoveOrphanNodes()

  skinBlocMeshes = list()
  for i_aux, groups_faceCommuneEllipsoideBloc_i in enumerate(groups_faceCommuneEllipsoideBloc):
    name = "faceCommuneEllipsoideBloc_{}".format(i_aux)
    skinBlocMeshes.append(smesh.CopyMesh(groups_faceCommuneEllipsoideBloc_i, name, 0, 0))
  for i_aux, groups_faceExterneBloc_i in enumerate(groups_faceExterneBloc):
    name = "faceExterneBloc_{}".format(i_aux)
    skinBlocMeshes.append(smesh.CopyMesh(groups_faceExterneBloc_i, name, 0, 0))

  meshesBloc = [internalBoundary.GetMesh()]
  for skinBlocMeshes_i in skinBlocMeshes:
    meshesBloc.append(skinBlocMeshes_i.GetMesh())
  blocMesh = smesh.Concatenate(meshesBloc, 1, 1, 1e-05,False)

  algo3d = blocMesh.Tetrahedron(algo=smeshBuilder.NETGEN)
  putName(algo3d.GetSubMesh(), "bloc", i_pref=nro_cas)
  hypo3d = algo3d.MaxElementVolume(1000.0)
  putName(hypo3d, "bloc", i_pref=nro_cas)

  is_done = blocMesh.Compute()
  text = "blocMesh.Compute"
  if is_done:
    logging.info(text+" OK")
  else:
    text = "Erreur au calcul du maillage.\n" + text
    logging.info(text)
    raise Exception(text)

  blocComplet = smesh.Concatenate([bloc1.GetMesh(), blocMesh.GetMesh()], 1, 1, 1e-05,False)

  return bloc1, blocComplet
