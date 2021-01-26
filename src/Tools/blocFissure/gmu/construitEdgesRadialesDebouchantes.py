# -*- coding: utf-8 -*-
# Copyright (C) 2014-2020  EDF R&D
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
"""Construit les arêtes débouchantes"""

import logging

from .geomsmesh import geompy
from .geomsmesh import geomPublish
#from .geomsmesh import geomPublishInFather
from . import initLog
import GEOM
from .sortEdges import sortEdges

def construitEdgesRadialesDebouchantes(idisklim, idiskout, gptsdisks, raydisks,
                                       facesPipePeau, edgeRadFacePipePeau, nbsegCercle):
  """construction des listes d'edges radiales sur chaque extrémité débouchante"""
  logging.info('start')

  # --- listes de nappes radiales en filling à chaque extrémité débouchante

  facesDebouchantes = [False, False]
  idFacesDebouchantes = [-1, -1] # contiendra les indices des faces disque débouchantes (facesPipePeau)
  listNappes =[]
  for i, idisk in enumerate(idisklim):
    numout = idiskout[i]
    logging.debug("extremité %s, indices disques interne %s, externe %s",i, idisk, numout)
    nappes = []
    if  (idisk != 0) and (idisk != len(gptsdisks)-1): # si extrémité débouchante
      for k in range(nbsegCercle):
        if i == 0:
          iddeb = max(0, numout)
          idfin = max(iddeb+3,idisk+1) # il faut 3 rayons pour faire un filling qui suive le fond de fissure
          #logging.debug("extremité %s, indices retenus interne %s, externe %s",i, idfin, iddeb)
          comp = geompy.MakeCompound(raydisks[k][iddeb:idfin])
          name='compoundRay%d'%k
          geomPublish(initLog.debug, comp, name)
        else:
          idfin = min(len(gptsdisks), numout+1)
          iddeb = min(idfin-3, idisk) # il faut 3 rayons pour faire un filling qui suive le fond de fissure
          #logging.debug("extremité %s, indices retenus interne %s, externe %s",i, idfin, iddeb)
          comp = geompy.MakeCompound(raydisks[k][iddeb:idfin])
          name='compoundRay%d'%k
          geomPublish(initLog.debug, comp, name)
        nappe = geompy.MakeFilling(comp, 2, 5, 0.0001, 0.0001, 0, GEOM.FOM_Default)
        nappes.append(nappe)
        name='nappe%d'%k
        geomPublish(initLog.debug, nappe, name)
        facesDebouchantes[i] = True
    listNappes.append(nappes)

  # --- mise en correspondance avec les indices des faces disque débouchantes (facesPipePeau)
  for i, nappes in enumerate(listNappes):
    if facesDebouchantes[i]:
      for k, face in enumerate(facesPipePeau):
        #logging.debug('i, k, face, nappes[0] %s %s %s %s', i, k, face, nappes[0])
        #geomPublish(initLog.debug, nappes[0], 'lanappe')
        #geomPublish(initLog.debug, face, 'laface')
        edge = geompy.MakeSection(face, nappes[0])
        if geompy.NbShapes(edge, geompy.ShapeType["EDGE"]) > 0:
          idFacesDebouchantes[i] = k
          break
  logging.debug("idFacesDebouchantes: %s", idFacesDebouchantes)

  # --- construction des listes d'edges radiales sur chaque extrémité débouchante
  listEdges = []
  for i, nappes in enumerate(listNappes):
    ifd = idFacesDebouchantes[i] # indice de face débouchante (facesPipePeau)
    if ifd < 0:
      listEdges.append([])
    else:
      face = facesPipePeau[ifd]
      edges = [edgeRadFacePipePeau[ifd]]
      for k, nappe in enumerate(nappes):
        if k > 0:
          obj = geompy.MakeSection(face, nappes[k]) # normalement une edge, parfois un compound d'edges dont un tout petit
          edge = obj
          vs = geompy.ExtractShapes(obj, geompy.ShapeType["VERTEX"], False)
          if len(vs) > 2:
            eds = geompy.ExtractShapes(obj, geompy.ShapeType["EDGE"], False)
            [edsorted, _, maxl] = sortEdges(eds)
            edge = edsorted[-1]
          else:
            maxl = geompy.BasicProperties(edge)[0]
          if maxl < 0.01: # problème MakeSection
            logging.info("problème MakeSection recherche edge radiale %s, longueur trop faible: %s, utilisation partition", k, maxl)
            partNappeFace = geompy.MakePartition([face, nappes[k]], [] , [], [], geompy.ShapeType["FACE"], 0, [], 0)
            edps= geompy.ExtractShapes(partNappeFace, geompy.ShapeType["EDGE"], False)
            ednouv = []
            for ii, ed in enumerate(edps):
              dmax=100.
              vxs = geompy.ExtractShapes(ed, geompy.ShapeType["VERTEX"], False)
              distx = [geompy.MinDistance(vx, face) for vx in vxs]
              distx += [geompy.MinDistance(vx, nappes[k]) for vx in vxs]
              dmax = max(distx)
              lgedge = geompy.BasicProperties(ed)[0]
              logging.debug("  dmax %s, longueur edge %s",dmax, lgedge)
              if dmax < 0.01 and  lgedge > 0.01:
                ednouv.append(ed)
            if (len(ednouv) > 0):
              logging.debug("  edges issues de la partition: %s", ednouv)
              for ii, ed in enumerate(ednouv):
                geomPublish(initLog.debug, ed, "ednouv%d"%ii)
              [edsorted, _, maxl] = sortEdges(ednouv)
              logging.debug("  longueur edge trouvée: %s", maxl)
              edge = edsorted[-1]
            else:
              logging.info("problème partition recherche edge radiale %s", k)
              vxs = geompy.ExtractShapes(partNappeFace, geompy.ShapeType["VERTEX"], False)
              vxnouv=[]
              for ii,vx in enumerate(vxs):
                distx = geompy.MinDistance(vx, face)
                distx += geompy.MinDistance(vx, nappes[k])
                logging.debug("vertex distance: %s", distx)
                if distx < 0.005:
                  vxnouv.append(vx)
              logging.debug("nombre vertex candidats %s", len(vxnouv))
              if len(vxnouv) >= 2:
                eds = [geompy.MakeEdge(vxnouv[j],vxnouv[(j+1)%len(vxnouv)]) for j in range(len(vxnouv))]
                [edsorted2, _, maxl] = sortEdges(eds)
                edge = edsorted2[-1]
                logging.debug("lg edge: %s", maxl)
              else:
                logging.debug("problème recherche edge radiale %s non résolu", k)
          edges.append(edge)
          name = 'edgeEndPipe%d'%k
          geomPublish(initLog.debug, edge, name)
      listEdges.append(edges)

  return (listEdges, idFacesDebouchantes)
