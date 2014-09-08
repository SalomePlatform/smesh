# -*- coding: utf-8 -*-

from cubeAngle import cubeAngle

class cubeAngle2(cubeAngle):
  """
  problème de fissure plane coupant 2 faces (angle), débouches normaux, objet plan
  detection d'un probleme de tolerance sur les edges de jonction pipe et face fissure externe 
  """

  nomProbleme = "cubeAngle2"

  # ---------------------------------------------------------------------------
  def setParamShapeFissure(self):
    """
    paramètres de la fissure pour méthode construitFissureGenerale
    lgInfluence : distance autour de la shape de fissure a remailler (A ajuster selon le maillage)
    rayonPipe   : le rayon du pile maillé en hexa autour du fond de fissure
    """
    self.shapeFissureParams = dict(lgInfluence = 20,
                                   rayonPipe   = 5)

  # ---------------------------------------------------------------------------
  def setParamMaillageFissure(self):
    self.maillageFissureParams = dict(nomRep           = '.',
                                      nomFicSain       = self.nomCas,
                                      nomFicFissure    = 'fissure_' + self.nomCas,
                                      nbsegRad         = 5,
                                      nbsegCercle      = 32,
                                      areteFaceFissure = 5)

  # ---------------------------------------------------------------------------
  def setReferencesMaillageFissure(self):
    self.referencesMaillageFissure = dict(Entity_Quad_Pyramid    = 748,
                                          Entity_Quad_Triangle   = 1228,
                                          Entity_Quad_Edge       = 351,
                                          Entity_Quad_Penta      = 640,
                                          Entity_Quad_Hexa       = 5827,
                                          Entity_Node            = 42865,
                                          Entity_Quad_Tetra      = 9216,
                                          Entity_Quad_Quadrangle = 2518)

