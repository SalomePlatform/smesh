# -*- coding: iso-8859-1 -*-
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
"""Géométrie et maillage de base nécessaire au cas-test :
. disquePerce
"""

import os

import logging

import salome
from salome.smesh import smeshBuilder
from salome.StdMeshers import StdMeshersBuilder
import GEOM
import SMESH
import SALOMEDS

from blocFissure import gmu
from blocFissure.gmu.geomsmesh import geompy
from blocFissure.gmu.geomsmesh import geomPublish
from blocFissure.gmu.geomsmesh import geomPublishInFather

from blocFissure.gmu.triedreBase import triedreBase
from blocFissure.gmu.putName import putName
from blocFissure.gmu import initLog

###
### GEOM component
###

O, OX, OY, OZ = triedreBase()

Cylinder_1 = geompy.MakeCylinderRH(100, 300)
Cylinder_2 = geompy.MakeCylinderRH(600, 200)
Cut_1 = geompy.MakeCut(Cylinder_2, Cylinder_1)
Face_1 = geompy.MakeFaceHW(500, 1500, 3)
Disque = geompy.MakePartition([Cut_1], [Face_1], [], [], geompy.ShapeType["SOLID"], 0, [], 0)
[Compound_1, Compound_2, Compound_3, Compound_4] = geompy.Propagate(Disque)
geompy.addToStudy( Disque, 'Disque' )
geompy.addToStudyInFather( Disque, Compound_1, 'Compound_1' )
geompy.addToStudyInFather( Disque, Compound_2, 'Compound_2' )
geompy.addToStudyInFather( Disque, Compound_3, 'Compound_3' )
geompy.addToStudyInFather( Disque, Compound_4, 'Compound_4' )

geomPublish(initLog.debug, Cylinder_1, 'Cylinder_1' )
geomPublish(initLog.debug, Cylinder_2, 'Cylinder_2' )
geomPublish(initLog.debug, Cut_1, 'Cut_1' )
geomPublish(initLog.debug, Face_1, 'Face_1' )

###
### SMESH component
###

smesh = smeshBuilder.New()
Disque_1 = smesh.Mesh(Disque)
putName(Disque_1.GetMesh(), 'Disque')

Regular_1D = Disque_1.Segment()
Nb_Segments_1 = Regular_1D.NumberOfSegments(10)
Nb_Segments_1.SetDistrType( 0 )
Hexa_3D = Disque_1.Hexahedron(algo=smeshBuilder.Hexa)
Regular_1D_1 = Disque_1.Segment(geom=Compound_3)
Nb_Segments_2 = Regular_1D_1.NumberOfSegments(20)
Nb_Segments_2.SetDistrType( 0 )
Regular_1D_2 = Disque_1.Segment(geom=Compound_4)
status = Disque_1.AddHypothesis(Nb_Segments_2,Compound_4)
Quadrangle_2D = Disque_1.Quadrangle(algo=smeshBuilder.QUADRANGLE)

## set object names
#smesh.SetName(Regular_1D.GetAlgorithm(), 'Regular_1D')
#smesh.SetName(Quadrangle_2D.GetAlgorithm(), 'Quadrangle_2D')
#smesh.SetName(Hexa_3D.GetAlgorithm(), 'Hexa_3D')
putName(Nb_Segments_1, 'Nb. Segments_1', i_pref='Disque')
putName(Nb_Segments_2, 'Nb. Segments_2', i_pref='Disque')

is_done = Disque_1.Compute()
text = "Disque_1.Compute"
if is_done:
  logging.info(text+" OK")
else:
  text = "Erreur au calcul du maillage.\n" + text
  logging.info(text)
  raise Exception(text)

Disque_1.ExportMED(os.path.join(gmu.pathBloc, "materielCasTests", "disque.med"))

if salome.sg.hasDesktop():
  salome.sg.updateObjBrowser()
