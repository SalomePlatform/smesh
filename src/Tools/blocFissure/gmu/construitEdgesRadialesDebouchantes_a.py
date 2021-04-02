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
"""listes de nappes radiales en filling à chaque extrémité débouchante"""

import logging
import GEOM

from .geomsmesh import geompy
from .geomsmesh import geomPublish
from . import initLog

def construitEdgesRadialesDebouchantes_a(idisklim, idiskout, gptsdisks, raydisks, nbsegCercle):
  """listes de nappes radiales en filling à chaque extrémité débouchante"""
  logging.info('start')

  facesDebouchantes = [False, False]
  listNappes = list()
  for n_extr, idisk in enumerate(idisklim):
    numout = idiskout[n_extr]
    logging.debug("extremité %s, indices disques interne %s, externe %s",n_extr, idisk, numout)
    nappes = list()
    if  (idisk != 0) and (idisk != len(gptsdisks)-1): # si extrémité débouchante
      for n_ray in range(nbsegCercle):
        if n_extr == 0:
          iddeb = max(0, numout)
          idfin = max(iddeb+3,idisk+1) # il faut 3 rayons pour faire un filling qui suive le fond de fissure
          #logging.debug("extremité %s, indices retenus interne %s, externe %s",n_extr, idfin, iddeb)
          comp = geompy.MakeCompound(raydisks[n_ray][iddeb:idfin])
          name='compoundRay%d'%n_ray
          geomPublish(initLog.debug, comp, name)
        else:
          idfin = min(len(gptsdisks), numout+1)
          iddeb = min(idfin-3, idisk) # il faut 3 rayons pour faire un filling qui suive le fond de fissure
          #logging.debug("extremité %s, indices retenus interne %s, externe %s",n_extr, idfin, iddeb)
          comp = geompy.MakeCompound(raydisks[n_ray][iddeb:idfin])
          name='compoundRay%d'%n_ray
          geomPublish(initLog.debug, comp, name)
        nappe = geompy.MakeFilling(comp, 2, 5, 0.0001, 0.0001, 0, GEOM.FOM_Default)
        nappes.append(nappe)
        name='nappe%d'%n_ray
        geomPublish(initLog.debug, nappe, name)
        facesDebouchantes[n_extr] = True
    listNappes.append(nappes)

  return facesDebouchantes, listNappes
