# -*- coding: utf-8 -*-

###
### This file is generated automatically by SALOME v7.7.1 with dump python functionality
###


import sys, numpy
import salome

salome.salome_init()
theStudy = salome.myStudy

import salome_notebook
notebook = salome_notebook.NoteBook(theStudy)

###
### GEOM component
###

import GEOM
from salome.geom import geomBuilder
import math
import SALOMEDS
import utilityFunctions as uF
from output import message

#ellipse.generate(data_demi_grand_axe, data_centre, data_normale,data_direction, data_demi_petit_axe, data_angle, rayon_entaille,extension, outFile)
#if True:
  #data_demi_grand_axe = 2.
  #data_centre = [0., 0., 0.]
  #data_normale = [1., 0., 0.]
  #data_direction = [0., 1., 0.]
  #data_demi_petit_axe = 1.
  #data_angle=180.
  #rayon_entaille=0.1
  #extension=0.1
  #outFile='/home/I60976/00_PROJETS/2015_INTEGRATION_ZCRACKS/zcracks_salome/test.med'


def generate(data_demi_grand_axe, data_centre, data_normale,
             data_direction, data_demi_petit_axe, data_angle,
             rayon_entaille, extension, outFile):

  Vnormale, Vdirection, Vortho = uF.calcCoordVectors(data_normale, data_direction)
  Vcentre = numpy.array(data_centre)

  geompy = geomBuilder.New(theStudy)

  O = geompy.MakeVertex(0, 0, 0)
  OX = geompy.MakeVectorDXDYDZ(1, 0, 0)
  OY = geompy.MakeVectorDXDYDZ(0, 1, 0)
  OZ = geompy.MakeVectorDXDYDZ(0, 0, 1)
  CENTRE = geompy.MakeVertex(Vcentre[0], Vcentre[1], Vcentre[2])
  NORMALE = geompy.MakeVectorDXDYDZ(Vnormale[0], Vnormale[1], Vnormale[2])
  DIRECTION = geompy.MakeVectorDXDYDZ(Vdirection[0], Vdirection[1], Vdirection[2])
  DIRECTION_op = geompy.MakeVectorDXDYDZ(-Vdirection[0], -Vdirection[1], -Vdirection[2])
  V3 = geompy.MakeVectorDXDYDZ(Vortho[0], Vortho[1], Vortho[2])
  V3_op = geompy.MakeVectorDXDYDZ(-Vortho[0], -Vortho[1], -Vortho[2])
  if data_demi_grand_axe >= data_demi_petit_axe:
    ELLIPSE = geompy.MakeEllipse(CENTRE, NORMALE, data_demi_grand_axe, data_demi_petit_axe, DIRECTION)
  else:
    ELLIPSE = geompy.MakeEllipse(CENTRE, NORMALE, data_demi_petit_axe, data_demi_grand_axe, V3)

  if rayon_entaille<1.e-12:
    FELLIPSE = geompy.MakeFaceWires([ELLIPSE], 1)
    dim=2
  else:
    dim=3
    VP1=Vcentre+Vdirection*(data_demi_grand_axe-rayon_entaille)+Vnormale*rayon_entaille
    VP2=Vcentre+Vdirection*(data_demi_grand_axe)
    VP3=Vcentre+Vdirection*(data_demi_grand_axe-rayon_entaille)-Vnormale*rayon_entaille
    PE1=geompy.MakeVertex(VP1[0], VP1[1], VP1[2])
    PE2=geompy.MakeVertex(VP2[0], VP2[1], VP2[2])
    PE3=geompy.MakeVertex(VP3[0], VP3[1], VP3[2])
    ARC = geompy.MakeArc(PE1, PE2, PE3)
    TUYAU = geompy.MakePipe(ARC, ELLIPSE)
    subShapesList=geompy.GetFreeBoundary(TUYAU)[1]
    entailleFace1 = geompy.MakeFaceWires([subShapesList[0]], 1)
    entailleFace2 = geompy.MakeFaceWires([subShapesList[1]], 1)
    FELLIPSE = geompy.MakeShell([TUYAU, entailleFace1, entailleFace2])

  edgesIDs = geompy.SubShapeAllIDs(FELLIPSE, geompy.ShapeType["EDGE"])
  edges = geompy.CreateGroup(FELLIPSE, geompy.ShapeType["EDGE"])
  geompy.UnionIDs(edges, edgesIDs)
  geompy.addToStudy( FELLIPSE, 'FELLIPSE' )
  geompy.addToStudyInFather( FELLIPSE , edges, 'edges' )

  hauteur=numpy.max([data_demi_grand_axe,data_demi_petit_axe])*1.1

  eps=1.E-05
  bool_boite=True
  extrusion=numpy.max([1.,rayon_entaille])*1.1
  if ( (data_angle>(eps)) and (data_angle<(180.-eps)) ):
    rayon2=hauteur*numpy.tan(data_angle*numpy.pi/180./2.)

    B1=geompy.MakeTranslationVectorDistance(CENTRE,DIRECTION,hauteur)
    B2=geompy.MakeTranslationVectorDistance(B1,V3,rayon2)
    geompy.TranslateVectorDistance(B1,V3_op,rayon2, False)
    LB01 = geompy.MakeLineTwoPnt(CENTRE, B1)
    LB02 = geompy.MakeLineTwoPnt(CENTRE, B2)
    LB12 = geompy.MakeLineTwoPnt(B1, B2)
    plan_BOITE = geompy.MakeFaceWires([LB01, LB02, LB12], True)

    BOITE = geompy.MakePrismVecH2Ways(plan_BOITE, NORMALE, extrusion)

    FACE_FISSURE = geompy.MakeCommonList([FELLIPSE, BOITE])

  elif ( (data_angle>=(180.-eps)) and (data_angle<=(180.+eps)) ):
    VP1=Vcentre+Vortho*hauteur
    VP2=Vcentre-Vortho*hauteur
    VP3=Vcentre-Vortho*hauteur+Vdirection*hauteur
    VP4=Vcentre+Vortho*hauteur+Vdirection*hauteur

    Sommet_1 = geompy.MakeVertex(VP1[0], VP1[1], VP1[2])
    Sommet_2 = geompy.MakeVertex(VP2[0], VP2[1], VP2[2])
    Sommet_3 = geompy.MakeVertex(VP3[0], VP3[1], VP3[2])
    Sommet_4 = geompy.MakeVertex(VP4[0], VP4[1], VP4[2])

    Ligne_1 = geompy.MakeLineTwoPnt(Sommet_1, Sommet_2)
    Ligne_2 = geompy.MakeLineTwoPnt(Sommet_2, Sommet_3)
    Ligne_3 = geompy.MakeLineTwoPnt(Sommet_3, Sommet_4)
    Ligne_4 = geompy.MakeLineTwoPnt(Sommet_4, Sommet_1)

    Contour_1 = geompy.MakeWire([Ligne_1, Ligne_2, Ligne_3, Ligne_4], 1e-07)
    Face_1 = geompy.MakeFaceWires([Contour_1], 1)
    BOITE = geompy.MakePrismVecH2Ways(Face_1, NORMALE, extrusion)
    FACE_FISSURE = geompy.MakeCommonList([FELLIPSE, BOITE])

  elif ( (data_angle>(180.+eps)) and (data_angle<(360.-eps)) ):
    rayon2=hauteur*numpy.tan((360.-data_angle)*numpy.pi/180./2.)

    B1=geompy.MakeTranslationVectorDistance(CENTRE,DIRECTION_op,hauteur)
    B2=geompy.MakeTranslationVectorDistance(B1,V3,rayon2)
    geompy.TranslateVectorDistance(B1,V3_op,rayon2, False)
    LB01 = geompy.MakeLineTwoPnt(CENTRE, B1)
    LB02 = geompy.MakeLineTwoPnt(CENTRE, B2)
    LB12 = geompy.MakeLineTwoPnt(B1, B2)
    plan_BOITE = geompy.MakeFaceWires([LB01, LB02, LB12], True)
    extrusion=numpy.max([1.,rayon_entaille])*1.1
    BOITE = geompy.MakePrismVecH2Ways(plan_BOITE, NORMALE, extrusion)

    FACE_FISSURE = geompy.MakeCutList(FELLIPSE, [BOITE])

  elif ( (data_angle<=(eps)) or (data_angle>=(360.-eps)) ):
    bool_boite=False
    FACE_FISSURE = FELLIPSE

  else:
    message('E','Angle non prevu')

  if bool_boite:
    #geompy.addToStudy( BOITE, 'BOITE' )
    newEdgesIDs = geompy.SubShapeAllIDs(FACE_FISSURE, geompy.ShapeType["EDGE"])
    newEdges = geompy.CreateGroup(FACE_FISSURE, geompy.ShapeType["EDGE"])
    geompy.UnionIDs(newEdges, newEdgesIDs)

    [oldEdges] = geompy.RestoreGivenSubShapes(FACE_FISSURE, [FELLIPSE, edges], GEOM.FSM_GetInPlace, True, False)

    toExtrude = geompy.CutListOfGroups([newEdges], [oldEdges])

    if extension>1.e-12:
      extrusion = geompy.MakePrismVecH(toExtrude, DIRECTION_op, extension)
      try:
        FACE_FISSURE = geompy.MakeFuseList([FACE_FISSURE, extrusion], False, True)
      except:
        FACE_FISSURE = geompy.MakeFuseList([FACE_FISSURE, extrusion], False, False)

  #geompy.addToStudy( FACE_FISSURE, 'FACE_FISSURE' )

  #geompy.addToStudy( FACE_FISSURE, 'FACE_FISSURE' )

  import  SMESH, SALOMEDS
  from salome.smesh import smeshBuilder
  smesh = smeshBuilder.New(theStudy)

  A=numpy.pi/(30.)
  minAxes=numpy.min([data_demi_grand_axe,data_demi_petit_axe])
  maxAxes=numpy.max([data_demi_grand_axe,data_demi_petit_axe])
  R=minAxes**2/maxAxes

  if rayon_entaille>1.e-12:
    R=numpy.min([R,rayon_entaille])
    A=numpy.pi/(15.)

  chordal, minSize = uF.calcElemSize(A, R)
  maxSize=maxAxes/5.

  Maillage=uF.meshCrack(FACE_FISSURE, minSize, maxSize, chordal, dim)

  try:
    Maillage.ExportMED( outFile, 0, SMESH.MED_V2_2, 1, None ,1)
    smesh.SetName(Maillage.GetMesh(), 'MAILLAGE_FISSURE')
  except:
    print 'ExportToMEDX() failed. Invalid file name?'


  ## Set names of Mesh objects


  if salome.sg.hasDesktop():
    salome.sg.updateObjBrowser(1)
