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
//  $Header$Header$


#include "SMESH_DeviceActor.h"
#include "SMESH_ExtractGeometry.h"
#include "SMESH_ControlsDef.hxx"
#include "SMESH_ActorUtils.h"

#include "SALOME_Transform.h"
#include "SALOME_TransformFilter.h"
#include "SALOME_PassThroughFilter.h"
#include "SALOME_ExtractUnstructuredGrid.h"

// VTK Includes
#include <vtkObjectFactory.h>
#include <vtkShrinkFilter.h>
#include <vtkShrinkPolyData.h>

#include <vtkProperty.h>
#include <vtkPolyData.h>
#include <vtkMergeFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkUnstructuredGrid.h>

#include <vtkScalarBarActor.h>
#include <vtkLookupTable.h>
#include <vtkDoubleArray.h>
#include <vtkCellData.h>

#include <vtkCell.h>
#include <vtkIdList.h>
#include <vtkIntArray.h>
#include <vtkCellArray.h>
#include <vtkUnsignedCharArray.h>

#include <vtkImplicitBoolean.h>

#include "utilities.h"

#ifdef _DEBUG_
static int MYDEBUG = 0;
#else
static int MYDEBUG = 0;
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

  myMapper->UseLookupTableScalarRangeOn();
  myMapper->SetColorModeToMapScalars();

  myShrinkFilter = vtkShrinkFilter::New();

  myExtractGeometry = SMESH_ExtractGeometry::New();
  myExtractGeometry->SetStoreMapping(true);

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

  myExtractGeometry->UnRegisterAllOutputs();
  myExtractGeometry->Delete();

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


void SMESH_DeviceActor::Init(TVisualObjPtr theVisualObj, 
			     vtkImplicitBoolean* theImplicitBoolean)
{
  myVisualObj = theVisualObj;
  myExtractGeometry->SetImplicitFunction(theImplicitBoolean);
  SetUnstructuredGrid(myVisualObj->GetUnstructuredGrid());
}


