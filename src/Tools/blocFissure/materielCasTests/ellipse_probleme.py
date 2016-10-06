# -*- coding: utf-8 -*-

import sys
import salome

salome.salome_init()
theStudy = salome.myStudy

import salome_notebook
notebook = salome_notebook.notebook

import os
from blocFissure import gmu

###
### GEOM component
###


import GEOM
from salome.geom import geomBuilder
import math
import SALOMEDS

geompy = geomBuilder.New(theStudy)

Disk_1 = geompy.MakeDiskR(100, 1)
O = geompy.MakeVertex(0, 0, 0)
OX = geompy.MakeVectorDXDYDZ(1, 0, 0)
OY = geompy.MakeVectorDXDYDZ(0, 1, 0)
OZ = geompy.MakeVectorDXDYDZ(0, 0, 1)
Scale_1 = geompy.MakeScaleAlongAxes(Disk_1, None, 0.4, 0.75, 1)
[Vertex_1] = geompy.ExtractShapes(Scale_1, geompy.ShapeType["VERTEX"], True)
Rotation_1 = geompy.MakeRotation(Scale_1, OY, 190*math.pi/180.0)
Rotation_2 = geompy.MakeRotation(Rotation_1, OZ, 10*math.pi/180.0)
Vertex_2 = geompy.MakeVertex(20, -100, -50)
Vertex_4 = geompy.MakeVertex(100, 100, 50)
Box_1 = geompy.MakeBoxTwoPnt(Vertex_2, Vertex_4)
Cut_1 = geompy.MakeCut(Rotation_1, Box_1)
ellipse1 = geompy.MakeTranslation(Cut_1, 400, 0, 400)
geompy.addToStudy( Disk_1, 'Disk_1' )
geompy.addToStudy( O, 'O' )
geompy.addToStudy( OX, 'OX' )
geompy.addToStudy( OY, 'OY' )
geompy.addToStudy( OZ, 'OZ' )
geompy.addToStudy( Scale_1, 'Scale_1' )
geompy.addToStudyInFather( Scale_1, Vertex_1, 'Vertex_1' )
geompy.addToStudy( Rotation_1, 'Rotation_1' )
geompy.addToStudy( Rotation_2, 'Rotation_2' )
geompy.addToStudy( Vertex_2, 'Vertex_2' )
geompy.addToStudy( Box_1, 'Box_1' )
geompy.addToStudy( Vertex_4, 'Vertex_4' )
geompy.addToStudy( Cut_1, 'Cut_1' )
geompy.addToStudy( ellipse1, 'ellipse1_pb' )
geompy.ExportBREP(ellipse1, os.path.join(gmu.pathBloc, "materielCasTests/ellipse1_pb.brep"))


if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser(True)
