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


#include "SMESH_ActorDef.h"
#include "SMESH_ActorUtils.h"
#include "SMESH_DeviceActor.h"
#include "SMESH_ControlsDef.hxx"
#include "VTKViewer_ExtractUnstructuredGrid.h"

#include "SUIT_Session.h"
#include "SUIT_ResourceMgr.h"

#include <qstringlist.h>

#include <vtkProperty.h>
#include <vtkTimeStamp.h>
#include <vtkObjectFactory.h>
#include <vtkShrinkPolyData.h>
#include <vtkMergeFilter.h>

#include <vtkMatrix4x4.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPointData.h>
#include <vtkCellData.h>

#include <vtkMapper.h>
#include <vtkRenderer.h>

#include <vtkCell.h>
#include <vtkIdList.h>
#include <vtkIntArray.h>

#include <vtkActor2D.h>
#include <vtkProperty2D.h>
#include <vtkPolyData.h>
#include <vtkMaskPoints.h>
#include <vtkCellCenters.h>
#include <vtkTextProperty.h>
#include <vtkLabeledDataMapper.h>
#include <vtkSelectVisiblePoints.h>

#include <vtkScalarBarActor.h>
#include <vtkLookupTable.h>

#include <vtkMath.h>
#include <vtkPlane.h>
#include <vtkImplicitBoolean.h>
#include <vtkImplicitFunctionCollection.h>

#include "utilities.h"

#ifdef _DEBUG_
static int MYDEBUG = 1;
#else
static int MYDEBUG = 0;
#endif

static int aLineWidthInc = 2;
static int aPointSizeInc = 2;


SMESH_ActorDef* SMESH_ActorDef::New(){
  return new SMESH_ActorDef();
}


SMESH_Actor* SMESH_Actor::New(TVisualObjPtr theVisualObj, 
			      const char* theEntry, 
			      const char* theName,
			      int theIsClear)
{
  SMESH_ActorDef* anActor = SMESH_ActorDef::New();
  if(!anActor->Init(theVisualObj,theEntry,theName,theIsClear)){
    anActor->Delete();
    anActor = NULL;
  }
  if( anActor )
    anActor->UpdateScalarBar();
  return anActor;
}


