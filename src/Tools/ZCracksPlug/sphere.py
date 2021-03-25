# -*- coding: utf-8 -*-
# Copyright (C) 2016-2021  EDF R&D
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

###
### This file is generated automatically by SALOME v7.7.1 with dump python functionality
###

import sys, numpy
import salome

salome.salome_init()

import salome_notebook
notebook = salome_notebook.NoteBook()

###
### GEOM component
###

import GEOM
from salome.geom import geomBuilder
import math
import SALOMEDS
from . import utilityFunctions as uF
from .output import message

#import GEOM_Gen.ild

def generate(data_rayon,data_centre,outFile):
  #data_rayon = 0.1
  #data_centre = [1., 1., 01.]

  geompy = geomBuilder.New()

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

  smesh = smeshBuilder.New()

  A=numpy.pi/(20.)
  chordal, minSize = uF.calcElemSize(A, data_rayon)
  maxSize=data_rayon/3.

  Maillage=uF.meshCrack(FACE_FISSURE, minSize, maxSize, chordal, dim=3)

  try:
    Maillage.ExportMED(outFile)
    smesh.SetName(Maillage.GetMesh(), 'MAILLAGE_FISSURE')
  except:
    print('ExportMED() failed. Invalid file name?')


  ## Set names of Mesh objects


  if salome.sg.hasDesktop():
    salome.sg.updateObjBrowser()
