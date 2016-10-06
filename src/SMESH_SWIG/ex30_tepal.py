#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

# ====================================================================
#
import os

import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

# Parameters
# ----------

results = "/tmp/ZZ"

radius =  50
height = 200

# Build a cylinder
# ----------------

base = geompy.MakeVertex(0, 0, 0)
direction = geompy.MakeVectorDXDYDZ(0, 0, 1)

cylinder = geompy.MakeCylinder(base, direction, radius, height)

geompy.addToStudy(cylinder, "Cylinder")

# Define a mesh on a geometry
# ---------------------------

smesh.SetCurrentStudy(salome.myStudy)

m = smesh.Mesh(cylinder)

# 2D mesh with BLSURF
# -------------------

algo2d = m.Triangle(smeshBuilder.BLSURF)

algo2d.SetPhysicalMesh(1)
algo2d.SetPhySize(5)

algo2d.SetGeometricMesh(0)

# 3D mesh with tepal
# ------------------

algo3d = m.Tetrahedron(smeshBuilder.GHS3DPRL)

algo3d.SetMEDName(results)
algo3d.SetNbPart(4)
algo3d.SetBackground(False)
algo3d.SetKeepFiles(False)
algo3d.SetToMeshHoles(True)

# Launch meshers
# --------------

status = m.Compute()

# Test if ok
# ----------

if os.access(results+".xml", os.F_OK):
    print "Ok: tepal"
else:
    print "KO: tepal"

# Update object browser
# ---------------------

salome.sg.updateObjBrowser(True)
