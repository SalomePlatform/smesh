using namespace std;
// File:	SMESH_Grid.cxx
// Created:	Fri Sep 27 15:47:42 2002
// Author:	Nicolas REJNERI

#include "SMESH_Grid.h"

#include "utilities.h"

// VTK Includes
#include <vtkObjectFactory.h>

SMESH_Grid* SMESH_Grid::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("SMESH_Grid");
  if(ret)
    {
      return (SMESH_Grid*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new SMESH_Grid;
}

void SMESH_Grid::AddNode(int idSMESHDSnode,int idVTKnode)
{
  myMapNodeSMDStoVTK.Bind(idSMESHDSnode, idVTKnode);
  myMapNodeVTKtoSMDS.Bind(idVTKnode, idSMESHDSnode);
}
void SMESH_Grid::AddElement(int idSMESHDSelement, int idVTKelement)
{
  myMapElementSMDStoVTK.Bind(idSMESHDSelement, idVTKelement);
  myMapElementVTKtoSMDS.Bind(idVTKelement, idSMESHDSelement);
}

void SMESH_Grid::SetIdsVTKNode(const TColStd_DataMapOfIntegerInteger& mapVTK)
{
  myMapNodeVTKtoSMDS = mapVTK;
}
void SMESH_Grid::SetIdsSMESHDSNode(const TColStd_DataMapOfIntegerInteger& mapSMESHDS)
{
  myMapNodeSMDStoVTK = mapSMESHDS;
}

void SMESH_Grid::SetIdsVTKElement(const TColStd_DataMapOfIntegerInteger& mapVTK)
{
  myMapElementVTKtoSMDS = mapVTK;
}
void SMESH_Grid::SetIdsSMESHDSElement(const TColStd_DataMapOfIntegerInteger& mapSMESHDS)
{
  myMapElementSMDStoVTK = mapSMESHDS;
}

int SMESH_Grid::GetIdVTKNode(int idSMESHDSnode)
{
  if ( myMapNodeSMDStoVTK.IsBound( idSMESHDSnode ) )
    return myMapNodeSMDStoVTK.Find(idSMESHDSnode);
  else {
    MESSAGE("GetIdVTKNode(): SMDS node not found: " << idSMESHDSnode);
    return -1;
  }
}
int SMESH_Grid::GetIdVTKElement(int idSMESHDSelement)
{
  if ( myMapElementSMDStoVTK.IsBound( idSMESHDSelement ) )
    return myMapElementSMDStoVTK.Find(idSMESHDSelement);
  else {
    MESSAGE("GetIdVTKElement(): SMDS element not found: " << idSMESHDSelement);    
    return -1;
  }
}

int SMESH_Grid::GetIdSMESHDSNode(int idVTKnode)
{
  if ( myMapNodeVTKtoSMDS.IsBound( idVTKnode ) )
    return myMapNodeVTKtoSMDS.Find(idVTKnode);
  else {
    MESSAGE("GetIdSMESHDSNode(): VTK node not found: " << idVTKnode);    
    return -1;
  }
}
int SMESH_Grid::GetIdSMESHDSElement(int idVTKelement)
{
  if ( myMapElementVTKtoSMDS.IsBound( idVTKelement ) )
    return myMapElementVTKtoSMDS.Find(idVTKelement);
  else {
    MESSAGE("GetIdSMESHDSElement(): VTK element not found: " << idVTKelement);
    return -1;
  }
}

void SMESH_Grid::ClearNode()
{
  myMapNodeVTKtoSMDS.Clear();
  myMapNodeSMDStoVTK.Clear();
}
void SMESH_Grid::ClearElement()
{
  myMapElementVTKtoSMDS.Clear();
  myMapElementSMDStoVTK.Clear();
}

void SMESH_Grid::RemoveNode(int id)
{
  if ( myMapNodeSMDStoVTK.IsBound( id ) ) {
    int idVTK = myMapNodeSMDStoVTK.Find(id);
    myMapNodeSMDStoVTK.UnBind(id);
    if ( myMapNodeVTKtoSMDS.IsBound( idVTK ) ) {
      myMapNodeVTKtoSMDS.UnBind(idVTK);
    }
  }
}
void SMESH_Grid::RemoveElement(int id)
{
  if ( myMapElementSMDStoVTK.IsBound( id ) ) {
    int idVTK = myMapElementSMDStoVTK.Find(id);
    myMapElementSMDStoVTK.UnBind(id);
    if ( myMapElementVTKtoSMDS.IsBound( idVTK ) ) {
      myMapElementVTKtoSMDS.UnBind(idVTK);
    }
  }
}

void SMESH_Grid::DeepCopy(vtkDataObject *src)
{
  SMESH_Grid* srcGrid = SMESH_Grid::SafeDownCast(src);

  if (srcGrid != NULL) {
    CopyMaps(srcGrid);
  }

  vtkUnstructuredGrid::DeepCopy(src);
}

void SMESH_Grid::CopyMaps(SMESH_Grid *srcGrid)
{
  this->myMapNodeVTKtoSMDS = srcGrid->myMapNodeVTKtoSMDS;
  this->myMapNodeSMDStoVTK = srcGrid->myMapNodeSMDStoVTK;
  this->myMapElementVTKtoSMDS = srcGrid->myMapElementVTKtoSMDS;
  this->myMapElementSMDStoVTK = srcGrid->myMapElementSMDStoVTK;
}
