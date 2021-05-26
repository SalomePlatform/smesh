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
"""construction de la fissure générale - inventaire des faces de peau coupées par la fissure"""

import logging
from .geomsmesh import geomPublish
from . import initLog

from .identifieElementsGeometriquesPeau import identifieElementsGeometriquesPeau

# -----------------------------------------------------------------------------

def construitFissureGenerale_b( partitionsPeauFissFond, \
                                edgesPipeFiss, edgesFondFiss, wireFondFiss, aretesVivesC, \
                                facesDefaut, centreFondFiss, rayonPipe, aretesVivesCoupees, \
                                nro_cas=None ):
  """construction de la fissure générale - inventaire des faces de peau coupées par la fissure

pour chaque face de peau : 0, 1 ou 2 faces débouchante du fond de fissure
                           0, 1 ou plus edges de la face de fissure externe au pipe
  """
  logging.info('start')

  nbFacesFilling = len(partitionsPeauFissFond)
  texte = "nbFacesFilling : {} ".format(nbFacesFilling)
  logging.info(texte)

  ptEdgeFond = [ list() for _ in range(nbFacesFilling)] # pour chaque face [points edge fond de fissure aux débouchés du pipe]
  fsPipePeau = [ list() for _ in range(nbFacesFilling)] # pour chaque face [faces du pipe débouchantes]
  edRadFPiPo = [ list() for _ in range(nbFacesFilling)] # pour chaque face [edge radiale des faces du pipe débouchantes ]
  fsFissuExt = [ list() for _ in range(nbFacesFilling)] # pour chaque face [faces de fissure externes au pipe]
  edFisExtPe = [ list() for _ in range(nbFacesFilling)] # pour chaque face [edge en peau des faces de fissure externes (pas subshape facePeau)]
  edFisExtPi = [ list() for _ in range(nbFacesFilling)] # pour chaque face [edge commun au pipe des faces de fissure externes]
  facesPeaux = [None for _ in range(nbFacesFilling)] # pour chaque face : la face de peau finale a mailler (percée des faces débouchantes)
  edCircPeau = [ list() for _ in range(nbFacesFilling)] # pour chaque face de peau : [subshape edge circulaire aux débouchés du pipe]
  gpedgeBord = [None for _ in range(nbFacesFilling)] # pour chaque face de peau : groupe subshape des edges aux bords liés à la partie saine
  gpedgeVifs = [None for _ in range(nbFacesFilling)] # pour chaque face de peau : groupes subshape des edges aux arêtes vives entre fillings
  edFissPeau = [ list() for _ in range(nbFacesFilling)] # pour chaque face de peau : [subshape edge en peau des faces de fissure externes]
  ptFisExtPi = [ list() for _ in range(nbFacesFilling)] # pour chaque face de peau : [point commun edFissPeau edCircPeau]

  for ifil, partitionPeauFissFond in enumerate(partitionsPeauFissFond):
    if partitionPeauFissFond is not None:
      dataPPFF,aretesVivesCoupees = identifieElementsGeometriquesPeau(ifil, partitionPeauFissFond, \
                                                                      edgesPipeFiss, edgesFondFiss, wireFondFiss, aretesVivesC, \
                                                                      facesDefaut, centreFondFiss, rayonPipe, aretesVivesCoupees, \
                                                                      nro_cas)
      ptEdgeFond[ifil] = dataPPFF['endsEdgeFond']
      fsPipePeau[ifil] = dataPPFF['facesPipePeau']
      edRadFPiPo[ifil] = dataPPFF['edgeRadFacePipePeau']
      fsFissuExt[ifil] = dataPPFF['facesFissExt']
      edFisExtPe[ifil] = dataPPFF['edgesFissExtPeau']
      edFisExtPi[ifil] = dataPPFF['edgesFissExtPipe']
      facesPeaux[ifil] = dataPPFF['facePeau']
      edCircPeau[ifil] = dataPPFF['edgesCircPeau']
      gpedgeBord[ifil] = dataPPFF['groupEdgesBordPeau']
      gpedgeVifs[ifil] = dataPPFF['bordsVifs']
      edFissPeau[ifil] = dataPPFF['edgesFissurePeau']
      ptFisExtPi[ifil] = dataPPFF['verticesPipePeau']

  facesPipePeau = list()
  edgeRadFacePipePeau = list()
  for ifil in range(nbFacesFilling):
    facesPipePeau += fsPipePeau[ifil]
    edgeRadFacePipePeau += edRadFPiPo[ifil]

  for i_aux, avc in enumerate(aretesVivesCoupees):
    name = "areteViveCoupee{}".format(i_aux)
    geomPublish(initLog.debug, avc, name, nro_cas)

  return ptEdgeFond, fsFissuExt, edFisExtPe, edFisExtPi, facesPeaux, edCircPeau, \
         gpedgeBord, gpedgeVifs, edFissPeau, ptFisExtPi, \
         edgeRadFacePipePeau, facesPipePeau
