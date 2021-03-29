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

"""Cas-test de blocFissure pour un cube"""

import logging
import os

from blocFissure import gmu

import salome
import GEOM
from salome.geom import geomBuilder
import SMESH
from salome.smesh import smeshBuilder

#=============== Options ====================
# 1. NOM_OBJET = nom de l'objet
NOM_OBJET = "CubeAngle"
#============================================

salome.salome_init()

###
### GEOM component
###

geompy = geomBuilder.New()

OZ = geompy.MakeVectorDXDYDZ(0, 0, 1)
Box_1 = geompy.MakeBoxDXDYDZ(200, 200, 200)
Vertex_1 = geompy.MakeVertex(0, 0, 100)
Disk_1 = geompy.MakeDiskPntVecR(Vertex_1, OZ, 60)
Vertex_2 = geompy.MakeVertex(-5, -5, 90)
Vertex_3 = geompy.MakeVertex(65, 65, 110)
Box_2 = geompy.MakeBoxTwoPnt(Vertex_3, Vertex_2)
Common_1 = geompy.MakeCommon(Disk_1, Box_2)

geompy.addToStudy( OZ, 'OZ' )
geompy.addToStudy( Box_1, 'Box_1' )
geompy.addToStudy( Vertex_1, 'Vertex_1' )
geompy.addToStudy( Disk_1, 'Disk_1' )
geompy.addToStudy( Vertex_2, 'Vertex_2' )
geompy.addToStudy( Vertex_3, 'Vertex_3' )
geompy.addToStudy( Box_2, 'Box_2' )
geompy.addToStudy( Common_1, NOM_OBJET )

ficcao = os.path.join(gmu.pathBloc, "materielCasTests", "{}Fiss.brep".format(NOM_OBJET))
text = ".. Exportation de la géométrie de la fissure dans le fichier '{}'".format(ficcao)
logging.info(text)
geompy.ExportBREP(Common_1, ficcao)

###
### SMESH component
###


smesh = smeshBuilder.New()
Mesh_1 = smesh.Mesh(Box_1)
smesh.SetName(Mesh_1, NOM_OBJET)
Regular_1D = Mesh_1.Segment()
Nb_Segments_1 = Regular_1D.NumberOfSegments(15)
Nb_Segments_1.SetDistrType( 0 )
Quadrangle_2D = Mesh_1.Quadrangle(algo=smeshBuilder.QUADRANGLE)
Hexa_3D = Mesh_1.Hexahedron(algo=smeshBuilder.Hexa)

is_done = Mesh_1.Compute()
text = "Mesh_1.Compute"
if is_done:
  logging.info(text+" OK")
else:
  text = "Erreur au calcul du maillage.\n" + text
  logging.info(text)
  raise Exception(text)

ficmed = os.path.join(gmu.pathBloc, "materielCasTests","{}.med".format(NOM_OBJET))
text = ".. Archivage du maillage dans le fichier '{}'".format(ficmed)
logging.info(text)
Mesh_1.ExportMED(ficmed)

## set object names
smesh.SetName(Regular_1D.GetAlgorithm(), 'Regular_1D')
smesh.SetName(Nb_Segments_1, 'Nb. Segments_1')
smesh.SetName(Quadrangle_2D.GetAlgorithm(), 'Quadrangle_2D')
smesh.SetName(Hexa_3D.GetAlgorithm(), 'Hexa_3D')

if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()
