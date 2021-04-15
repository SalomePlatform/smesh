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
"""Construction des listes d'edges radiales sur chaque extrémité débouchante"""

import logging

from .geomsmesh import geompy
from .geomsmesh import geomPublish

from . import initLog

from .sortEdges import sortEdges

def construitEdgesRadialesDebouchantes_c(facesPipePeau, edgeRadFacePipePeau, \
                                         listNappes, idFacesDebouchantes):
  """Construction des listes d'edges radiales sur chaque extrémité débouchante"""
  logging.info('start')
  listEdges = list()
  for n_nappe, nappes in enumerate(listNappes):
    ifd = idFacesDebouchantes[n_nappe] # indice de face débouchante (facesPipePeau)
    if ifd < 0:
      listEdges.append([])
    else:
      face = facesPipePeau[ifd]
      edges = [edgeRadFacePipePeau[ifd]]
      for n_nappe, nappe in enumerate(nappes):
        if n_nappe > 0:
          obj = geompy.MakeSection(face, nappe) # normalement une edge, parfois un compound d'edges dont un tout petit
          edge = obj

          l_shapes = geompy.ExtractShapes(obj, geompy.ShapeType["VERTEX"], False)
          if len(l_shapes) > 2:
            eds = geompy.ExtractShapes(obj, geompy.ShapeType["EDGE"], False)
            [edsorted, _, maxl] = sortEdges(eds)
            edge = edsorted[-1]
          else:
            maxl = geompy.BasicProperties(edge)[0]

          if maxl < 0.01: # problème MakeSection
            logging.info("problème MakeSection recherche edge radiale %s, longueur trop faible: %s, utilisation partition", n_nappe, maxl)
            partNappeFace = geompy.MakePartition([face, nappe], [] , [], [], geompy.ShapeType["FACE"], 0, [], 0)
            l_shapes= geompy.ExtractShapes(partNappeFace, geompy.ShapeType["EDGE"], False)
            l_nouv_edges = list()
            for shape in l_shapes:
              dmax=100.
              l_vertex = geompy.ExtractShapes(shape, geompy.ShapeType["VERTEX"], False)
              distx = [geompy.MinDistance(vertex, face) for vertex in l_vertex]
              distx += [geompy.MinDistance(vertex, nappe) for vertex in l_vertex]
              dmax = max(distx)
              lgedge = geompy.BasicProperties(shape)[0]
              logging.debug("  dmax %s, longueur edge %s",dmax, lgedge)
              if dmax < 0.01 and  lgedge > 0.01:
                l_nouv_edges.append(shape)

            if l_nouv_edges:
              logging.debug("  edges issues de la partition: %s", l_nouv_edges)
              for num, edge in enumerate(l_nouv_edges):
                geomPublish(initLog.debug, edge, "l_nouv_edges%d"%num)
              [edsorted, _, maxl] = sortEdges(l_nouv_edges)
              logging.debug("  longueur edge trouvée: %s", maxl)
              edge = edsorted[-1]
            else:
              logging.info("problème partition recherche edge radiale %s", n_nappe)
              l_vertex = geompy.ExtractShapes(partNappeFace, geompy.ShapeType["VERTEX"], False)
              l_vertexnouv= list()
              for vertex in l_vertex:
                distx = geompy.MinDistance(vertex, face)
                distx += geompy.MinDistance(vertex, nappe)
                logging.debug("vertex distance: %s", distx)
                if distx < 0.005:
                  l_vertexnouv.append(vertex)
              logging.debug("nombre vertex candidats %s", len(l_vertexnouv))
              if len(l_vertexnouv) >= 2:
                l_edges = [geompy.MakeEdge(l_vertexnouv[j],l_vertexnouv[(j+1)%len(l_vertexnouv)]) for j in range(len(l_vertexnouv))]
                [edsorted2, _, maxl] = sortEdges(l_edges)
                edge = edsorted2[-1]
                logging.debug("lg edge: %s", maxl)
              else:
                logging.debug("problème recherche edge radiale %s non résolu", n_nappe)
          edges.append(edge)
          name = 'edgeEndPipe%d'%n_nappe
          geomPublish(initLog.debug, edge, name)
      listEdges.append(edges)

  return listEdges
