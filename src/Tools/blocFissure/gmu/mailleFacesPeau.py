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

import SMESH
from salome.smesh import smeshBuilder

from .geomsmesh import geompy
from .geomsmesh import geomPublishInFather
from .geomsmesh import smesh

from . import initLog

from .putName import putName

def mailleFacesPeau(partitionsPeauFissFond, idFillingFromBout, facesDefaut, \
                    facesPeaux, edCircPeau, gpedgeBord, gpedgeVifs, edFissPeau, \
                    bordsLibres, grpEdgesPeauFissureExterne, grpAretesVives, \
                    edgesCircPipeGroup, dmoyen, rayonPipe, nbsegRad, \
                    mailleur="MeshGems", nro_cas=None):
  """maillage faces de peau"""
  logging.info('start')
  logging.info("Maillage avec %s pour le cas n°%s", mailleur, nro_cas)

  nbFacesFilling = len(partitionsPeauFissFond)
  boutFromIfil = [None for _ in range(nbFacesFilling)]
  # repérage des extremites du pipe quand elles débouchent sur des faces différentes :
  if ( idFillingFromBout[0] != idFillingFromBout[1] ):
    boutFromIfil[idFillingFromBout[0]] = 0
    boutFromIfil[idFillingFromBout[1]] = 1

  logging.info("---------------------------- maillage faces de peau --------------")
  logging.info("nbFacesFilling = %d", nbFacesFilling)
  meshesFacesPeau = list()
  for ifil in range(nbFacesFilling):

    if partitionsPeauFissFond[ifil] is None: # face de peau maillage sain intacte
      logging.info("face de peau %d intacte", ifil)
      facePeau = facesDefaut[ifil]
    else:
      logging.info("face de peau %d coupée par la fissure", ifil)
      # pour chaque face : la face de peau finale a mailler (percée des faces débouchantes)
      facePeau = facesPeaux[ifil]
    meshFacePeau = smesh.Mesh(facePeau)
    putName(meshFacePeau.GetMesh(), "facePeau", ifil, nro_cas)

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
      hypo2d.SetChordalErrorEnabled (True)
      hypo2d.SetChordalError( dmoyen*0.25 )
      hypo2d.SetUseSurfaceCurvature (True)
      hypo2d.SetQuadAllowed( 0 )
    putName(hypo2d, "{}_2d_facePeau".format(mailleur), ifil, nro_cas)

    if partitionsPeauFissFond[ifil] is None: # face de peau maillage sain intacte

      logging.debug("face de peau %d intacte", ifil)
      # --- edges de bord de la face de filling
      filling = facesDefaut[ifil]
      edgesFilling = geompy.ExtractShapes(filling, geompy.ShapeType["EDGE"], False)
      gpedgeBord[ifil] = geompy.CreateGroup(filling, geompy.ShapeType["EDGE"])
      geompy.UnionList(gpedgeBord[ifil], edgesFilling)
      geomPublishInFather(initLog.always, filling, gpedgeBord[ifil], "EdgesBords", nro_cas)

      logging.info("UseExisting1DElements sur la géométrie '%s' avec les mailles de '%s'", \
                    gpedgeBord[ifil].GetName(), bordsLibres.GetName())
      algo1d = meshFacePeau.UseExisting1DElements(geom=gpedgeBord[ifil])
      putName(algo1d.GetSubMesh(), "bordsLibres", ifil, nro_cas)
      hypo1d = algo1d.SourceEdges([ bordsLibres ],0,0)
      putName(hypo1d, "SourceEdges_{}".format(bordsLibres.GetName()), ifil, nro_cas)

    else:

      logging.info("face de peau %d coupée par la fissure", ifil)
      # edCircPeau[ifil] # pour chaque face de peau : [subshape edge circulaire aux débouchés du pipe]
      # gpedgeBord[ifil] : pour chaque face de peau : groupe subshape des edges aux bords liés à la partie saine
      # gpedgeVifs[ifil] : pour chaque face de peau : groupe subshape des edges aux bords correspondant à des arêtes vives
      # edFissPeau[ifil] : pour chaque face de peau : [subshape edge en peau des faces de fissure externes]

      logging.info("UseExisting1DElements sur la géométrie '%s' avec les mailles de '%s'", \
                    gpedgeBord[ifil].GetName(), bordsLibres.GetName())
      algo1d = meshFacePeau.UseExisting1DElements(geom=gpedgeBord[ifil])
      putName(algo1d.GetSubMesh(), "bordsLibres", ifil, nro_cas)
      hypo1d = algo1d.SourceEdges([ bordsLibres ],0,0)
      putName(hypo1d, "SourceEdges_{}".format(bordsLibres.GetName()), i_pref=nro_cas)

      objet = geompy.MakeCompound(edFissPeau[ifil])
      geomPublishInFather(initLog.always, facePeau, objet, "edFissPeau_{}".format(ifil))
      logging.info("UseExisting1DElements sur la géométrie '%s' avec les mailles de '%s'", \
                    objet.GetName(), grpEdgesPeauFissureExterne.GetName())
      algo1d = meshFacePeau.UseExisting1DElements(geom=objet)
      putName(algo1d.GetSubMesh(), objet.GetName(), i_pref=nro_cas)
      hypo1d = algo1d.SourceEdges([ grpEdgesPeauFissureExterne ],0,0)
      putName(hypo1d, "SourceEdges_{}".format(grpEdgesPeauFissureExterne.GetName()), i_pref=nro_cas)

      if gpedgeVifs[ifil] is not None:
        logging.info("UseExisting1DElements sur la géométrie '%s' avec les mailles de '%s'", \
                      gpedgeVifs[ifil].GetName(), grpAretesVives.GetName())
        algo1d = meshFacePeau.UseExisting1DElements(geom=gpedgeVifs[ifil])
        putName(algo1d.GetSubMesh(), "gpedgeVifs", ifil, nro_cas)
        hypo1d = algo1d.SourceEdges([ grpAretesVives ],0,0)
        putName(hypo1d, "SourceEdges_{}".format(grpAretesVives.GetName()), i_pref=nro_cas)

      for i_aux, edgeCirc in enumerate(edCircPeau[ifil]):
        texte = "i_aux = {}".format(i_aux)
        logging.info(texte)
        if edgeCirc is not None:
          if boutFromIfil[ifil] is None:
            groupe = edgesCircPipeGroup[i_aux]
          else:
            groupe = edgesCircPipeGroup[boutFromIfil[ifil]]
          logging.info("UseExisting1DElements sur la géométrie '%s' avec les mailles de '%s'", \
                        edgeCirc.GetName(), groupe.GetName())
          algo1d = meshFacePeau.UseExisting1DElements(geom=edgeCirc) # addToStudy() failed ?
          putName(algo1d.GetSubMesh(), groupe.GetName(), i_pref=nro_cas)
          hypo1d = algo1d.SourceEdges([ groupe ],0,0)
          putName(hypo1d, "SourceEdges_{}".format(groupe.GetName()), i_pref=nro_cas)

    is_done = meshFacePeau.Compute()
    text = "meshFacePeau {} .Compute".format(ifil)
    if is_done:
      logging.info(text+" OK")
    else:
      text = "Erreur au calcul du maillage.\n" + text
      logging.info(text)
      raise Exception(text)

    groupe_des_faces = meshFacePeau.CreateEmptyGroup( SMESH.FACE, "facePeau%d"%ifil )
    _ = groupe_des_faces.AddFrom( meshFacePeau.GetMesh() )
    meshesFacesPeau.append(meshFacePeau)

  return meshesFacesPeau
