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
"""création des points du maillage du pipe sur la face de peau"""

import logging

from . import initLog
from .geomsmesh import geompy
from .geomsmesh import geomPublish
from .geomsmesh import geomPublishInFather
from .projettePointSurCourbe import projettePointSurCourbe

def creePointsPipePeau(listEdges, idFacesDebouchantes, idFillingFromBout,
                       ptEdgeFond, ptFisExtPi, edCircPeau, gptsdisks, idisklim, nbsegRad):
  """création des points du maillage du pipe sur la face de peau"""
  logging.info('start')

  for n_edges, edges in enumerate(listEdges):

     # idf = indice de face débouchante (facesPipePeau) ; idf vaut 0 ou 1
    idf = idFacesDebouchantes[n_edges]
    logging.info("idf: %d", idf)
    if ( idf >= 0 ):
      gptdsk = list()
      # si idf vaut 1, on prend le dernier élément de la liste (1 ou 2 extrémités débouchent sur la face)
      if ( idf > 0 ):
        idf = -1
      centre = ptEdgeFond[idFillingFromBout[n_edges]][idf]
      name = "centre_{}".format(idf)
      geomPublish(initLog.debug, centre, name)
      vertPipePeau = ptFisExtPi[idFillingFromBout[n_edges]][idf]
      geomPublishInFather(initLog.debug, centre, vertPipePeau, "vertPipePeau")
      grpsEdgesCirc = edCircPeau[idFillingFromBout[n_edges]] # liste de groupes

      edgesCirc = list()
      for grpEdgesCirc in grpsEdgesCirc:
        edgesCirc += geompy.ExtractShapes(grpEdgesCirc, geompy.ShapeType["EDGE"], False)
      logging.debug("edgesCirc: %s", edgesCirc)

      for i_aux, edge in enumerate(edges):
        extrems = geompy.ExtractShapes(edge, geompy.ShapeType["VERTEX"], True)
        if geompy.MinDistance(centre, extrems[0]) < geompy.MinDistance(centre, extrems[1]):
          bout = extrems[1]
        else:
          bout = extrems[0]
        # ajustement du point extrémité (bout) sur l'edge circulaire en face de peau
        distEdgeCirc = [(geompy.MinDistance(bout, edgeCirc), k2, edgeCirc) for k2, edgeCirc in enumerate(edgesCirc)]
        distEdgeCirc.sort()
        logging.debug("distEdgeCirc: %s", distEdgeCirc)
        dist = projettePointSurCourbe(bout, distEdgeCirc[0][2])
        # les points très proches d'une extrémité doivent y être mis précisément.
        if (abs(dist) < 0.02) or (abs(1.-dist) < 0.02):
          extrCircs = geompy.ExtractShapes(distEdgeCirc[0][2], geompy.ShapeType["VERTEX"], True)
          if geompy.MinDistance(bout, extrCircs[0]) < geompy.MinDistance(bout, extrCircs[1]):
            bout = extrCircs[0]
          else:
            bout = extrCircs[1]
        else:
          bout = geompy.MakeVertexOnCurve(distEdgeCirc[0][2], dist)
        name = "bout_{}".format(i_aux)
        geomPublishInFather(initLog.debug, centre, bout, name)
        # enregistrement des points dans la structure
        points = list()
        for n_seg in range(nbsegRad +1):
          points.append(geompy.MakeVertexOnCurve(edge, float(n_seg)/float(nbsegRad)))
        if geompy.MinDistance(bout, points[0]) < geompy.MinDistance(centre, points[0]):
          points.reverse()
        points[0] = centre
        points[-1] = bout
        gptdsk.append(points)

      # Enregistrement des extrémités
      if n_edges == 0:
        gptsdisks[idisklim[0] -1] = gptdsk
        idisklim[0] = idisklim[0] -1
      else:
        gptsdisks[idisklim[1] +1] = gptdsk
        idisklim[1] = idisklim[1] +1

  return gptsdisks
