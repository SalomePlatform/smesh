# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy
from geomsmesh import smesh
from salome.smesh import smeshBuilder
import SMESH
from salome.StdMeshers import StdMeshersBuilder
from putName import putName

# -----------------------------------------------------------------------------
# --- maillage du bloc partitionne

def meshBlocPart(blocPartition, faceFissure, tore, centres, edges, diams, circles, faces,
                gencnt, facefissoutore, edgeext, facesExternes, facesExtBloc, facesExtElli,
                aretesInternes, internalBoundary, ellipsoidep, sharedFaces, sharedEdges, edgesBords,
                nbsegExt, nbsegGen, nbsegRad, scaleRad, reverses, reverext, nbsegCercle, nbsegFis, dmoyen, lensegEllipsoide):
  """
  Maillage du bloc partitionné
  TODO: a completer
  """
  logging.info('start')

  # --- edges de bord à respecter

  aFilterManager = smesh.CreateFilterManager()
  nbAdded, internalBoundary, _NoneGroup = internalBoundary.MakeBoundaryElements( SMESH.BND_1DFROM2D, '', '', 0, [  ])
  criteres = []
  unCritere = smesh.GetCriterion(SMESH.EDGE,SMESH.FT_FreeBorders,SMESH.FT_Undefined,0)
  criteres.append(unCritere)
  filtre = smesh.GetFilterFromCriteria(criteres)
  bordsLibres = internalBoundary.MakeGroupByFilter( 'bords', filtre )
  smesh.SetName(bordsLibres, 'bordsLibres')

  # --- maillage bloc

  bloc1 = smesh.Mesh(blocPartition)

  for i in range(len(sharedFaces)):
    algo2d = bloc1.Triangle(algo=smeshBuilder.NETGEN, geom=sharedFaces[i])
    hypo2d = algo2d.Parameters(which=smesh.SIMPLE)
    hypo2d.SetLocalLength(lensegEllipsoide)
    hypo2d.LengthFromEdges()
    hypo2d.SetAllowQuadrangles(0)
    putName(algo2d.GetSubMesh(), "sharedFaces", i)
    putName(algo2d, "algo2d_sharedFaces", i)
    putName(hypo2d, "hypo2d_sharedFaces", i)

  for i in range(len(sharedEdges)):
    algo1d = bloc1.Segment(geom=sharedEdges[i])
    hypo1d = algo1d.LocalLength(lensegEllipsoide)
    putName(algo1d.GetSubMesh(), "sharedEdges", i)
    putName(algo1d, "algo1d_sharedEdges", i)
    putName(hypo1d, "hypo1d_sharedEdges", i)

  declareAlgoEllipsoideFirst = False
  if declareAlgoEllipsoideFirst:
    algo3d = bloc1.Tetrahedron(algo=smeshBuilder.NETGEN,geom=ellipsoidep)
    hypo3d = algo3d.MaxElementVolume(1000.0)
    putName(algo3d.GetSubMesh(), "ellipsoide")
    putName(algo3d, "algo3d_ellipsoide")
    putName(hypo3d, "hypo3d_ellipsoide")

  algo3d = bloc1.Prism(geom=tore)
  algo2d = bloc1.Quadrangle(geom=tore)
  algo1d = bloc1.Segment(geom=tore)
  hypo1d = algo1d.NumberOfSegments(nbsegGen)
  putName(algo3d.GetSubMesh(), "tore")
  putName(algo3d, "algo3d_tore")
  putName(algo2d, "algo2d_tore")
  putName(algo1d, "algo1d_tore")
  putName(hypo1d, "hypo1d_tore")

  for i in range(len(faces)):
    algo2d = bloc1.Quadrangle(geom=faces[i])
    hypo2d = smesh.CreateHypothesis('QuadrangleParams')
    hypo2d.SetTriaVertex( geompy.GetSubShapeID(blocPartition,centres[i]) )
    hypo2d.SetQuadType( StdMeshersBuilder.QUAD_STANDARD )
    status = bloc1.AddHypothesis(hypo2d,faces[i])
    putName(algo2d.GetSubMesh(), "faces", i)
    putName(algo2d, "algo2d_faces", i)
    putName(hypo2d, "hypo2d_faces", i)

  for i in range(len(edges)):
    algo1d = bloc1.Segment(geom=edges[i])
    if reverses[i] > 0:
      hypo1d = algo1d.NumberOfSegments(nbsegRad, scaleRad,[ geompy.GetSubShapeID(blocPartition,edges[i]) ])
    else:
      hypo1d = algo1d.NumberOfSegments(nbsegRad, scaleRad,[  ])
    putName(algo1d.GetSubMesh(), "edges", i)
    putName(algo1d, "algo1d_edges", i)
    putName(hypo1d, "hypo1d_edges", i)

  for i in range(len(circles)):
    algo1d = bloc1.Segment(geom=circles[i])
    hypo1d = algo1d.NumberOfSegments(nbsegCercle)
    putName(algo1d.GetSubMesh(), "circles", i)
    putName(algo1d, "algo1d_circles", i)
    putName(hypo1d, "hypo1d_circles", i)

  if len(edgeext) == 1:
    densite = int(round(nbsegFis/2))
    algo1d = bloc1.Segment(geom=edgeext[0])
    hypo1d = algo1d.NumberOfSegments(nbsegFis)
    hypo1d.SetDistrType( 2 )
    hypo1d.SetConversionMode( 1 )
    hypo1d.SetTableFunction( [ 0, densite, 0.4, 1, 0.6, 1, 1, densite ] )
    putName(algo1d.GetSubMesh(), "edgeext")
    putName(algo1d, "algo1d_edgeext")
    putName(hypo1d, "hypo1d_edgeext")
  else:
    longTotal = 0
    longEdgeExts = []
    for i in range(len(edgeext)):
      props = geompy.BasicProperties(edgeext[i])
      longEdgeExts.append(props[0])
      longTotal += props[0]
    for i in range(len(edgeext)):
      local = longTotal/nbsegFis
      nbLocal = int(round(nbsegFis*longEdgeExts[i]/longTotal))
      densite = int(round(nbLocal/2))
      algo1d = bloc1.Segment(geom=edgeext[i])
      hypo1d = algo1d.NumberOfSegments(nbLocal)
      hypo1d.SetDistrType( 2 )
      hypo1d.SetConversionMode( 1 )
      hypo1d.SetTableFunction( [ 0, densite, 0.8, 1, 1, 1 ] )
      if reverext[i]:
        hypo1d.SetReversedEdges([ geompy.GetSubShapeID(blocPartition, edgeext[i]) ])
      putName(algo1d.GetSubMesh(), "edgeext", i)
      putName(algo1d, "algo1d_edgeext", i)
      putName(hypo1d, "hypo1d_edgeext", i)

  algo2d = bloc1.Triangle(algo=smeshBuilder.NETGEN_2D, geom=facefissoutore)
  hypo2d = algo2d.LengthFromEdges()
  putName(algo2d.GetSubMesh(), "facefissoutore")
  putName(algo2d, "algo2d_facefissoutore")
  putName(hypo2d, "hypo2d_facefissoutore")


  maxElemArea = 0.5*dmoyen*dmoyen
  logging.debug("dmoyen %s, maxElemArea %s", dmoyen, maxElemArea)

  for i in range(len(facesExternes)):
    algo2d = bloc1.Triangle(algo=smeshBuilder.NETGEN_2D, geom=facesExternes[i])
    hypo2d = algo2d.MaxElementArea(maxElemArea)
    if edgesBords is None:
      algo1d = bloc1.Segment(geom=facesExternes[i])
      hypo1d = algo1d.NumberOfSegments(1)
    putName(algo2d.GetSubMesh(), "facesExternes", i)
    putName(algo2d, "algo2d_facesExternes", i)
    putName(hypo2d, "hypo2d_facesExternes", i)
    if edgesBords is None:
      putName(algo1d, "algo1d_facesExternes", i)
      putName(hypo1d, "hypo1d_facesExternes", i)

  for i in range(len(aretesInternes)):
    algo1d = bloc1.Segment(geom=aretesInternes[i])
    hypo1d = algo1d.NumberOfSegments(nbsegExt)
    putName(algo1d.GetSubMesh(), "aretesInternes", i)
    putName(algo1d, "algo1d_aretesInternes", i)
    putName(hypo1d, "hypo1d_aretesInternes", i)

  if edgesBords is not None:
    algo1d = bloc1.UseExisting1DElements(geom=edgesBords)
    hypo1d = algo1d.SourceEdges([ bordsLibres ],0,0)
    putName(algo1d.GetSubMesh(), "bordsLibres")
    putName(algo1d, "algo1d_bordsLibres")
    putName(hypo1d, "hypo1d_bordsLibres")

  #isDone = bloc1.Compute()

  if not declareAlgoEllipsoideFirst:
    algo3d = bloc1.Tetrahedron(algo=smeshBuilder.NETGEN,geom=ellipsoidep)
    hypo3d = algo3d.MaxElementVolume(1000.0)
    putName(algo3d.GetSubMesh(), "ellipsoide")
    putName(algo3d, "algo3d_ellipsoide")
    putName(hypo3d, "hypo3d_ellipsoide")

  isDone = bloc1.Compute()

  nbRemoved = bloc1.RemoveOrphanNodes()

  faceFissure1 = bloc1.GroupOnGeom(faceFissure,'FACE1',SMESH.FACE)
  noeudsFondFissure = bloc1.GroupOnGeom(gencnt,'nfondfis',SMESH.NODE)

  groups_faceCommuneEllipsoideBloc = []
  for i in range(len(sharedFaces)):
    name = "faceCommuneEllipsoideBloc_%d"%i
    groups_faceCommuneEllipsoideBloc.append(bloc1.GroupOnGeom(sharedFaces[i], name, SMESH.FACE))
  groups_faceExterneBloc = []
  for i in range(len(facesExtBloc)):
    name = "faceExterneBloc_%d"%i
    groups_faceExterneBloc.append(bloc1.GroupOnGeom(facesExtBloc[i], name, SMESH.FACE))

  skinBlocMeshes = []
  for i in range(len(groups_faceCommuneEllipsoideBloc)):
    name = "faceCommuneEllipsoideBloc_%d"%i
    skinBlocMeshes.append(smesh.CopyMesh(groups_faceCommuneEllipsoideBloc[i], name, 0, 0))
  for i in range(len(groups_faceExterneBloc)):
    name = "faceExterneBloc_%d"%i
    skinBlocMeshes.append(smesh.CopyMesh(groups_faceExterneBloc[i], name, 0, 0))

  meshesBloc = [internalBoundary.GetMesh()]
  for i in range(len(skinBlocMeshes)):
    meshesBloc.append(skinBlocMeshes[i].GetMesh())
  blocMesh = smesh.Concatenate(meshesBloc, 1, 1, 1e-05,False)

  algo3d = blocMesh.Tetrahedron(algo=smeshBuilder.NETGEN)
  hypo3d = algo3d.MaxElementVolume(1000.0)
  putName(algo3d.GetSubMesh(), "bloc")
  putName(algo3d, "algo3d_bloc")
  putName(hypo3d, "hypo3d_bloc")

  is_done = blocMesh.Compute()

  blocComplet = smesh.Concatenate([bloc1.GetMesh(), blocMesh.GetMesh()], 1, 1, 1e-05,False)

  return bloc1, blocComplet