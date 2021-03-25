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

"""maillage faces de peau"""

import logging

from .geomsmesh import geompy
from .geomsmesh import geomPublish
from .geomsmesh import geomPublishInFather
from . import initLog
from .geomsmesh import smesh
from salome.smesh import smeshBuilder
import SMESH

from .putName import putName

def mailleFacesPeau(partitionsPeauFissFond, idFillingFromBout, facesDefaut, \
                    facesPeaux, edCircPeau, ptCircPeau, gpedgeBord, gpedgeVifs, edFissPeau, \
                    bordsLibres, grpEdgesPeauFissureExterne, grpAretesVives, \
                    edgesCircPipeGroup, dmoyen, rayonPipe, nbsegRad, \
                    mailleur="MeshGems"):
  """maillage faces de peau"""
  logging.info('start')
  logging.info(mailleur)
  nbFacesFilling = len(partitionsPeauFissFond)
  boutFromIfil = [None for i in range(nbFacesFilling)]
  if idFillingFromBout[0] != idFillingFromBout[1]: # repérage des extremites du pipe quand elles débouchent sur des faces différentes
    boutFromIfil[idFillingFromBout[0]] = 0
    boutFromIfil[idFillingFromBout[1]] = 1

  logging.info("---------------------------- maillage faces de peau --------------")
  logging.info("nbFacesFilling = %d", nbFacesFilling)
  meshesFacesPeau = list()
  for ifil in range(nbFacesFilling):
    meshFacePeau = None
    if partitionsPeauFissFond[ifil] is None: # face de peau maillage sain intacte

      logging.debug("meshFacePeau %d intacte", ifil)
      # --- edges de bord de la face de filling
      filling = facesDefaut[ifil]
      edgesFilling = geompy.ExtractShapes(filling, geompy.ShapeType["EDGE"], False)
      groupEdgesBordPeau = geompy.CreateGroup(filling, geompy.ShapeType["EDGE"])
      geompy.UnionList(groupEdgesBordPeau, edgesFilling)
      geomPublishInFather(initLog.debug,filling, groupEdgesBordPeau , "EdgesBords")

      meshFacePeau = smesh.Mesh(facesDefaut[ifil])

      algo1d = meshFacePeau.UseExisting1DElements(geom=groupEdgesBordPeau)
      hypo1d = algo1d.SourceEdges([ bordsLibres ],0,0)
      putName(algo1d.GetSubMesh(), "bordsLibres", ifil)
      putName(algo1d, "algo1d_bordsLibres", ifil)
      putName(hypo1d, "hypo1d_bordsLibres", ifil)

    else:

      logging.info("meshFacePeau %d coupée par la fissure", ifil)
      facePeau           = facesPeaux[ifil] # pour chaque face : la face de peau finale a mailler (percée des faces débouchantes)
      edgesCircPeau      = edCircPeau[ifil] # pour chaque face de peau : [subshape edge circulaire aux débouchés du pipe]
      _                  = ptCircPeau[ifil] # pour chaque face de peau : [subshape point sur edge circulaire aux débouchés du pipe]
      groupEdgesBordPeau = gpedgeBord[ifil] # pour chaque face de peau : groupe subshape des edges aux bords liés à la partie saine
      bordsVifs          = gpedgeVifs[ifil] # pour chaque face de peau : groupe subshape des edges aux bords correspondant à des arêtes vives
      edgesFissurePeau   = edFissPeau[ifil] # pour chaque face de peau : [subshape edge en peau des faces de fissure externes]

      logging.info("a")
      meshFacePeau = smesh.Mesh(facePeau)

      algo1d = meshFacePeau.UseExisting1DElements(geom=groupEdgesBordPeau)
      hypo1d = algo1d.SourceEdges([ bordsLibres ],0,0)
      putName(algo1d.GetSubMesh(), "bordsLibres", ifil)
      putName(algo1d, "algo1d_bordsLibres", ifil)
      putName(hypo1d, "hypo1d_bordsLibres", ifil)

      logging.info("b")
      algo1d = meshFacePeau.UseExisting1DElements(geom=geompy.MakeCompound(edgesFissurePeau))
      hypo1d = algo1d.SourceEdges([ grpEdgesPeauFissureExterne ],0,0)
      putName(algo1d.GetSubMesh(), "edgePeauFiss", ifil)
      putName(algo1d, "algo1d_edgePeauFiss", ifil)
      putName(hypo1d, "hypo1d_edgePeauFiss", ifil)

      if bordsVifs is not None:
        algo1d = meshFacePeau.UseExisting1DElements(geom=bordsVifs)
        hypo1d = algo1d.SourceEdges([ grpAretesVives ],0,0)
        putName(algo1d.GetSubMesh(), "bordsVifs", ifil)
        putName(algo1d, "algo1d_bordsVifs", ifil)
        putName(hypo1d, "hypo1d_bordsVifs", ifil)

      for i, edgeCirc in enumerate(edgesCircPeau):
        logging.info("i = {}".format(i))
        if edgeCirc is not None:
          algo1d = meshFacePeau.UseExisting1DElements(geom=edgeCirc)
          if boutFromIfil[ifil] is None:
            hypo1d = algo1d.SourceEdges([ edgesCircPipeGroup[i] ],0,0)
          else:
            hypo1d = algo1d.SourceEdges([ edgesCircPipeGroup[boutFromIfil[ifil]] ],0,0)
          name = "cercle%d"%i
          putName(algo1d.GetSubMesh(), name, ifil)
          putName(algo1d, "algo1d_" + name, ifil)
          putName(hypo1d, "hypo1d_" + name, ifil)

    logging.info("c")
    logging.info("Maillage avec %s", mailleur)
    if ( mailleur == "MeshGems"):
      algo2d = meshFacePeau.Triangle(algo=smeshBuilder.MG_CADSurf)
      hypo2d = algo2d.Parameters()
      hypo2d.SetPhySize( dmoyen )
      hypo2d.SetMinSize( rayonPipe/float(nbsegRad) )
      hypo2d.SetMaxSize( dmoyen*2. )
      hypo2d.SetChordalError( dmoyen*0.25 )
      hypo2d.SetVerbosity( 0 )
    else:
      algo2d = meshFacePeau.Triangle(algo=smeshBuilder.NETGEN_1D2D)
      hypo2d = algo2d.Parameters()
      hypo2d.SetMaxSize( dmoyen )
      hypo2d.SetOptimize( 1 )
      hypo2d.SetFineness( 2 )
      hypo2d.SetMinSize( rayonPipe/float(nbsegRad) )
      hypo2d.SetQuadAllowed( 0 )
    logging.info("d")
    putName(algo2d.GetSubMesh(), "facePeau", ifil)
    putName(algo2d, "algo2d_facePeau", ifil)
    putName(hypo2d, "hypo2d_facePeau", ifil)

    is_done = meshFacePeau.Compute()
    text = "meshFacePeau {} .Compute".format(ifil)
    if is_done:
      logging.info(text+" OK")
    else:
      text = "Erreur au calcul du maillage.\n" + text
      logging.info(text)
      raise Exception(text)

    GroupFaces = meshFacePeau.CreateEmptyGroup( SMESH.FACE, "facePeau%d"%ifil )
    _ = GroupFaces.AddFrom( meshFacePeau.GetMesh() )
    meshesFacesPeau.append(meshFacePeau)

  return meshesFacesPeau