SMESH_ActorDef::SMESH_ActorDef()
{
  if(MYDEBUG) MESSAGE("SMESH_ActorDef - "<<this);

  myTimeStamp = vtkTimeStamp::New();

  myIsPointsVisible = false;

  myIsShrinkable = false;
  myIsShrunk = false;

  myControlsPrecision = -1;
  SUIT_ResourceMgr* mgr = SUIT_Session::session()->resourceMgr();
  if ( mgr && mgr->booleanValue( "SMESH", "use_precision", false ) )
    myControlsPrecision = (long)SMESH::GetFloat( "SMESH", "controls_precision", -1 );

  vtkFloatingPointType aPointSize = SMESH::GetFloat("SMESH:node_size",3);
  vtkFloatingPointType aLineWidth = SMESH::GetFloat("SMESH:element_width",1);

  vtkMatrix4x4 *aMatrix = vtkMatrix4x4::New();
  VTKViewer_ExtractUnstructuredGrid* aFilter = NULL;

  //Definition 2D and 3D divices of the actor
  //-----------------------------------------
  vtkFloatingPointType anRGB[3] = {1,1,1};
  mySurfaceProp = vtkProperty::New();
  SMESH::GetColor( "SMESH", "fill_color", anRGB[0], anRGB[1], anRGB[2], QColor( 0, 170, 255 ) );
  mySurfaceProp->SetColor( anRGB[0], anRGB[1], anRGB[2] );

  myBackSurfaceProp = vtkProperty::New();
  SMESH::GetColor( "SMESH", "backface_color", anRGB[0], anRGB[1], anRGB[2], QColor( 0, 0, 255 ) );
  myBackSurfaceProp->SetColor( anRGB[0], anRGB[1], anRGB[2] );

  my2DActor = SMESH_DeviceActor::New();
  my2DActor->SetUserMatrix(aMatrix);
  my2DActor->PickableOff();
  my2DActor->SetProperty(mySurfaceProp);
  my2DActor->SetBackfaceProperty(myBackSurfaceProp);
  my2DActor->SetRepresentation(SMESH_DeviceActor::eSurface);
  aFilter = my2DActor->GetExtractUnstructuredGrid();
  aFilter->SetModeOfChanging(VTKViewer_ExtractUnstructuredGrid::eAdding);
  aFilter->RegisterCellsWithType(VTK_TRIANGLE);
  aFilter->RegisterCellsWithType(VTK_POLYGON);
  aFilter->RegisterCellsWithType(VTK_QUAD);
  aFilter->RegisterCellsWithType(VTK_QUADRATIC_TRIANGLE);
  aFilter->RegisterCellsWithType(VTK_QUADRATIC_QUAD);

  my3DActor = SMESH_DeviceActor::New();
  my3DActor->SetUserMatrix(aMatrix);
  my3DActor->PickableOff();
  my3DActor->SetProperty(mySurfaceProp);
  my3DActor->SetBackfaceProperty(myBackSurfaceProp);
  my3DActor->SetRepresentation(SMESH_DeviceActor::eSurface);
  aFilter = my3DActor->GetExtractUnstructuredGrid();
  aFilter->SetModeOfChanging(VTKViewer_ExtractUnstructuredGrid::eAdding);
  aFilter->RegisterCellsWithType(VTK_TETRA);
  aFilter->RegisterCellsWithType(VTK_VOXEL);
  aFilter->RegisterCellsWithType(VTK_HEXAHEDRON);
  aFilter->RegisterCellsWithType(VTK_WEDGE);
  aFilter->RegisterCellsWithType(VTK_PYRAMID);
  aFilter->RegisterCellsWithType(VTK_QUADRATIC_TETRA);
  aFilter->RegisterCellsWithType(VTK_QUADRATIC_HEXAHEDRON);
  aFilter->RegisterCellsWithType(VTK_CONVEX_POINT_SET);

  //Definition 1D divice of the actor
  //---------------------------------
  myEdgeProp = vtkProperty::New();
  myEdgeProp->SetAmbient(1.0);
  myEdgeProp->SetDiffuse(0.0);
  myEdgeProp->SetSpecular(0.0);
  SMESH::GetColor( "SMESH", "outline_color", anRGB[0], anRGB[1], anRGB[2], QColor( 0, 170, 255 ) );
  myEdgeProp->SetColor(anRGB[0],anRGB[1],anRGB[2]);
  myEdgeProp->SetLineWidth(aLineWidth);

  my1DActor = SMESH_DeviceActor::New();
  my1DActor->SetUserMatrix(aMatrix);
  my1DActor->PickableOff();
  my1DActor->SetHighlited(true);
  my1DActor->SetProperty(myEdgeProp);
  my1DActor->SetRepresentation(SMESH_DeviceActor::eSurface);
  aFilter = my1DActor->GetExtractUnstructuredGrid();
  aFilter->SetModeOfChanging(VTKViewer_ExtractUnstructuredGrid::eAdding);
  aFilter->RegisterCellsWithType(VTK_LINE);
  aFilter->RegisterCellsWithType(VTK_QUADRATIC_EDGE);

  my1DProp = vtkProperty::New();
  my1DProp->DeepCopy(myEdgeProp);
  my1DProp->SetLineWidth(aLineWidth + aLineWidthInc);
  my1DProp->SetPointSize(aPointSize);

  my1DExtProp = vtkProperty::New();
  my1DExtProp->DeepCopy(myEdgeProp);
  anRGB[0] = 1 - anRGB[0];
  anRGB[1] = 1 - anRGB[1];
  anRGB[2] = 1 - anRGB[2];
  my1DExtProp->SetColor(anRGB[0],anRGB[1],anRGB[2]);
  my1DExtProp->SetLineWidth(aLineWidth + aLineWidthInc);
  my1DExtProp->SetPointSize(aPointSize + aPointSizeInc);

  my1DExtActor = SMESH_DeviceActor::New();
  my1DExtActor->SetUserMatrix(aMatrix);
  my1DExtActor->PickableOff();
  my1DExtActor->SetHighlited(true);
  my1DExtActor->SetVisibility(false);
  my1DExtActor->SetProperty(my1DExtProp);
  my1DExtActor->SetRepresentation(SMESH_DeviceActor::eInsideframe);
  aFilter = my1DExtActor->GetExtractUnstructuredGrid();
  aFilter->SetModeOfChanging(VTKViewer_ExtractUnstructuredGrid::eAdding);
  aFilter->RegisterCellsWithType(VTK_LINE);
  aFilter->RegisterCellsWithType(VTK_QUADRATIC_EDGE);


  //Definition 0D divice of the actor
  //---------------------------------
  myNodeProp = vtkProperty::New();
  SMESH::GetColor( "SMESH", "node_color", anRGB[0], anRGB[1], anRGB[2], QColor( 255, 0, 0 ) );
  myNodeProp->SetColor(anRGB[0],anRGB[1],anRGB[2]);
  myNodeProp->SetPointSize(aPointSize);

  myNodeActor = SMESH_DeviceActor::New();
  myNodeActor->SetUserMatrix(aMatrix);
  myNodeActor->SetStoreClippingMapping(true);
  myNodeActor->PickableOff();
  myNodeActor->SetVisibility(false);
  myNodeActor->SetProperty(myNodeProp);
  myNodeActor->SetRepresentation(SMESH_DeviceActor::ePoint);
  aFilter = myNodeActor->GetExtractUnstructuredGrid();
  aFilter->SetModeOfExtraction(VTKViewer_ExtractUnstructuredGrid::ePoints);


  //Definition of Pickable and Highlitable engines
  //----------------------------------------------

  myBaseActor = SMESH_DeviceActor::New();
  myBaseActor->SetUserMatrix(aMatrix);
  myBaseActor->SetStoreGemetryMapping(true);
  myBaseActor->GetProperty()->SetOpacity(0.0);

  myPickableActor = myBaseActor;
  
  myHighlightProp = vtkProperty::New();
  myHighlightProp->SetAmbient(1.0);
  myHighlightProp->SetDiffuse(0.0);
  myHighlightProp->SetSpecular(0.0);
  SMESH::GetColor( "SMESH", "selection_object_color", anRGB[0], anRGB[1], anRGB[2], QColor( 255, 255, 255 ) );
  myHighlightProp->SetColor(anRGB[0],anRGB[1],anRGB[2]);
  myHighlightProp->SetPointSize(aPointSize);
  myHighlightProp->SetRepresentation(1);
 
  myPreselectProp = vtkProperty::New();
  myPreselectProp->SetAmbient(1.0);
  myPreselectProp->SetDiffuse(0.0);
  myPreselectProp->SetSpecular(0.0);
  SMESH::GetColor( "SMESH", "highlight_color", anRGB[0], anRGB[1], anRGB[2], QColor( 0, 255, 255 ) );
  myPreselectProp->SetColor(anRGB[0],anRGB[1],anRGB[2]);
  myPreselectProp->SetPointSize(aPointSize);
  myPreselectProp->SetRepresentation(1);

  myHighlitableActor = SMESH_DeviceActor::New();
  myHighlitableActor->SetUserMatrix(aMatrix);
  myHighlitableActor->PickableOff();
  myHighlitableActor->SetRepresentation(SMESH_DeviceActor::eWireframe);

  myName = "";
  myIO = NULL;

  myControlMode = eNone;
  myControlActor = my2DActor;

  //Definition of myScalarBarActor
  //------------------------------
  myLookupTable = vtkLookupTable::New();
  //Fix for Bug PAL5195 - SMESH764: 
  //Controls - Aspect Ratio: incorrect colors of the best and worst values
  myLookupTable->SetHueRange(0.667,0.0);

  myScalarBarActor = vtkScalarBarActor::New();
  myScalarBarActor->SetVisibility(false);
  myScalarBarActor->SetLookupTable(myLookupTable);

  mgr = SUIT_Session::session()->resourceMgr();
  if( !mgr )
    return;

  //Definition of points numbering pipeline
  //---------------------------------------
  myPointsNumDataSet = vtkUnstructuredGrid::New();

  myPtsMaskPoints = vtkMaskPoints::New();
  myPtsMaskPoints->SetInput(myPointsNumDataSet);
  myPtsMaskPoints->SetOnRatio(1);

  myPtsSelectVisiblePoints = vtkSelectVisiblePoints::New();
  myPtsSelectVisiblePoints->SetInput(myPtsMaskPoints->GetOutput());
  myPtsSelectVisiblePoints->SelectInvisibleOff();
  myPtsSelectVisiblePoints->SetTolerance(0.1);
    
  myPtsLabeledDataMapper = vtkLabeledDataMapper::New();
  myPtsLabeledDataMapper->SetInput(myPtsSelectVisiblePoints->GetOutput());
  myPtsLabeledDataMapper->SetLabelFormat("%g");
  myPtsLabeledDataMapper->SetLabelModeToLabelScalars();
    
  vtkTextProperty* aPtsTextProp = vtkTextProperty::New();
  aPtsTextProp->SetFontFamilyToTimes();
  static int aPointsFontSize = 10;
  aPtsTextProp->SetFontSize(aPointsFontSize);
  aPtsTextProp->SetBold(1);
  aPtsTextProp->SetItalic(0);
  aPtsTextProp->SetShadow(0);
  myPtsLabeledDataMapper->SetLabelTextProperty(aPtsTextProp);
  aPtsTextProp->Delete();
  
  myEntityMode = eAllEntity;

  myIsPointsLabeled = false;

  myPointLabels = vtkActor2D::New();
  myPointLabels->SetMapper(myPtsLabeledDataMapper);
  myPointLabels->GetProperty()->SetColor(1,1,1);
  myPointLabels->SetVisibility(myIsPointsLabeled);


  //Definition of cells numbering pipeline
  //---------------------------------------
  myCellsNumDataSet = vtkUnstructuredGrid::New();

  myCellCenters = vtkCellCenters::New();
  myCellCenters->SetInput(myCellsNumDataSet);

  myClsMaskPoints = vtkMaskPoints::New();
  myClsMaskPoints->SetInput(myCellCenters->GetOutput());
  myClsMaskPoints->SetOnRatio(1);
    
  myClsSelectVisiblePoints = vtkSelectVisiblePoints::New();
  myClsSelectVisiblePoints->SetInput(myClsMaskPoints->GetOutput());
  myClsSelectVisiblePoints->SelectInvisibleOff();
  myClsSelectVisiblePoints->SetTolerance(0.1);
    
  myClsLabeledDataMapper = vtkLabeledDataMapper::New();
  myClsLabeledDataMapper->SetInput(myClsSelectVisiblePoints->GetOutput());
  myClsLabeledDataMapper->SetLabelFormat("%g");
  myClsLabeledDataMapper->SetLabelModeToLabelScalars();
    
  vtkTextProperty* aClsTextProp = vtkTextProperty::New();
  aClsTextProp->SetFontFamilyToTimes();
  static int aCellsFontSize = 12;
  aClsTextProp->SetFontSize(aCellsFontSize);
  aClsTextProp->SetBold(1);
  aClsTextProp->SetItalic(0);
  aClsTextProp->SetShadow(0);
  myClsLabeledDataMapper->SetLabelTextProperty(aClsTextProp);
  aClsTextProp->Delete();
    
  myIsCellsLabeled = false;

  myCellsLabels = vtkActor2D::New();
  myCellsLabels->SetMapper(myClsLabeledDataMapper);
  myCellsLabels->GetProperty()->SetColor(0,1,0);
  myCellsLabels->SetVisibility(myIsCellsLabeled);

  // Clipping planes
  myImplicitBoolean = vtkImplicitBoolean::New();
  myImplicitBoolean->SetOperationTypeToIntersection();
}


