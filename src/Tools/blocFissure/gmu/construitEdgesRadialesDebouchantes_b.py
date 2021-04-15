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
"""Mise en correspondance avec les indices des faces disque débouchantes (facesPipePeau)"""

import logging

from .geomsmesh import geompy
#from .geomsmesh import geomPublish

#from . import initLog

def construitEdgesRadialesDebouchantes_b(facesPipePeau, facesDebouchantes, listNappes):
  """Mise en correspondance avec les indices des faces disque débouchantes (facesPipePeau)"""

  idFacesDebouchantes = [-1, -1] # contiendra les indices des faces disque débouchantes (facesPipePeau)
  for n_nappe, nappes in enumerate(listNappes):
    if facesDebouchantes[n_nappe]:
      for n_face, face in enumerate(facesPipePeau):
        #logging.debug('n_nappe, n_face, face, nappes[0] %s %s %s %s', n_nappe, n_face, face, nappes[0])
        #geomPublish(initLog.debug, nappes[0], 'lanappe')
        #geomPublish(initLog.debug, face, 'laface')
        edge = geompy.MakeSection(face, nappes[0])
        if geompy.NbShapes(edge, geompy.ShapeType["EDGE"]) > 0:
          idFacesDebouchantes[n_nappe] = n_face
          break
  logging.debug("idFacesDebouchantes: %s", idFacesDebouchantes)

  return idFacesDebouchantes
