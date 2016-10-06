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

O = geompy.MakeVertex(0, 0, 0)
OX = geompy.MakeVectorDXDYDZ(1, 0, 0)
OY = geompy.MakeVectorDXDYDZ(0, 1, 0)
OZ = geompy.MakeVectorDXDYDZ(0, 0, 1)
Vertex_1 = geompy.MakeVertex(400, 0, 400)
Vertex_2 = geompy.MakeVertex(400, 0, 500)
Vector_1 = geompy.MakeVector(Vertex_1, Vertex_2)
Disk_1 = geompy.MakeDiskPntVecR(Vertex_1, Vector_1, 50)
Rotation_1 = geompy.MakeRotation(Disk_1, Vector_1, 180*math.pi/180.0)
Scale_1 = geompy.MakeScaleAlongAxes(Rotation_1, Vertex_1, 1, 1.5, 1)
Vertex_3 = geompy.MakeVertex(420, -400, 300)
Vertex_4 = geompy.MakeVertex(500, 400, 500)
Box_1 = geompy.MakeBoxTwoPnt(Vertex_4, Vertex_3)
ellipse1 = geompy.MakeCut(Scale_1, Box_1)
[fondFiss] = geompy.SubShapes(ellipse1, [4])
geompy.addToStudy( O, 'O' )
geompy.addToStudy( OX, 'OX' )
geompy.addToStudy( OY, 'OY' )
geompy.addToStudy( OZ, 'OZ' )
geompy.addToStudy( Vertex_1, 'Vertex_1' )
geompy.addToStudy( Vertex_2, 'Vertex_2' )
geompy.addToStudy( Vector_1, 'Vector_1' )
geompy.addToStudy( Disk_1, 'Disk_1' )
geompy.addToStudy( Rotation_1, 'Rotation_1' )
geompy.addToStudy( Scale_1, 'Scale_1' )
geompy.addToStudy( Vertex_3, 'Vertex_3' )
geompy.addToStudy( Vertex_4, 'Vertex_4' )
geompy.addToStudy( Box_1, 'Box_1' )
geompy.addToStudy( ellipse1, 'ellipse1' )
geompy.addToStudyInFather( ellipse1, fondFiss, 'fondFiss' )
geompy.ExportBREP(ellipse1, os.path.join(gmu.pathBloc, "materielCasTests/ellipse1.brep"))


if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser(True)