SMESH_ActorDef::~SMESH_ActorDef()
{
  if(MYDEBUG) MESSAGE("~SMESH_ActorDef - "<<this);

  myScalarBarActor->Delete();
  myLookupTable->Delete();

  mySurfaceProp->Delete();
  myBackSurfaceProp->Delete();

  myEdgeProp->Delete();
  myHighlightProp->Delete();
  myPreselectProp->Delete();

  myNodeProp->Delete();

  my1DProp->Delete();
  my1DActor->Delete();

  my1DExtProp->Delete();
  my1DExtActor->Delete();

  my2DActor->Delete();
  my3DActor->Delete();

  myNodeActor->Delete();
  myBaseActor->Delete();

  myHighlitableActor->Delete();

  //Deleting of pints numbering pipeline
  //---------------------------------------
  myPointsNumDataSet->Delete();

  myPtsLabeledDataMapper->RemoveAllInputs();
  myPtsLabeledDataMapper->Delete();

  myPtsSelectVisiblePoints->UnRegisterAllOutputs();
  myPtsSelectVisiblePoints->Delete();

  myPtsMaskPoints->UnRegisterAllOutputs();
  myPtsMaskPoints->Delete();

  myPointLabels->Delete();


  //Deleting of cells numbering pipeline
  //---------------------------------------
  myCellsNumDataSet->Delete();

  myClsLabeledDataMapper->RemoveAllInputs();
  myClsLabeledDataMapper->Delete();

  myClsSelectVisiblePoints->UnRegisterAllOutputs();
  myClsSelectVisiblePoints->Delete();

  myClsMaskPoints->UnRegisterAllOutputs();
  myClsMaskPoints->Delete();

  myCellCenters->UnRegisterAllOutputs();
  myCellCenters->Delete();

  myCellsLabels->Delete();

  myImplicitBoolean->Delete();

  myTimeStamp->Delete();
}


void SMESH_ActorDef::SetPointsLabeled( bool theIsPointsLabeled )
{
  vtkUnstructuredGrid* aGrid = GetUnstructuredGrid();
  myIsPointsLabeled = theIsPointsLabeled && aGrid->GetNumberOfPoints();

  if ( myIsPointsLabeled )
  {
    myPointsNumDataSet->ShallowCopy(aGrid);
    vtkDataSet *aDataSet = myPointsNumDataSet;
    
    int aNbElem = aDataSet->GetNumberOfPoints();
    
    vtkIntArray *anArray = vtkIntArray::New();
    anArray->SetNumberOfValues( aNbElem );
    
    for ( int anId = 0; anId < aNbElem; anId++ )
    {
      int aSMDSId = myVisualObj->GetNodeObjId( anId );
      anArray->SetValue( anId, aSMDSId );
    }
    
    aDataSet->GetPointData()->SetScalars( anArray );
    anArray->Delete();
    myPtsMaskPoints->SetInput( aDataSet );
    myPointLabels->SetVisibility( GetVisibility() );
  }
  else
  {
    myPointLabels->SetVisibility( false );
  }
  SetRepresentation(GetRepresentation());
  myTimeStamp->Modified();
}


void SMESH_ActorDef::SetCellsLabeled(bool theIsCellsLabeled)
{
  vtkUnstructuredGrid* aGrid = GetUnstructuredGrid();
  myIsCellsLabeled = theIsCellsLabeled && aGrid->GetNumberOfPoints();
  if(myIsCellsLabeled){
    myCellsNumDataSet->ShallowCopy(aGrid);
    vtkDataSet *aDataSet = myCellsNumDataSet;
    int aNbElem = aDataSet->GetNumberOfCells();
    vtkIntArray *anArray = vtkIntArray::New();
    anArray->SetNumberOfValues(aNbElem);
    for(int anId = 0; anId < aNbElem; anId++){
      int aSMDSId = myVisualObj->GetElemObjId(anId);
      anArray->SetValue(anId,aSMDSId);
    }
    aDataSet->GetCellData()->SetScalars(anArray);
    myCellCenters->SetInput(aDataSet);
    myCellsLabels->SetVisibility(GetVisibility());
  }else{
    myCellsLabels->SetVisibility(false);
  }
  myTimeStamp->Modified();
}


void 
SMESH_ActorDef::
SetControlMode(eControl theMode)
{
  SetControlMode(theMode,true);
}


void 
SMESH_ActorDef::
SetControlMode(eControl theMode,
	       bool theCheckEntityMode)
{
  SUIT_ResourceMgr* mgr = SUIT_Session::session()->resourceMgr();  
  if( !mgr )
    return;

  myControlMode = eNone;
  theCheckEntityMode &= mgr->booleanValue( "SMESH", "display_entity", false );

  my1DActor->GetMapper()->SetScalarVisibility(false);
  my2DActor->GetMapper()->SetScalarVisibility(false);
  my3DActor->GetMapper()->SetScalarVisibility(false);
  myScalarBarActor->SetVisibility(false);

  bool anIsScalarVisible = theMode > eNone;

  if(anIsScalarVisible){
    SMESH::Controls::FunctorPtr aFunctor;
    switch(theMode){
    case eLength:
    {
      SMESH::Controls::Length* aControl = new SMESH::Controls::Length();
      aControl->SetPrecision( myControlsPrecision );
      aFunctor.reset( aControl );
      myControlActor = my1DActor;
      break;
    }
    case eLength2D:
    {
      aFunctor.reset(new SMESH::Controls::Length2D());
      myControlActor = my2DActor;
      break;
    }
    case eFreeBorders:
      aFunctor.reset(new SMESH::Controls::FreeBorders());
      myControlActor = my1DActor;
      break;
    case eFreeEdges:
      aFunctor.reset(new SMESH::Controls::FreeEdges());
      myControlActor = my2DActor;
      break;
    case eMultiConnection:
      aFunctor.reset(new SMESH::Controls::MultiConnection());
      myControlActor = my1DActor;
      break;
    case eMultiConnection2D:
      aFunctor.reset(new SMESH::Controls::MultiConnection2D());
      myControlActor = my2DActor;
      break;
    case eArea:
    {
      SMESH::Controls::Area* aControl = new SMESH::Controls::Area();
      aControl->SetPrecision( myControlsPrecision );
      aFunctor.reset( aControl );
      myControlActor = my2DActor;
      break;
    }
    case eTaper:
    {
      SMESH::Controls::Taper* aControl = new SMESH::Controls::Taper();
      aControl->SetPrecision( myControlsPrecision );
      aFunctor.reset( aControl );
      myControlActor = my2DActor;
      break;
    }
    case eAspectRatio:
    {
      SMESH::Controls::AspectRatio* aControl = new SMESH::Controls::AspectRatio();
      aControl->SetPrecision( myControlsPrecision );
      aFunctor.reset( aControl );
      myControlActor = my2DActor;
      break;
    }
    case eAspectRatio3D:
    {
      SMESH::Controls::AspectRatio3D* aControl = new SMESH::Controls::AspectRatio3D();
      aControl->SetPrecision( myControlsPrecision );
      aFunctor.reset( aControl );
      myControlActor = my3DActor;
      break;
    }
    case eVolume3D:
    {
      SMESH::Controls::Volume* aControl = new SMESH::Controls::Volume();
      aControl->SetPrecision( myControlsPrecision );
      aFunctor.reset( aControl );
      myControlActor = my3DActor;
      break;
    }
    case eMinimumAngle:
    {
      SMESH::Controls::MinimumAngle* aControl = new SMESH::Controls::MinimumAngle();
      aControl->SetPrecision( myControlsPrecision );
      aFunctor.reset( aControl );
      myControlActor = my2DActor;
      break;
    }
    case eWarping:
    {
      SMESH::Controls::Warping* aControl = new SMESH::Controls::Warping();
      aControl->SetPrecision( myControlsPrecision );
      aFunctor.reset( aControl );
      myControlActor = my2DActor;
      break;
    }
    case eSkew:
    {
      SMESH::Controls::Skew* aControl = new SMESH::Controls::Skew();
      aControl->SetPrecision( myControlsPrecision );
      aFunctor.reset( aControl );
      myControlActor = my2DActor;
      break;
    }
    default:
      return;
    }

    vtkUnstructuredGrid* aGrid = myControlActor->GetUnstructuredGrid();
    vtkIdType aNbCells = aGrid->GetNumberOfCells();
    if(aNbCells){
      myControlMode = theMode;
      switch(myControlMode){
      case eFreeEdges:
      case eFreeBorders:
	my1DExtActor->SetExtControlMode(aFunctor);
	break;
      case eLength2D:
      case eMultiConnection2D:
	my1DExtActor->SetExtControlMode(aFunctor,myScalarBarActor,myLookupTable);
	break;
      default:
	myControlActor->SetControlMode(aFunctor,myScalarBarActor,myLookupTable);
      }
    }

    if(theCheckEntityMode){
      if(myControlActor == my1DActor)
	SetEntityMode(eEdges);
      else if(myControlActor == my2DActor){
	switch(myControlMode){
	case eLength2D:
	case eFreeEdges:
	case eMultiConnection2D:
	  //SetEntityMode(eEdges);
	  SetEntityMode(eFaces);
	  break;
	default:
	  SetEntityMode(eFaces);
	}
      }else if(myControlActor == my3DActor)
	SetEntityMode(eVolumes);
    }

  }else if(theCheckEntityMode){
    myEntityMode = eAllEntity;
  }

  SetRepresentation(GetRepresentation());

  myTimeStamp->Modified();
  Modified();
}


