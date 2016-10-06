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

#  File   : SMESH_test5.py
#  Module : SMESH
#
import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

import CORBA
import os
import os.path

def SetSObjName(theSObj,theName) :
    ok, anAttr = theSObj.FindAttribute("AttributeName")
    if ok:
        aName = anAttr._narrow(SALOMEDS.AttributeName)
        #print aName.__dict__
        aName.SetValue(theName)

def ConvertMED2UNV(thePath,theFile) :
    anInitFileName = thePath + theFile
    aMeshes,aResult = smesh.CreateMeshesFromMED(anInitFileName)
    print aResult, aMeshes

    for iMesh in range(len(aMeshes)) :
        aMesh = aMeshes[iMesh]
        print aMesh.GetName(),
        aFileName = anInitFileName
        aFileName = os.path.basename(aFileName)
        aMesh.SetName(aFileName)
        print aMesh.GetName()

        aOutPath = '/tmp/'
        aFileName = aOutPath + theFile + "." + str(iMesh) + ".unv"
        aMesh.ExportUNV(aFileName)
        aMesh = smesh.CreateMeshesFromUNV(aFileName)
        print aMesh.GetName(),
        os.remove(aFileName)
        aFileName = os.path.basename(aFileName)
        aMesh.SetName(aFileName)
        print aMesh.GetName()

aPath = os.getenv('DATA_DIR') + '/MedFiles/'
aListDir = os.listdir(aPath)
print aListDir

for iFile in range(len(aListDir)) :
    aFileName = aListDir[iFile]
    aName,anExt = os.path.splitext(aFileName)
    if anExt == ".med" :
        aFileName = os.path.basename(aFileName)
        print aFileName
        ConvertMED2UNV(aPath,aFileName)
        #break

salome.sg.updateObjBrowser(True)
