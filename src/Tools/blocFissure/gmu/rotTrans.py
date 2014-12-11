# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy
from geomsmesh import geomPublish
from geomsmesh import geomPublishInFather
import initLog
import math
from triedreBase import triedreBase
O, OX, OY, OZ = triedreBase()

# -----------------------------------------------------------------------------
# --- operateur de rotation translation d'un objet centré à l'origine

def rotTrans(objet, orientation, point, normal, trace = False):
  """
  Déplacement par rotation translation d'un objet centré à l'origine, vers un point de la surface de la pièce saine
  dans laquelle on insère le défaut.
  @param objet : objet original centré à l'origine (geomObject)
  @param orientation : rotation selon OX de l'objet original (degrés)
  @param point : le point qui sera le centre de l'objet déplacé (geomObject), en général sur la surface de la pièce saine
  @param normal : la normale à la surface de la pièce saine au point central (geomObject)
  @return trans : objet transformé (geomObject)
  """
  logging.info("start")
  planXY = geompy.MakePlaneLCS(None, 2000, 1)
  projXY = geompy.MakeProjection(normal, planXY)
  [v1,v2] = geompy.ExtractShapes(projXY, geompy.ShapeType["VERTEX"], False)
  xyz1 = geompy.PointCoordinates(v1)
  xyz2 = geompy.PointCoordinates(v2)
  x = xyz2[0] - xyz1[0]
  y = xyz2[1] - xyz1[1]
  sinalpha = y / math.sqrt(x*x + y*y)
  cosalpha = x / math.sqrt(x*x + y*y)
  alpha = math.asin(sinalpha)
  if cosalpha < 0:
    alpha = math.pi -alpha

  beta = geompy.GetAngleRadians(OZ, normal)
  [v1,v2] = geompy.ExtractShapes(normal, geompy.ShapeType["VERTEX"], False)
  xyz1 = geompy.PointCoordinates(v1)
  xyz2 = geompy.PointCoordinates(v2)
  z = xyz2[2] - xyz1[2]
  if z < 0:
    beta = math.pi -beta

  rot0 = geompy.MakeRotation(objet, OX, orientation*math.pi/180.0)
  rot1 = geompy.MakeRotation(rot0, OZ, alpha)
  axe2 = geompy.MakeRotation(OY, OZ, alpha)
  rot2 = geompy.MakeRotation(rot1, axe2, beta -math.pi/2.)
  logging.debug("alpha",alpha)
  logging.debug("beta",beta)
  if trace:
    geomPublish(initLog.debug,  rot1, 'rot1' )
    geomPublish(initLog.debug,  axe2, 'axe2' )
    geomPublish(initLog.debug,  rot2, 'rot2' )

  xyz = geompy.PointCoordinates(point)
  trans = geompy.MakeTranslation(rot2, xyz[0], xyz[1], xyz[2])
  return trans
