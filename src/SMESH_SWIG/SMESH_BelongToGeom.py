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
