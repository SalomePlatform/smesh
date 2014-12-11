# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy
from geomsmesh import geomPublish
from geomsmesh import geomPublishInFather
import initLog

# -----------------------------------------------------------------------------
# --- crée zone géométrique défaut a partir d'un filling

def creeZoneDefautFilling(filling, shapeDefaut, lgExtrusion=50):
  """
  Construction CAO de la zone à remailler, quand on utilise un filling,
  après appel creeZoneDefautMaillage et quadranglesToShapeNoCorner
  @param filling : la CAO de la peau du défaut reconstituée
  @param shapeDefaut : objet géométrique représentant la fissure
  (selon les cas, un point central, ou une shape plus complexe,
  dont on ne garde que les vertices)
  @return (facesDefaut = filling, centreDefaut, normalDefaut, extrusionDefaut)
  """
  logging.info("start")

  trace = True
  facesDefaut = filling
  centreSphere = geompy.MakeCDG(shapeDefaut)
  geomPublish(initLog.debug, centreSphere, "cdg_defaut")
  centreDefaut = geompy.MakeProjection(centreSphere, filling)
  if trace:
    geomPublish(initLog.debug, centreDefaut, "centreDefaut")
  normalDefaut = geompy.GetNormal(filling, centreDefaut)
  if trace:
    geomPublish(initLog.debug, normalDefaut, "normalDefaut")
  extrusionDefaut = geompy.MakePrismVecH(filling, normalDefaut, -lgExtrusion)
  if trace:
    geomPublish(initLog.debug, extrusionDefaut, "extrusionDefaut")

  return facesDefaut, centreDefaut, normalDefaut, extrusionDefaut
