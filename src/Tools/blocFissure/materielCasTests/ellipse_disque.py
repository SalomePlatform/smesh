# -*- coding: utf-8 -*-

import sys
import salome

salome.salome_init()
theStudy = salome.myStudy

import salome_notebook
notebook = salome_notebook.NoteBook(theStudy)

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
Vertex_1 = geompy.MakeVertex(100, 0, 0)
Disk_1 = geompy.MakeDiskPntVecR(Vertex_1, OY, 12.5)
Scale_1 = geompy.MakeScaleAlongAxes(Disk_1, Vertex_1, 1, 1, 4.1)
Vertex_2 = geompy.MakeVertex(98, -2, -2)
Vertex_3 = geompy.MakeVertex(120, 2, 60)
Box_1 = geompy.MakeBoxTwoPnt(Vertex_3, Vertex_2)
Ellipse_disque = geompy.MakeCommon(Box_1, Scale_1)
geompy.ExportBREP(Ellipse_disque, os.path.join(gmu.pathBloc, "materielCasTests/ellipse_disque.brep"))
geompy.addToStudy( O, 'O' )
geompy.addToStudy( OX, 'OX' )
geompy.addToStudy( OY, 'OY' )
geompy.addToStudy( OZ, 'OZ' )
geompy.addToStudy( Vertex_1, 'Vertex_1' )
geompy.addToStudy( Disk_1, 'Disk_1' )
geompy.addToStudy( Vertex_2, 'Vertex_2' )
geompy.addToStudy( Scale_1, 'Scale_1' )
geompy.addToStudy( Vertex_3, 'Vertex_3' )
geompy.addToStudy( Box_1, 'Box_1' )
geompy.addToStudy( Ellipse_disque, 'Ellipse_disque' )


if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser(True)