void SMESH_ActorDef::AddToRender(vtkRenderer* theRenderer){
  SALOME_Actor::AddToRender(theRenderer);

  theRenderer->AddActor(myNodeActor);
  theRenderer->AddActor(myBaseActor);

  theRenderer->AddActor(my3DActor);
  theRenderer->AddActor(my2DActor);

  theRenderer->AddActor(my1DActor);
  theRenderer->AddActor(my1DExtActor);

  theRenderer->AddActor(myHighlitableActor);

  theRenderer->AddActor2D(myScalarBarActor);

  myPtsSelectVisiblePoints->SetRenderer(theRenderer);
  myClsSelectVisiblePoints->SetRenderer(theRenderer);

  theRenderer->AddActor2D(myPointLabels);
  theRenderer->AddActor2D(myCellsLabels);
}

void SMESH_ActorDef::RemoveFromRender(vtkRenderer* theRenderer){
  SALOME_Actor::RemoveFromRender(theRenderer);

  theRenderer->RemoveActor(myNodeActor);
  theRenderer->RemoveActor(myBaseActor);

  theRenderer->RemoveActor(myHighlitableActor);

  theRenderer->RemoveActor(my1DActor);
  theRenderer->RemoveActor(my1DExtActor);

  theRenderer->RemoveActor(my2DActor);
  theRenderer->RemoveActor(my3DActor);

  theRenderer->RemoveActor(myScalarBarActor);
  theRenderer->RemoveActor(myPointLabels);
  theRenderer->RemoveActor(myCellsLabels);
}


bool SMESH_ActorDef::Init(TVisualObjPtr theVisualObj, 
			  const char* theEntry, 
			  const char* theName,
			  int theIsClear)
{
  Handle(SALOME_InteractiveObject) anIO = new SALOME_InteractiveObject(theEntry,"SMESH",theName);
  setIO(anIO);
  setName(theName);

  myVisualObj = theVisualObj;
  myVisualObj->Update(theIsClear);

  myNodeActor->Init(myVisualObj,myImplicitBoolean);
  myBaseActor->Init(myVisualObj,myImplicitBoolean);
  
  myHighlitableActor->Init(myVisualObj,myImplicitBoolean);
  
  my1DActor->Init(myVisualObj,myImplicitBoolean);
  my1DExtActor->Init(myVisualObj,myImplicitBoolean);
  
  my2DActor->Init(myVisualObj,myImplicitBoolean);
  my3DActor->Init(myVisualObj,myImplicitBoolean);
  
  my1DActor->GetMapper()->SetLookupTable(myLookupTable);
  my1DExtActor->GetMapper()->SetLookupTable(myLookupTable);

  my2DActor->GetMapper()->SetLookupTable(myLookupTable);
  my3DActor->GetMapper()->SetLookupTable(myLookupTable);
    
  vtkFloatingPointType aFactor, aUnits;
  my2DActor->GetPolygonOffsetParameters(aFactor,aUnits);
  my2DActor->SetPolygonOffsetParameters(aFactor,aUnits*0.75);

  SUIT_ResourceMgr* mgr = SUIT_Session::session()->resourceMgr();
  if( !mgr )
    return false;

  //SetIsShrunkable(theGrid->GetNumberOfCells() > 10);
  SetIsShrunkable(true);

  SetShrinkFactor( SMESH::GetFloat( "SMESH:shrink_coeff", 0.75 ) );

  int aMode = mgr->integerValue( "SMESH", "display_mode" );
  SetRepresentation(-1);
  
  if(aMode == 0){
    SetRepresentation(eEdge);
  }else if(aMode == 1){
    SetRepresentation(eSurface);
  }else if(aMode == 2){
    SetRepresentation(ePoint);
  }
  
  if(aMode == 3){
    SetShrink();
  }

  myTimeStamp->Modified();
  Modified();
  return true;
}


vtkFloatingPointType* SMESH_ActorDef::GetBounds(){
  return myNodeActor->GetBounds();
}


vtkDataSet* SMESH_ActorDef::GetInput(){
  return GetUnstructuredGrid();
}


void SMESH_ActorDef::SetTransform(VTKViewer_Transform* theTransform){
  Superclass::SetTransform(theTransform);

  myNodeActor->SetTransform(theTransform);
  myBaseActor->SetTransform(theTransform);

  myHighlitableActor->SetTransform(theTransform);

  my1DActor->SetTransform(theTransform);
  my1DExtActor->SetTransform(theTransform);

  my2DActor->SetTransform(theTransform);
  my3DActor->SetTransform(theTransform);

  Modified();
}


void SMESH_ActorDef::SetMapper(vtkMapper* theMapper){
  vtkLODActor::SetMapper(theMapper);
}


void SMESH_ActorDef::ShallowCopy(vtkProp *prop){
  SALOME_Actor::ShallowCopy(prop);
}


vtkMapper* SMESH_ActorDef::GetMapper(){
  return myPickableActor->GetMapper();
}


vtkUnstructuredGrid* SMESH_ActorDef::GetUnstructuredGrid(){ 
  return myVisualObj->GetUnstructuredGrid();
}


