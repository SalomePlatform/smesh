# -*- coding: utf-8 -*-

import logging

from geomsmesh import geompy
from geomsmesh import smesh
import SMESH

from putName import putName
  
def mailleAretesEtJonction(internalBoundary, aretesVivesCoupees, lgAretesVives):
  """
  edges de bord, faces défaut à respecter
  """
  logging.info('start')

  aFilterManager = smesh.CreateFilterManager()
  nbAdded, internalBoundary, _NoneGroup = internalBoundary.MakeBoundaryElements( SMESH.BND_1DFROM2D, '', '', 0, [  ])
  criteres = []
  unCritere = smesh.GetCriterion(SMESH.EDGE,SMESH.FT_FreeBorders,SMESH.FT_Undefined,0)
  criteres.append(unCritere)
  filtre = smesh.GetFilterFromCriteria(criteres)
  bordsLibres = internalBoundary.MakeGroupByFilter( 'bords', filtre )
  smesh.SetName(bordsLibres, 'bordsLibres')

  # --- pour aider l'algo hexa-tetra à ne pas mettre de pyramides à l'exterieur des volumes repliés sur eux-mêmes
  #     on désigne les faces de peau en quadrangles par le groupe "skinFaces"

  skinFaces = internalBoundary.CreateEmptyGroup( SMESH.FACE, 'skinFaces' )
  nbAdd = skinFaces.AddFrom( internalBoundary.GetMesh() )

  # --- maillage des éventuelles arêtes vives entre faces reconstruites
  
  grpAretesVives = None
  if len(aretesVivesCoupees) > 0:
    aretesVivesC = geompy.MakeCompound(aretesVivesCoupees)
    meshAretesVives = smesh.Mesh(aretesVivesC)
    algo1d = meshAretesVives.Segment()
    hypo1d = algo1d.LocalLength(lgAretesVives,[],1e-07)
    putName(algo1d.GetSubMesh(), "aretesVives")
    putName(algo1d, "algo1d_aretesVives")
    putName(hypo1d, "hypo1d_aretesVives")
    isDone = meshAretesVives.Compute()
    logging.info("aretesVives fini")
    grpAretesVives = meshAretesVives.CreateEmptyGroup( SMESH.EDGE, 'grpAretesVives' )
    nbAdd = grpAretesVives.AddFrom( meshAretesVives.GetMesh() )

  return (internalBoundary, bordsLibres, grpAretesVives)