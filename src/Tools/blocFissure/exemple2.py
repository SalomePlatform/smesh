# -*- coding: iso-8859-1 -*-

import sys
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


geompy = geomBuilder.New(theStudy)

O = geompy.MakeVertex(0, 0, 0)
OX = geompy.MakeVectorDXDYDZ(1, 0, 0)
OY = geompy.MakeVectorDXDYDZ(0, 1, 0)
OZ = geompy.MakeVectorDXDYDZ(0, 0, 1)
Box_1 = geompy.MakeBoxDXDYDZ(200, 200, 200)
Vertex_1 = geompy.MakeVertex(0, 0, 100)
Disk_1 = geompy.MakeDiskPntVecR(Vertex_1, OZ, 50)
Rotation_1 = geompy.MakeRotation(Box_1, OZ, 45*math.pi/180.0)
geompy.TranslateDXDYDZ(Rotation_1, -50, -250, 0)
Cut_1 = geompy.MakeCut(Disk_1, Rotation_1)
geompy.Export(Cut_1, "disk.brep", "BREP")
geompy.addToStudy( O, 'O' )
geompy.addToStudy( OX, 'OX' )
geompy.addToStudy( OY, 'OY' )
geompy.addToStudy( OZ, 'OZ' )
geompy.addToStudy( Box_1, 'Box_1' )
geompy.addToStudy( Vertex_1, 'Vertex_1' )
geompy.addToStudy( Disk_1, 'Disk_1' )
geompy.addToStudy( Rotation_1, 'Rotation_1' )
geompy.addToStudy( Cut_1, 'Cut_1' )

###
### SMESH component
###

import  SMESH, SALOMEDS
from salome.smesh import smeshBuilder

smesh = smeshBuilder.New(theStudy)
from salome.StdMeshers import StdMeshersBuilder
boite = smesh.Mesh(Box_1)
Regular_1D = boite.Segment()
Nb_Segments_1 = Regular_1D.NumberOfSegments(15)
Nb_Segments_1.SetDistrType( 0 )
Quadrangle_2D = boite.Quadrangle(algo=smeshBuilder.QUADRANGLE)
Hexa_3D = boite.Hexahedron(algo=smeshBuilder.Hexa)
isDone = boite.Compute()
smesh.SetName(boite, 'boite')
boite.ExportMED( r'boite.med', 0, SMESH.MED_V2_2, 1 )

## set object names
smesh.SetName(boite.GetMesh(), 'boite')
smesh.SetName(Regular_1D.GetAlgorithm(), 'Regular_1D')
smesh.SetName(Nb_Segments_1, 'Nb. Segments_1')
smesh.SetName(Quadrangle_2D.GetAlgorithm(), 'Quadrangle_2D')
smesh.SetName(Hexa_3D.GetAlgorithm(), 'Hexa_3D')

import os
from blocFissure import gmu
from blocFissure.gmu import initLog
#initLog.setDebug()
initLog.setVerbose()

from blocFissure.gmu.casStandard import casStandard

dicoParams = dict(nomCas            = 'angleCube2',
                  maillageSain      = 'boite.med',
                  brepFaceFissure   = "disk.brep",
                  edgeFissIds       = [4],
                  lgInfluence       = 50,
                  meshBrep          = (5,10),
                  rayonPipe         = 15,
                  lenSegPipe        = 20,
                  nbSegRad          = 5,
                  nbSegCercle       = 12,
                  areteFaceFissure  = 2)

execInstance = casStandard(dicoParams)

if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser(True)
