# -*- coding: utf-8 -*-

import logging
from geomsmesh import geompy
import math
from distance2 import distance2
import traceback
from fissError import fissError

# -----------------------------------------------------------------------------
# --- zone de defaut extraite du maillage

def creeZoneDefautMaillage(maillagesSains, shapeDefaut, tailleDefaut,
                           nomZones, coordsNoeudsFissure):
  """
  Identification de la zone à remailler, opérations sur le maillage
  de l'objet sain.
  La zone à remailler est définie à partir d'un objet géométrique
  ou à partir d'un jeu de points et d'une distance d'influence.
  @param maillagesSains : (le maillage de l'objet initial, booleen isHexa)
  @param shapeDefaut : objet géométrique représentant la fissure
  (selon les cas, un point central, ou une shape plus complexe,
  dont on ne garde que les vertices)
  @param tailleDefaut : distance d'influence définissant la zone à remailler:
  tous les éléments du maillage initial qui pénètrent dans cette zone
  sont détectés
  @param nomZones : préfixe des noms de groupes créés dans le maillage initial. S'il y a un groupe de noeuds
  @coordsNoeudsFissure : jeu de points donné par une liste (x1,y1,z1, x2,y2,z2, ...)
  @return (origShapes, verticesShapes, dmoyen) liste id subShapes,
  listes noeuds de bord, longueur arête moyenne bord
  """
  logging.info("start")
  
  maillageSain = maillagesSains[0]
  isHexa = maillagesSains[1]
  lists = maillageSain.CreateHoleSkin(tailleDefaut, shapeDefaut, nomZones, coordsNoeudsFissure)

  logging.debug("lists=%s", lists)

  trace = True
  origShapes = []
  verticesShapes = []

  cumul = 0 # somme des distances carrées entre point ordonnés (taille des arêtes)
  nb = 0    # nombre d'arêtes évaluées

  for aList in lists:
    aShape = aList[0]
    origShapes.append(aShape)
    logging.debug("  shapeId %s", aShape)
    vertices = []
    xyz0 = None
    for inode in range(1, len(aList)):
      xyz = maillageSain.GetNodeXYZ(aList[inode])
      if xyz0 is not None:
        cumul += distance2(xyz, xyz0)
        nb += 1
      xyz0 = xyz
      #logging.debug("    node %s %s", aList[inode], xyz)
      vertices.append(geompy.MakeVertex(xyz[0], xyz[1], xyz[2]))
      pass
    verticesShapes.append(vertices)
    pass

  if (nb == 0) :
    texte = "La zone à remailler n'est pas détectée correctement.<br>"
    texte += "Cause possible :<ul>"
    texte += "<li>La distance d'influence est trop petite. "
    texte += "L'ordre de grandeur minimal correspond à la taille des mailles du maillage sain dans la zone à remailler.</li></ul>"
    raise fissError(traceback.extract_stack(),texte)

  dmoyen = math.sqrt(cumul/nb) # ~ taille de l'arête moyenne du maillage global
  return origShapes, verticesShapes, dmoyen
