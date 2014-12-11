# -*- coding: utf-8 -*-

from geomsmesh import geompy, smesh
from geomsmesh import geomPublish
from geomsmesh import geomPublishInFather
import initLog

import math
import GEOM
import SALOMEDS
import SMESH
#import StdMeshers
#import GHS3DPlugin
#import NETGENPlugin
import logging

from fissureGenerique import fissureGenerique

from triedreBase import triedreBase
from genereMeshCalculZoneDefaut import genereMeshCalculZoneDefaut
from creeZoneDefautDansObjetSain import creeZoneDefautDansObjetSain
from construitFissureGenerale import construitFissureGenerale
from sortEdges import sortEdges

O, OX, OY, OZ = triedreBase()

class fissureCoude(fissureGenerique):
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
    diskext = geompy.MakeDiskPntVecR(centre, OZ, de/2.)
    diskint = geompy.MakeDiskPntVecR(centre, OZ, de/2. -epais)
    couronne = geompy.MakeCut(diskext, diskint)
    tube_1 = geompy.MakePrismVecH(couronne, OZ, l_tube_p1)
    axe = geompy.MakeTranslation(OY, -r_cintr, 0, -l_tube_p1)
    coude = geompy.MakeRevolution(couronne, axe, angleCoude*math.pi/180.0)
    Rotation_1 = geompy.MakeRotation(couronne, axe, angleCoude*math.pi/180.0)
    Rotation_2 = geompy.MakeRotation(OZ, OY, angleCoude*math.pi/180.0)
    tube_2 = geompy.MakePrismVecH(Rotation_1, Rotation_2, -l_tube_p2)
    plan_y = geompy.MakePlaneLCS(None, 100000, 3)
    geomPublish(initLog.debug,  plan_y, "plan_y" )
    geomPublish(initLog.debug,  tube_1, "tube_1" )
    geomPublish(initLog.debug,  coude, "coude" )
    geomPublish(initLog.debug,  tube_2, "tube_2" )

    P1 = O
    geompy.addToStudy(P1, "P1" )
    op2 = geompy.MakeVertex(0, 0, -l_tube_p1)
    P2 = geompy.MakeRotation(op2, axe, angleCoude*math.pi/180.0)
    P2 = geompy.MakeTranslationVectorDistance(P2, Rotation_2, -l_tube_p2)
    geompy.addToStudy(P2, "P2" )

    # --- tube coude sain

    geometrieSaine = geompy.MakePartition([tube_1, coude, tube_2, P1, P2], [plan_y], [], [], geompy.ShapeType["SOLID"], 0, [], 1)
    geomPublish(initLog.debug,  geometrieSaine, self.nomCas )
    [P1, P2] = geompy.RestoreGivenSubShapes(geometrieSaine, [P1, P2], GEOM.FSM_GetInPlaceByHistory, False, True)

    xmin = -de -r_cintr -l_tube_p2
    zmin = -l_tube_p1 -r_cintr -l_tube_p2 -de
    ymax = de +100.
    boxypos = geompy.MakeBox(xmin, 0, zmin, ymax, ymax, 100, "boxypos")
    boxyneg = geompy.MakeBox(xmin, 0, zmin, ymax, -ymax, 100, "boxyneg")
    edgesypos = geompy.GetShapesOnShape(boxypos, geometrieSaine, geompy.ShapeType["EDGE"], GEOM.ST_IN)
    edgesyneg = geompy.GetShapesOnShape(boxyneg, geometrieSaine, geompy.ShapeType["EDGE"], GEOM.ST_IN)
    circ_g = geompy.CreateGroup(geometrieSaine, geompy.ShapeType["EDGE"])
    geompy.UnionList(circ_g, edgesyneg)
    circ_d = geompy.CreateGroup(geometrieSaine, geompy.ShapeType["EDGE"])
    geompy.UnionList(circ_d, edgesypos)
    edgesy0pos = geompy.GetShapesOnShape(boxypos, geometrieSaine, geompy.ShapeType["EDGE"], GEOM.ST_ONIN)
    grpedpos = geompy.CreateGroup(geometrieSaine, geompy.ShapeType["EDGE"])
    geompy.UnionList(grpedpos, edgesy0pos)
    grpedy0 = geompy.CutGroups(grpedpos, circ_d, "edges_y0")
    boxtub1 = geompy.MakeBox(-de/2.0 -1, -1, -l_tube_p1, de, de, 0, "boxtub1")
    edgestub1 = geompy.GetShapesOnShape(boxtub1, geometrieSaine, geompy.ShapeType["EDGE"], GEOM.ST_IN)
    grped = geompy.CreateGroup(geometrieSaine, geompy.ShapeType["EDGE"])
    geompy.UnionList(grped, edgestub1)
    long_p1 = geompy.IntersectGroups(grped, grpedy0)
    boxtub  = geompy.MakeBox(-de/2.0 -1, -1, -l_tube_p1 -l_tube_p2, de, de, -l_tube_p1)
    boxtub2 = geompy.MakeRotation(boxtub, axe, angleCoude*math.pi/180.0, "boxttub2")
    edgestub2 = geompy.GetShapesOnShape(boxtub2, geometrieSaine, geompy.ShapeType["EDGE"], GEOM.ST_IN)
    grped = geompy.CreateGroup(geometrieSaine, geompy.ShapeType["EDGE"])
    geompy.UnionList(grped, edgestub2)
    long_p2 = geompy.IntersectGroups(grped, grpedy0)
    boxtub1t = geompy.MakeTranslationVectorDistance(boxtub1, OZ, -l_tube_p1)
    facer = geompy.GetShapesOnShape(boxtub1t, boxtub1, geompy.ShapeType["FACE"], GEOM.ST_ONIN, "facer")
    boxcoud = geompy.MakeRevolution(facer[0], axe, angleCoude*math.pi/180.0, "boxcoud")
    edgescoud = geompy.GetShapesOnShape(boxcoud, geometrieSaine, geompy.ShapeType["EDGE"], GEOM.ST_IN)
    grped = geompy.CreateGroup(geometrieSaine, geompy.ShapeType["EDGE"])
    geompy.UnionList(grped, edgescoud)
    long_coude = geompy.IntersectGroups(grped, grpedy0)
    grped = geompy.CutGroups(grpedy0, long_p1)
    grped = geompy.CutGroups(grped, long_p2)
    ep = geompy.CutGroups(grped, long_coude)
    geomPublishInFather(initLog.debug, geometrieSaine, long_p1, 'long_p1' )
    geomPublishInFather(initLog.debug, geometrieSaine, ep, 'ep' )
    geomPublishInFather(initLog.debug, geometrieSaine, long_coude, 'long_coude' )
    geomPublishInFather(initLog.debug, geometrieSaine, circ_g, 'circ_g' )
    geomPublishInFather(initLog.debug, geometrieSaine, circ_d, 'circ_d' )
    geomPublishInFather(initLog.debug, geometrieSaine, long_p2, 'long_p2' )

    # --- face extremite tube (EXTUBE)

    facesIds = geompy.GetShapesOnPlaneIDs(geometrieSaine, geompy.ShapeType["FACE"], OZ, GEOM.ST_ON)
    EXTUBE = geompy.CreateGroup(geometrieSaine, geompy.ShapeType["FACE"])
    geompy.UnionIDs(EXTUBE, facesIds)
    geomPublishInFather(initLog.debug, geometrieSaine, EXTUBE, 'EXTUBE' )

    # --- edge bord extremite tube (BORDTU)

    edge1Ids = geompy.GetShapesOnPlaneIDs(geometrieSaine, geompy.ShapeType["EDGE"], OZ, GEOM.ST_ON)
    edge2Ids = geompy.GetShapesOnCylinderIDs(geometrieSaine, geompy.ShapeType["EDGE"], OZ, de/2. -epais, GEOM.ST_ON)
    edgesIds = []
    for edge in edge1Ids:
      if edge in edge2Ids:
        edgesIds.append(edge)
    BORDTU = geompy.CreateGroup(geometrieSaine, geompy.ShapeType["EDGE"])
    geompy.UnionIDs(BORDTU, edgesIds)
    geomPublishInFather(initLog.debug, geometrieSaine, BORDTU, 'BORDTU' )

    # --- face origine tube (CLGV)

    pp2 = geompy.MakeTranslationVectorDistance(P2, Rotation_2, 10)
    vec2 = geompy.MakeVector(P2, pp2)
    #geomPublish(initLog.debug, vec2, 'vec2')
    facesIds = geompy.GetShapesOnPlaneIDs(geometrieSaine, geompy.ShapeType["FACE"], vec2, GEOM.ST_ON)
    CLGV = geompy.CreateGroup(geometrieSaine, geompy.ShapeType["FACE"])
    geompy.UnionIDs(CLGV, facesIds)
    geomPublishInFather(initLog.debug, geometrieSaine, CLGV, 'CLGV' )

    # --- peau tube interieur (PEAUINT)

    extru1 = geompy.MakePrismVecH(diskint, OZ, l_tube_p1)
    revol1 = geompy.MakeRevolution(diskint, axe, angleCoude*math.pi/180.0)
    rot1 = geompy.MakeRotation(diskint, axe, angleCoude*math.pi/180.0)
    extru2 = geompy.MakePrismVecH(rot1, Rotation_2, -l_tube_p2)
    interne = geompy.MakeFuse(extru1, revol1)
    interne = geompy.MakeFuse(extru2, interne)
    geomPublish(initLog.debug, interne, 'interne')
    facesIds = geompy.GetShapesOnShapeIDs(interne, geometrieSaine, geompy.ShapeType["FACE"], GEOM.ST_ONIN)
    PEAUINT = geompy.CreateGroup(geometrieSaine, geompy.ShapeType["FACE"])
    geompy.UnionIDs(PEAUINT, facesIds)
    geomPublishInFather(initLog.debug, geometrieSaine, PEAUINT, 'PEAUINT' )

    # --- peau tube exterieur (PEAUEXT)

    cercle1 = geompy.MakeCircle(centre, OZ, de/2.)
    extru1 = geompy.MakePrismVecH(cercle1, OZ, l_tube_p1)
    revol1 = geompy.MakeRevolution(cercle1, axe, angleCoude*math.pi/180.0)
    rot1 = geompy.MakeRotation(cercle1, axe, angleCoude*math.pi/180.0)
    extru2 = geompy.MakePrismVecH(rot1, Rotation_2, -l_tube_p2)
    externe = geompy.MakeFuse(extru1, revol1)
    externe = geompy.MakeFuse(extru2, externe)
    geomPublish(initLog.debug, externe, 'externe')
    facesIds = geompy.GetShapesOnShapeIDs(externe, geometrieSaine, geompy.ShapeType["FACE"], GEOM.ST_ON)
    PEAUEXT = geompy.CreateGroup(geometrieSaine, geompy.ShapeType["FACE"])
    geompy.UnionIDs(PEAUEXT, facesIds)
    geomPublishInFather(initLog.debug, geometrieSaine, PEAUEXT, 'PEAUEXT' )

    # --- solide sain

    volIds = geompy.SubShapeAllIDs(geometrieSaine, geompy.ShapeType["SOLID"])
    COUDE = geompy.CreateGroup(geometrieSaine, geompy.ShapeType["SOLID"])
    geompy.UnionIDs(COUDE, volIds)
    geomPublishInFather(initLog.debug, geometrieSaine, COUDE, 'COUDSAIN' )

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
    logging.info("setParamShapeFissure %s", self.nomCas)
    self.shapeFissureParams = dict(profondeur  = 10,
                                   rayonPipe   = 2.5,
                                   lenSegPipe  = 2.5,
                                   azimut      = 160,
                                   alpha       = 20,
                                   longueur    = 400,
                                   orientation = 90,
                                   lgInfluence = 50,
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
    self.elliptique  = False
    if shapeFissureParams.has_key('elliptique'):
      self.elliptique = shapeFissureParams['elliptique']
      


    azimut = -azimut # axe inverse / ASCOUF
    axe = geompy.MakeTranslation(OY, -r_cintr, 0, -l_tube_p1)
    geomPublish(initLog.debug, axe,"axe")
    
    if not lgInfluence:
      lgInfluence = profondeur

    if longueur > 2*profondeur:
      self.fissureLongue=True
    else:
      self.fissureLongue=False
      self.elliptique = True

    self.circonferentielle = False
    self.longitudinale = False
    if self.fissureLongue and not self.elliptique:
      if abs(orientation) < 45 :
        self.longitudinale = True
      else:
        self.circonferentielle = True

    nbp1 = 10
    if self.circonferentielle:
      if externe:
        dp = -1.0
        raybor = de/2.
        rayint = raybor - profondeur
        rayext = raybor + profondeur/5.0
      else:
        dp = 1.0
        raybor = de/2. - epais
        rayint = raybor + profondeur
        rayext = raybor - profondeur/5.0
      lgfond = longueur -2*profondeur
      angle = lgfond/(2*raybor)
      pb = geompy.MakeVertex(raybor, 0, 0)
      pi = geompy.MakeVertex(rayint, 0, 0)      
      pbl = geompy.MakeRotation(pb, OZ, angle)
      pbr = geompy.MakeRotation(pb, OZ, -angle)
      geomPublish(initLog.debug, pbl,"pbl")
      geomPublish(initLog.debug, pbr,"pbr")
      pal = geompy.MakeTranslationVector(pbl, OZ)
      par = geompy.MakeTranslationVector(pbr, OZ)
      axl = geompy.MakeVector(pbl,pal)
      axr = geompy.MakeVector(pbr,par)
      pil = geompy.MakeRotation(pi, OZ, angle)
      pir = geompy.MakeRotation(pi, OZ, -angle)
      points = []
      nbp = 3*nbp1
      for i in range(nbp):
        angi = dp*(nbp -i)*(2.0*math.pi/3.0)/nbp
        pt = geompy.MakeRotation(pil, axl, angi)
        points.append(pt)
      for i in range(nbp):
        angi = angle -2.0*i*angle/nbp      
        pt = geompy.MakeRotation(pi, OZ, angi)
        points.append(pt)
      for i in range(nbp+1):
        angi = -dp*i*(2.0*math.pi/3.0)/nbp
        pt = geompy.MakeRotation(pir, axr, angi)
        points.append(pt)
      for i, pt in enumerate(points):
        pt = geompy.MakeRotation(pt, OZ, azimut*math.pi/180.)
        pt = geompy.MakeTranslation(pt, 0, 0, -l_tube_p1)
        pt = geompy.MakeRotation(pt, axe, alpha*math.pi/180.)
        points[i] = pt
      wire0 = geompy.MakeInterpol(points[0:nbp+1])     
      wire1 = geompy.MakeInterpol(points[nbp:2*nbp+1])     
      wire2 = geompy.MakeInterpol(points[2*nbp:3*nbp+1])     
      #wiretube = geompy.MakeInterpol(points)
      wiretube=geompy.MakeWire([wire0,wire1,wire2])
      geomPublish(initLog.debug, wiretube,"wiretube")
      
      pe = geompy.MakeVertex(rayext, 0, 0)
      pe = geompy.MakeRotation(pe, OZ, azimut*math.pi/180.)
      pe = geompy.MakeTranslation(pe, 0, 0, -l_tube_p1)
      pe = geompy.MakeRotation(pe, axe, alpha*math.pi/180.)
     
      arce = geompy.MakeArc(points[0], pe, points[-1])
      geomPublish(initLog.debug, arce,"arce")
      
      facefiss = geompy.MakeFaceWires([arce, wiretube], 1)
      geomPublish(initLog.debug,  facefiss, 'facefissPlace' )
      
      pc = geompy.MakeVertex((raybor + rayint)/2.0, 0, 0)
      centre = geompy.MakeRotation(pc, OZ, azimut*math.pi/180.)
      centre = geompy.MakeTranslation(centre, 0, 0, -l_tube_p1)
      centre = geompy.MakeRotation(centre, axe, alpha*math.pi/180.)
      geomPublish(initLog.debug,  centre, 'centrefissPlace' )
      
      wiretube = geompy.GetInPlace(facefiss, wiretube)
      geomPublish(initLog.debug, wiretube, 'wiretubePlace' )
      try:
        edgetube = geompy.MakeEdgeWire(wiretube)
        geomPublish(initLog.debug, edgetube,"edgetube")
      except:
        logging.debug("erreur MakeEdgeWire sur fond de fissure, on fait sans")
        edgetube = None

    # ---------------------------------------------------------

    elif self.longitudinale:
      if externe:
        raybor = de/2.
        dp = -1.0
      else:
        raybor = de/2. - epais
        dp = +1.0
      prof = dp * profondeur     
      lgfond = longueur -2*profondeur
      cosaz = math.cos(azimut*math.pi/180.)
      sinaz = math.sin(azimut*math.pi/180.)
      alfrd = alpha*math.pi/180.
      rayxy = r_cintr + raybor*cosaz
      angle = lgfond/(2.*rayxy)
      logging.debug("longueur: %s, angle: %s, rayon: %s",lgfond, angle, rayxy)
      pb = geompy.MakeVertex(raybor*cosaz, raybor*sinaz, -l_tube_p1, "pb")
      pi = geompy.MakeTranslation(pb, prof*cosaz, prof*sinaz, 0., "pi")
      pbv = geompy.MakeTranslation(pb, -sinaz, cosaz, 0., "pbv")
      axb  = geompy.MakeVector(pb,pbv, "axb")
      pbl = geompy.MakeRotation(pb, axe, alfrd -angle, "pbl")
      pbr = geompy.MakeRotation(pb, axe, alfrd +angle, "pbr")
      axl = geompy.MakeRotation(axb, axe, alfrd -angle, "axl")
      axr = geompy.MakeRotation(axb, axe, alfrd +angle, "axr")
      pil = geompy.MakeRotation(pi, axe, alfrd -angle, "pil")
      pir = geompy.MakeRotation(pi, axe, alfrd +angle, "pir")

      curves = []

      points = []
      nbp = 3*nbp1
      xs = []
      totx = 0
      for i in range(nbp+2):
        x = math.sin(i*math.pi/(nbp+1)) # fonction de répartition des points : distance relative
        x2 = x*x
        totx += x2        
        xs.append(totx)
        logging.debug("x2: %s, totx: %s", x2, totx)     
      for i in range(nbp+1):
        #posi = nbp -i             # répartition équidistante des points sur la courbe
        posi = nbp*(1 -xs[i]/totx) # points plus resserrés aux extrémités de la courbe
        angi = -dp*posi*(5.0*math.pi/8.0)/nbp
        pt = geompy.MakeRotation(pil, axl, angi)
        points.append(pt)
      curves.append(geompy.MakeInterpol(points))
      point0 = points[0]
      geomPublish(initLog.debug, curves[-1],"curve0")
#      for i, pt in enumerate(points):
#        name = "point%d"%i
#        geomPublishInFather(initLog.debug,curves[-1], pt, name)

      points = []
      nbp = 3*nbp1
      xs =[]
      totx = 0
      for i in range(nbp+1):
        x = math.sin(i*math.pi/nbp)
        #x = 1.0 # répartition équidistante des points sur la courbe
        x2 = x*x # points plus resserrés aux extrémités de la courbe
        totx += x2        
        xs.append(totx)
        logging.debug("x2: %s, totx: %s", x2, totx)     
      for i in range(nbp):
        angi = alfrd -angle +2.0*angle*xs[i]/totx      
        pt = geompy.MakeRotation(pi, axe, angi)
        points.append(pt)
      curves.append(geompy.MakeInterpol(points))
      geomPublish(initLog.debug, curves[-1],"curve1")
#      for i, pt in enumerate(points):
#        name = "point%d"%i
#        geomPublishInFather(initLog.debug,curves[-1], pt, name)

      points = []
      nbp = 3*nbp1
      xs = []
      totx = 0
      for i in range(nbp+2):
        x = math.sin(i*math.pi/(nbp+1))
        x2 = x*x
        totx += x2        
        xs.append(totx)
        logging.debug("x2: %s, totx: %s", x2, totx)     
      for i in range(nbp+1):
        #posi = nbp -i        # répartition équidistante des points sur la courbe
        posi = nbp*xs[i]/totx # points plus resserrés aux extrémités de la courbe
        angi = dp*posi*(5.0*math.pi/8.0)/nbp
        pt = geompy.MakeRotation(pir, axr, angi)
        points.append(pt)
      curves.append(geompy.MakeInterpol(points))
      point1 = points[-1]
      geomPublish(initLog.debug, curves[-1],"curve2")
#      for i, pt in enumerate(points):
#        name = "point%d"%i
#        geomPublishInFather(initLog.debug,curves[-1], pt, name)
      
      wiretube = geompy.MakeWire(curves)
      geomPublish(initLog.debug, wiretube,"wiretube")
      try:
        edgetube = geompy.MakeEdgeWire(wiretube)
        geomPublish(initLog.debug, edgetube,"edgetube")
      except:
        logging.debug("erreur MakeEdgeWire sur fond de fissure, on fait sans")
        edgetube = None
      
      pts = []
      pts.append(point0)
      dpr = prof*math.cos(5.0*math.pi/8.0)
      pe = geompy.MakeTranslation(pb, dpr*cosaz, dpr*sinaz, 0., "pe")
      for i in range(nbp):
        angi = alfrd -angle +2.0*i*angle/nbp      
        pt = geompy.MakeRotation(pe, axe, angi)
        pts.append(pt)
      pts.append(point1)
      arce = geompy.MakeInterpol(pts)
      geomPublish(initLog.debug, arce,"arce")
      
      facefiss = geompy.MakeFaceWires([arce, wiretube], 0)
      geomPublish(initLog.debug,  facefiss, 'facefissPlace' )
      
      pc = geompy.MakeTranslation(pb, 0.5*prof*cosaz, 0.5*prof*sinaz, 0.)
      centre = geompy.MakeRotation(pc, axe, alfrd)
      geomPublish(initLog.debug,  centre, 'centrefissPlace' )
      
      edges = geompy.ExtractShapes(facefiss, geompy.ShapeType["EDGE"], True)
      edgesTriees, minl, maxl = sortEdges(edges)
      edges = edgesTriees[:-1] # la plus grande correspond à arce, on l'elimine
      wiretube = geompy.MakeWire(edges)
      #wiretube = edgesTriees[-1]
      geomPublish(initLog.debug, wiretube, 'wiretubePlace' )

    # ---------------------------------------------------------

    else: # fissure elliptique, longue ou courte
      if externe:
        raybor = de/2.
        dp = -1.0
      else:
        raybor = de/2. - epais
        dp = +1.0
      prof = dp * profondeur     
      cosaz = math.cos(azimut*math.pi/180.)
      sinaz = math.sin(azimut*math.pi/180.)
      alfrd = alpha*math.pi/180.
      pb = geompy.MakeVertex(raybor*cosaz, raybor*sinaz, -l_tube_p1, "pb")
      pi = geompy.MakeTranslation(pb, prof*cosaz, prof*sinaz, 0., "pi")
      pbv = geompy.MakeTranslation(pb, -profondeur*sinaz, profondeur*cosaz, 0., "pbv")
      ayb  = geompy.MakeVector(pb,pbv, "ayb")
      pb0 = geompy.MakeRotation(pb, axe, alfrd, "pb0")
      ay0 = geompy.MakeRotation(ayb, axe, alfrd, "ay0")
      pi0 = geompy.MakeRotation(pi, axe, alfrd, "pi0")
      az_ = geompy.MakeVector(pi0, pb0, "az_")
      az0 = geompy.MakeTranslationVector(az_, az_, "az0") #normale sortante
      ax0 = geompy.MakeRotation(ay0, az0, -math.pi/2.0, "ax0")
      ax1 = geompy.MakeRotation(ax0, az0, orientation*math.pi/180., "ax1")
      ay1 = geompy.MakeRotation(ay0, az0, orientation*math.pi/180., "ay1")
      originLCS = geompy.MakeMarker(0, 0, 0, 1, 0, 0, 0, 1, 0, "originLCS")
      coo = geompy.PointCoordinates(pb0)
      cox = geompy.VectorCoordinates(ax1)
      coy = geompy.VectorCoordinates(ay1)
      localLCS = geompy.MakeMarker(coo[0], coo[1], coo[2], cox[0], cox[1], cox[2], coy[0], coy[1], coy[2], "localLCS")
      
      pco = geompy.MakeVertex(0, 0, -profondeur, "pco")
      pao = geompy.MakeRotation(pco, OY, 0.6*math.pi, "pao")
      pbo = geompy.MakeRotation(pco, OY, -0.6*math.pi, "pbo")
      pce = geompy.MakeVertex(0, 0, 0.1*profondeur,"pce")
      arcoo = geompy.MakeArc(pao, pco, pbo, "arcoo")
      linoo = geompy.MakeArc(pao, pce, pbo, "linoo")
      scalex = longueur/profondeur
      arco =geompy.MakeScaleAlongAxes(arcoo, O, scalex, 1., 1., "arco")
      lino =geompy.MakeScaleAlongAxes(linoo, O, scalex, 1., 1., "lino")
      arci = geompy.MakePosition(arco, originLCS, localLCS, "arci")
      arce = geompy.MakePosition(lino, originLCS, localLCS, "arce")
      facefiss = geompy.MakeFaceWires([arce, arci], 0)
      geomPublish(initLog.debug,  facefiss, 'facefissPlace' )
      edges = geompy.ExtractShapes(facefiss, geompy.ShapeType["EDGE"], True)
      edgesTriees, minl, maxl = sortEdges(edges)
      edgetube = edgesTriees[-1] # la plus grande correspond à arci
      wiretube = edgetube
      
      pc = geompy.MakeTranslation(pb, 0.5*prof*cosaz, 0.5*prof*sinaz, 0.)
      centre = geompy.MakeRotation(pc, axe, alfrd)
      geomPublish(initLog.debug,  centre, 'centrefissPlace' )

    coordsNoeudsFissure = genereMeshCalculZoneDefaut(facefiss, profondeur/2. ,profondeur)

    return [facefiss, centre, lgInfluence, coordsNoeudsFissure, wiretube, edgetube]

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
  def genereZoneDefaut(self, geometriesSaines, maillagesSains, shapesFissure, shapeFissureParams, maillageFissureParams):
    elementsDefaut = creeZoneDefautDansObjetSain(geometriesSaines, maillagesSains, shapesFissure, shapeFissureParams, maillageFissureParams)
    return elementsDefaut

  # ---------------------------------------------------------------------------
  def genereMaillageFissure(self, geometriesSaines, maillagesSains,
                            shapesFissure, shapeFissureParams,
                            maillageFissureParams, elementsDefaut, step):
    maillageFissure = construitFissureGenerale(maillagesSains,
                                               shapesFissure, shapeFissureParams,
                                               maillageFissureParams, elementsDefaut, step)
    return maillageFissure

  # ---------------------------------------------------------------------------
  def setReferencesMaillageFissure(self):
    self.referencesMaillageFissure = dict(Entity_Node            = 77917,
                                          Entity_Quad_Edge       = 975,
                                          Entity_Quad_Triangle   = 2182,
                                          Entity_Quad_Quadrangle = 6842,
                                          Entity_Quad_Tetra      = 20135,
                                          Entity_Quad_Hexa       = 8994,
                                          Entity_Quad_Penta      = 972,
                                          Entity_Quad_Pyramid    = 1038)

