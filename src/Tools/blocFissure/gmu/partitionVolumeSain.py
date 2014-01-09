# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy

# -----------------------------------------------------------------------------
# --- partition volume sain et bloc, face du bloc recevant la fissure

def partitionVolumeSain(volumeSain,boiteDefaut):
  """
  Partition du volume complet sur lequel porte le calcul par le cube qui contiendra le defaut
  @param volumeSain : volume complet (geomObject)
  @param boiteDefaut : cube qui contiendra le defaut, positionné dans l'espace (son centre doit être au voisinage
  immediat de la peau de l'objet sain: le tore elliptique debouche de paroi)
  @return (volumeSainPart, partieSaine, volDefaut, faceBloc) : volume complet partionné par le cube, partie saine,
  bloc du defaut (solide commun au cube et au volume complet), face du bloc defaut correspondant à la paroi.
  """
  logging.info("start")

  volumeSainPart = geompy.MakePartition([volumeSain], [boiteDefaut], [], [], geompy.ShapeType["SOLID"], 0, [], 0)
  [a,b] = geompy.ExtractShapes(volumeSainPart, geompy.ShapeType["SOLID"], True)
  volDefaut = geompy.GetInPlaceByHistory(volumeSainPart, boiteDefaut)
  if geompy.GetSubShapeID(volumeSainPart,b) == geompy.GetSubShapeID(volumeSainPart,volDefaut):
    partieSaine = a
  else:
    partieSaine = b
  faceBloc = geompy.GetShapesOnShapeAsCompound(volumeSain, volDefaut, geompy.ShapeType["FACE"], GEOM.ST_ON)

  geompy.addToStudy( volumeSainPart, 'volumeSainPart' )
  geompy.addToStudyInFather( volumeSainPart, partieSaine, 'partieSaine' )
  geompy.addToStudyInFather( volumeSainPart, volDefaut, 'volDefaut' )
  geompy.addToStudyInFather( volDefaut, faceBloc, 'faceBloc' )
  return volumeSainPart, partieSaine, volDefaut, faceBloc
