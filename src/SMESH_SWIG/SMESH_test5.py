#  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
#  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
#
#
#
#  File   : SMESH_test1.py
#  Module : SMESH

import salome
import SMESH
import SALOMEDS
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
        anSObj = salome.ObjectToSObject(aMesh)
        print anSObj.GetName(),
        aFileName = anInitFileName
        aFileName = os.path.basename(aFileName)
        SetSObjName(anSObj,aFileName)
        print anSObj.GetName()

        aOutPath = '/tmp/'
        aFileName = aOutPath + theFile + "." + str(iMesh) + ".unv"
        aMesh.ExportUNV(aFileName)
        aMesh = smesh.CreateMeshesFromUNV(aFileName)
        anSObj = salome.ObjectToSObject(aMesh)
        print anSObj.GetName(),
        os.remove(aFileName)
        aFileName = os.path.basename(aFileName)
        SetSObjName(anSObj,aFileName)
        print anSObj.GetName()

smesh = salome.lcc.FindOrLoadComponent("FactoryServer", "SMESH")
smesh.SetCurrentStudy(salome.myStudy)

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

salome.sg.updateObjBrowser(1)
