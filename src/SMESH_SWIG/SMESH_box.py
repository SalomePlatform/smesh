#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
#
# Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
# CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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

#==============================================================================
#  Info.
#  Bug (from script, bug)   : box.py, PAL5223
#  Modified                 : 25/11/2004
#  Author                   : Kovaltchuk Alexey
#  Project                  : PAL/SALOME
#==============================================================================
# Salome geometry and meshing for a box
#
import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

from salome import sg

# Plate

box    = geompy.MakeBox(0.,0.,0.,1.,1.,1.)
boxId  = geompy.addToStudy(box,"box")

# ---- SMESH

# ---- init a Mesh

box_mesh=smesh.Mesh(box, "box_mesh")

# set Hypothesis and Algorithm

alg1D = box_mesh.Segment()
alg1D.SetName("algo1D")
hypL1 = alg1D.LocalLength(0.25)
smesh.SetName(hypL1, "LocalLength")
    
alg2D = box_mesh.Quadrangle()
alg2D.SetName("algo2D")

alg3D = box_mesh.Hexahedron()
alg3D.SetName("algo3D")

# compute mesh

box_mesh.Compute()

sg.updateObjBrowser(True)
