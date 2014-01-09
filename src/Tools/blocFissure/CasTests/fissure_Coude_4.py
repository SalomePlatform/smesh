# -*- coding: utf-8 -*-

from fissure_Coude  import fissure_Coude

class fissure_Coude_4(fissure_Coude):
  """
  probleme de fissure du Coude : ASCOU09A
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
                           n_ep         = 5,
                           n_long_coude = 30,
                           n_circ_g     = 50,
                           n_circ_d     = 20,
                           n_long_p2    = 12)

# ---------------------------------------------------------------------------
  def setParamShapeFissure(self):
    """
    paramètres de la fissure
    profondeur  : 0 < profondeur <= épaisseur
    azimut      : entre 0 et 360°
    alpha       : 0 < alpha < angleCoude
    longueur    : <=2*profondeur ==> ellipse, >2*profondeur = fissure longue
    orientation : 0° : longitudinale, 90° : circonférentielle, autre : uniquement fissures elliptiques
    externe     : True : fissure face externe, False : fissure face interne
    """
    print "setParamShapeFissure", self.nomCas
    self.shapeFissureParams = dict(nomRep        = '.',
                                   nomFicSain    = self.nomCas,
                                   nomFicFissure = 'fissure_' + self.nomCas,
                                   profondeur  = 10,
                                   azimut      = 90,
                                   alpha       = 20,
                                   longueur    = 240,
                                   orientation = 90,
                                   lgInfluence = 30,
                                   elliptique  = False,
                                   convexe     = True,
                                   externe     = True)

  # ---------------------------------------------------------------------------
  def setReferencesMaillageFissure(self):
    self.referencesMaillageFissure = dict(Entity_Quad_Pyramid    = 948,
                                          Entity_Quad_Triangle   = 1562,
                                          Entity_Quad_Edge       = 1192,
                                          Entity_Quad_Penta      = 732,
                                          Entity_Quad_Hexa       = 22208,
                                          Entity_Node            = 133418,
                                          Entity_Quad_Tetra      = 18759,
                                          Entity_Quad_Quadrangle = 11852)

