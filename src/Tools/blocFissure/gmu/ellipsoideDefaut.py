# -*- coding: utf-8 -*-

import logging
import math
from geomsmesh import geompy
from triedreBase import triedreBase
O, OX, OY, OZ = triedreBase()

# -----------------------------------------------------------------------------
# --- ellipsoide defaut

def ellipsoideDefaut(minRad,allonge,rayTore):
  """
  Le bloc contenant la fissure est un ellipsoide construit centre a l'origine,
  contenant le tore elliptique de fissure
  @param minRad :petit rayon
  @param allonge :rapport grand rayon / petit rayon
  @param rayTore :rayon du tore construit autour de la generatrice de l'ellipse
  @return  ellipsoide (geomObject)
  """
  logging.info("start") 
  boule = geompy.MakeSphereR(2)
  bouler = geompy.MakeRotation(boule, OY, math.pi/2.0)
  face = geompy.MakeFaceHW(100, 100, 3)
  boulepart = geompy.MakePartition([bouler], [face], [], [], geompy.ShapeType["SOLID"], 0, [], 0)
  solids = geompy.ExtractShapes(boulepart, geompy.ShapeType["SOLID"], True)
  solid0 = solids[0]
  for i in range(1,len(solids)):
    solid0 = geompy.MakeFuse(solid0, solids[i])
  ellipsoide = geompy.MakeScaleAlongAxes(solid0, O, minRad, minRad*(allonge+2.0)/2.0, minRad) # on limite l'allongement de l'ellipsoide
  #geompy.addToStudy( ellipsoide, 'ellipsoide' )
  return ellipsoide
