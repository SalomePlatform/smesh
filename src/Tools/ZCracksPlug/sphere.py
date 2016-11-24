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

#import GEOM_Gen.ild

def generate(data_rayon,data_centre,outFile):
  #data_rayon = 0.1
  #data_centre = [1., 1., 01.]

  geompy = geomBuilder.New(theStudy)

  O = geompy.MakeVertex(0, 0, 0)
  OX = geompy.MakeVectorDXDYDZ(1, 0, 0)
  OY = geompy.MakeVectorDXDYDZ(0, 1, 0)
  OZ = geompy.MakeVectorDXDYDZ(0, 0, 1)

  SPHERE = geompy.MakeSphereR(data_rayon)
  geompy.TranslateDXDYDZ(SPHERE, data_centre[0], data_centre[1], data_centre[2])
  [FACE_FISSURE] = geompy.ExtractShapes(SPHERE, geompy.ShapeType["FACE"], True)

  #
  # SMESH component
  #

  import  SMESH, SALOMEDS
  from salome.smesh import smeshBuilder

  smesh = smeshBuilder.New(theStudy)

  A=numpy.pi/(20.)
  chordal, minSize = uF.calcElemSize(A, data_rayon)
  maxSize=data_rayon/3.

  Maillage=uF.meshCrack(FACE_FISSURE, minSize, maxSize, chordal, dim=3)

  try:
    Maillage.ExportMED( outFile, 0, SMESH.MED_V2_2, 1, None ,1)
    smesh.SetName(Maillage.GetMesh(), 'MAILLAGE_FISSURE')
  except:
    print 'ExportToMEDX() failed. Invalid file name?'


  ## Set names of Mesh objects


  if salome.sg.hasDesktop():
    salome.sg.updateObjBrowser(1)