bool SMESH_ActorDef::IsInfinitive(){
  vtkDataSet *aDataSet = myPickableActor->GetUnstructuredGrid();
  aDataSet->Update();
  myIsInfinite = aDataSet->GetNumberOfCells() == 0 ||
    aDataSet->GetNumberOfCells() == 1 && 
    aDataSet->GetCell(0)->GetCellType() == VTK_VERTEX;
  return SALOME_Actor::IsInfinitive();
}


void SMESH_ActorDef::SetIsShrunkable(bool theShrunkable){
  if ( myIsShrinkable == theShrunkable )
    return;
  myIsShrinkable = theShrunkable;
  Modified();
}

vtkFloatingPointType SMESH_ActorDef::GetShrinkFactor(){
  return myBaseActor->GetShrinkFactor();
}

void SMESH_ActorDef::SetShrinkFactor(vtkFloatingPointType theValue){
  myBaseActor->SetShrinkFactor(theValue);

  my1DActor->SetShrinkFactor(theValue);
  my1DExtActor->SetShrinkFactor(theValue);

  my2DActor->SetShrinkFactor(theValue);
  my3DActor->SetShrinkFactor(theValue);

  Modified();
}

void SMESH_ActorDef::SetShrink(){
  if(!myIsShrinkable) return;

  myBaseActor->SetShrink();

  my1DActor->SetShrink();
  my1DExtActor->SetShrink();

  my2DActor->SetShrink();
  my3DActor->SetShrink();

  myIsShrunk = true;
  Modified();
}

void SMESH_ActorDef::UnShrink(){
  if(!myIsShrunk) return;

  myBaseActor->UnShrink();

  my1DActor->UnShrink();
  my1DExtActor->UnShrink();

  my2DActor->UnShrink();
  my3DActor->UnShrink();

  myIsShrunk = false;
  Modified();
}


int SMESH_ActorDef::GetNodeObjId(int theVtkID){
  return myPickableActor->GetNodeObjId(theVtkID);
}

vtkFloatingPointType* SMESH_ActorDef::GetNodeCoord(int theObjID){
  return myPickableActor->GetNodeCoord(theObjID);
}


int SMESH_ActorDef::GetElemObjId(int theVtkID){
  return myPickableActor->GetElemObjId(theVtkID);
}

vtkCell* SMESH_ActorDef::GetElemCell(int theObjID){
  return myPickableActor->GetElemCell(theObjID);
}


void SMESH_ActorDef::SetVisibility(int theMode){
  SetVisibility(theMode,true);
}


void SMESH_ActorDef::SetVisibility(int theMode, bool theIsUpdateRepersentation){
  SALOME_Actor::SetVisibility(theMode);

  myNodeActor->VisibilityOff();
  myBaseActor->VisibilityOff();
  
  my1DActor->VisibilityOff();
  my1DExtActor->VisibilityOff();
  
  my2DActor->VisibilityOff();
  my3DActor->VisibilityOff();
  
  myScalarBarActor->VisibilityOff();
  myPointLabels->VisibilityOff();
  myCellsLabels->VisibilityOff();
  
  if(GetVisibility()){
    if(theIsUpdateRepersentation)
      SetRepresentation(GetRepresentation());

    if(myControlMode != eNone){
      switch(myControlMode){
      case eFreeEdges:
      case eFreeBorders:
	my1DExtActor->VisibilityOn();
	break;
      case eLength2D:
      case eMultiConnection2D:
	my1DExtActor->VisibilityOn();
      default:
	if(myControlActor->GetUnstructuredGrid()->GetNumberOfCells())
	  myScalarBarActor->VisibilityOn();
      }
    }

    if(myRepresentation != ePoint)
      myPickableActor->VisibilityOn();
    else {
      myNodeActor->VisibilityOn();
    }

    if(myEntityMode & eEdges){
      my1DActor->VisibilityOn();
    }
    
    if(myEntityMode & eFaces){
      my2DActor->VisibilityOn();
    }
    
    if(myEntityMode & eVolumes){
      my3DActor->VisibilityOn();
    }
    
    if(myIsPointsLabeled){ 
      myPointLabels->VisibilityOn();
      myNodeActor->VisibilityOn();
    }

    if(myIsCellsLabeled) 
      myCellsLabels->VisibilityOn();
  }

  Modified();
}


void SMESH_ActorDef::SetEntityMode(unsigned int theMode){
  myEntityState = eAllEntity;

  if(!myVisualObj->GetNbEntities(SMDSAbs_Edge)){
    myEntityState &= ~eEdges;
    theMode &= ~eEdges;
  }

  if(!myVisualObj->GetNbEntities(SMDSAbs_Face)){
    myEntityState &= ~eFaces;
    theMode &= ~eFaces;
  }

  if(!myVisualObj->GetNbEntities(SMDSAbs_Volume)){
    myEntityState &= ~eVolumes;
    theMode &= ~eVolumes;
  }

  if(!theMode){
    if(myVisualObj->GetNbEntities(SMDSAbs_Edge))
      theMode |= eEdges;

    if(myVisualObj->GetNbEntities(SMDSAbs_Face))
      theMode |= eFaces;

    if(myVisualObj->GetNbEntities(SMDSAbs_Volume))
      theMode |= eVolumes;
  }

  myBaseActor->myGeomFilter->SetInside(myEntityMode != myEntityState);

  myEntityMode = theMode;
  VTKViewer_ExtractUnstructuredGrid* aFilter = NULL;
  aFilter = myBaseActor->GetExtractUnstructuredGrid();
  aFilter->ClearRegisteredCellsWithType();
  aFilter->SetModeOfChanging(VTKViewer_ExtractUnstructuredGrid::eAdding);
  
  if(myEntityMode & eEdges){
    if (MYDEBUG) MESSAGE("EDGES");
    aFilter->RegisterCellsWithType(VTK_LINE);
    aFilter->RegisterCellsWithType(VTK_QUADRATIC_EDGE);
  }

  if(myEntityMode & eFaces){
    if (MYDEBUG) MESSAGE("FACES");
    aFilter->RegisterCellsWithType(VTK_TRIANGLE);
    aFilter->RegisterCellsWithType(VTK_POLYGON);
    aFilter->RegisterCellsWithType(VTK_QUAD);
    aFilter->RegisterCellsWithType(VTK_QUADRATIC_TRIANGLE);
    aFilter->RegisterCellsWithType(VTK_QUADRATIC_QUAD);
  }

  if(myEntityMode & eVolumes){
    if (MYDEBUG) MESSAGE("VOLUMES");
    aFilter->RegisterCellsWithType(VTK_TETRA);
    aFilter->RegisterCellsWithType(VTK_VOXEL);
    aFilter->RegisterCellsWithType(VTK_HEXAHEDRON);
    aFilter->RegisterCellsWithType(VTK_WEDGE);
    aFilter->RegisterCellsWithType(VTK_PYRAMID);
    aFilter->RegisterCellsWithType(VTK_QUADRATIC_TETRA);
    aFilter->RegisterCellsWithType(VTK_QUADRATIC_HEXAHEDRON);
    aFilter->RegisterCellsWithType(VTK_CONVEX_POINT_SET);
  }
  aFilter->Update();
  if (MYDEBUG) MESSAGE(aFilter->GetOutput()->GetNumberOfCells());
  SetVisibility(GetVisibility(),false);
}

