# -*- coding: utf-8 -*-

from blocFissure.gmu.fissureCoude  import fissureCoude

class fissureCoude_3(fissureCoude):
  """
  problème de fissure du Coude
  adaptation maillage
  """

# ---------------------------------------------------------------------------
  def setParamGeometrieSaine(self):
    """
    Paramètres géométriques du tuyau coudé sain:
    angleCoude
    r_cintr
    l_tube_p1
    l_tube_p2
    epais
    de
    """
    self.geomParams = dict(angleCoude = 40,
                           r_cintr    = 654,
                           l_tube_p1  = 1700,
                           l_tube_p2  = 1700,
                           epais      = 62.5,
                           de         = 912.4)

  # ---------------------------------------------------------------------------
  def setParamMaillageSain(self):
    self.meshParams = dict(n_long_p1    = 16,
                           n_ep         = 3,
                           n_long_coude = 30,
                           n_circ_g     = 50,
                           n_circ_d     = 20,
                           n_long_p2    = 12)

# ---------------------------------------------------------------------------
  def setParamShapeFissure(self):
    """
    paramètres de la fissure pour le tuyau coude
    profondeur  : 0 < profondeur <= épaisseur
    rayonPipe   : rayon du pipe correspondant au maillage rayonnant
    lenSegPipe  : longueur des mailles rayonnantes le long du fond de fissure (= rayonPipe par défaut)
    azimut      : entre 0 et 360°
    alpha       : 0 < alpha < angleCoude
    longueur    : <=2*profondeur ==> force une fissure elliptique (longueur/profondeur = grand axe/petit axe).
    orientation : 0° : longitudinale, 90° : circonférentielle, autre : uniquement fissures elliptiques
    lgInfluence : distance autour de la shape de fissure a remailler (si 0, pris égal à profondeur. A ajuster selon le maillage)
    elliptique  : True : fissure elliptique (longueur/profondeur = grand axe/petit axe); False : fissure longue (fond de fissure de profondeur constante, demi-cercles aux extrémites)
    pointIn_x   : optionnel coordonnées x d'un point dans le solide, pas trop loin du centre du fond de fissure (idem y,z)
    externe     : True : fissure face externe, False : fissure face interne
    """
    self.shapeFissureParams = dict(profondeur  = 10,
                                   rayonPipe   = 2.5,
                                   lenSegPipe  = 2.5,
                                   azimut      = 90,
                                   alpha       = 20,
                                   longueur    = 240,
                                   orientation = 90,
                                   lgInfluence = 30,
                                   elliptique  = False,
                                   externe     = False)

  # ---------------------------------------------------------------------------
  def setParamMaillageFissure(self):
    """
    Paramètres du maillage de la fissure pour le tuyau coudé
    Voir également setParamShapeFissure, paramètres rayonPipe et lenSegPipe.
    nbSegRad = nombre de couronnes
    nbSegCercle = nombre de secteurs
    areteFaceFissure = taille cible de l'arête des triangles en face de fissure.
    """
    self.maillageFissureParams = dict(nomRep        = '.',
                                      nomFicSain    = self.nomCas,
                                      nomFicFissure = 'fissure_' + self.nomCas,
                                      nbsegRad      = 5,
                                      nbsegCercle   = 6,
                                      areteFaceFissure = 5)

  # ---------------------------------------------------------------------------
  def setReferencesMaillageFissure(self):
    self.referencesMaillageFissure = dict(Entity_Node            = 98643,
                                          Entity_Quad_Edge       = 1130,
                                          Entity_Quad_Triangle   = 1476,
                                          Entity_Quad_Quadrangle = 11100,
                                          Entity_Quad_Tetra      = 15993,
                                          Entity_Quad_Hexa       = 14508,
                                          Entity_Quad_Penta      = 624,
                                          Entity_Quad_Pyramid    = 788)

