# -*- coding: utf-8 -*-

import logging
from geomsmesh import smesh
import SMESH
from geomsmesh import geompy

# -----------------------------------------------------------------------------
# --- maillage complet et fissure

def RegroupeSainEtDefaut(maillageSain, blocComplet, extrusionFaceFissure, faceGeomFissure, nomVolume, normal = None):
  """
  Maillage sain sans la zone de defaut
  TODO: a completer
  """
  logging.info('Concatenation')

  maillageComplet = smesh.Concatenate([maillageSain.GetMesh(), blocComplet.GetMesh()], 1, 1, 1e-05,False)

  groups = maillageComplet.GetGroups()
  grps = [ grp for grp in groups if grp.GetName() == 'FACE1']
  faceFissure = grps[0]
  grps = [ grp for grp in groups if grp.GetName() == 'nfondfis']
  noeudsFondFissure = grps[0]
  grps = [ grp for grp in groups if grp.GetName() == 'fisInPi']
  fisInPi = grps[0]
  grps = [ grp for grp in groups if grp.GetName() == 'fisOutPi']
  fisOutPi = grps[0]

  # --- TODO: fiabiliser l'orientation dans le cas general
  if normal is None:
    normal  = smesh.MakeDirStruct( 0, 0, 1 )
  maillageComplet.Reorient2D( fisInPi,  normal, [0,0,0])
  maillageComplet.Reorient2D( fisOutPi, normal, [0,0,0])
    
  shapes = []
  if extrusionFaceFissure is not None:
    subIds = geompy.SubShapeAllIDs(extrusionFaceFissure, geompy.ShapeType["SOLID"])
    if len(subIds) > 1:
      shapes = geompy.ExtractShapes(extrusionFaceFissure, geompy.ShapeType["SOLID"], False)
    else:
      shapes = [extrusionFaceFissure]
#  else:
#    subIds = geompy.SubShapeAllIDs(faceGeomFissure, geompy.ShapeType["FACE"])
#    if len(subIds) > 1:
#      shapes = geompy.ExtractShapes(faceGeomFissure, geompy.ShapeType["FACE"], False)
#    else:
#      shapes = [faceGeomFissure]
    
  grpEdges = []
  grpFaces = []
  grpVolumes = []
  if len(shapes) == 0:
    shapes = [None] # calcul uniquement avec les normales des faces mailles de la fissure
  for i, aShape in enumerate(shapes):
    logging.info('Detection elements affectes par le dedoublement de la face %d'%i)
    affectedGroups = maillageComplet.AffectedElemGroupsInRegion([faceFissure], [noeudsFondFissure], aShape)
    grps = [ grp for grp in affectedGroups if grp.GetName() == 'affectedEdges']
    affectedEdges = grps[0]
    affectedEdges.SetName('affEd%d'%i)
    grpEdges.append(affectedEdges)
    grps = [ grp for grp in affectedGroups if grp.GetName() == 'affectedFaces']
    affectedFaces = grps[0]
    affectedFaces.SetName('affFa%d'%i)
    grpFaces.append(affectedFaces)
    grps = [ grp for grp in affectedGroups if grp.GetName() == 'affectedVolumes']
    affectedVolumes = grps[0]
    affectedVolumes.SetName('affVo%d'%i)
    grpVolumes.append(affectedVolumes)
  logging.info("union des groupes d'edges") 
  affectedEdges = maillageComplet.UnionListOfGroups(grpEdges, 'affEdges')
  logging.info("union des groupes de faces") 
  affectedFaces = maillageComplet.UnionListOfGroups(grpFaces, 'affFaces')
  logging.info("union des groupes de volumes") 
  affectedVolumes = maillageComplet.UnionListOfGroups(grpVolumes, 'affVols')
  for grp in affectedGroups:
    logging.debug("nom groupe %s",grp.GetName())
  [ FACE2, FACE2_nodes ] = maillageComplet.DoubleNodeElemGroups([faceFissure], [noeudsFondFissure], affectedGroups, True, True)
  FACE2.SetName( 'FACE2' )

  GroupVol = maillageComplet.CreateEmptyGroup( SMESH.VOLUME, nomVolume )
  nbAdd = GroupVol.AddFrom( maillageComplet.GetMesh() )

  return maillageComplet

