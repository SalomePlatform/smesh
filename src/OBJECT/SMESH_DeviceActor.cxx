//  SMESH OBJECT : interactive object for SMESH visualization
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
// 
//  This library is free software; you can redistribute it and/or 
//  modify it under the terms of the GNU Lesser General Public 
//  License as published by the Free Software Foundation; either 
//  version 2.1 of the License. 
// 
//  This library is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//  Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free Software 
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA 
// 
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : SMESH_Actor.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$


#include "SMESH_DeviceActor.h"

#include "SALOME_Transform.h"
#include "SALOME_TransformFilter.h"
#include "SALOME_PassThroughFilter.h"
#include "SALOME_ExtractUnstructuredGrid.h"

#include "utilities.h"

// VTK Includes
#include <vtkObjectFactory.h>
#include <vtkShrinkFilter.h>
#include <vtkShrinkPolyData.h>

#include <vtkProperty.h>
#include <vtkPolyData.h>
#include <vtkMergeFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkUnstructuredGrid.h>

#ifdef _DEBUG_
static int MYDEBUG = 0;
static int MYDEBUGWITHFILES = 0;
#else
static int MYDEBUG = 0;
static int MYDEBUGWITHFILES = 0;
#endif

using namespace std;


vtkStandardNewMacro(SMESH_DeviceActor);


SMESH_DeviceActor::SMESH_DeviceActor(){
  if(MYDEBUG) MESSAGE("SMESH_DeviceActor");
  myIsShrunk = false;
  myIsShrinkable = false;
  myRepresentation = eSurface;

  myProperty = vtkProperty::New();
  myMapper = vtkPolyDataMapper::New();

  vtkMapper::GetResolveCoincidentTopologyPolygonOffsetParameters(myPolygonOffsetFactor,
								 myPolygonOffsetUnits);
  //myMapper->SetResolveCoincidentTopologyToShiftZBuffer();
  //myMapper->SetResolveCoincidentTopologyZShift(0.02);
  
  myMapper->UseLookupTableScalarRangeOn();
  myMapper->SetColorModeToMapScalars();

  myShrinkFilter = vtkShrinkFilter::New();

  myExtractUnstructuredGrid = SALOME_ExtractUnstructuredGrid::New();
  myExtractUnstructuredGrid->SetStoreMapping(true);

  myMergeFilter = vtkMergeFilter::New();

  myStoreMapping = false;
  myGeomFilter = SALOME_GeometryFilter::New();

  myTransformFilter = SALOME_TransformFilter::New();

  for(int i = 0; i < 6; i++)
    myPassFilter.push_back(SALOME_PassThroughFilter::New());
}


SMESH_DeviceActor::~SMESH_DeviceActor(){
  if(MYDEBUG) MESSAGE("~SMESH_DeviceActor");
  myProperty->Delete();

  myMapper->RemoveAllInputs();
  myMapper->Delete();

  myShrinkFilter->UnRegisterAllOutputs();
  myShrinkFilter->Delete();

  myExtractUnstructuredGrid->UnRegisterAllOutputs();
  myExtractUnstructuredGrid->Delete();

  myMergeFilter->UnRegisterAllOutputs();
  myMergeFilter->Delete();

  myGeomFilter->UnRegisterAllOutputs();
  myGeomFilter->Delete();

  myTransformFilter->UnRegisterAllOutputs();
  myTransformFilter->Delete();

  for(int i = 0, iEnd = myPassFilter.size(); i < iEnd; i++){
    myPassFilter[i]->UnRegisterAllOutputs(); 
    myPassFilter[i]->Delete();
  }
}


void SMESH_DeviceActor::SetStoreMapping(int theStoreMapping){
  myStoreMapping = theStoreMapping;
  Modified();
}