void SMESH_ActorDef::SetRepresentation(int theMode){ 
  int aNbEdges = myVisualObj->GetNbEntities(SMDSAbs_Edge);
  int aNbFaces = myVisualObj->GetNbEntities(SMDSAbs_Face);
  int aNbVolumes = myVisualObj->GetNbEntities(SMDSAbs_Volume);
  if(theMode < 0){
    myRepresentation = eSurface;
    if(!aNbFaces && !aNbVolumes && aNbEdges){
      myRepresentation = eEdge;
    }else if(!aNbFaces && !aNbVolumes && !aNbEdges){
      myRepresentation = ePoint;
    }
  }else{
    switch(theMode){
    case eEdge:
      if(!aNbFaces && !aNbVolumes && !aNbEdges) return;
      break;
    case eSurface:
      if(!aNbFaces && !aNbVolumes) return;
      break;
    }    
    myRepresentation = theMode;
  }

  if(!GetUnstructuredGrid()->GetNumberOfCells())
    myRepresentation = ePoint;

  if(myIsShrunk){
    if(myRepresentation == ePoint){
      UnShrink();
      myIsShrunk = true;
    }else{
      SetShrink();
    }      
  }

  myPickableActor = myBaseActor;
  myNodeActor->SetVisibility(false);
  vtkProperty *aProp = NULL, *aBackProp = NULL;
  SMESH_DeviceActor::EReperesent aReperesent = SMESH_DeviceActor::EReperesent(-1);
  switch(myRepresentation){
  case ePoint:
    myPickableActor = myNodeActor;
    myNodeActor->SetVisibility(true);
    
    aProp = aBackProp = myNodeProp;
    aReperesent = SMESH_DeviceActor::ePoint;
    break;
  case eEdge:
    aProp = aBackProp = myEdgeProp;
    aReperesent = SMESH_DeviceActor::eInsideframe;
    break;
  case eSurface:
    aProp = mySurfaceProp;
    aBackProp = myBackSurfaceProp;
    aReperesent = SMESH_DeviceActor::eSurface;
    break;
  }    

  my2DActor->SetProperty(aProp);
  my2DActor->SetBackfaceProperty(aBackProp);
  my2DActor->SetRepresentation(aReperesent);
  
  my3DActor->SetProperty(aProp);
  my3DActor->SetBackfaceProperty(aBackProp);
  my3DActor->SetRepresentation(aReperesent);

  my1DExtActor->SetVisibility(false);

  switch(myControlMode){
  case eLength:
  case eMultiConnection:
    aProp = aBackProp = my1DProp;
    if(myRepresentation != ePoint)
      aReperesent = SMESH_DeviceActor::eInsideframe;
    break;
  }
  
  my1DActor->SetProperty(aProp);
  my1DActor->SetBackfaceProperty(aBackProp);
  my1DActor->SetRepresentation(aReperesent);

  my1DExtActor->SetRepresentation(aReperesent);
  
  if(myIsPointsVisible)
    myPickableActor = myNodeActor;
  if(GetPointRepresentation())
    myNodeActor->SetVisibility(true);

  SetMapper(myPickableActor->GetMapper());

  SetVisibility(GetVisibility(),false);

  Modified();
}


void SMESH_ActorDef::SetPointRepresentation(bool theIsPointsVisible){
  if ( myIsPointsVisible == theIsPointsVisible )
    return;
  myIsPointsVisible = theIsPointsVisible;
  SetRepresentation(GetRepresentation());
}

bool SMESH_ActorDef::GetPointRepresentation(){ 
  return myIsPointsVisible || myIsPointsLabeled;
}


void SMESH_ActorDef::UpdateHighlight(){
  myHighlitableActor->SetVisibility(false);
  myHighlitableActor->SetHighlited(false);

  if(myIsHighlighted){
    myHighlitableActor->SetProperty(myHighlightProp);
  }else if(myIsPreselected){
    myHighlitableActor->SetProperty(myPreselectProp);
  }

  bool anIsVisible = GetVisibility();

  if(myIsHighlighted || myIsPreselected){
    if(GetUnstructuredGrid()->GetNumberOfCells()){
      myHighlitableActor->SetHighlited(anIsVisible);
      myHighlitableActor->SetVisibility(anIsVisible);
      myHighlitableActor->GetExtractUnstructuredGrid()->
	SetModeOfExtraction(VTKViewer_ExtractUnstructuredGrid::eCells);
      myHighlitableActor->SetRepresentation(SMESH_DeviceActor::eWireframe);
    }else if(myRepresentation == ePoint || GetPointRepresentation()){
      myHighlitableActor->SetHighlited(anIsVisible);
      myHighlitableActor->SetVisibility(anIsVisible);
      myHighlitableActor->GetExtractUnstructuredGrid()->
	SetModeOfExtraction(VTKViewer_ExtractUnstructuredGrid::ePoints);
      myHighlitableActor->SetRepresentation(SMESH_DeviceActor::ePoint);
    }
  }
}


void SMESH_ActorDef::highlight(bool theHighlight){
  if ( myIsHighlighted == theHighlight )
    return;
  myIsHighlighted = theHighlight;
  UpdateHighlight();
}


void SMESH_ActorDef::SetPreSelected(bool thePreselect){ 
  if ( myIsPreselected == thePreselect )
    return;
  myIsPreselected = thePreselect; 
  UpdateHighlight();
}


// From vtkFollower
int SMESH_ActorDef::RenderOpaqueGeometry(vtkViewport *vp)
{
  if (myPickableActor->GetIsOpaque())
    {
    vtkRenderer *ren = static_cast<vtkRenderer *>(vp);
    this->Render(ren);
    return 1;
    }
  return 0;
}


int SMESH_ActorDef::RenderTranslucentGeometry(vtkViewport *vp)
{
  if (!myPickableActor->GetIsOpaque())
    {
    vtkRenderer *ren = static_cast<vtkRenderer *>(vp);
    this->Render(ren);
    return 1;
    }
  return 0;
}


void SMESH_ActorDef::Render(vtkRenderer *ren){
  unsigned long aTime = myTimeStamp->GetMTime();
  unsigned long anObjTime = myVisualObj->GetUnstructuredGrid()->GetMTime();
  unsigned long aClippingTime = myImplicitBoolean->GetMTime();
  if(anObjTime > aTime || aClippingTime > aTime)
    Update();
}


void SMESH_ActorDef::Update(){
  if(MYDEBUG) MESSAGE("SMESH_ActorDef::Update");

  if(GetControlMode() != eNone) {
    unsigned long aTime = myTimeStamp->GetMTime();
    unsigned long anObjTime = myVisualObj->GetUnstructuredGrid()->GetMTime();
    if (anObjTime > aTime)
      SetControlMode(GetControlMode(),false);
  }
  if(myIsPointsLabeled){
    SetPointsLabeled(myIsPointsLabeled);
  }
  if(myIsCellsLabeled){
    SetCellsLabeled(myIsCellsLabeled);
  }
  SetEntityMode(GetEntityMode());
  SetVisibility(GetVisibility());
  
  myTimeStamp->Modified();
  Modified();
}


void SMESH_ActorDef::ReleaseGraphicsResources(vtkWindow *renWin){
  SALOME_Actor::ReleaseGraphicsResources(renWin);

  myPickableActor->ReleaseGraphicsResources(renWin);
}


static void GetColor(vtkProperty *theProperty, vtkFloatingPointType& r,vtkFloatingPointType& g,vtkFloatingPointType& b){
  vtkFloatingPointType* aColor = theProperty->GetColor();
  r = aColor[0];
  g = aColor[1];
  b = aColor[2];
}


void SMESH_ActorDef::SetOpacity(vtkFloatingPointType theValue){
  mySurfaceProp->SetOpacity(theValue);
  myBackSurfaceProp->SetOpacity(theValue);
  myEdgeProp->SetOpacity(theValue);
  myNodeProp->SetOpacity(theValue);

  my1DProp->SetOpacity(theValue);
}


vtkFloatingPointType SMESH_ActorDef::GetOpacity(){
  return mySurfaceProp->GetOpacity();
}


void SMESH_ActorDef::SetSufaceColor(vtkFloatingPointType r,vtkFloatingPointType g,vtkFloatingPointType b){
  mySurfaceProp->SetColor(r,g,b);
  Modified();
}

