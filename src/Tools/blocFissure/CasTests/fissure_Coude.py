# -*- coding: utf-8 -*-
# Copyright (C) 2014-2021  EDF R&D
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#
"""Fissure dans un coude"""

import logging
import os
import math

import GEOM
import SMESH

from blocFissure.gmu.geomsmesh import geompy
from blocFissure.gmu.geomsmesh import smesh
from blocFissure.gmu.geomsmesh import geomPublish
from blocFissure.gmu.geomsmesh import geomPublishInFather

from blocFissure.gmu import initLog
from blocFissure.gmu.putName import putName
from blocFissure.gmu.fissureGenerique import fissureGenerique
from blocFissure.gmu.triedreBase import triedreBase
from blocFissure.gmu.genereMeshCalculZoneDefaut import genereMeshCalculZoneDefaut
from blocFissure.gmu.creeZoneDefautDansObjetSain import creeZoneDefautDansObjetSain
from blocFissure.gmu.insereFissureLongue import insereFissureLongue

O, OX, OY, OZ = triedreBase()

class fissure_Coude(fissureGenerique):
  """problème de fissure du Coude : version de base

  maillage hexa
  """

  nomProbleme = "fissure_Coude"
  geomParams = dict()
  meshParams = dict()
  shapeFissureParams = dict()
  maillageFissureParams = dict()
  referencesMaillageFissure = dict()
  circonferentielle = False
  longitudinale = False

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
    """a écrire"""
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
    geomPublish(initLog.debug, Plane_1, "Plane_1" )
    geomPublish(initLog.debug, Extrusion_1, "Extrusion_1" )
    geomPublish(initLog.debug, Revolution_1, "Revolution_1" )
    geomPublish(initLog.debug, Extrusion_2, "Extrusion_2" )

    P1 = O
    geomPublish(initLog.always, P1, "P1", self.numeroCas )
    op2 = geompy.MakeVertex(0, 0, -l_tube_p1)
    P2 = geompy.MakeRotation(op2, axe, angleCoude*math.pi/180.0)
    P2 = geompy.MakeTranslationVectorDistance(P2, Rotation_2, -l_tube_p2)
    geomPublish(initLog.always, P2, "P2", self.numeroCas )

    # --- tube coude sain

    geometrieSaine = geompy.MakePartition([Extrusion_1, Revolution_1, Extrusion_2, P1, P2], [Plane_1], [], [], geompy.ShapeType["SOLID"], 0, [], 1)
    geomPublish(initLog.always, geometrieSaine, self.nomCas, self.numeroCas )
    [P1, P2] = geompy.RestoreGivenSubShapes(geometrieSaine, [P1, P2], GEOM.FSM_GetInPlaceByHistory, False, True)

    [ep, circ_g, circ_d, long_p2, long_coude, long_p1] = geompy.Propagate(geometrieSaine)
    geomPublishInFather(initLog.always, geometrieSaine, long_p1, 'long_p1' )
    geomPublishInFather(initLog.always, geometrieSaine, ep, 'ep' )
    geomPublishInFather(initLog.always, geometrieSaine, long_coude, 'long_coude' )
    geomPublishInFather(initLog.always, geometrieSaine, circ_g, 'circ_g' )
    geomPublishInFather(initLog.always, geometrieSaine, circ_d, 'circ_d' )
    geomPublishInFather(initLog.always, geometrieSaine, long_p2, 'long_p2' )

    # --- face extremite tube (EXTUBE)

    facesIds = geompy.GetShapesOnPlaneIDs(geometrieSaine, geompy.ShapeType["FACE"], OZ, GEOM.ST_ON)
    EXTUBE = geompy.CreateGroup(geometrieSaine, geompy.ShapeType["FACE"])
    geompy.UnionIDs(EXTUBE, facesIds)
    geomPublishInFather(initLog.always, geometrieSaine, EXTUBE, 'EXTUBE' )

    # --- edge bord extremite tube (BORDTU)

    edge1Ids = geompy.GetShapesOnPlaneIDs(geometrieSaine, geompy.ShapeType["EDGE"], OZ, GEOM.ST_ON)
    edge2Ids = geompy.GetShapesOnCylinderIDs(geometrieSaine, geompy.ShapeType["EDGE"], OZ, de/2., GEOM.ST_ON)
    edgesIds = list()
    for edge in edge1Ids:
      if edge in edge2Ids:
        edgesIds.append(edge)
    BORDTU = geompy.CreateGroup(geometrieSaine, geompy.ShapeType["EDGE"])
    geompy.UnionIDs(BORDTU, edgesIds)
    geomPublishInFather(initLog.always, geometrieSaine, BORDTU, 'BORDTU' )

    # --- face origine tube (CLGV)

    pp2 = geompy.MakeTranslationVectorDistance(P2, Rotation_2, 10)
    vec2 = geompy.MakeVector(P2, pp2)
    #geomPublish(initLog.debug, vec2, 'vec2', self.numeroCas)
    facesIds = geompy.GetShapesOnPlaneIDs(geometrieSaine, geompy.ShapeType["FACE"], vec2, GEOM.ST_ON)
    CLGV = geompy.CreateGroup(geometrieSaine, geompy.ShapeType["FACE"])
    geompy.UnionIDs(CLGV, facesIds)
    geomPublishInFather(initLog.always, geometrieSaine, CLGV, 'CLGV' )

    # --- peau tube interieur (PEAUINT)

    extru1 = geompy.MakePrismVecH(Disk_2, OZ, l_tube_p1)
    revol1 = geompy.MakeRevolution(Disk_2, axe, angleCoude*math.pi/180.0)
    rot1 = geompy.MakeRotation(Disk_2, axe, angleCoude*math.pi/180.0)
    extru2 = geompy.MakePrismVecH(rot1, Rotation_2, -l_tube_p2)
    interne = geompy.MakeFuse(extru1, revol1)
    interne = geompy.MakeFuse(extru2, interne)
    geomPublish(initLog.debug, interne, 'interne', self.numeroCas)
    facesIds = geompy.GetShapesOnShapeIDs(interne, geometrieSaine, geompy.ShapeType["FACE"], GEOM.ST_ONIN)
    PEAUINT = geompy.CreateGroup(geometrieSaine, geompy.ShapeType["FACE"])
    geompy.UnionIDs(PEAUINT, facesIds)
    geomPublishInFather(initLog.always, geometrieSaine, PEAUINT, 'PEAUINT' )

    # --- peau tube exterieur (PEAUEXT)

    Disk_3 = geompy.MakeDiskPntVecR(centre, OZ, de/2. +epais)
    couronne1 = geompy.MakeCut(Disk_3, Disk_1)
    extru1 = geompy.MakePrismVecH(couronne1, OZ, l_tube_p1)
    revol1 = geompy.MakeRevolution(couronne1, axe, angleCoude*math.pi/180.0)
    rot1 = geompy.MakeRotation(couronne1, axe, angleCoude*math.pi/180.0)
    extru2 = geompy.MakePrismVecH(rot1, Rotation_2, -l_tube_p2)
    externe = geompy.MakeFuse(extru1, revol1)
    externe = geompy.MakeFuse(extru2, externe)
    geomPublish(initLog.debug, externe, 'externe', self.numeroCas)
    facesIds = geompy.GetShapesOnShapeIDs(externe, geometrieSaine, geompy.ShapeType["FACE"], GEOM.ST_ON)
    PEAUEXT = geompy.CreateGroup(geometrieSaine, geompy.ShapeType["FACE"])
    geompy.UnionIDs(PEAUEXT, facesIds)
    geomPublishInFather(initLog.always, geometrieSaine, PEAUEXT, 'PEAUEXT' )

    # --- solide sain

    volIds = geompy.SubShapeAllIDs(geometrieSaine, geompy.ShapeType["SOLID"])
    COUDE = geompy.CreateGroup(geometrieSaine, geompy.ShapeType["SOLID"])
    geompy.UnionIDs(COUDE, volIds)
    geomPublishInFather(initLog.always, geometrieSaine, COUDE, 'COUDSAIN' )

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
    putName(maillageSain, "maillageSain", i_pref=self.numeroCas)

    algo3d = maillageSain.Hexahedron()
    algo2d = maillageSain.Quadrangle()

    algo1d_long_p1 = maillageSain.Segment(geom=long_p1)
    hypo1d_long_p1 = algo1d_long_p1.NumberOfSegments(n_long_p1)
    putName(hypo1d_long_p1, "n_long_p1={}".format(n_long_p1), i_pref=self.numeroCas)

    algo1d_ep = maillageSain.Segment(geom=ep)
    hypo1d_ep = algo1d_ep.NumberOfSegments(n_ep)
    putName(hypo1d_ep, "n_ep={}".format(n_ep), i_pref=self.numeroCas)

    algo1d_long_coude = maillageSain.Segment(geom=long_coude)
    hypo1d_long_coude = algo1d_long_coude.NumberOfSegments(n_long_coude)
    putName(hypo1d_long_coude, "n_long_coude={}".format(n_long_coude), i_pref=self.numeroCas)

    algo1d_circ_g = maillageSain.Segment(geom=circ_g)
    hypo1d_circ_g = algo1d_circ_g.NumberOfSegments(n_circ_g)
    putName(hypo1d_circ_g, "n_circ_g={}".format(n_circ_g), i_pref=self.numeroCas)

    algo1d_circ_d = maillageSain.Segment(geom=circ_d)
    hypo1d_circ_d = algo1d_circ_d.NumberOfSegments(n_circ_d)
    putName(hypo1d_circ_d, "n_circ_d={}".format(n_circ_d), i_pref=self.numeroCas)

    algo1d_long_p2 = maillageSain.Segment(geom=long_p2)
    hypo1d_long_p2 = algo1d_long_p2.NumberOfSegments(n_long_p2)
    putName(hypo1d_long_p2, "n_long_p2={}".format(n_long_p2), i_pref=self.numeroCas)

    _ = maillageSain.GroupOnGeom(P1,'P1',SMESH.NODE)
    _ = maillageSain.GroupOnGeom(P2,'P2',SMESH.NODE)
    _ = maillageSain.GroupOnGeom(EXTUBE,'EXTUBE',SMESH.FACE)
    _ = maillageSain.GroupOnGeom(BORDTU,'BORDTU',SMESH.EDGE)
    _ = maillageSain.GroupOnGeom(CLGV,'CLGV',SMESH.FACE)
    _ = maillageSain.GroupOnGeom(PEAUINT,'PEAUINT',SMESH.FACE)
    _ = maillageSain.GroupOnGeom(PEAUEXT,'PEAUEXT',SMESH.FACE)
    _ = maillageSain.GroupOnGeom(COUDE,'COUDSAIN',SMESH.VOLUME)

    is_done = maillageSain.Compute()
    text = "maillageSain.Compute"
    if is_done:
      logging.info(text+" OK")
    else:
      text = "Erreur au calcul du maillage.\n" + text
      logging.info(text)
      raise Exception(text)

    return [maillageSain, True] # True : maillage hexa

  # ---------------------------------------------------------------------------
  def setParamShapeFissure(self):
    """
    paramètres de la fissure pour le tuyau coude
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
  def genereShapeFissure( self, geometriesSaines, geomParams, shapeFissureParams, \
                                mailleur="MeshGems"):
    logging.info("genereShapeFissure %s", self.nomCas)
    logging.info("shapeFissureParams %s", shapeFissureParams)

    #angleCoude = geomParams['angleCoude']
    r_cintr    = geomParams['r_cintr']
    l_tube_p1  = geomParams['l_tube_p1']
    #l_tube_p2  = geomParams['l_tube_p2']
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

    self.fissureLongue = bool(longueur > 2*profondeur)

    if self.fissureLongue and (abs(orientation) < 45.) :
      self.longitudinale = True
    elif self.fissureLongue:
      self.circonferentielle = True
    #logging.info("longitudinale %s", self.longitudinale)
    #logging.info("circonferentielle %s", self.circonferentielle)

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
      geomPublish(initLog.debug, face0, 'facefissOrig' )
      face1 = geompy.MakeRotation(face0, OZ, azimut*math.pi/180.)
      face2 = geompy.MakeTranslation(face1, 0, 0, -l_tube_p1)
      facefiss = geompy.MakeRotation(face2, axe, alpha*math.pi/180.)
      geomPublish(initLog.always, facefiss, "facefissPlace", self.numeroCas )
      centre = geompy.MakeRotation(pb, OZ, azimut*math.pi/180.)
      centre = geompy.MakeTranslation(centre, 0, 0, -l_tube_p1)
      centre = geompy.MakeRotation(centre, axe, alpha*math.pi/180.)
      geomPublish(initLog.always, centre, "centrefissPlace", self.numeroCas )
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
      geomPublish(initLog.always,wiretube, "wiretubePlace", self.numeroCas )
      facetubel = geompy.MakeRotation(facetubel, OZ, azimut*math.pi/180.)
      facetubel = geompy.MakeTranslation(facetubel, 0, 0, -l_tube_p1)
      facetubel = geompy.MakeRotation(facetubel, axe, alpha*math.pi/180.)
      geomPublish(initLog.debug,facetubel, 'facetubeGauche' )
      facetuber = geompy.MakeRotation(facetuber, OZ, azimut*math.pi/180.)
      facetuber = geompy.MakeTranslation(facetuber, 0, 0, -l_tube_p1)
      facetuber = geompy.MakeRotation(facetuber, axe, alpha*math.pi/180.)
      geomPublish(initLog.debug,facetuber, 'facetubeDroit' )
      planfiss = geompy.MakeRotation(plan0, OZ, azimut*math.pi/180.)
      planfiss = geompy.MakeTranslation(planfiss, 0, 0, -l_tube_p1)
      planfiss = geompy.MakeRotation(planfiss, axe, alpha*math.pi/180.)
      geomPublish(initLog.always,planfiss, "planfissPlace", self.numeroCas )
      pipefissl = geompy.MakePipe(facetubel, arcl)
      pipefissi = geompy.MakePipe(facetubel, arci)
      pipefissr = geompy.MakePipe(facetuber, arcr)
      pipefiss = geompy.MakePartition([pipefissl, pipefissi, pipefissr], [planfiss, wiretube], [], [], geompy.ShapeType["SOLID"], 0, [], 0)
      geomPublish(initLog.always,pipefiss, "pipefissPlace", self.numeroCas )


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
      geomPublish(initLog.debug, arcl, 'arcl' )
      geomPublish(initLog.debug, arcr, 'arcr' )
      geomPublish(initLog.debug, arci, 'arci' )
      geomPublish(initLog.debug, arce, 'arce' )
      wire0 = geompy.MakeWire([arcr, arci, arcl])
      cercle0 = geompy.MakeCircle(O, OZ, profondeur/4.0)
      #cercle0 = geompy.MakeRotation(cercle0, OZ, math.pi/2.0)
      cercle0 = geompy.MakeTranslationTwoPoints(cercle0, O, pi)
      geomPublish(initLog.debug, cercle0, 'cercle0' )
      facetube0 = geompy.MakeFaceWires([cercle0], 1)
      facetubel = geompy.MakeRotation(facetube0, OY, -angle)
      facetuber = geompy.MakeRotation(facetube0, OY, angle)
      geomPublish(initLog.debug,facetubel , 'facetubel' )
      geomPublish(initLog.debug, facetuber, 'facetuber' )
      face0 = geompy.MakeFaceWires([arcl,arci, arce, arcr], 1)
      plan0 = geompy.MakePlane(O, OY, 10000)
      geomPublish(initLog.debug, face0, 'facefissOrig' )
      facefiss = geompy.MakeRotation(face0, OY, alpha*math.pi/180.)
      geomPublish(initLog.always, facefiss, "facefissPlace", self.numeroCas )
      centre = geompy.MakeRotation(pb, OY, alpha*math.pi/180.)
      geomPublish(initLog.always, centre, "centrefissPlace", self.numeroCas )
      arcr = geompy.MakeRotation(arcr, OY, alpha*math.pi/180.)
      arci = geompy.MakeRotation(arci, OY, alpha*math.pi/180.)
      arcl = geompy.MakeRotation(arcl, OY, alpha*math.pi/180.)
      wiretube = geompy.MakeRotation(wire0, OY, alpha*math.pi/180.)
      geomPublish(initLog.always,wiretube, "wiretubePlace", self.numeroCas )
      facetubel = geompy.MakeRotation(facetubel, OY, alpha*math.pi/180.)
      geomPublish(initLog.debug,facetubel, 'facetubeGauche' )
      facetuber = geompy.MakeRotation(facetuber, OY, alpha*math.pi/180.)
      geomPublish(initLog.debug,facetubel, 'facetubeDroit' )
      planfiss = geompy.MakeRotation(plan0, OY, alpha*math.pi/180.)
      geomPublish(initLog.always,planfiss, "planfissPlace", self.numeroCas )
      pipefissl = geompy.MakePipe(facetubel, arcl)
      pipefissi = geompy.MakePipe(facetubel, arci)
      pipefissr = geompy.MakePipe(facetuber, arcr)
      pipefiss = geompy.MakePartition([pipefissl, pipefissi, pipefissr], [planfiss, wiretube], [], [], geompy.ShapeType["SOLID"], 0, [], 0)
      geomPublish(initLog.always,pipefiss, "pipefissPlace", self.numeroCas )
    else:
      pass

    mailleur = self.mailleur2d3d()
    coordsNoeudsFissure = genereMeshCalculZoneDefaut(facefiss, 5 ,10, mailleur, self.numeroCas)

    return [facefiss, centre, lgInfluence, coordsNoeudsFissure, wiretube, facetubel, facetuber, planfiss, pipefiss]

  # ---------------------------------------------------------------------------
  def setParamMaillageFissure(self):
    self.maillageFissureParams = dict(nomRep        = os.curdir,
                                      nomFicSain    = self.nomProbleme,
                                      nomFicFissure = self.nomProbleme + "_fissure",
                                      nbsegExt      = 5,
                                      nbsegGen      = 25,
                                      nbsegRad      = 5,
                                      scaleRad      = 4,
                                      nbsegCercle   = 6,
                                      nbsegFis      = 20,
                                      lensegEllipso = 1.0)

  # ---------------------------------------------------------------------------
  def genereZoneDefaut(self, geometriesSaines, maillagesSains, shapesFissure, shapeFissureParams, maillageFissureParams):
    elementsDefaut = creeZoneDefautDansObjetSain(geometriesSaines, maillagesSains, shapesFissure, shapeFissureParams, maillageFissureParams, \
                                                 self.numeroCas)
    return elementsDefaut

  # ---------------------------------------------------------------------------
  def genereMaillageFissure(self, geometriesSaines, maillagesSains, \
                                  shapesFissure, shapeFissureParams, \
                                  maillageFissureParams, elementsDefaut, step, \
                                  mailleur="MeshGems"):

    mailleur = self.mailleur2d3d()
    maillageFissure = insereFissureLongue(shapesFissure, shapeFissureParams, \
                                          maillageFissureParams, elementsDefaut, \
                                          mailleur, self.numeroCas)
    return maillageFissure

  # ---------------------------------------------------------------------------
  def setReferencesMaillageFissure(self):
    self.referencesMaillageFissure = dict( \
                                          Entity_Quad_Quadrangle = 6710, \
                                          Entity_Quad_Hexa = 8656, \
                                          Entity_Node = 76807, \
                                          Entity_Quad_Edge = 1006, \
                                          Entity_Quad_Triangle = 2342, \
                                          Entity_Quad_Tetra = 20392, \
                                          Entity_Quad_Pyramid = 1232, \
                                          Entity_Quad_Penta = 1176 \
                                         )