void SMESH_DeviceActor::SetUnstructuredGrid(vtkUnstructuredGrid* theGrid){
  if(theGrid){
    //myIsShrinkable = theGrid->GetNumberOfCells() > 10;
    myIsShrinkable = true;

    myExtractUnstructuredGrid->SetInput(theGrid);
    myMergeFilter->SetGeometry(myExtractUnstructuredGrid->GetOutput());

    theGrid = static_cast<vtkUnstructuredGrid*>(myMergeFilter->GetOutput());

    int anId = 0;
    myPassFilter.at( anId )->SetInput( theGrid );
    myPassFilter.at( anId + 1)->SetInput( myPassFilter.at( anId )->GetOutput() );
    
    anId++; // 1
    myGeomFilter->SetStoreMapping( myStoreMapping );
    myGeomFilter->SetInput( myPassFilter.at( anId )->GetOutput() );

    anId++; // 2
    myPassFilter.at( anId )->SetInput( myGeomFilter->GetOutput() ); 
    myPassFilter.at( anId + 1 )->SetInput( myPassFilter.at( anId )->GetOutput() );

    anId++; // 3
    myTransformFilter->SetInput( myPassFilter.at( anId )->GetPolyDataOutput() );

    anId++; // 4
    myPassFilter.at( anId )->SetInput( myTransformFilter->GetOutput() );
    myPassFilter.at( anId + 1 )->SetInput( myPassFilter.at( anId )->GetOutput() );

    anId++; // 5
    myMapper->SetInput( myPassFilter.at( anId )->GetPolyDataOutput() );

    vtkLODActor::SetMapper( myMapper );
    Modified();
  }
}


SALOME_ExtractUnstructuredGrid* SMESH_DeviceActor::GetExtractUnstructuredGrid(){
  return myExtractUnstructuredGrid;
}


vtkUnstructuredGrid* SMESH_DeviceActor::GetUnstructuredGrid(){
  myExtractUnstructuredGrid->Update();
  return myExtractUnstructuredGrid->GetOutput();
}


vtkMergeFilter* SMESH_DeviceActor::GetMergeFilter(){
  return myMergeFilter;
}


vtkPolyData* SMESH_DeviceActor::GetPolyDataInput(){
  return myPassFilter.back()->GetPolyDataOutput();
}


unsigned long int SMESH_DeviceActor::GetMTime(){
  unsigned long mTime = this->Superclass::GetMTime();
  mTime = max(mTime,myExtractUnstructuredGrid->GetMTime());
  mTime = max(mTime,myMergeFilter->GetMTime());
  mTime = max(mTime,myGeomFilter->GetMTime());
  mTime = max(mTime,myTransformFilter->GetMTime());
  return mTime;
}


void SMESH_DeviceActor::SetTransform(SALOME_Transform* theTransform){
  myTransformFilter->SetTransform(theTransform);
}


void SMESH_DeviceActor::SetShrink()
{
  if ( !myIsShrinkable ) return;
  if ( vtkDataSet* aDataSet = myPassFilter.at( 0 )->GetOutput() )
  {
    myShrinkFilter->SetInput( aDataSet );
    myPassFilter.at( 1 )->SetInput( myShrinkFilter->GetOutput() );
    myIsShrunk = true;
  }
}

void SMESH_DeviceActor::UnShrink()
{
  if ( !myIsShrunk ) return;
  if ( vtkDataSet* aDataSet = myPassFilter.at( 0 )->GetOutput() )
  {
    myPassFilter.at( 1 )->SetInput( aDataSet );
    myPassFilter.at( 1 )->Modified();
    myIsShrunk = false;
    Modified();
  }
}


void SMESH_DeviceActor::SetRepresentation(EReperesent theMode){ 
  switch(theMode){
  case ePoint: 
    myGeomFilter->SetInside(true);
    GetProperty()->SetRepresentation(0);
    break;
  case eInsideframe: 
    myGeomFilter->SetInside(true);
    GetProperty()->SetRepresentation(1);
    break;
  default :
    GetProperty()->SetRepresentation(theMode);
    myGeomFilter->SetInside(false);
  }
  myRepresentation = theMode;
  myMapper->Modified();
  Modified();
}


void SMESH_DeviceActor::SetVisibility(int theMode){
  if(!myExtractUnstructuredGrid->GetInput() || GetUnstructuredGrid()->GetNumberOfCells()){
    vtkLODActor::SetVisibility(theMode);
  }else{
    vtkLODActor::SetVisibility(false);
  }
}


