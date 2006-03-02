from smesh import *

def BuildGroupLyingOn(theMesh, theElemType, theName, theShape):
    aFilterMgr = smesh.CreateFilterManager()
    aFilter = aFilterMgr.CreateFilter()
   
    aLyingOnGeom = aFilterMgr.CreateLyingOnGeom()
    aLyingOnGeom.SetGeom(theShape)
    aLyingOnGeom.SetElementType(theElemType)
    
    aFilter.SetPredicate(aLyingOnGeom)
    anIds = aFilter.GetElementsId(theMesh)

    aGroup = theMesh.CreateGroup(theElemType, theName)
    aGroup.Add(anIds)

#Example
from SMESH_test1 import *

smesh.Compute(mesh, box)
BuildGroupLyingOn(mesh, SMESH.FACE, "Group of faces lying on edge", edge )

salome.sg.updateObjBrowser(1);