void SMESH_ActorDef::GetSufaceColor(vtkFloatingPointType& r,vtkFloatingPointType& g,vtkFloatingPointType& b){
  ::GetColor(mySurfaceProp,r,g,b);
}

void SMESH_ActorDef::SetBackSufaceColor(vtkFloatingPointType r,vtkFloatingPointType g,vtkFloatingPointType b){
  myBackSurfaceProp->SetColor(r,g,b);
  Modified();
}

void SMESH_ActorDef::GetBackSufaceColor(vtkFloatingPointType& r,vtkFloatingPointType& g,vtkFloatingPointType& b){
  ::GetColor(myBackSurfaceProp,r,g,b);
}

void SMESH_ActorDef::SetEdgeColor(vtkFloatingPointType r,vtkFloatingPointType g,vtkFloatingPointType b){
  myEdgeProp->SetColor(r,g,b);
  my1DProp->SetColor(r,g,b);
  my1DExtProp->SetColor(1.0-r,1.0-g,1.0-b);
  Modified();
}

void SMESH_ActorDef::GetEdgeColor(vtkFloatingPointType& r,vtkFloatingPointType& g,vtkFloatingPointType& b){
  ::GetColor(myEdgeProp,r,g,b);
}

void SMESH_ActorDef::SetNodeColor(vtkFloatingPointType r,vtkFloatingPointType g,vtkFloatingPointType b){ 
  myNodeProp->SetColor(r,g,b);
  Modified();
}

void SMESH_ActorDef::GetNodeColor(vtkFloatingPointType& r,vtkFloatingPointType& g,vtkFloatingPointType& b){ 
  ::GetColor(myNodeProp,r,g,b);
}

void SMESH_ActorDef::SetHighlightColor(vtkFloatingPointType r,vtkFloatingPointType g,vtkFloatingPointType b){ 
  myHighlightProp->SetColor(r,g,b);
  Modified();
}

void SMESH_ActorDef::GetHighlightColor(vtkFloatingPointType& r,vtkFloatingPointType& g,vtkFloatingPointType& b){ 
  ::GetColor(myHighlightProp,r,g,b);
}

void SMESH_ActorDef::SetPreHighlightColor(vtkFloatingPointType r,vtkFloatingPointType g,vtkFloatingPointType b){ 
  myPreselectProp->SetColor(r,g,b);
  Modified();
}

void SMESH_ActorDef::GetPreHighlightColor(vtkFloatingPointType& r,vtkFloatingPointType& g,vtkFloatingPointType& b){ 
  ::GetColor(myPreselectProp,r,g,b);
}


vtkFloatingPointType SMESH_ActorDef::GetLineWidth(){
  return myEdgeProp->GetLineWidth();
}


void SMESH_ActorDef::SetLineWidth(vtkFloatingPointType theVal){
  myEdgeProp->SetLineWidth(theVal);

  my1DProp->SetLineWidth(theVal + aLineWidthInc);
  my1DExtProp->SetLineWidth(theVal + aLineWidthInc);

  Modified();
}


void SMESH_ActorDef::SetNodeSize(vtkFloatingPointType theVal){
  myNodeProp->SetPointSize(theVal);
  myHighlightProp->SetPointSize(theVal);
  myPreselectProp->SetPointSize(theVal);

  my1DProp->SetPointSize(theVal + aPointSizeInc);
  my1DExtProp->SetPointSize(theVal + aPointSizeInc);

  Modified();
}

vtkFloatingPointType SMESH_ActorDef::GetNodeSize(){
  return myNodeProp->GetPointSize();
}

int SMESH_ActorDef::GetObjDimension( const int theObjId )
{
  return myVisualObj->GetElemDimension( theObjId );
}

bool
SMESH_ActorDef::
IsImplicitFunctionUsed() const
{
  return myBaseActor->IsImplicitFunctionUsed();
}

void
SMESH_ActorDef::
SetImplicitFunctionUsed(bool theIsImplicitFunctionUsed)
{
  myNodeActor->SetImplicitFunctionUsed(theIsImplicitFunctionUsed);
  myBaseActor->SetImplicitFunctionUsed(theIsImplicitFunctionUsed);
  
  myHighlitableActor->SetImplicitFunctionUsed(theIsImplicitFunctionUsed);
  
  my1DActor->SetImplicitFunctionUsed(theIsImplicitFunctionUsed);
  my1DExtActor->SetImplicitFunctionUsed(theIsImplicitFunctionUsed);
  
  my2DActor->SetImplicitFunctionUsed(theIsImplicitFunctionUsed);
  my3DActor->SetImplicitFunctionUsed(theIsImplicitFunctionUsed);
}

vtkIdType 
SMESH_ActorDef::
AddClippingPlane(vtkPlane* thePlane)
{
  if(thePlane){
    myImplicitBoolean->GetFunction()->AddItem(thePlane);
    myCippingPlaneCont.push_back(thePlane);
    if(!IsImplicitFunctionUsed())
      SetImplicitFunctionUsed(true);
  }
  return myCippingPlaneCont.size();
}

void
SMESH_ActorDef::
RemoveAllClippingPlanes()
{
  myImplicitBoolean->GetFunction()->RemoveAllItems();
  myImplicitBoolean->GetFunction()->Modified(); // VTK bug
  myCippingPlaneCont.clear();
  SetImplicitFunctionUsed(false);
}

vtkIdType
SMESH_ActorDef::
GetNumberOfClippingPlanes()
{
  return myCippingPlaneCont.size();
}

vtkPlane* 
SMESH_ActorDef::
GetClippingPlane(vtkIdType theID)
{
  if(theID >= myCippingPlaneCont.size())
    return NULL;
  return myCippingPlaneCont[theID].Get();
}


static void ComputeBoundsParam(vtkDataSet* theDataSet,
			       vtkFloatingPointType theDirection[3], vtkFloatingPointType theMinPnt[3],
			       vtkFloatingPointType& theMaxBoundPrj, vtkFloatingPointType& theMinBoundPrj)
{
  vtkFloatingPointType aBounds[6];
  theDataSet->GetBounds(aBounds);

  //Enlarge bounds in order to avoid conflicts of precision
  for(int i = 0; i < 6; i += 2){
    static double EPS = 1.0E-3;
    vtkFloatingPointType aDelta = (aBounds[i+1] - aBounds[i])*EPS;
    aBounds[i] -= aDelta;
    aBounds[i+1] += aDelta;
  }

  vtkFloatingPointType aBoundPoints[8][3] = { {aBounds[0],aBounds[2],aBounds[4]},
			       {aBounds[1],aBounds[2],aBounds[4]},
			       {aBounds[0],aBounds[3],aBounds[4]},
			       {aBounds[1],aBounds[3],aBounds[4]},
			       {aBounds[0],aBounds[2],aBounds[5]},
			       {aBounds[1],aBounds[2],aBounds[5]}, 
			       {aBounds[0],aBounds[3],aBounds[5]}, 
			       {aBounds[1],aBounds[3],aBounds[5]}};

  int aMaxId = 0, aMinId = aMaxId;
  theMaxBoundPrj = vtkMath::Dot(theDirection,aBoundPoints[aMaxId]);
  theMinBoundPrj = theMaxBoundPrj;
  for(int i = 1; i < 8; i++){
    vtkFloatingPointType aTmp = vtkMath::Dot(theDirection,aBoundPoints[i]);
    if(theMaxBoundPrj < aTmp){
      theMaxBoundPrj = aTmp;
      aMaxId = i;
    }
    if(theMinBoundPrj > aTmp){
      theMinBoundPrj = aTmp;
      aMinId = i;
    }
  }
  vtkFloatingPointType *aMinPnt = aBoundPoints[aMaxId];
  theMinPnt[0] = aMinPnt[0];
  theMinPnt[1] = aMinPnt[1];
  theMinPnt[2] = aMinPnt[2];
}