int SMESH_DeviceActor::GetVisibility(){
  if(!GetUnstructuredGrid()->GetNumberOfCells()){
    vtkLODActor::SetVisibility(false);
  }
  return vtkLODActor::GetVisibility();
}


int SMESH_DeviceActor::GetObjId(int theVtkID){
  if (GetRepresentation() == ePoint){
    return GetNodeObjId(theVtkID);
  }else{
    return GetElemObjId(theVtkID);
  }
}


SMESH_DeviceActor::TVectorId SMESH_DeviceActor::GetVtkId(int theObjID){
  if (GetRepresentation() == ePoint){
    return GetNodeVtkId(theObjID);
  }else{
    return GetElemVtkId(theObjID);
  }
}


int SMESH_DeviceActor::GetNodeObjId(int theVtkID){
  vtkIdType aRetID = myVisualObj->GetNodeObjId(theVtkID);
  if(MYDEBUG) MESSAGE("GetNodeObjId - theVtkID = "<<theVtkID<<"; aRetID = "<<aRetID);
  return aRetID;
}

SMESH_DeviceActor::TVectorId SMESH_DeviceActor::GetNodeVtkId(int theObjID){
  SMESH_DeviceActor::TVectorId aVecId;
  vtkIdType anID = myVisualObj->GetNodeVTKId(theObjID);
  if(anID < 0) 
    return aVecId;
  aVecId.push_back(anID);
  return aVecId;
}


int SMESH_DeviceActor::GetElemObjId(int theVtkID){
  vtkIdType aGridID = myGeomFilter->GetObjId(theVtkID);
  if(aGridID < 0) 
    return -1;
  vtkIdType anExtractID = myExtractUnstructuredGrid->GetOutId(aGridID);
  if(anExtractID < 0) 
    return -1;
  vtkIdType aRetID = myVisualObj->GetElemObjId(anExtractID);
  if(MYDEBUG) 
    MESSAGE("GetElemObjId - theVtkID = "<<theVtkID<<"; anExtractID = "<<anExtractID<<"; aGridID = "<<aGridID<<"; aRetID = "<<aRetID);
  return aRetID;
}

SMESH_DeviceActor::TVectorId SMESH_DeviceActor::GetElemVtkId(int theObjID){
  TVectorId aVecId;
  vtkIdType aGridID = myVisualObj->GetElemVTKId(theObjID);
  if(aGridID < 0) 
    return aVecId;
  aVecId = myGeomFilter->GetVtkId(aGridID);
  if(MYDEBUG) 
    MESSAGE("GetElemVtkId - theObjID = "<<theObjID<<"; aGridID = "<<aGridID<<"; aGridID = "<<aGridID<<"; aVecId[0] = "<<aVecId[0]);
  return aVecId;
}


float SMESH_DeviceActor::GetShrinkFactor(){
  return myShrinkFilter->GetShrinkFactor();
}

void SMESH_DeviceActor::SetShrinkFactor(float theValue){
  theValue = theValue > 0.1? theValue: 0.8;
  myShrinkFilter->SetShrinkFactor(theValue);
  Modified();
}


void SMESH_DeviceActor::Render(vtkRenderer *ren, vtkMapper* m){
  int aResolveCoincidentTopology = vtkMapper::GetResolveCoincidentTopology();
  float aFactor, aUnit; 
  vtkMapper::GetResolveCoincidentTopologyPolygonOffsetParameters(aFactor,aUnit);

  vtkMapper::SetResolveCoincidentTopologyToPolygonOffset();
  vtkMapper::SetResolveCoincidentTopologyPolygonOffsetParameters(myPolygonOffsetFactor,
								 myPolygonOffsetUnits);
  vtkLODActor::Render(ren,m);

  vtkMapper::SetResolveCoincidentTopologyPolygonOffsetParameters(aFactor,aUnit);
  vtkMapper::SetResolveCoincidentTopology(aResolveCoincidentTopology);
}


void SMESH_DeviceActor::SetPolygonOffsetParameters(float factor, float units){
  myPolygonOffsetFactor = factor;
  myPolygonOffsetUnits = units;
}

