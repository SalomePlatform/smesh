#  Copyright (C) 2005  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
#  See http://www.salome-platform.org/
#
import SMESH
from SMESH_test1 import *

def CheckBelongToGeomFilter(theMeshGen, theMesh, theShape, theSubShape, theElemType):
    import geompy
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
    return aFilter.GetElementsId(theMesh)

anElemType = SMESH.ALL;
print "anElemType =", anElemType
#anIds = CheckBelongToGeomFilter(smesh,mesh,box,subShapeList[1],SMESH.FACE)
anIds = CheckBelongToGeomFilter(smesh,mesh,box,box,SMESH.FACE)
print "anIds = ", anIds

salome.sg.updateObjBrowser(1);
