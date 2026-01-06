#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2026  CEA, EDF, OPEN CASCADE
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

#  File   : SMESH_test5.py
#  Module : SMESH
#

from salome.kernel import salome
salome.salome_init()
from salome.kernel import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New()

from salome.kernel import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New()

import CORBA
import os
import shutil
import tempfile

aOutPath = tempfile.mkdtemp()

def SetSObjName(theSObj,theName) :
    ok, anAttr = theSObj.FindAttribute("AttributeName")
    if ok:
        aName = anAttr._narrow(SALOMEDS.AttributeName)
        #print aName.__dict__
        aName.SetValue(theName)

def ConvertMED2UNV(theFile):
    print(theFile)
    aMeshes, aResult = smesh.CreateMeshesFromMED(theFile)
    print(aMeshes, aResult)

    for aMesh in aMeshes:
        print(aMesh.GetName(), end=' ')
        aMesh.SetName(os.path.basename(theFile))
        print(aMesh.GetName())

        aFileName = os.path.join(aOutPath, theFile + ".unv")
        aMesh.ExportUNV(aFileName)

        aMesh = smesh.CreateMeshesFromUNV(aFileName)
        print(aMesh.GetName(), end=' ')
        aMesh.SetName(os.path.basename(aFileName))
        print(aMesh.GetName())

aPath = os.path.join(os.getenv('DATA_DIR'), 'MedFiles')
      
for aFileName in sorted(os.listdir(aPath)):
    if os.path.splitext(aFileName)[-1] == ".med":
        ConvertMED2UNV(os.path.join(aPath, aFileName))

if os.getenv('SMESH_KEEP_TMP_DIR') != '1':
    shutil.rmtree(aOutPath)

salome.sg.updateObjBrowser()
