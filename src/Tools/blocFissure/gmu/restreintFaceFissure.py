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

import logging
from .geomsmesh import geompy
from .geomsmesh import geomPublish
from .geomsmesh import geomPublishInFather
from . import initLog
from .sortFaces import sortFaces
import traceback
from .fissError import fissError

def restreintFaceFissure(shapeDefaut, facesDefaut, pointInterne):
  """restriction de la face de fissure au domaine solide

  partition face fissure étendue par fillings
  """
  logging.info('start')
  partShapeDefaut = geompy.MakePartition([shapeDefaut], facesDefaut, [], [], geompy.ShapeType["FACE"], 0, [], 0)
  geomPublish(initLog.debug, partShapeDefaut, 'partShapeDefaut')
  facesPartShapeDefaut = geompy.ExtractShapes(partShapeDefaut, geompy.ShapeType["FACE"], False)
  if pointInterne is not None:
    distfaces = [(geompy.MinDistance(face,pointInterne), i, face) for i, face in enumerate(facesPartShapeDefaut)]
    distfaces.sort()
    logging.debug("selection de la face la plus proche du point interne, distance={}".format(distfaces[0][0]))
    facesPortFissure = distfaces[0][2]
  else:
    try:
      facesPartShapeDefautSorted, minSurf, maxSurf = sortFaces(facesPartShapeDefaut) # la face de fissure dans le volume doit être la plus grande
    except:
      texte = "Restriction de la face de fissure au domaine solide impossible.<br>"
      texte += "Causes possibles :<ul>"
      texte += "<li>La face de fissure est tangente à la paroi solide."
      texte += "Elle doit déboucher franchement, sans que la surface dehors ne devienne plus grande que la surface dans le solide.</li>"
      texte += "<li>le prémaillage de la face de fissure est trop grossier, les mailles à enlever dans le maillage sain "
      texte += "n'ont pas toutes été détectées.</li></ul>"
      raise fissError(traceback.extract_stack(),texte)
    logging.debug("surfaces faces fissure étendue, min {}, max {}".format(minSurf, maxSurf))
    facesPortFissure = facesPartShapeDefautSorted[-1]

  geomPublish(initLog.debug, facesPortFissure, "facesPortFissure")
  return facesPortFissure
