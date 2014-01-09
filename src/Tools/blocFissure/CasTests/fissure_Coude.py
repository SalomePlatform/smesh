# -*- coding: utf-8 -*-

from blocFissure import gmu
from blocFissure.gmu.geomsmesh import geompy, smesh

import math
import GEOM
import SALOMEDS
import SMESH
#import StdMeshers
#import GHS3DPlugin
#import NETGENPlugin
import logging

from blocFissure.gmu.fissureGenerique import fissureGenerique

from blocFissure.gmu.triedreBase import triedreBase
from blocFissure.gmu.genereMeshCalculZoneDefaut import genereMeshCalculZoneDefaut
from blocFissure.gmu.creeZoneDefautDansObjetSain import creeZoneDefautDansObjetSain
from blocFissure.gmu.insereFissureLongue import insereFissureLongue

O, OX, OY, OZ = triedreBase()

class fissure_Coude(fissureGenerique):
  """
  problème de fissure du Coude : version de base
  maillage hexa
  """

  nomProbleme = "tuyau_Coude"

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
    self.geomParams = dict(angleCoude = 60,
                           r_cintr    = 1200,
                           l_tube_p1  = 1600,
                           l_tube_p2  = 1200,
                           epais      = 40,
                           de         = 760)

  # ---------------------------------------------------------------------------
  def genereGeometrieSaine(self, geomParams):
    logging.info("genereGeometrieSaine %s", self.nomCas)

    angleCoude = geomParams['angleCoude']
    r_cintr    = geomParams['r_cintr']
    l_tube_p1  = geomParams['l_tube_p1']
    l_tube_p2  = geomParams['l_tube_p2']
    epais      = geomParams['epais']
    de         = geomParams['de']

    centre = geompy.MakeVertex(0, 0, -l_tube_p1)
    Disk_1 = geompy.MakeDiskPntVecR(centre, OZ, de/2.)
    Disk_2 = geompy.MakeDiskPntVecR(centre, OZ, de/2. -epais)
    Cut_1 = geompy.MakeCut(Disk_1, Disk_2)
    Extrusion_1 = geompy.MakePrismVecH(Cut_1, OZ, l_tube_p1)
    axe = geompy.MakeTranslation(OY, -r_cintr, 0, -l_tube_p1)
    Revolution_1 = geompy.MakeRevolution(Cut_1, axe, angleCoude*math.pi/180.0)
    Rotation_1 = geompy.MakeRotation(Cut_1, axe, angleCoude*math.pi/180.0)
    Rotation_2 = geompy.MakeRotation(OZ, OY, angleCoude*math.pi/180.0)
    Extrusion_2 = geompy.MakePrismVecH(Rotation_1, Rotation_2, -l_tube_p2)
    Plane_1 = geompy.MakePlaneLCS(None, 100000, 3)
    geompy.addToStudy( Plane_1, "Plane_1" )
    geompy.addToStudy( Extrusion_1, "Extrusion_1" )
    geompy.addToStudy( Revolution_1, "Revolution_1" )
    geompy.addToStudy( Extrusion_2, "Extrusion_2" )

    P1 = O
    geompy.addToStudy( P1, "P1" )
    op2 = geompy.MakeVertex(0, 0, -l_tube_p1)
    P2 = geompy.MakeRotation(op2, axe, angleCoude*math.pi/180.0)
    P2 = geompy.MakeTranslationVectorDistance(P2, Rotation_2, -l_tube_p2)
    geompy.addToStudy( P2, "P2" )

    # --- tube coude sain

    geometrieSaine = geompy.MakePartition([Extrusion_1, Revolution_1, Extrusion_2, P1, P2], [Plane_1], [], [], geompy.ShapeType["SOLID"], 0, [], 1)
    geompy.addToStudy( geometrieSaine, self.nomCas )
    [P1, P2] = geompy.RestoreGivenSubShapes(geometrieSaine, [P1, P2], GEOM.FSM_GetInPlaceByHistory, False, True)

    [ep, circ_g, circ_d, long_p2, long_coude, long_p1] = geompy.Propagate(geometrieSaine)
    geompy.addToStudyInFather( geometrieSaine, long_p1, 'long_p1' )
    geompy.addToStudyInFather( geometrieSaine, ep, 'ep' )
    geompy.addToStudyInFather( geometrieSaine, long_coude, 'long_coude' )
    geompy.addToStudyInFather( geometrieSaine, circ_g, 'circ_g' )
    geompy.addToStudyInFather( geometrieSaine, circ_d, 'circ_d' )
    geompy.addToStudyInFather( geometrieSaine, long_p2, 'long_p2' )

    # --- face extremite tube (EXTUBE)

    facesIds = geompy.GetShapesOnPlaneIDs(geometrieSaine, geompy.ShapeType["FACE"], OZ, GEOM.ST_ON)
    EXTUBE = geompy.CreateGroup(geometrieSaine, geompy.ShapeType["FACE"])
    geompy.UnionIDs(EXTUBE, facesIds)
    geompy.addToStudyInFather( geometrieSaine, EXTUBE, 'EXTUBE' )

    # --- edge bord extremite tube (BORDTU)

    edge1Ids = geompy.GetShapesOnPlaneIDs(geometrieSaine, geompy.ShapeType["EDGE"], OZ, GEOM.ST_ON)
    edge2Ids = geompy.GetShapesOnCylinderIDs(geometrieSaine, geompy.ShapeType["EDGE"], OZ, de/2., GEOM.ST_ON)
    edgesIds = []
    for edge in edge1Ids:
      if edge in edge2Ids:
        edgesIds.append(edge)
    BORDTU = geompy.CreateGroup(geometrieSaine, geompy.ShapeType["EDGE"])
    geompy.UnionIDs(BORDTU, edgesIds)
    geompy.addToStudyInFather( geometrieSaine, BORDTU, 'BORDTU' )

    # --- face origine tube (CLGV)

    pp2 = geompy.MakeTranslationVectorDistance(P2, Rotation_2, 10)
    vec2 = geompy.MakeVector(P2, pp2)
    #geompy.addToStudy(vec2, 'vec2')
    facesIds = geompy.GetShapesOnPlaneIDs(geometrieSaine, geompy.ShapeType["FACE"], vec2, GEOM.ST_ON)
    CLGV = geompy.CreateGroup(geometrieSaine, geompy.ShapeType["FACE"])
    geompy.UnionIDs(CLGV, facesIds)
    geompy.addToStudyInFather( geometrieSaine, CLGV, 'CLGV' )

    # --- peau tube interieur (PEAUINT)

    extru1 = geompy.MakePrismVecH(Disk_2, OZ, l_tube_p1)
    revol1 = geompy.MakeRevolution(Disk_2, axe, angleCoude*math.pi/180.0)
    rot1 = geompy.MakeRotation(Disk_2, axe, angleCoude*math.pi/180.0)
    extru2 = geompy.MakePrismVecH(rot1, Rotation_2, -l_tube_p2)
    interne = geompy.MakeFuse(extru1, revol1)
    interne = geompy.MakeFuse(extru2, interne)
    geompy.addToStudy(interne, 'interne')
    facesIds = geompy.GetShapesOnShapeIDs(interne, geometrieSaine, geompy.ShapeType["FACE"], GEOM.ST_ONIN)
    PEAUINT = geompy.CreateGroup(geometrieSaine, geompy.ShapeType["FACE"])
    geompy.UnionIDs(PEAUINT, facesIds)
    geompy.addToStudyInFather( geometrieSaine, PEAUINT, 'PEAUINT' )

    # --- peau tube exterieur (PEAUEXT)

    cercle1 = geompy.MakeCircle(centre, OZ, de/2.)
    extru1 = geompy.MakePrismVecH(cercle1, OZ, l_tube_p1)
    revol1 = geompy.MakeRevolution(cercle1, axe, angleCoude*math.pi/180.0)
    rot1 = geompy.MakeRotation(cercle1, axe, angleCoude*math.pi/180.0)
    extru2 = geompy.MakePrismVecH(rot1, Rotation_2, -l_tube_p2)
    externe = geompy.MakeFuse(extru1, revol1)
    externe = geompy.MakeFuse(extru2, externe)
    geompy.addToStudy(externe, 'externe')
    facesIds = geompy.GetShapesOnShapeIDs(externe, geometrieSaine, geompy.ShapeType["FACE"], GEOM.ST_ON)
    PEAUEXT = geompy.CreateGroup(geometrieSaine, geompy.ShapeType["FACE"])
    geompy.UnionIDs(PEAUEXT, facesIds)
    geompy.addToStudyInFather( geometrieSaine, PEAUEXT, 'PEAUEXT' )

    # --- solide sain

    volIds = geompy.SubShapeAllIDs(geometrieSaine, geompy.ShapeType["SOLID"])
    COUDE = geompy.CreateGroup(geometrieSaine, geompy.ShapeType["SOLID"])
    geompy.UnionIDs(COUDE, volIds)
    geompy.addToStudyInFather( geometrieSaine, COUDE, 'COUDSAIN' )

    geometriesSaines = [geometrieSaine, long_p1, ep, long_coude, circ_g, circ_d, long_p2, P1, P2, EXTUBE, BORDTU, CLGV, PEAUINT, PEAUEXT, COUDE]

    return geometriesSaines

  # ---------------------------------------------------------------------------
  def setParamMaillageSain(self):
    self.meshParams = dict(n_long_p1    = 16,
                           n_ep         = 3,
                           n_long_coude = 15,
                           n_circ_g     = 20,
                           n_circ_d     = 20,
                           n_long_p2    = 12)

  # ---------------------------------------------------------------------------
  def genereMaillageSain(self, geometriesSaines, meshParams):
    logging.info("genereMaillageSain %s", self.nomCas)

    geometrieSaine = geometriesSaines[0]
    long_p1        = geometriesSaines[1]
    ep             = geometriesSaines[2]
    long_coude     = geometriesSaines[3]
    circ_g         = geometriesSaines[4]
    circ_d         = geometriesSaines[5]
    long_p2        = geometriesSaines[6]
    P1             = geometriesSaines[7]
    P2             = geometriesSaines[8]
    EXTUBE         = geometriesSaines[9]
    BORDTU         = geometriesSaines[10]
    CLGV           = geometriesSaines[11]
    PEAUINT        = geometriesSaines[12]
    PEAUEXT        = geometriesSaines[13]
    COUDE          = geometriesSaines[14]

    n_long_p1    = meshParams['n_long_p1']
    n_ep         = meshParams['n_ep']
    n_long_coude = meshParams['n_long_coude']
    n_circ_g     = meshParams['n_circ_g']
    n_circ_d     = meshParams['n_circ_d']
    n_long_p2    = meshParams['n_long_p2']

    maillageSain = smesh.Mesh(geometrieSaine)

    algo3d = maillageSain.Hexahedron()
    algo2d = maillageSain.Quadrangle()
    smesh.SetName(algo3d, "algo3d_maillageSain")
    smesh.SetName(algo2d, "algo2d_maillageSain")

    algo1d_long_p1 = maillageSain.Segment(geom=long_p1)
    hypo1d_long_p1 = algo1d_long_p1.NumberOfSegments(n_long_p1)
    smesh.SetName(algo1d_long_p1, "algo1d_long_p1")
    smesh.SetName(hypo1d_long_p1, "hypo1d_long_p1")

    algo1d_ep = maillageSain.Segment(geom=ep)
    hypo1d_ep = algo1d_ep.NumberOfSegments(n_ep)
    smesh.SetName(algo1d_ep, "algo1d_ep")
    smesh.SetName(hypo1d_ep, "hypo1d_ep")

    algo1d_long_coude = maillageSain.Segment(geom=long_coude)
    hypo1d_long_coude = algo1d_long_coude.NumberOfSegments(n_long_coude)
    smesh.SetName(algo1d_long_coude, "algo1d_long_coude")
    smesh.SetName(hypo1d_long_coude, "hypo1d_long_coude")

    algo1d_circ_g = maillageSain.Segment(geom=circ_g)
    hypo1d_circ_g = algo1d_circ_g.NumberOfSegments(n_circ_g)
    smesh.SetName(algo1d_circ_g, "algo1d_circ_g")
    smesh.SetName(hypo1d_circ_g, "hypo1d_circ_g")

    algo1d_circ_d = maillageSain.Segment(geom=circ_d)
    hypo1d_circ_d = algo1d_circ_d.NumberOfSegments(n_circ_d)
    smesh.SetName(algo1d_circ_d, "algo1d_circ_d")
    smesh.SetName(hypo1d_circ_d, "hypo1d_circ_d")

    algo1d_long_p2 = maillageSain.Segment(geom=long_p2)
    hypo1d_long_p2 = algo1d_long_p2.NumberOfSegments(n_long_p2)
    smesh.SetName(algo1d_long_p2, "algo1d_long_p2")
    smesh.SetName(hypo1d_long_p2, "hypo1d_long_p2")

    isDone = maillageSain.Compute()

    mp1 = maillageSain.GroupOnGeom(P1,'P1',SMESH.NODE)
    mp2 = maillageSain.GroupOnGeom(P2,'P2',SMESH.NODE)
    ext = maillageSain.GroupOnGeom(EXTUBE,'EXTUBE',SMESH.FACE)
    btu = maillageSain.GroupOnGeom(BORDTU,'BORDTU',SMESH.EDGE)
    clg = maillageSain.GroupOnGeom(CLGV,'CLGV',SMESH.FACE)
    pei = maillageSain.GroupOnGeom(PEAUINT,'PEAUINT',SMESH.FACE)
    pex = maillageSain.GroupOnGeom(PEAUEXT,'PEAUEXT',SMESH.FACE)
    cou = maillageSain.GroupOnGeom(COUDE,'COUDSAIN',SMESH.VOLUME)

    return [maillageSain, True] # True : maillage hexa

  # ---------------------------------------------------------------------------
  def setParamShapeFissure(self):
    """
    paramètres de la fissure
    profondeur  : 0 < profondeur <= épaisseur
    azimut      : entre 0 et 360°
    alpha       : 0 < alpha < angleCoude
    longueur    : <=2*profondeur ==> ellipse, >2*profondeur = fissure longue
    lgInfluence : distance autour de la shape de fissure a remailler (si 0, pris égal à profondeur. A ajuster selon le maillage)
    orientation : 0° : longitudinale, 90° : circonférentielle, autre : uniquement fissures elliptiques
    externe     : True : fissure face externe, False : fissure face interne
    """
    logging.info("setParamShapeFissure %s", self.nomCas)
    self.shapeFissureParams = dict(profondeur  = 10,
                                   azimut      = 160,
                                   alpha       = 20,
                                   longueur    = 400,
                                   orientation = 90,
                                   lgInfluence = 0,
                                   elliptique  = False,
                                   externe     = True)

  # ---------------------------------------------------------------------------
  def genereShapeFissure( self, geometriesSaines, geomParams, shapeFissureParams):
    logging.info("genereShapeFissure %s", self.nomCas)
    logging.info("shapeFissureParams %s", shapeFissureParams)

    angleCoude = geomParams['angleCoude']
    r_cintr    = geomParams['r_cintr']
    l_tube_p1  = geomParams['l_tube_p1']
    l_tube_p2  = geomParams['l_tube_p2']
    epais      = geomParams['epais']
    de         = geomParams['de']

    profondeur  = shapeFissureParams['profondeur']
    azimut      = shapeFissureParams['azimut']
    alpha       = shapeFissureParams['alpha']
    longueur    = shapeFissureParams['longueur']
    orientation = shapeFissureParams['orientation']
    externe     = shapeFissureParams['externe']
    lgInfluence = shapeFissureParams['lgInfluence']

    azimut = -azimut # axe inverse / ASCOUF
    axe = geompy.MakeTranslation(OY, -r_cintr, 0, -l_tube_p1)
    
    if not lgInfluence:
      lgInfluence = profondeur

    if longueur > 2*profondeur:
      self.fissureLongue=True
    else:
      self.fissureLongue=False

    self.circonferentielle = False
    self.longitudinale = False
    if self.fissureLongue and (abs(orientation) < 45) :
      self.longitudinale = True
    elif self.fissureLongue:
      self.circonferentielle = True

    if self.circonferentielle:
      if externe:
        raybor = de/2.
        rayint = raybor - profondeur
        rayext = raybor + profondeur
      else:
        rayext = de/2. - epais
        rayint = raybor + profondeur
        rayext = raybor - profondeur
      lgfond = longueur -2*profondeur
      angle = lgfond/(2*raybor)
      pb = geompy.MakeVertex(raybor, 0, 0)
      pi = geompy.MakeVertex(rayint, 0, 0)
      pe = geompy.MakeVertex(rayext, 0, 0)
      pl = geompy.MakeVertex(raybor, profondeur, 0)
      pr = geompy.MakeVertex(raybor, -profondeur, 0)
      pil = geompy.MakeRotation(pi, OZ, angle)
      pll = geompy.MakeRotation(pl, OZ, angle)
      pel = geompy.MakeRotation(pe, OZ, angle)
      pir = geompy.MakeRotation(pi, OZ, -angle)
      prr = geompy.MakeRotation(pr, OZ, -angle)
      per = geompy.MakeRotation(pe, OZ, -angle)
      arcl = geompy.MakeArc(pil, pll, pel)
      arcr = geompy.MakeArc(pir, prr, per)
      arci = geompy.MakeArc(pil, pi, pir)
      arce = geompy.MakeArc(pel, pe, per)
      wire0 = geompy.MakeWire([arcr, arci, arcl])
      cercle0 = geompy.MakeCircle(O, OY, profondeur/4.0)
      cercle0 = geompy.MakeRotation(cercle0, OY, math.pi/2.0)
      cercle0 = geompy.MakeTranslationTwoPoints(cercle0, O, pi)
      facetube0 = geompy.MakeFaceWires([cercle0], 1)
      facetubel = geompy.MakeRotation(facetube0, OZ, angle)
      facetuber = geompy.MakeRotation(facetube0, OZ, -angle)
      face0 = geompy.MakeFaceWires([arcl,arci, arce, arcr], 1)
      plan0 = geompy.MakePlane(O, OZ, 10000)
      geompy.addToStudy( face0, 'facefissOrig' )
      face1 = geompy.MakeRotation(face0, OZ, azimut*math.pi/180.)
      face2 = geompy.MakeTranslation(face1, 0, 0, -l_tube_p1)
      facefiss = geompy.MakeRotation(face2, axe, alpha*math.pi/180.)
      geompy.addToStudy( facefiss, 'facefissPlace' )
      centre = geompy.MakeRotation(pb, OZ, azimut*math.pi/180.)
      centre = geompy.MakeTranslation(centre, 0, 0, -l_tube_p1)
      centre = geompy.MakeRotation(centre, axe, alpha*math.pi/180.)
      geompy.addToStudy( centre, 'centrefissPlace' )
      arcr = geompy.MakeRotation(arcr, OZ, azimut*math.pi/180.)
      arcr = geompy.MakeTranslation(arcr, 0, 0, -l_tube_p1)
      arcr = geompy.MakeRotation(arcr, axe, alpha*math.pi/180.)
      arci = geompy.MakeRotation(arci, OZ, azimut*math.pi/180.)
      arci = geompy.MakeTranslation(arci, 0, 0, -l_tube_p1)
      arci = geompy.MakeRotation(arci, axe, alpha*math.pi/180.)
      arcl = geompy.MakeRotation(arcl, OZ, azimut*math.pi/180.)
      arcl = geompy.MakeTranslation(arcl, 0, 0, -l_tube_p1)
      arcl = geompy.MakeRotation(arcl, axe, alpha*math.pi/180.)
      wiretube = geompy.MakeRotation(wire0, OZ, azimut*math.pi/180.)
      wiretube = geompy.MakeTranslation(wiretube, 0, 0, -l_tube_p1)
      wiretube = geompy.MakeRotation(wiretube, axe, alpha*math.pi/180.)
      geompy.addToStudy(wiretube, 'wiretubePlace' )
      facetubel = geompy.MakeRotation(facetubel, OZ, azimut*math.pi/180.)
      facetubel = geompy.MakeTranslation(facetubel, 0, 0, -l_tube_p1)
      facetubel = geompy.MakeRotation(facetubel, axe, alpha*math.pi/180.)
      geompy.addToStudy(facetubel, 'facetubeGauche' )
      facetuber = geompy.MakeRotation(facetuber, OZ, azimut*math.pi/180.)
      facetuber = geompy.MakeTranslation(facetuber, 0, 0, -l_tube_p1)
      facetuber = geompy.MakeRotation(facetuber, axe, alpha*math.pi/180.)
      geompy.addToStudy(facetuber, 'facetubeDroit' )
      planfiss = geompy.MakeRotation(plan0, OZ, azimut*math.pi/180.)
      planfiss = geompy.MakeTranslation(planfiss, 0, 0, -l_tube_p1)
      planfiss = geompy.MakeRotation(planfiss, axe, alpha*math.pi/180.)
      geompy.addToStudy(planfiss, 'planfissPlace' )
      pipefissl = geompy.MakePipe(facetubel, arcl)
      pipefissi = geompy.MakePipe(facetubel, arci)
      pipefissr = geompy.MakePipe(facetuber, arcr)
      pipefiss = geompy.MakePartition([pipefissl, pipefissi, pipefissr], [planfiss, wiretube], [], [], geompy.ShapeType["SOLID"], 0, [], 0)
      geompy.addToStudy(pipefiss, 'pipefissPlace' )


    elif self.longitudinale:
      if externe:
        raybor = r_cintr + de/2.
        rayint = raybor - profondeur
        rayext = raybor + profondeur
      else:
        rayext = r_cintr + de/2. - epais
        rayint = raybor + profondeur
        rayext = raybor - profondeur
      lgfond = longueur -2*profondeur
      angle = lgfond/(2*raybor)
      pb = geompy.MakeVertex(-raybor, 0, 0)
      pi = geompy.MakeVertex(-rayint, 0, 0)
      pe = geompy.MakeVertex(-rayext, 0, 0)
      pl = geompy.MakeVertex(-raybor, 0, -profondeur)
      pr = geompy.MakeVertex(-raybor, 0, profondeur)
      pil = geompy.MakeRotation(pi, OY, -angle)
      pll = geompy.MakeRotation(pl, OY, -angle)
      pel = geompy.MakeRotation(pe, OY, -angle)
      pir = geompy.MakeRotation(pi, OY, angle)
      prr = geompy.MakeRotation(pr, OY, angle)
      per = geompy.MakeRotation(pe, OY, angle)
      arcl = geompy.MakeArc(pil, pll, pel)
      arcr = geompy.MakeArc(pir, prr, per)
      arci = geompy.MakeArc(pil, pi, pir)
      arce = geompy.MakeArc(pel, pe, per)
      geompy.addToStudy( arcl, 'arcl' )
      geompy.addToStudy( arcr, 'arcr' )
      geompy.addToStudy( arci, 'arci' )
      geompy.addToStudy( arce, 'arce' )
      wire0 = geompy.MakeWire([arcr, arci, arcl])
      cercle0 = geompy.MakeCircle(O, OZ, profondeur/4.0)
      #cercle0 = geompy.MakeRotation(cercle0, OZ, math.pi/2.0)
      cercle0 = geompy.MakeTranslationTwoPoints(cercle0, O, pi)
      geompy.addToStudy( cercle0, 'cercle0' )
      facetube0 = geompy.MakeFaceWires([cercle0], 1)
      facetubel = geompy.MakeRotation(facetube0, OY, -angle)
      facetuber = geompy.MakeRotation(facetube0, OY, angle)
      geompy.addToStudy(facetubel , 'facetubel' )
      geompy.addToStudy( facetuber, 'facetuber' )
      face0 = geompy.MakeFaceWires([arcl,arci, arce, arcr], 1)
      plan0 = geompy.MakePlane(O, OY, 10000)
      geompy.addToStudy( face0, 'facefissOrig' )
      facefiss = geompy.MakeRotation(face0, OY, alpha*math.pi/180.)
      geompy.addToStudy( facefiss, 'facefissPlace' )
      centre = geompy.MakeRotation(pb, OY, alpha*math.pi/180.)
      geompy.addToStudy( centre, 'centrefissPlace' )
      arcr = geompy.MakeRotation(arcr, OY, alpha*math.pi/180.)
      arci = geompy.MakeRotation(arci, OY, alpha*math.pi/180.)
      arcl = geompy.MakeRotation(arcl, OY, alpha*math.pi/180.)
      wiretube = geompy.MakeRotation(wire0, OY, alpha*math.pi/180.)
      geompy.addToStudy(wiretube, 'wiretubePlace' )
      facetubel = geompy.MakeRotation(facetubel, OY, alpha*math.pi/180.)
      geompy.addToStudy(facetubel, 'facetubeGauche' )
      facetuber = geompy.MakeRotation(facetuber, OY, alpha*math.pi/180.)
      geompy.addToStudy(facetubel, 'facetubeDroit' )
      planfiss = geompy.MakeRotation(plan0, OY, alpha*math.pi/180.)
      geompy.addToStudy(planfiss, 'planfissPlace' )
      pipefissl = geompy.MakePipe(facetubel, arcl)
      pipefissi = geompy.MakePipe(facetubel, arci)
      pipefissr = geompy.MakePipe(facetuber, arcr)
      pipefiss = geompy.MakePartition([pipefissl, pipefissi, pipefissr], [planfiss, wiretube], [], [], geompy.ShapeType["SOLID"], 0, [], 0)
      geompy.addToStudy(pipefiss, 'pipefissPlace' )
    else:
      pass

    coordsNoeudsFissure = genereMeshCalculZoneDefaut(facefiss, 5 ,10)

    return [facefiss, centre, lgInfluence, coordsNoeudsFissure, wiretube, facetubel, facetuber, planfiss, pipefiss]

  # ---------------------------------------------------------------------------
  def setParamMaillageFissure(self):
    self.maillageFissureParams = dict(nomRep        = '.',
                                      nomFicSain    = self.nomCas,
                                      nomFicFissure = 'fissure_' + self.nomCas,
                                      nbsegExt      = 5,
                                      nbsegGen      = 25,
                                      nbsegRad      = 5,
                                      scaleRad      = 4,
                                      nbsegCercle   = 6,
                                      nbsegFis      = 20,
                                      lensegEllipso = 1.0)

  # ---------------------------------------------------------------------------
  def genereZoneDefaut(self, geometriesSaines, maillagesSains, shapesFissure, shapeFissureParams, maillageFissureParams):
    elementsDefaut = creeZoneDefautDansObjetSain(geometriesSaines, maillagesSains, shapesFissure, shapeFissureParams, maillageFissureParams)
    return elementsDefaut

  # ---------------------------------------------------------------------------
  def genereMaillageFissure(self, geometriesSaines, maillagesSains,
                            shapesFissure, shapeFissureParams,
                            maillageFissureParams, elementsDefaut, step):
    maillageFissure = insereFissureLongue(geometriesSaines, maillagesSains,
                                          shapesFissure, shapeFissureParams,
                                          maillageFissureParams, elementsDefaut, step)
    return maillageFissure

  # ---------------------------------------------------------------------------
  def setReferencesMaillageFissure(self):
    self.referencesMaillageFissure = dict(Entity_Node            = 77491,
                                          Entity_Quad_Edge       = 1006,
                                          Entity_Quad_Triangle   = 2412,
                                          Entity_Quad_Quadrangle = 6710,
                                          Entity_Quad_Tetra      = 20853,
                                          Entity_Quad_Hexa       = 8656,
                                          Entity_Quad_Penta      = 1176,
                                          Entity_Quad_Pyramid    = 1232)

