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

from SMESH_test1 import *

## Old style
def CheckBelongToGeomFilterOld(theMeshGen, theMesh, theShape, theSubShape, theElemType):
    if theShape != theSubShape:
        aName = str(theSubShape)
        geompy.addToStudyInFather(theShape,theSubShape,aName)

    theMeshGen.Compute(theMesh,theShape)

    aFilterMgr = theMeshGen.CreateFilterManager()
    aFilter = aFilterMgr.CreateFilter()
   
    aBelongToGeom = aFilterMgr.CreateBelongToGeom()
    aBelongToGeom.SetGeom(theSubShape)
    aBelongToGeom.SetElementType(theElemType)
    
    aFilter.SetPredicate(aBelongToGeom)
    aFilterMgr.UnRegister()
    return aFilter.GetElementsId(theMesh)

## Current style
def CheckBelongToGeomFilter(theMesh, theShape, theSubShape, theElemType):
    if theShape != theSubShape:
        aName = str(theSubShape)
        geompy.addToStudyInFather(theShape,theSubShape,aName)

    theMesh.Compute()
    aFilter = smesh.GetFilter(theElemType, SMESH.FT_BelongToGeom, theSubShape)
    return aFilter.GetElementsId(theMesh.GetMesh())
    

anElemType = SMESH.FACE;
print "anElemType =", anElemType
#anIds = CheckBelongToGeomFilter(mesh,box,subShapeList[1],anElemType)
anIds = CheckBelongToGeomFilter(mesh,box,box,anElemType)
print "Number of ids = ", len(anIds)
print "anIds = ", anIds
## Check old version
#anIds = CheckBelongToGeomFilterOld(smesh,mesh.GetMesh(),box,box,anElemType)
#print "anIds = ", anIds

salome.sg.updateObjBrowser(True)