void SMESH_DeviceActor::SetUnstructuredGrid(vtkUnstructuredGrid* theGrid){
  if(theGrid){
    //myIsShrinkable = theGrid->GetNumberOfCells() > 10;
    myIsShrinkable = true;

    myExtractGeometry->SetInput(theGrid);

    myExtractUnstructuredGrid->SetInput(myExtractGeometry->GetOutput());
    myMergeFilter->SetGeometry(myExtractUnstructuredGrid->GetOutput());
    
    theGrid = static_cast<vtkUnstructuredGrid*>(myMergeFilter->GetOutput());

    int anId = 0;
    myPassFilter[ anId ]->SetInput( theGrid );
    myPassFilter[ anId + 1]->SetInput( myPassFilter[ anId ]->GetOutput() );
    
    anId++; // 1
    myGeomFilter->SetStoreMapping( myStoreMapping );
    myGeomFilter->SetInput( myPassFilter[ anId ]->GetOutput() );

    anId++; // 2
    myPassFilter[ anId ]->SetInput( myGeomFilter->GetOutput() ); 
    myPassFilter[ anId + 1 ]->SetInput( myPassFilter[ anId ]->GetOutput() );

    anId++; // 3
    myTransformFilter->SetInput( myPassFilter[ anId ]->GetPolyDataOutput() );
    myTransformFilter->SetInput( myPassFilter[ anId ]->GetPolyDataOutput() );

    anId++; // 4
    myPassFilter[ anId ]->SetInput( myTransformFilter->GetOutput() );
    myPassFilter[ anId + 1 ]->SetInput( myPassFilter[ anId ]->GetOutput() );
    myPassFilter[ anId ]->SetInput( myTransformFilter->GetOutput() );
    myPassFilter[ anId + 1 ]->SetInput( myPassFilter[ anId ]->GetOutput() );

    anId++; // 5
    myMapper->SetInput( myPassFilter[ anId ]->GetPolyDataOutput() );
    myMapper->SetInput( myPassFilter[ anId ]->GetPolyDataOutput() );

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


void SMESH_DeviceActor::SetControlMode(SMESH::Controls::FunctorPtr theFunctor,
				       vtkScalarBarActor* theScalarBarActor,
				       vtkLookupTable* theLookupTable)
{
  bool anIsInitialized = theFunctor;
  if(anIsInitialized){
    vtkUnstructuredGrid* aDataSet = vtkUnstructuredGrid::New();
    vtkUnstructuredGrid* aGrid = myExtractUnstructuredGrid->GetOutput();
    aDataSet->ShallowCopy(aGrid);
    
    vtkDoubleArray *aScalars = vtkDoubleArray::New();
    vtkIdType aNbCells = aGrid->GetNumberOfCells();
    aScalars->SetNumberOfComponents(1);
    aScalars->SetNumberOfTuples(aNbCells);
    
    myVisualObj->UpdateFunctor(theFunctor);

    using namespace SMESH::Controls;
    if(NumericalFunctor* aNumericalFunctor = dynamic_cast<NumericalFunctor*>(theFunctor.get())){
      for(vtkIdType i = 0; i < aNbCells; i++){
	vtkIdType anId = myExtractUnstructuredGrid->GetInputId(i);
	vtkIdType anId2 = myExtractGeometry->GetElemObjId(anId);
	vtkIdType anObjId = myVisualObj->GetElemObjId(anId2);
	double aValue = aNumericalFunctor->GetValue(anObjId);
	aScalars->SetValue(i,aValue);
      }
    }else if(Predicate* aPredicate = dynamic_cast<Predicate*>(theFunctor.get())){
      for(vtkIdType i = 0; i < aNbCells; i++){
	vtkIdType anId = myExtractUnstructuredGrid->GetInputId(i);
	vtkIdType anId2 = myExtractGeometry->GetElemObjId(anId);
	vtkIdType anObjId = myVisualObj->GetElemObjId(anId2);
	bool aValue = aPredicate->IsSatisfy(anObjId);
	aScalars->SetValue(i,aValue);
      }
    }

    aDataSet->GetCellData()->SetScalars(aScalars);
    aScalars->Delete();
	
    theLookupTable->SetRange(aScalars->GetRange());
    theLookupTable->Build();
    
    myMergeFilter->SetScalars(aDataSet);
    aDataSet->Delete();
  }
  GetMapper()->SetScalarVisibility(anIsInitialized);
  theScalarBarActor->SetVisibility(anIsInitialized);
}

void SMESH_DeviceActor::SetExtControlMode(SMESH::Controls::FunctorPtr theFunctor,
					  SMESH_DeviceActor* theDeviceActor,
					  vtkScalarBarActor* theScalarBarActor,
					  vtkLookupTable* theLookupTable)
{
  bool anIsInitialized = theFunctor;

  using namespace SMESH::Controls;
  if (anIsInitialized){
    if (Length2D* aLength2D = dynamic_cast<Length2D*>(theFunctor.get())){
      SMESH::Controls::Length2D::TValues aValues;

      myVisualObj->UpdateFunctor(theFunctor);

      aLength2D->GetValues(aValues);
      vtkUnstructuredGrid* aDataSet = vtkUnstructuredGrid::New();
      vtkUnstructuredGrid* aGrid = myVisualObj->GetUnstructuredGrid();

      aDataSet->SetPoints(aGrid->GetPoints());
      
      vtkIdType aNbCells = aValues.size();
      
      vtkDoubleArray *aScalars = vtkDoubleArray::New();
      aScalars->SetNumberOfComponents(1);
      aScalars->SetNumberOfTuples(aNbCells);

      vtkIdType aCellsSize = 3*aNbCells;
      vtkCellArray* aConnectivity = vtkCellArray::New();
      aConnectivity->Allocate( aCellsSize, 0 );
      
      vtkUnsignedCharArray* aCellTypesArray = vtkUnsignedCharArray::New();
      aCellTypesArray->SetNumberOfComponents( 1 );
      aCellTypesArray->Allocate( aNbCells * aCellTypesArray->GetNumberOfComponents() );
      
      vtkIdList *anIdList = vtkIdList::New();
      anIdList->SetNumberOfIds(2);
      
      Length2D::TValues::const_iterator anIter = aValues.begin();
      for(vtkIdType aVtkId = 0; anIter != aValues.end(); anIter++,aVtkId++){
	const Length2D::Value& aValue = *anIter;
	int aNode[2] = {
	  myVisualObj->GetNodeVTKId(aValue.myPntId[0]),
	  myVisualObj->GetNodeVTKId(aValue.myPntId[1])
	};
	if(aNode[0] >= 0 && aNode[1] >= 0){
	  anIdList->SetId( 0, aNode[0] );
	  anIdList->SetId( 1, aNode[1] );
	  aConnectivity->InsertNextCell( anIdList );
	  aCellTypesArray->InsertNextValue( VTK_LINE );
	  aScalars->SetValue(aVtkId,aValue.myLength);
	}
      }
      
      vtkIntArray* aCellLocationsArray = vtkIntArray::New();
      aCellLocationsArray->SetNumberOfComponents( 1 );
      aCellLocationsArray->SetNumberOfTuples( aNbCells );
      
      aConnectivity->InitTraversal();
      for( vtkIdType idType = 0, *pts, npts; aConnectivity->GetNextCell( npts, pts ); idType++ )
	aCellLocationsArray->SetValue( idType, aConnectivity->GetTraversalLocation( npts ) );
      
      aDataSet->SetCells( aCellTypesArray, aCellLocationsArray,aConnectivity );
      SetUnstructuredGrid(aDataSet);

      aDataSet->GetCellData()->SetScalars(aScalars);
      aScalars->Delete();
      
      theLookupTable->SetRange(aScalars->GetRange());
      theLookupTable->Build();
      
      myMergeFilter->SetScalars(aDataSet);
      aDataSet->Delete();
    }
    else if (MultiConnection2D* aMultiConnection2D = dynamic_cast<MultiConnection2D*>(theFunctor.get())){
      SMESH::Controls::MultiConnection2D::MValues aValues;

      myVisualObj->UpdateFunctor(theFunctor);

      aMultiConnection2D->GetValues(aValues);
      vtkUnstructuredGrid* aDataSet = vtkUnstructuredGrid::New();
      vtkUnstructuredGrid* aGrid = myVisualObj->GetUnstructuredGrid();
      aDataSet->SetPoints(aGrid->GetPoints());
      
      vtkIdType aNbCells = aValues.size();
      vtkDoubleArray *aScalars = vtkDoubleArray::New();
      aScalars->SetNumberOfComponents(1);
      aScalars->SetNumberOfTuples(aNbCells);

      vtkIdType aCellsSize = 3*aNbCells;
      vtkCellArray* aConnectivity = vtkCellArray::New();
      aConnectivity->Allocate( aCellsSize, 0 );
      
      vtkUnsignedCharArray* aCellTypesArray = vtkUnsignedCharArray::New();
      aCellTypesArray->SetNumberOfComponents( 1 );
      aCellTypesArray->Allocate( aNbCells * aCellTypesArray->GetNumberOfComponents() );
      
      vtkIdList *anIdList = vtkIdList::New();
      anIdList->SetNumberOfIds(2);
      
      MultiConnection2D::MValues::const_iterator anIter = aValues.begin();
      int i = 0;
      for(vtkIdType aVtkId; anIter != aValues.end(); anIter++,i++){
	const MultiConnection2D::Value& aValue = (*anIter).first;
	int aNode[2] = {
	  myVisualObj->GetNodeVTKId(aValue.myPntId[0]),
	  myVisualObj->GetNodeVTKId(aValue.myPntId[1])
	};
	if(aNode[0] >= 0 && aNode[1] >= 0){
	  anIdList->SetId( 0, aNode[0] );
	  anIdList->SetId( 1, aNode[1] );
	  aConnectivity->InsertNextCell( anIdList );
	  aCellTypesArray->InsertNextValue( VTK_LINE );
	  aScalars->SetValue(i,(*anIter).second);
	}
      }
      
      vtkIntArray* aCellLocationsArray = vtkIntArray::New();
      aCellLocationsArray->SetNumberOfComponents( 1 );
      aCellLocationsArray->SetNumberOfTuples( aNbCells );
      
      aConnectivity->InitTraversal();
      for( vtkIdType idType = 0, *pts, npts; aConnectivity->GetNextCell( npts, pts ); idType++ )
	aCellLocationsArray->SetValue( idType, aConnectivity->GetTraversalLocation( npts ) );
      
      aDataSet->SetCells( aCellTypesArray, aCellLocationsArray,aConnectivity );
      SetUnstructuredGrid(aDataSet);

      aDataSet->GetCellData()->SetScalars(aScalars);
      aScalars->Delete();
      
      theLookupTable->SetRange(aScalars->GetRange());
      theLookupTable->Build();
      
      myMergeFilter->SetScalars(aDataSet);
      aDataSet->Delete();
    }
  }
  GetMapper()->SetScalarVisibility(anIsInitialized);
  theScalarBarActor->SetVisibility(anIsInitialized);
}

void SMESH_DeviceActor::SetExtControlMode(SMESH::Controls::FunctorPtr theFunctor,
					  SMESH_DeviceActor* theDeviceActor)
{
  myExtractUnstructuredGrid->ClearRegisteredCells();
  myExtractUnstructuredGrid->ClearRegisteredCellsWithType();
  myExtractUnstructuredGrid->SetModeOfChanging(SALOME_ExtractUnstructuredGrid::ePassAll);
  myVisualObj->UpdateFunctor(theFunctor);

  using namespace SMESH::Controls;
  if(FreeBorders* aFreeBorders = dynamic_cast<FreeBorders*>(theFunctor.get())){
    myExtractUnstructuredGrid->SetModeOfChanging(SALOME_ExtractUnstructuredGrid::eAdding);
    myExtractUnstructuredGrid->ClearRegisteredCells();
    vtkUnstructuredGrid* aGrid = theDeviceActor->GetUnstructuredGrid();
    vtkIdType aNbCells = aGrid->GetNumberOfCells();
    for( vtkIdType i = 0; i < aNbCells; i++ ){
      vtkIdType anObjId = theDeviceActor->GetElemObjId(i);
      if(aFreeBorders->IsSatisfy(anObjId))
	myExtractUnstructuredGrid->RegisterCell(i);
    }
    if(!myExtractUnstructuredGrid->IsCellsRegistered())
      myExtractUnstructuredGrid->RegisterCell(-1);
    SetUnstructuredGrid(myVisualObj->GetUnstructuredGrid());
  }else if(FreeEdges* aFreeEdges = dynamic_cast<FreeEdges*>(theFunctor.get())){
    SMESH::Controls::FreeEdges::TBorders aBorders;
    aFreeEdges->GetBoreders(aBorders);
    vtkUnstructuredGrid* aDataSet = vtkUnstructuredGrid::New();
    vtkUnstructuredGrid* aGrid = myVisualObj->GetUnstructuredGrid();
    aDataSet->SetPoints(aGrid->GetPoints());

    vtkIdType aNbCells = aBorders.size();
    vtkIdType aCellsSize = 3*aNbCells;
    vtkCellArray* aConnectivity = vtkCellArray::New();
    aConnectivity->Allocate( aCellsSize, 0 );
    
    vtkUnsignedCharArray* aCellTypesArray = vtkUnsignedCharArray::New();
    aCellTypesArray->SetNumberOfComponents( 1 );
    aCellTypesArray->Allocate( aNbCells * aCellTypesArray->GetNumberOfComponents() );
    
    vtkIdList *anIdList = vtkIdList::New();
    anIdList->SetNumberOfIds(2);
    
    FreeEdges::TBorders::const_iterator anIter = aBorders.begin();
    for(; anIter != aBorders.end(); anIter++){
      const FreeEdges::Border& aBorder = *anIter;
      int aNode[2] = {
	myVisualObj->GetNodeVTKId(aBorder.myPntId[0]),
	myVisualObj->GetNodeVTKId(aBorder.myPntId[1])
      };
      //cout<<"aNode = "<<aBorder.myPntId[0]<<"; "<<aBorder.myPntId[1]<<endl;
      if(aNode[0] >= 0 && aNode[1] >= 0){
	anIdList->SetId( 0, aNode[0] );
	anIdList->SetId( 1, aNode[1] );
	aConnectivity->InsertNextCell( anIdList );
	aCellTypesArray->InsertNextValue( VTK_LINE );
      }
    }
    
    vtkIntArray* aCellLocationsArray = vtkIntArray::New();
    aCellLocationsArray->SetNumberOfComponents( 1 );
    aCellLocationsArray->SetNumberOfTuples( aNbCells );
    
    aConnectivity->InitTraversal();
    for( vtkIdType idType = 0, *pts, npts; aConnectivity->GetNextCell( npts, pts ); idType++ )
      aCellLocationsArray->SetValue( idType, aConnectivity->GetTraversalLocation( npts ) );
    
    aDataSet->SetCells( aCellTypesArray, aCellLocationsArray,aConnectivity );

    SetUnstructuredGrid(aDataSet);
    aDataSet->Delete();
  }
}




unsigned long int SMESH_DeviceActor::GetMTime(){
  unsigned long mTime = this->Superclass::GetMTime();
  mTime = max(mTime,myExtractGeometry->GetMTime());
  mTime = max(mTime,myExtractUnstructuredGrid->GetMTime());
  mTime = max(mTime,myMergeFilter->GetMTime());
  mTime = max(mTime,myGeomFilter->GetMTime());
  mTime = max(mTime,myTransformFilter->GetMTime());
  return mTime;
}


void SMESH_DeviceActor::SetTransform(SALOME_Transform* theTransform){
  myTransformFilter->SetTransform(theTransform);
}


void SMESH_DeviceActor::SetShrink() {
  if ( !myIsShrinkable ) return;
  if ( vtkDataSet* aDataSet = myPassFilter[ 0 ]->GetOutput() )
  {
    myShrinkFilter->SetInput( aDataSet );
    myPassFilter[ 1 ]->SetInput( myShrinkFilter->GetOutput() );
    myIsShrunk = true;
  }
}

void SMESH_DeviceActor::UnShrink() {
  if ( !myIsShrunk ) return;
  if ( vtkDataSet* aDataSet = myPassFilter[ 0 ]->GetOutput() )
  {    
    myPassFilter[ 1 ]->SetInput( aDataSet );
    myPassFilter[ 1 ]->Modified();
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
  GetProperty()->Modified();
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


int SMESH_DeviceActor::GetNodeObjId(int theVtkID){
  vtkIdType anID = myExtractGeometry->GetNodeObjId(theVtkID);
  vtkIdType aRetID = myVisualObj->GetNodeObjId(anID);
  if(MYDEBUG) MESSAGE("GetNodeObjId - theVtkID = "<<theVtkID<<"; aRetID = "<<aRetID);
  return aRetID;
}

float* SMESH_DeviceActor::GetNodeCoord(int theObjID){
  vtkDataSet* aDataSet = myExtractGeometry->GetInput();
  vtkIdType anID = myVisualObj->GetNodeVTKId(theObjID);
  float* aCoord = aDataSet->GetPoint(anID);
  if(MYDEBUG) MESSAGE("GetNodeCoord - theObjID = "<<theObjID<<"; anID = "<<anID);
  return aCoord;
}


int SMESH_DeviceActor::GetElemObjId(int theVtkID){
  vtkIdType anId = myGeomFilter->GetElemObjId(theVtkID);
  if(anId < 0) 
    return -1;
  vtkIdType anId2 = myExtractUnstructuredGrid->GetInputId(anId);
  if(anId2 < 0) 
    return -1;
  vtkIdType anId3 = myExtractGeometry->GetElemObjId(anId2);
  if(anId3 < 0) 
    return -1;
  vtkIdType aRetID = myVisualObj->GetElemObjId(anId3);
  if(MYDEBUG) 
    MESSAGE("GetElemObjId - theVtkID = "<<theVtkID<<"; anId2 = "<<anId2<<"; anId3 = "<<anId3<<"; aRetID = "<<aRetID);
  return aRetID;
}

vtkCell* SMESH_DeviceActor::GetElemCell(int theObjID){
  vtkDataSet* aDataSet = myExtractGeometry->GetInput();
  vtkIdType aGridID = myVisualObj->GetElemVTKId(theObjID);
  vtkCell* aCell = aDataSet->GetCell(aGridID);
  if(MYDEBUG) 
    MESSAGE("GetElemCell - theObjID = "<<theObjID<<"; aGridID = "<<aGridID);
  return aCell;
}


float SMESH_DeviceActor::GetShrinkFactor(){
  return myShrinkFilter->GetShrinkFactor();
}

void SMESH_DeviceActor::SetShrinkFactor(float theValue){
  theValue = theValue > 0.1? theValue: 0.8;
  myShrinkFilter->SetShrinkFactor(theValue);
  Modified();
}


void SMESH_DeviceActor::SetHighlited(bool theIsHighlited){
  myIsHighlited = theIsHighlited;
  Modified();
}

void SMESH_DeviceActor::Render(vtkRenderer *ren, vtkMapper* m){
  int aResolveCoincidentTopology = vtkMapper::GetResolveCoincidentTopology();
  float aStoredFactor, aStoredUnit; 
  vtkMapper::GetResolveCoincidentTopologyPolygonOffsetParameters(aStoredFactor,aStoredUnit);

  vtkMapper::SetResolveCoincidentTopologyToPolygonOffset();
  float aFactor = myPolygonOffsetFactor, aUnits = myPolygonOffsetUnits;
  if(myIsHighlited){
    static float EPS = .01;
    aUnits *= (1.0-EPS);
  }
  vtkMapper::SetResolveCoincidentTopologyPolygonOffsetParameters(aFactor,aUnits);
  vtkLODActor::Render(ren,m);

  vtkMapper::SetResolveCoincidentTopologyPolygonOffsetParameters(aStoredFactor,aStoredUnit);
  vtkMapper::SetResolveCoincidentTopology(aResolveCoincidentTopology);
}


void SMESH_DeviceActor::SetPolygonOffsetParameters(float factor, float units){
  myPolygonOffsetFactor = factor;
  myPolygonOffsetUnits = units;
}

