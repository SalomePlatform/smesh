# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy

# -----------------------------------------------------------------------------
# --- teste si l'opération de partition a produit une modification

def checkDecoupePartition(shapes, part):
  """
  Teste si l'opération de partition a produit une découpe
  (plus de shapes dans la partition).
  Résultat non garanti si recouvrement des shapes d'origine.
  @param shapes : liste des shapes d'origine
  @param part : résultat de la partition
  @return True si la partition a découpé les shapes d'origine
  """
  logging.info('start')
  # TODO: ShapeInfo donne des résultats faux (deux faces au lieu de une)
  
  isPart = False
  orig = {}
  for shape in shapes:
    info = geompy.ShapeInfo(shape)
    logging.debug("shape info %s", info)
    for k in ['VERTEX', 'EDGE', 'FACE', 'SOLID']:
      if k in orig.keys():
        orig[k] += info[k]
      else:
        orig[k] = info[k]
  logging.debug("original shapes info %s", orig)
  info = geompy.ShapeInfo(part)
  logging.debug("partition info %s", info)
  for k in ['VERTEX', 'EDGE', 'FACE', 'SOLID']:
    if orig[k] < info[k]:
      isPart = True
      break
  logging.debug("partition modifie l'original %s", isPart)

  return isPart

