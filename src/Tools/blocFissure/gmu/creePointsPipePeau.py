# -*- coding: utf-8 -*-

import logging

from geomsmesh import geompy
from geomsmesh import geomPublish
from geomsmesh import geomPublishInFather
import initLog
from projettePointSurCourbe import projettePointSurCourbe

def creePointsPipePeau(listEdges, idFacesDebouchantes, idFillingFromBout,
                       ptEdgeFond, ptFisExtPi, edCircPeau, gptsdisks, idisklim, nbsegRad):
  """
  création des points du maillage du pipe sur la face de peau
  """
  logging.info('start')
  
  for i, edges in enumerate(listEdges):
    idf = idFacesDebouchantes[i] # indice de face débouchante (facesPipePeau)
    if idf >= 0:
      gptdsk = []
      if idf > 0: # idf vaut 0 ou 1
        idf = -1  # si idf vaut 1, on prend le dernier élément de la liste (1 ou 2 extrémités débouchent sur la face)
      centre = ptEdgeFond[idFillingFromBout[i]][idf]
      name = "centre%d"%idf
      geomPublish(initLog.debug, centre, name)
      vertPipePeau = ptFisExtPi[idFillingFromBout[i]][idf]
      geomPublishInFather(initLog.debug, centre, vertPipePeau, "vertPipePeau")
      grpsEdgesCirc = edCircPeau[idFillingFromBout[i]] # liste de groupes
      edgesCirc = []
      for grpEdgesCirc in grpsEdgesCirc:
        edgesCirc += geompy.ExtractShapes(grpEdgesCirc, geompy.ShapeType["EDGE"], False)
      for k, edge in enumerate(edges):
        extrems = geompy.ExtractShapes(edge, geompy.ShapeType["VERTEX"], True)
        if geompy.MinDistance(centre, extrems[0]) < geompy.MinDistance(centre, extrems[1]):
          bout = extrems[1]
        else:
          bout = extrems[0]
        # ajustement du point extrémité (bout) sur l'edge circulaire en face de peau
        logging.debug("edgesCirc: %s", edgesCirc)
        distEdgeCirc = [(geompy.MinDistance(bout, edgeCirc), k2, edgeCirc) for k2, edgeCirc in enumerate(edgesCirc)]
        distEdgeCirc.sort()
        logging.debug("distEdgeCirc: %s", distEdgeCirc)
        u = projettePointSurCourbe(bout, distEdgeCirc[0][2])
        if (abs(u) < 0.02) or (abs(1-u) < 0.02): # les points très proches d'une extrémité doivent y être mis précisément.
          extrCircs = geompy.ExtractShapes(distEdgeCirc[0][2], geompy.ShapeType["VERTEX"], True)
          if geompy.MinDistance(bout, extrCircs[0]) < geompy.MinDistance(bout, extrCircs[1]):
            bout = extrCircs[0]
          else:
            bout = extrCircs[1]
        else:
          bout = geompy.MakeVertexOnCurve(distEdgeCirc[0][2], u)
        name ="bout%d"%k
        geomPublishInFather(initLog.debug, centre, bout, name)
        # enregistrement des points dans la structure
        points = []
        for j in range(nbsegRad +1):
          u = j/float(nbsegRad)
          points.append(geompy.MakeVertexOnCurve(edge, u))
        if geompy.MinDistance(bout, points[0]) < geompy.MinDistance(centre, points[0]):
          points.reverse()
        points[0] = centre
        points[-1] = bout
        gptdsk.append(points)
      if i == 0:
        gptsdisks[idisklim[0] -1] = gptdsk
        idisklim[0] = idisklim[0] -1
      else:
        gptsdisks[idisklim[1] +1] = gptdsk
        idisklim[1] = idisklim[1] +1
        
  return (gptsdisks, idisklim)