static void DistanceToPosition(vtkDataSet* theDataSet,
			       vtkFloatingPointType theDirection[3], vtkFloatingPointType theDist, vtkFloatingPointType thePos[3])
{
  vtkFloatingPointType aMaxBoundPrj, aMinBoundPrj, aMinPnt[3];
  ComputeBoundsParam(theDataSet,theDirection,aMinPnt,aMaxBoundPrj,aMinBoundPrj);
  vtkFloatingPointType aLength = (aMaxBoundPrj-aMinBoundPrj)*theDist;
  thePos[0] = aMinPnt[0]-theDirection[0]*aLength;
  thePos[1] = aMinPnt[1]-theDirection[1]*aLength;
  thePos[2] = aMinPnt[2]-theDirection[2]*aLength;
}


static void PositionToDistance(vtkDataSet* theDataSet, 
			       vtkFloatingPointType theDirection[3], vtkFloatingPointType thePos[3], vtkFloatingPointType& theDist)
{
  vtkFloatingPointType aMaxBoundPrj, aMinBoundPrj, aMinPnt[3];
  ComputeBoundsParam(theDataSet,theDirection,aMinPnt,aMaxBoundPrj,aMinBoundPrj);
  vtkFloatingPointType aPrj = vtkMath::Dot(theDirection,thePos);
  theDist = (aPrj-aMinBoundPrj)/(aMaxBoundPrj-aMinBoundPrj);
}


void SMESH_ActorDef::SetPlaneParam(vtkFloatingPointType theDir[3], vtkFloatingPointType theDist, vtkPlane* thePlane)
{
  thePlane->SetNormal(theDir);
  vtkFloatingPointType anOrigin[3];
  ::DistanceToPosition(GetUnstructuredGrid(),theDir,theDist,anOrigin);
  thePlane->SetOrigin(anOrigin);
}


void SMESH_ActorDef::GetPlaneParam(vtkFloatingPointType theDir[3], vtkFloatingPointType& theDist, vtkPlane* thePlane)
{
  thePlane->GetNormal(theDir);

  vtkFloatingPointType anOrigin[3];
  thePlane->GetOrigin(anOrigin);
  ::PositionToDistance(GetUnstructuredGrid(),theDir,anOrigin,theDist);
}

void SMESH_ActorDef::UpdateScalarBar()
{
  SUIT_ResourceMgr* mgr = SUIT_Session::session()->resourceMgr();
  if( !mgr )
    return;

  vtkTextProperty* aScalarBarTitleProp = vtkTextProperty::New();

  QColor aTColor = mgr->colorValue( "SMESH", "scalar_bar_title_color", QColor( 255, 255, 255 ) );
  aScalarBarTitleProp->SetColor( aTColor.red()/255., aTColor.green()/255., aTColor.blue()/255. );

  aScalarBarTitleProp->SetFontFamilyToArial();

  if ( mgr->hasValue( "SMESH", "scalar_bar_title_font" ) )
  {
    QFont f = mgr->fontValue( "SMESH", "scalar_bar_title_font" );
    if ( f.family() == "Arial" )
      aScalarBarTitleProp->SetFontFamilyToArial();
    else if ( f.family() == "Courier" )
      aScalarBarTitleProp->SetFontFamilyToCourier();
    else if ( f.family() == "Times" )
      aScalarBarTitleProp->SetFontFamilyToTimes();

    if ( f.bold() )
      aScalarBarTitleProp->BoldOn();
    else
      aScalarBarTitleProp->BoldOff();

    if ( f.italic() )
      aScalarBarTitleProp->ItalicOn();
    else
     aScalarBarTitleProp->ItalicOff();

    if ( f.underline() )
      aScalarBarTitleProp->ShadowOn();
    else
      aScalarBarTitleProp->ShadowOff();
  }

  myScalarBarActor->SetTitleTextProperty( aScalarBarTitleProp );
  aScalarBarTitleProp->Delete();

  vtkTextProperty* aScalarBarLabelProp = vtkTextProperty::New();

  aTColor = mgr->colorValue( "SMESH", "scalar_bar_label_color", QColor( 255, 255, 255 ) );
  aScalarBarLabelProp->SetColor( aTColor.red()/255., aTColor.green()/255., aTColor.blue()/255. );

  aScalarBarLabelProp->SetFontFamilyToArial();
  if( mgr->hasValue( "SMESH", "scalar_bar_label_font" ) )
  {
    QFont f = mgr->stringValue( "SMESH", "scalar_bar_label_font" );
    if( f.family() == "Arial" )
      aScalarBarLabelProp->SetFontFamilyToArial();
    else if( f.family() == "Courier" )
      aScalarBarLabelProp->SetFontFamilyToCourier();
    else if( f.family() == "Times" )
      aScalarBarLabelProp->SetFontFamilyToTimes();

    if ( f.bold() )
      aScalarBarLabelProp->BoldOn();
    else
      aScalarBarLabelProp->BoldOff();

    if ( f.italic() )
      aScalarBarLabelProp->ItalicOn();
    else
      aScalarBarLabelProp->ItalicOff();

    if( f.underline() )
      aScalarBarLabelProp->ShadowOn();
    else
      aScalarBarLabelProp->ShadowOff();
  }

  myScalarBarActor->SetLabelTextProperty( aScalarBarLabelProp );
  aScalarBarLabelProp->Delete();

  bool horiz = ( mgr->integerValue( "SMESH", "scalar_bar_orientation" ) == 1 );
  QString name = QString( "scalar_bar_%1_" ).arg( horiz ? "horizontal" : "vertical" );
  if( horiz )
    myScalarBarActor->SetOrientationToHorizontal();
  else
    myScalarBarActor->SetOrientationToVertical();


  vtkFloatingPointType aXVal = horiz ? 0.20 : 0.01;
  if( mgr->hasValue( "SMESH", name + "x" ) )
    aXVal = mgr->doubleValue( "SMESH", name + "x", aXVal );

  vtkFloatingPointType aYVal = horiz ? 0.01 : 0.1;
  if( mgr->hasValue( "SMESH", name + "y" ) )
    aYVal = mgr->doubleValue( "SMESH", name + "y", aYVal );
  myScalarBarActor->SetPosition( aXVal, aYVal );

  vtkFloatingPointType aWVal = horiz ? 0.60 : 0.10;
  if( mgr->hasValue( "SMESH", name + "width" ) )
    aWVal = mgr->doubleValue( "SMESH", name + "width", aWVal );
  myScalarBarActor->SetWidth( aWVal );

  vtkFloatingPointType aHVal = horiz ? 0.12 : 0.80;
  if( mgr->hasValue( "SMESH", name + "height" ) )
    aHVal = mgr->doubleValue( "SMESH", name + "height", aHVal );
  myScalarBarActor->SetHeight( aHVal );

  int anIntVal = 5;
  if( mgr->hasValue( "SMESH", "scalar_bar_num_labels" ) )
    anIntVal = mgr->integerValue( "SMESH", "scalar_bar_num_labels", anIntVal );
  myScalarBarActor->SetNumberOfLabels( anIntVal == 0 ? 5: anIntVal );

  anIntVal = 64;
  if( mgr->hasValue( "SMESH", "scalar_bar_num_colors" ) )
    anIntVal = mgr->integerValue( "SMESH", "scalar_bar_num_colors", anIntVal );
  myScalarBarActor->SetMaximumNumberOfColors( anIntVal == 0 ? 64 : anIntVal );
  
}
