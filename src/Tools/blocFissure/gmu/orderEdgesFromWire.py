# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy

# -----------------------------------------------------------------------------
# --- trouver les vertices intermediaires d'un wire

def orderEdgesFromWire(aWire):
  """
  fournit les edges ordonnées d'un wire selon ExtractShapes(,,False), 
  et l'ordre des edges selon le sens de parcours (ordre des indices de la liste d'edges)
  """
  logging.info("start")
  edges = geompy.ExtractShapes(aWire, geompy.ShapeType["EDGE"], False)
  
  idverts = {}
  for i, edge in enumerate(edges):
    verts = geompy.ExtractShapes(edge, geompy.ShapeType["VERTEX"], True)
#    idverts[(i,0)] = verts[0]
#    idverts[(i,1)] = verts[1]
    v0 = geompy.MakeVertexOnCurve(edge, 0.0)
    dist = geompy.MinDistance(v0, verts[0])
    if dist < 1.e-4:
      idverts[(i,0)] = verts[0]
      idverts[(i,1)] = verts[1]
    else:
      idverts[(i,0)] = verts[1]
      idverts[(i,1)] = verts[0]
     
  idsubs = {}
  for kv, sub in idverts.iteritems():
    subid = geompy.GetSubShapeID(aWire, sub)
    if subid in idsubs.keys():
      idsubs[subid].append(kv)
    else:
      idsubs[subid] = [kv]
  
  debut = -1
  fin = -1    
  for k, kvs in idsubs.iteritems():
    if len(kvs) == 1: # une extremité
      kv = kvs[0]
      if kv[1] == 0:
        debut = kv[0]
      else:
        fin = kv[0]
  logging.debug("nombre d'edges: %s, indice edge début: %s, fin: %s",len(edges), debut, fin)
  if debut < 0:
    logging.critical("les edges du wire ne sont pas orientées dans le même sens: pas de début trouvé")
    return edges, range(len(edges))
  
  orderedList = [debut]
  while len(orderedList) < len(edges):
    bout = orderedList[-1]
    vertex = idverts[(bout,1)]
    for k, v in idverts.iteritems():
      if k[0] not in orderedList:
        if geompy.MinDistance(vertex, v) < 1.e-4:
          if k[1] == 0:
            orderedList.append(k[0])
            break
          else:
            logging.critical("les edges du wire ne sont pas orientées dans le même sens: une edge à l'envers")
            return edges, range(len(edges))

  logging.debug("liste des edges ordonnées selon le sens de parcours: %s", orderedList)
  accessList = range(len(orderedList))
  for i,k in enumerate(orderedList):
    accessList[k] = i
  logging.info("position ordonnée des edges selon le sens de parcours: %s", accessList)
  return edges, accessList
