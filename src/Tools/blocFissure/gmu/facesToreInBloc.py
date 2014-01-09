# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy

# -----------------------------------------------------------------------------
# --- identification des faces tore et fissure dans le solide hors tore du bloc partitionné

def facesToreInBloc(blocp, facefissoutore, facetore1, facetore2):
  """
  identification des faces tore et fissure dans le bloc partitionné : sous shapes du bloc
  @param blocp : bloc partitionné
  @param facefissoutore : la face de fissure externe au tore
  @param facetore1 : face du tore selon la génératrice
  @param facetore2 : face du tore selon la génératrice
  @return (blocFaceFiss, blocFaceTore1, blocFaceTore2) sous shapes reperées
  """
  logging.info('start')

  blocFaceFiss = geompy.GetInPlaceByHistory(blocp, facefissoutore)
  blocFaceTore1 = geompy.GetInPlaceByHistory(blocp, facetore1)
  blocFaceTore2 = geompy.GetInPlaceByHistory(blocp, facetore2)

  geompy.addToStudyInFather(blocp, blocFaceFiss,'blocFaceFiss')
  geompy.addToStudyInFather(blocp, blocFaceTore1,'blocFaceTore1')
  geompy.addToStudyInFather(blocp, blocFaceTore2,'blocFaceTore2')

  return blocFaceFiss, blocFaceTore1, blocFaceTore2

