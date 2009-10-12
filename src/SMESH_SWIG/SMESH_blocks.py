#  -*- coding: iso-8859-1 -*-
#  Copyright (C) 2007-2008  CEA/DEN, EDF R&D, OPEN CASCADE
#
#  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
#  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2.1 of the License.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
#  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#
#  SMESH SMESH_SWIG : binding of C++ implementaion with Python
#  File   : SMESH_blocks.py
#  Author : Julia DOROVSKIKH
#  Module : SMESH
#  $Header$
#
import salome
import geompy
import math
import smesh

import GEOM_Spanner

isBlocksTest = 0 # False
isMeshTest   = 1 # True


GEOM_Spanner.MakeSpanner(geompy, math, isBlocksTest, isMeshTest, smesh)

salome.sg.updateObjBrowser(1);
