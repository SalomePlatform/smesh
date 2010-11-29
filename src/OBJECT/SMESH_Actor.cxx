//  Copyright (C) 2007-2010  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

//  SMESH OBJECT : interactive object for SMESH visualization
//  File   : SMESH_Actor.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//
#include "SMESH_ActorDef.h"
#include "SMESH_ActorUtils.h"
#include "SMESH_DeviceActor.h"
#include "SMESH_ObjectDef.h"
#include "SMESH_ControlsDef.hxx"
#include "SMDS_UnstructuredGrid.hxx"
#include "SMESH_ScalarBarActor.h"
#include "VTKViewer_CellCenters.h"
#include "VTKViewer_ExtractUnstructuredGrid.h"
#include "VTKViewer_FramedTextActor.h"
#include "SALOME_InteractiveObject.hxx"

#include "SUIT_Session.h"
#include "SUIT_ResourceMgr.h"

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
#include <vtkTextProperty.h>
#include <vtkLabeledDataMapper.h>
#include <vtkSelectVisiblePoints.h>

#include <vtkLookupTable.h>

#include <vtkMath.h>
#include <vtkPlane.h>
#include <vtkImplicitBoolean.h>
#include <vtkImplicitFunctionCollection.h>

#include <vtkConfigure.h>
#if !defined(VTK_XVERSION)
#define VTK_XVERSION (VTK_MAJOR_VERSION<<16)+(VTK_MINOR_VERSION<<8)+(VTK_BUILD_VERSION)
#endif

#include "utilities.h"

#ifdef _DEBUG_
static int MYDEBUG = 1;
#else
static int MYDEBUG = 1;
#endif

static int aLineWidthInc = 2;


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

  myIsFacesOriented = false;

  myControlsPrecision = -1;
  SUIT_ResourceMgr* mgr = SUIT_Session::session()->resourceMgr();

  if ( mgr && mgr->booleanValue( "SMESH", "use_precision", false ) )
    myControlsPrecision = mgr->integerValue( "SMESH", "controls_precision", -1);

  vtkFloatingPointType aElem0DSize = SMESH::GetFloat("SMESH:elem0d_size",5);
  vtkFloatingPointType aLineWidth  = SMESH::GetFloat("SMESH:element_width",1);

  vtkMatrix4x4 *aMatrix = vtkMatrix4x4::New();
  VTKViewer_ExtractUnstructuredGrid* aFilter = NULL;

  //Definition 2D and 3D devices of the actor
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

  my2DExtProp = vtkProperty::New();
  my2DExtProp->DeepCopy(mySurfaceProp);
  SMESH::GetColor( "SMESH", "fill_color", anRGB[0], anRGB[1], anRGB[2], QColor( 0, 170, 255 ) );
  anRGB[0] = 1 - anRGB[0];
  anRGB[1] = 1 - anRGB[1];
  anRGB[2] = 1 - anRGB[2];
  my2DExtProp->SetColor(anRGB[0],anRGB[1],anRGB[2]);

  my2DExtActor = SMESH_DeviceActor::New();
  my2DExtActor->SetUserMatrix(aMatrix);
  my2DExtActor->PickableOff();
  my2DExtActor->SetProperty(my2DExtProp);
  my2DExtActor->SetBackfaceProperty(my2DExtProp);
  my2DExtActor->SetRepresentation(SMESH_DeviceActor::eInsideframe);
  aFilter = my2DExtActor->GetExtractUnstructuredGrid();
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
  aFilter->RegisterCellsWithType(VTK_QUADRATIC_WEDGE);
  aFilter->RegisterCellsWithType(VTK_QUADRATIC_PYRAMID);
  aFilter->RegisterCellsWithType(VTK_CONVEX_POINT_SET);
//#ifdef VTK_HAVE_POLYHEDRON
  MESSAGE("RegisterCellsWithType(VTK_POLYHEDRON)");
  aFilter->RegisterCellsWithType(VTK_POLYHEDRON);
//#endif

  //Definition 1D device of the actor
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
  my1DProp->SetPointSize(aElem0DSize);
  
  my1DExtProp = vtkProperty::New();
  my1DExtProp->DeepCopy(myEdgeProp);
  anRGB[0] = 1 - anRGB[0];
  anRGB[1] = 1 - anRGB[1];
  anRGB[2] = 1 - anRGB[2];
  my1DExtProp->SetColor(anRGB[0],anRGB[1],anRGB[2]);
  my1DExtProp->SetLineWidth(aLineWidth + aLineWidthInc);
  my1DExtProp->SetPointSize(aElem0DSize);

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


  //Definition 0D device of the actor (0d elements)
  //-----------------------------------------------
  my0DProp = vtkProperty::New();
  SMESH::GetColor( "SMESH", "elem0d_color", anRGB[0], anRGB[1], anRGB[2], QColor( 0, 255, 0 ) );
  my0DProp->SetColor(anRGB[0],anRGB[1],anRGB[2]);
  my0DProp->SetPointSize(aElem0DSize);

  my0DActor = SMESH_DeviceActor::New();
  my0DActor->SetUserMatrix(aMatrix);
  my0DActor->SetStoreClippingMapping(true);
  my0DActor->PickableOff();
  my0DActor->SetVisibility(false);
  my0DActor->SetProperty(my0DProp);
  my0DActor->SetRepresentation(SMESH_DeviceActor::eSurface);
  aFilter = my0DActor->GetExtractUnstructuredGrid();
  //aFilter->SetModeOfExtraction(VTKViewer_ExtractUnstructuredGrid::ePoints);
  aFilter->SetModeOfChanging(VTKViewer_ExtractUnstructuredGrid::eAdding);
  aFilter->RegisterCellsWithType(VTK_VERTEX);
  
  //my0DExtProp = vtkProperty::New();
  //my0DExtProp->DeepCopy(my0DProp);
  //anRGB[0] = 1 - anRGB[0];
  //anRGB[1] = 1 - anRGB[1];
  //anRGB[2] = 1 - anRGB[2];
  //my0DExtProp->SetColor(anRGB[0],anRGB[1],anRGB[2]);
  //my0DExtProp->SetPointSize(aElem0DSize);
  //
  //my0DExtActor = SMESH_DeviceActor::New();
  //my0DExtActor->SetUserMatrix(aMatrix);
  //my0DExtActor->SetStoreClippingMapping(true);
  //my0DExtActor->PickableOff();
  //my0DExtActor->SetHighlited(true);
  //my0DExtActor->SetVisibility(false);
  //my0DExtActor->SetProperty(my0DExtProp);
  //my0DExtActor->SetRepresentation(SMESH_DeviceActor::eInsideframe);
  //aFilter = my0DExtActor->GetExtractUnstructuredGrid();
  ////aFilter->SetModeOfExtraction(VTKViewer_ExtractUnstructuredGrid::ePoints);
  //aFilter->SetModeOfChanging(VTKViewer_ExtractUnstructuredGrid::eAdding);
  //aFilter->RegisterCellsWithType(VTK_VERTEX);


  //Definition 0D device of the actor (nodes)
  //-----------------------------------------
  myNodeProp = vtkProperty::New();
  SMESH::GetColor( "SMESH", "node_color", anRGB[0], anRGB[1], anRGB[2], QColor( 255, 0, 0 ) );
  myNodeProp->SetColor(anRGB[0],anRGB[1],anRGB[2]);

  myNodeActor = SMESH_DeviceActor::New();
  myNodeActor->SetUserMatrix(aMatrix);
  myNodeActor->SetStoreClippingMapping(true);
  myNodeActor->PickableOff();
  myNodeActor->SetVisibility(false);
  myNodeActor->SetProperty(myNodeProp);
  myNodeActor->SetRepresentation(SMESH_DeviceActor::ePoint);
  aFilter = myNodeActor->GetExtractUnstructuredGrid();
  aFilter->SetModeOfExtraction(VTKViewer_ExtractUnstructuredGrid::ePoints);
  
  myNodeExtProp = vtkProperty::New();
  myNodeExtProp->DeepCopy(myNodeProp);
  anRGB[0] = 1 - anRGB[0];
  anRGB[1] = 1 - anRGB[1];
  anRGB[2] = 1 - anRGB[2];
  myNodeExtProp->SetColor(anRGB[0],anRGB[1],anRGB[2]);

  myNodeExtActor = SMESH_DeviceActor::New();
  myNodeExtActor->SetUserMatrix(aMatrix);
  myNodeExtActor->SetStoreClippingMapping(true);
  myNodeExtActor->PickableOff();
  myNodeExtActor->SetHighlited(true);
  myNodeExtActor->SetVisibility(false);
  myNodeExtActor->SetProperty(myNodeExtProp);
  myNodeExtActor->SetRepresentation(SMESH_DeviceActor::ePoint);
  aFilter = myNodeExtActor->GetExtractUnstructuredGrid();
  aFilter->SetModeOfExtraction(VTKViewer_ExtractUnstructuredGrid::ePoints);
  aFilter->RegisterCellsWithType(VTK_VERTEX);

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
  myHighlightProp->SetPointSize(aElem0DSize); // ??
  myHighlightProp->SetRepresentation(1);

  myPreselectProp = vtkProperty::New();
  myPreselectProp->SetAmbient(1.0);
  myPreselectProp->SetDiffuse(0.0);
  myPreselectProp->SetSpecular(0.0);
  SMESH::GetColor( "SMESH", "highlight_color", anRGB[0], anRGB[1], anRGB[2], QColor( 0, 255, 255 ) );
  myPreselectProp->SetColor(anRGB[0],anRGB[1],anRGB[2]);
  myPreselectProp->SetPointSize(aElem0DSize); // ??
  myPreselectProp->SetRepresentation(1);

  myHighlitableActor = SMESH_DeviceActor::New();
  myHighlitableActor->SetUserMatrix(aMatrix);
  myHighlitableActor->PickableOff();
  myHighlitableActor->SetRepresentation(SMESH_DeviceActor::eWireframe);

  aMatrix->Delete();

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

  myScalarBarActor = SMESH_ScalarBarActor::New();
  myScalarBarActor->SetVisibility(false);
  myScalarBarActor->SetLookupTable(myLookupTable);

  //Fix for Bug 13314:
  //Incorrect "Min value" in Scalar Bar in Mesh:
  //  myScalarBarActor->SetLabelFormat("%.4g");
  // changes was commented because of regression bug IPAL 19981

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
#if (VTK_XVERSION < 0x050200)
  myPtsLabeledDataMapper->SetLabelFormat("%g");
#endif
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

  myCellCenters = VTKViewer_CellCenters::New();
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
#if (VTK_XVERSION < 0x050200)
  myClsLabeledDataMapper->SetLabelFormat("%g");
#endif
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
  


  //Quadratic 2D elements representation
  //-----------------------------------------------------------------------------
  int aQuadratic2DMode = mgr->integerValue( "SMESH", "quadratic_mode", 0);
  if(aQuadratic2DMode == 0){
    myHighlitableActor->SetQuadraticArcMode(false);
    my2DActor->SetQuadraticArcMode(false);
    my1DActor->SetQuadraticArcMode(false);
  }
  else if(aQuadratic2DMode == 1){
    myHighlitableActor->SetQuadraticArcMode(true);
    my2DActor->SetQuadraticArcMode(true);
    my1DActor->SetQuadraticArcMode(true);
  }
  
  int aQuadraticAngle = mgr->integerValue( "SMESH", "max_angle", 2);
  myHighlitableActor->SetQuadraticArcAngle(aQuadraticAngle);
  my2DActor->SetQuadraticArcAngle(aQuadraticAngle);
  
  // Set colors of the name actor
  SMESH::GetColor( "SMESH", "fill_color", anRGB[0], anRGB[1], anRGB[2], QColor( 0, 170, 255 ) );
  myNameActor->SetBackgroundColor(anRGB[0], anRGB[1], anRGB[2]);
  SMESH::GetColor( "SMESH", "group_name_color", anRGB[0], anRGB[1], anRGB[2], QColor( 255, 255, 255 ) );
  myNameActor->SetForegroundColor(anRGB[0], anRGB[1], anRGB[2]);
}


SMESH_ActorDef::~SMESH_ActorDef()
{
  if(MYDEBUG) MESSAGE("~SMESH_ActorDef - "<<this);

  // caught by SMESHGUI::ProcessEvents() static method
  this->InvokeEvent( SMESH::DeleteActorEvent, NULL );

  myScalarBarActor->Delete();
  myLookupTable->Delete();

  mySurfaceProp->Delete();
  myBackSurfaceProp->Delete();

  myEdgeProp->Delete();
  myHighlightProp->Delete();
  myPreselectProp->Delete();

  myNodeProp->Delete();
  myNodeExtProp->Delete();
 
  my0DProp->Delete();
  my0DActor->Delete();

  //my0DExtProp->Delete();
  //my0DExtActor->Delete();
 
  my1DProp->Delete();
  my1DActor->Delete();

  my1DExtProp->Delete();
  my1DExtActor->Delete();

  my2DActor->Delete();
  my2DExtProp->Delete();
  my2DExtActor->Delete();
  my3DActor->Delete();

  myNodeActor->Delete();
  myBaseActor->Delete();

  myNodeExtActor->Delete();
  
  myHighlitableActor->Delete();

  //Deleting of points numbering pipeline
  //---------------------------------------
  myPointsNumDataSet->Delete();

  // commented: porting to vtk 5.0
  //  myPtsLabeledDataMapper->RemoveAllInputs();
  myPtsLabeledDataMapper->Delete();

  // commented: porting to vtk 5.0
  //  myPtsSelectVisiblePoints->UnRegisterAllOutputs();
  myPtsSelectVisiblePoints->Delete();

  // commented: porting to vtk 5.0
  //  myPtsMaskPoints->UnRegisterAllOutputs();
  myPtsMaskPoints->Delete();

  myPointLabels->Delete();


  //Deleting of cells numbering pipeline
  //---------------------------------------
  myCellsNumDataSet->Delete();

  myClsLabeledDataMapper->RemoveAllInputs();
  myClsLabeledDataMapper->Delete();

  // commented: porting to vtk 5.0
  //  myClsSelectVisiblePoints->UnRegisterAllOutputs();
  myClsSelectVisiblePoints->Delete();

  // commented: porting to vtk 5.0
  //  myClsMaskPoints->UnRegisterAllOutputs();
  myClsMaskPoints->Delete();

  // commented: porting to vtk 5.0
  //  myCellCenters->UnRegisterAllOutputs();
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
    
    for ( vtkIdType anId = 0; anId < aNbElem; anId++ )
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


void SMESH_ActorDef::SetFacesOriented(bool theIsFacesOriented)
{
  myIsFacesOriented = theIsFacesOriented;

  my2DActor->SetFacesOriented(theIsFacesOriented);
  my3DActor->SetFacesOriented(theIsFacesOriented);

  myTimeStamp->Modified();
}

bool SMESH_ActorDef::GetFacesOriented()
{
  return myIsFacesOriented;
}

void SMESH_ActorDef::SetFacesOrientationColor(vtkFloatingPointType theColor[3])
{
  my2DActor->SetFacesOrientationColor( theColor );
  my3DActor->SetFacesOrientationColor( theColor );
}

void SMESH_ActorDef::GetFacesOrientationColor(vtkFloatingPointType theColor[3])
{
  my3DActor->GetFacesOrientationColor( theColor );
}

void SMESH_ActorDef::SetFacesOrientationScale(vtkFloatingPointType theScale)
{
  my2DActor->SetFacesOrientationScale( theScale );
  my3DActor->SetFacesOrientationScale( theScale );
}

vtkFloatingPointType SMESH_ActorDef::GetFacesOrientationScale()
{
  return my3DActor->GetFacesOrientationScale();
}

void SMESH_ActorDef::SetFacesOrientation3DVectors(bool theState)
{
  my2DActor->SetFacesOrientation3DVectors( theState );
  my3DActor->SetFacesOrientation3DVectors( theState );
}

bool SMESH_ActorDef::GetFacesOrientation3DVectors()
{
  return my3DActor->GetFacesOrientation3DVectors();
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

  my0DActor->GetMapper()->SetScalarVisibility(false);
  my1DActor->GetMapper()->SetScalarVisibility(false);
  my2DActor->GetMapper()->SetScalarVisibility(false);
  my3DActor->GetMapper()->SetScalarVisibility(false);
  myScalarBarActor->SetVisibility(false);

  bool anIsScalarVisible = theMode > eNone;

  if(anIsScalarVisible){
    switch(theMode){
    case eLength:
    {
      SMESH::Controls::Length* aControl = new SMESH::Controls::Length();
      aControl->SetPrecision( myControlsPrecision );
      myFunctor.reset( aControl );
      myControlActor = my1DActor;
      break;
    }
    case eLength2D:
    {
      myFunctor.reset(new SMESH::Controls::Length2D());
      myControlActor = my2DActor;
      break;
    }
    case eFreeBorders:
      myFunctor.reset(new SMESH::Controls::FreeBorders());
      myControlActor = my1DActor;
      break;
    case eFreeEdges:
      myFunctor.reset(new SMESH::Controls::FreeEdges());
      myControlActor = my2DActor;
      break;
    case eFreeNodes:
      myFunctor.reset(new SMESH::Controls::FreeNodes());
      myControlActor = myNodeActor;
      break;
    case eFreeFaces:
      myFunctor.reset(new SMESH::Controls::FreeFaces());
      myControlActor = my2DActor;
      break;
    case eMultiConnection:
      myFunctor.reset(new SMESH::Controls::MultiConnection());
      myControlActor = my1DActor;
      break;
    case eMultiConnection2D:
      myFunctor.reset(new SMESH::Controls::MultiConnection2D());
      myControlActor = my2DActor;
      break;
    case eArea:
    {
      SMESH::Controls::Area* aControl = new SMESH::Controls::Area();
      aControl->SetPrecision( myControlsPrecision );
      myFunctor.reset( aControl );
      myControlActor = my2DActor;
      break;
    }
    case eTaper:
    {
      SMESH::Controls::Taper* aControl = new SMESH::Controls::Taper();
      aControl->SetPrecision( myControlsPrecision );
      myFunctor.reset( aControl );
      myControlActor = my2DActor;
      break;
    }
    case eAspectRatio:
    {
      SMESH::Controls::AspectRatio* aControl = new SMESH::Controls::AspectRatio();
      aControl->SetPrecision( myControlsPrecision );
      myFunctor.reset( aControl );
      myControlActor = my2DActor;
      break;
    }
    case eAspectRatio3D:
    {
      SMESH::Controls::AspectRatio3D* aControl = new SMESH::Controls::AspectRatio3D();
      aControl->SetPrecision( myControlsPrecision );
      myFunctor.reset( aControl );
      myControlActor = my3DActor;
      break;
    }
    case eVolume3D:
    {
      SMESH::Controls::Volume* aControl = new SMESH::Controls::Volume();
      aControl->SetPrecision( myControlsPrecision );
      myFunctor.reset( aControl );
      myControlActor = my3DActor;
      break;
    }
    case eMaxElementLength2D:
    {
      SMESH::Controls::MaxElementLength2D* aControl = new SMESH::Controls::MaxElementLength2D();
      aControl->SetPrecision( myControlsPrecision );
      myFunctor.reset( aControl );
      myControlActor = my2DActor;
      break;
    }
    case eMaxElementLength3D:
    {
      SMESH::Controls::MaxElementLength3D* aControl = new SMESH::Controls::MaxElementLength3D();
      aControl->SetPrecision( myControlsPrecision );
      myFunctor.reset( aControl );
      myControlActor = my3DActor;
      break;
    }
    case eMinimumAngle:
    {
      SMESH::Controls::MinimumAngle* aControl = new SMESH::Controls::MinimumAngle();
      aControl->SetPrecision( myControlsPrecision );
      myFunctor.reset( aControl );
      myControlActor = my2DActor;
      break;
    }
    case eWarping:
    {
      SMESH::Controls::Warping* aControl = new SMESH::Controls::Warping();
      aControl->SetPrecision( myControlsPrecision );
      myFunctor.reset( aControl );
      myControlActor = my2DActor;
      break;
    }
    case eSkew:
    {
      SMESH::Controls::Skew* aControl = new SMESH::Controls::Skew();
      aControl->SetPrecision( myControlsPrecision );
      myFunctor.reset( aControl );
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
      case eFreeNodes:
        myNodeExtActor->SetExtControlMode(myFunctor);
        break;
      case eFreeEdges:
      case eFreeBorders:
        my1DExtActor->SetExtControlMode(myFunctor);
        break;
      case eFreeFaces:
        my2DExtActor->SetExtControlMode(myFunctor);
        break;
      case eLength2D:
      case eMultiConnection2D:
        my1DExtActor->SetExtControlMode(myFunctor,myScalarBarActor,myLookupTable);
        break;
      default:
        myControlActor->SetControlMode(myFunctor,myScalarBarActor,myLookupTable);
      }
    }

    if(theCheckEntityMode){
      if(myControlActor == my1DActor)
        SetEntityMode(eEdges);
      else if(myControlActor == my2DActor){
        switch(myControlMode){
        case eLength2D:
        case eFreeEdges:
        case eFreeFaces:
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

  }
  else {
    if(theCheckEntityMode)
      myEntityMode = eAllEntity;
    myFunctor.reset();
  }

  SetRepresentation(GetRepresentation());

  myTimeStamp->Modified();
  Modified();
}


void SMESH_ActorDef::AddToRender(vtkRenderer* theRenderer){
  theRenderer->AddActor(myNodeActor);
  theRenderer->AddActor(myBaseActor);
  
  theRenderer->AddActor(myNodeExtActor);

  my3DActor->AddToRender(theRenderer);
  my2DActor->AddToRender(theRenderer);
  my2DExtActor->AddToRender(theRenderer);

  theRenderer->AddActor(my1DActor);
  theRenderer->AddActor(my1DExtActor);

  theRenderer->AddActor(my0DActor);
  //theRenderer->AddActor(my0DExtActor);

  theRenderer->AddActor(myHighlitableActor);
  
  theRenderer->AddActor2D(myScalarBarActor);

  myPtsSelectVisiblePoints->SetRenderer(theRenderer);
  myClsSelectVisiblePoints->SetRenderer(theRenderer);

  theRenderer->AddActor2D(myPointLabels);
  theRenderer->AddActor2D(myCellsLabels);

  // the superclass' method should be called at the end
  // (in particular, for correct work of selection)
  SALOME_Actor::AddToRender(theRenderer);
}

void SMESH_ActorDef::RemoveFromRender(vtkRenderer* theRenderer){
  SALOME_Actor::RemoveFromRender(theRenderer);

  theRenderer->RemoveActor(myNodeActor);
  theRenderer->RemoveActor(myBaseActor);

  theRenderer->RemoveActor(myNodeExtActor);

  theRenderer->RemoveActor(myHighlitableActor);

  theRenderer->RemoveActor(my0DActor);
  //theRenderer->RemoveActor(my0DExtActor);

  theRenderer->RemoveActor(my1DActor);
  theRenderer->RemoveActor(my1DExtActor);

  my2DActor->RemoveFromRender(theRenderer);
  my2DExtActor->RemoveFromRender(theRenderer);
  my3DActor->RemoveFromRender(theRenderer);

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

  myNodeExtActor->Init(myVisualObj,myImplicitBoolean);
  
  my0DActor->Init(myVisualObj,myImplicitBoolean);
  //my0DExtActor->Init(myVisualObj,myImplicitBoolean);
  
  my1DActor->Init(myVisualObj,myImplicitBoolean);
  my1DExtActor->Init(myVisualObj,myImplicitBoolean);
  
  my2DActor->Init(myVisualObj,myImplicitBoolean);
  my2DExtActor->Init(myVisualObj,myImplicitBoolean);
  my3DActor->Init(myVisualObj,myImplicitBoolean);
  
  my0DActor->GetMapper()->SetLookupTable(myLookupTable);
  //my0DExtActor->GetMapper()->SetLookupTable(myLookupTable);
  
  my1DActor->GetMapper()->SetLookupTable(myLookupTable);
  my1DExtActor->GetMapper()->SetLookupTable(myLookupTable);

  my2DActor->GetMapper()->SetLookupTable(myLookupTable);
  my2DExtActor->GetMapper()->SetLookupTable(myLookupTable);
  my3DActor->GetMapper()->SetLookupTable(myLookupTable);
    
  vtkFloatingPointType aFactor, aUnits;
  my2DActor->GetPolygonOffsetParameters(aFactor,aUnits);
  my2DActor->SetPolygonOffsetParameters(aFactor,aUnits*0.75);
  my2DExtActor->SetPolygonOffsetParameters(aFactor,aUnits*0.5);

  SUIT_ResourceMgr* mgr = SUIT_Session::session()->resourceMgr();
  if( !mgr )
    return false;

  //SetIsShrunkable(theGrid->GetNumberOfCells() > 10);
  SetIsShrunkable(true);

  SetShrinkFactor( SMESH::GetFloat( "SMESH:shrink_coeff", 75 ) / 100. );

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

  if( dynamic_cast<SMESH_GroupObj*>( myVisualObj.get() ) )
    SetIsDisplayNameActor( true );

  int aMarkerType = mgr->integerValue( "SMESH", "type_of_marker", 1 ); // dot
  int aMarkerScale = mgr->integerValue( "SMESH", "marker_scale", 9 );  // 5 pixels
  SetMarkerStd( (VTK::MarkerType)aMarkerType, (VTK::MarkerScale)aMarkerScale );

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

  myNodeExtActor->SetTransform(theTransform);

  my0DActor->SetTransform(theTransform);
  //my0DExtActor->SetTransform(theTransform);

  my1DActor->SetTransform(theTransform);
  my1DExtActor->SetTransform(theTransform);

  my2DActor->SetTransform(theTransform);
  my2DExtActor->SetTransform(theTransform);
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
    ( aDataSet->GetNumberOfCells() == 1 && 
    aDataSet->GetCell(0)->GetCellType() == VTK_VERTEX );
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
  my2DExtActor->SetShrinkFactor(theValue);
  my3DActor->SetShrinkFactor(theValue);

  Modified();
}

void SMESH_ActorDef::SetShrink(){
  if(!myIsShrinkable) return;

  myBaseActor->SetShrink();

  my1DActor->SetShrink();
  my1DExtActor->SetShrink();

  my2DActor->SetShrink();
  my2DExtActor->SetShrink();
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
  my2DExtActor->UnShrink();
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
  
  myNodeExtActor->VisibilityOff();

  my0DActor->VisibilityOff();
  //my0DExtActor->VisibilityOff();

  my1DActor->VisibilityOff();
  my1DExtActor->VisibilityOff();
  
  my2DActor->VisibilityOff();
  my2DExtActor->VisibilityOff();
  my3DActor->VisibilityOff();
  
  myScalarBarActor->VisibilityOff();
  myPointLabels->VisibilityOff();
  myCellsLabels->VisibilityOff();
  
  if(GetVisibility()){
    if(theIsUpdateRepersentation)
      SetRepresentation(GetRepresentation());
    
    if(myControlMode != eNone){
      switch(myControlMode){
      case eFreeNodes:
        myNodeExtActor->VisibilityOn();
        break;
      case eFreeEdges:
      case eFreeBorders:
        my1DExtActor->VisibilityOn();
        break;
      case eFreeFaces:
        my2DExtActor->VisibilityOn();
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

    if(myEntityMode & e0DElements){
      my0DActor->VisibilityOn();
    }

    if(myEntityMode & eEdges && GetRepresentation() != ePoint){
      my1DActor->VisibilityOn();
    }
    
    if(myEntityMode & eFaces && GetRepresentation() != ePoint){
      my2DActor->VisibilityOn();
    }
    
    if(myEntityMode & eVolumes && GetRepresentation() != ePoint){
      my3DActor->VisibilityOn();
    }
    
    if(myIsPointsLabeled){ 
      myPointLabels->VisibilityOn();
      myNodeActor->VisibilityOn();
    }

    if(myIsCellsLabeled) 
      myCellsLabels->VisibilityOn();
  }
  UpdateHighlight();
  Modified();
}


void SMESH_ActorDef::SetEntityMode(unsigned int theMode)
{
  myEntityState = eAllEntity;

  if(!myVisualObj->GetNbEntities(SMDSAbs_0DElement)) {
    myEntityState &= ~e0DElements;
    theMode &= ~e0DElements;
  }

  if(!myVisualObj->GetNbEntities(SMDSAbs_Edge)) {
    myEntityState &= ~eEdges;
    theMode &= ~eEdges;
  }

  if(!myVisualObj->GetNbEntities(SMDSAbs_Face)) {
    myEntityState &= ~eFaces;
    theMode &= ~eFaces;
  }

  if(!myVisualObj->GetNbEntities(SMDSAbs_Volume)) {
    myEntityState &= ~eVolumes;
    theMode &= ~eVolumes;
  }

  if (!theMode) {
    if(myVisualObj->GetNbEntities(SMDSAbs_0DElement))
      theMode |= e0DElements;

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

  VTKViewer_ExtractUnstructuredGrid* aHightFilter = myHighlitableActor->GetExtractUnstructuredGrid();
  aHightFilter->ClearRegisteredCellsWithType();
  aHightFilter->SetModeOfChanging(VTKViewer_ExtractUnstructuredGrid::eAdding);

  if (myEntityMode & e0DElements) {
    if (MYDEBUG) MESSAGE("0D ELEMENTS");
    aFilter->RegisterCellsWithType(VTK_VERTEX);
    aHightFilter->RegisterCellsWithType(VTK_VERTEX);
  }

  if (myEntityMode & eEdges) {
    if (MYDEBUG) MESSAGE("EDGES");
    aFilter->RegisterCellsWithType(VTK_LINE);
    aFilter->RegisterCellsWithType(VTK_QUADRATIC_EDGE);

    aHightFilter->RegisterCellsWithType(VTK_LINE);
    aHightFilter->RegisterCellsWithType(VTK_QUADRATIC_EDGE);
  }

  if (myEntityMode & eFaces) {
    if (MYDEBUG) MESSAGE("FACES");
    aFilter->RegisterCellsWithType(VTK_TRIANGLE);
    aFilter->RegisterCellsWithType(VTK_POLYGON);
    aFilter->RegisterCellsWithType(VTK_QUAD);
    aFilter->RegisterCellsWithType(VTK_QUADRATIC_TRIANGLE);
    aFilter->RegisterCellsWithType(VTK_QUADRATIC_QUAD);

    aHightFilter->RegisterCellsWithType(VTK_TRIANGLE);
    aHightFilter->RegisterCellsWithType(VTK_POLYGON);
    aHightFilter->RegisterCellsWithType(VTK_QUAD);
    aHightFilter->RegisterCellsWithType(VTK_QUADRATIC_TRIANGLE);
    aHightFilter->RegisterCellsWithType(VTK_QUADRATIC_QUAD);
  }

  if (myEntityMode & eVolumes) {
    if (MYDEBUG) MESSAGE("VOLUMES");
    aFilter->RegisterCellsWithType(VTK_TETRA);
    aFilter->RegisterCellsWithType(VTK_VOXEL);
    aFilter->RegisterCellsWithType(VTK_HEXAHEDRON);
    aFilter->RegisterCellsWithType(VTK_WEDGE);
    aFilter->RegisterCellsWithType(VTK_PYRAMID);
    aFilter->RegisterCellsWithType(VTK_QUADRATIC_TETRA);
    aFilter->RegisterCellsWithType(VTK_QUADRATIC_HEXAHEDRON);
    aFilter->RegisterCellsWithType(VTK_QUADRATIC_WEDGE);
    aFilter->RegisterCellsWithType(VTK_CONVEX_POINT_SET);
//#ifdef VTK_HAVE_POLYHEDRON
    aFilter->RegisterCellsWithType(VTK_POLYHEDRON);
//#endif
    
    aHightFilter->RegisterCellsWithType(VTK_TETRA);
    aHightFilter->RegisterCellsWithType(VTK_VOXEL);
    aHightFilter->RegisterCellsWithType(VTK_HEXAHEDRON);
    aHightFilter->RegisterCellsWithType(VTK_WEDGE);
    aHightFilter->RegisterCellsWithType(VTK_PYRAMID);
    aHightFilter->RegisterCellsWithType(VTK_QUADRATIC_TETRA);
    aHightFilter->RegisterCellsWithType(VTK_QUADRATIC_HEXAHEDRON);
    aHightFilter->RegisterCellsWithType(VTK_QUADRATIC_WEDGE);
    aHightFilter->RegisterCellsWithType(VTK_QUADRATIC_PYRAMID);
    aHightFilter->RegisterCellsWithType(VTK_CONVEX_POINT_SET);
//#ifdef VTK_HAVE_POLYHEDRON
    aHightFilter->RegisterCellsWithType(VTK_POLYHEDRON);
//#endif
  }
  aFilter->Update();
  if (MYDEBUG) MESSAGE(aFilter->GetOutput()->GetNumberOfCells());
  SetVisibility(GetVisibility(),false);
}

void SMESH_ActorDef::SetRepresentation (int theMode)
{ 
  int aNbEdges = myVisualObj->GetNbEntities(SMDSAbs_Edge);
  int aNbFaces = myVisualObj->GetNbEntities(SMDSAbs_Face);
  int aNbVolumes = myVisualObj->GetNbEntities(SMDSAbs_Volume);

  if (theMode < 0) {
    myRepresentation = eSurface;
    if (!aNbFaces && !aNbVolumes && aNbEdges) {
      myRepresentation = eEdge;
    } else if (!aNbFaces && !aNbVolumes && !aNbEdges) {
      myRepresentation = ePoint;
    }
  } else {
    switch (theMode) {
    case eEdge:
      if (!aNbFaces && !aNbVolumes && !aNbEdges) return;
      break;
    case eSurface:
      if (!aNbFaces && !aNbVolumes) return;
      break;
    }    
    myRepresentation = theMode;
  }

  if (!GetUnstructuredGrid()->GetNumberOfCells())
    myRepresentation = ePoint;

  if (myIsShrunk) {
    if (myRepresentation == ePoint) {
      UnShrink();
      myIsShrunk = true;
    } else {
      SetShrink();
    }      
  }

  myPickableActor = myBaseActor;
  myNodeActor->SetVisibility(false);
  myNodeExtActor->SetVisibility(false);
  vtkProperty *aProp = NULL, *aBackProp = NULL;
  SMESH_DeviceActor::EReperesent aReperesent = SMESH_DeviceActor::EReperesent(-1);
  SMESH_Actor::EQuadratic2DRepresentation aQuadraticMode = GetQuadratic2DRepresentation();
  switch (myRepresentation) {
  case ePoint:
    myPickableActor = myNodeActor;
    myNodeActor->SetVisibility(true);
    aQuadraticMode = SMESH_Actor::eLines;
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

  if(aQuadraticMode == SMESH_Actor::eLines)
    my2DActor->SetQuadraticArcMode(false);
  else if(aQuadraticMode == SMESH_Actor::eArcs)
    my2DActor->SetQuadraticArcMode(true);

  my2DExtActor->SetRepresentation(aReperesent);
  
  my3DActor->SetProperty(aProp);
  my3DActor->SetBackfaceProperty(aBackProp);
  my3DActor->SetRepresentation(aReperesent);

  //my0DExtActor->SetVisibility(false);
  my1DExtActor->SetVisibility(false);
  my2DExtActor->SetVisibility(false);

  // ???
  //my0DActor->SetProperty(aProp);
  //my0DActor->SetBackfaceProperty(aBackProp);
  my0DActor->SetRepresentation(aReperesent);
  //my0DExtActor->SetRepresentation(aReperesent);

  switch(myControlMode){
  case eLength:
  case eMultiConnection:
    aProp = aBackProp = my1DProp;
    if(myRepresentation != ePoint)
      aReperesent = SMESH_DeviceActor::eInsideframe;
    break;
  }
  
  if(aQuadraticMode == SMESH_Actor::eLines)
    my1DActor->SetQuadraticArcMode(false);
  else if(aQuadraticMode == SMESH_Actor::eArcs)
    my1DActor->SetQuadraticArcMode(true);

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
      myHighlitableActor->GetExtractUnstructuredGrid()->
        SetModeOfExtraction(VTKViewer_ExtractUnstructuredGrid::ePoints);
      myHighlitableActor->SetVisibility(anIsVisible);
      myHighlitableActor->SetRepresentation(SMESH_DeviceActor::ePoint);

      VTK::MarkerType aMarkerType = GetMarkerType();
      if(aMarkerType != VTK::MT_USER)
        myHighlitableActor->SetMarkerStd(aMarkerType, GetMarkerScale());
      else
        myHighlitableActor->SetMarkerTexture(GetMarkerTexture(), myMarkerTexture);
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
  if(myIsFacesOriented){
    SetFacesOriented(myIsFacesOriented);
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
  if( SMESH_GroupObj* aGroupObj = dynamic_cast<SMESH_GroupObj*>( myVisualObj.get() ) )
    if( aGroupObj->GetElementType() == SMDSAbs_Face ||
        aGroupObj->GetElementType() == SMDSAbs_Volume )
      myNameActor->SetBackgroundColor(r,g,b);
  Modified();
}

void SMESH_ActorDef::GetSufaceColor(vtkFloatingPointType& r,vtkFloatingPointType& g,vtkFloatingPointType& b){
  ::GetColor(mySurfaceProp,r,g,b);
  my2DExtProp->SetColor(1.0-r,1.0-g,1.0-b);
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
  if( SMESH_GroupObj* aGroupObj = dynamic_cast<SMESH_GroupObj*>( myVisualObj.get() ) )
    if( aGroupObj->GetElementType() == SMDSAbs_Edge )
      myNameActor->SetBackgroundColor(r,g,b);
  Modified();
}

void SMESH_ActorDef::GetEdgeColor(vtkFloatingPointType& r,vtkFloatingPointType& g,vtkFloatingPointType& b){
  ::GetColor(myEdgeProp,r,g,b);
}

void SMESH_ActorDef::SetNodeColor(vtkFloatingPointType r,vtkFloatingPointType g,vtkFloatingPointType b){ 
  myNodeProp->SetColor(r,g,b);
  myNodeExtProp->SetColor(1.0-r,1.0-g,1.0-b);
  if( SMESH_GroupObj* aGroupObj = dynamic_cast<SMESH_GroupObj*>( myVisualObj.get() ) )
    if( aGroupObj->GetElementType() == SMDSAbs_Node )
      myNameActor->SetBackgroundColor(r,g,b);
  Modified();
}

void SMESH_ActorDef::GetNodeColor(vtkFloatingPointType& r,vtkFloatingPointType& g,vtkFloatingPointType& b){ 
  ::GetColor(myNodeProp,r,g,b);
}

void SMESH_ActorDef::Set0DColor(vtkFloatingPointType r,vtkFloatingPointType g,vtkFloatingPointType b){ 
  my0DProp->SetColor(r,g,b);
  if( SMESH_GroupObj* aGroupObj = dynamic_cast<SMESH_GroupObj*>( myVisualObj.get() ) )
    if( aGroupObj->GetElementType() == SMDSAbs_0DElement )
      myNameActor->SetBackgroundColor(r,g,b);
  Modified();
}

void SMESH_ActorDef::Get0DColor(vtkFloatingPointType& r,vtkFloatingPointType& g,vtkFloatingPointType& b){ 
  ::GetColor(my0DProp,r,g,b);
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


void SMESH_ActorDef::Set0DSize(vtkFloatingPointType theVal){
  my0DProp->SetPointSize(theVal);
  Modified();
}

vtkFloatingPointType SMESH_ActorDef::Get0DSize(){
  return my0DProp->GetPointSize();
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
SMESH_ActorDef::SetImplicitFunctionUsed(bool theIsImplicitFunctionUsed)
{
  myNodeActor->SetImplicitFunctionUsed(theIsImplicitFunctionUsed);
  myBaseActor->SetImplicitFunctionUsed(theIsImplicitFunctionUsed);

  myHighlitableActor->SetImplicitFunctionUsed(theIsImplicitFunctionUsed);

  myNodeExtActor->SetImplicitFunctionUsed(theIsImplicitFunctionUsed);

  my0DActor->SetImplicitFunctionUsed(theIsImplicitFunctionUsed);
  //my0DExtActor->SetImplicitFunctionUsed(theIsImplicitFunctionUsed);

  my1DActor->SetImplicitFunctionUsed(theIsImplicitFunctionUsed);
  my1DExtActor->SetImplicitFunctionUsed(theIsImplicitFunctionUsed);

  my2DActor->SetImplicitFunctionUsed(theIsImplicitFunctionUsed);
  my2DExtActor->SetImplicitFunctionUsed(theIsImplicitFunctionUsed);
  my3DActor->SetImplicitFunctionUsed(theIsImplicitFunctionUsed);
}

vtkIdType 
SMESH_ActorDef::AddClippingPlane(vtkPlane* thePlane)
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

    if ( f.overline() )
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
    QFont f = mgr->fontValue( "SMESH", "scalar_bar_label_font" );
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

    if( f.overline() )
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

  bool distributionVisibility = mgr->booleanValue("SMESH","distribution_visibility");
  myScalarBarActor->SetDistributionVisibility(distributionVisibility);

  int coloringType = mgr->integerValue("SMESH", "distribution_coloring_type", 0);
  myScalarBarActor->SetDistributionColoringType(coloringType);
  
  QColor distributionColor = mgr->colorValue("SMESH", "distribution_color",
					     QColor(255, 255, 255));
  double rgb[3];
  rgb[0]= distributionColor.red()/255.;
  rgb[1]= distributionColor.green()/255.;
  rgb[2]= distributionColor.blue()/255.;
  myScalarBarActor->SetDistributionColor(rgb);

  
}

void SMESH_ActorDef::SetQuadratic2DRepresentation(EQuadratic2DRepresentation theMode)
{
  switch(theMode) {
  case SMESH_Actor::eLines :
    myHighlitableActor->SetQuadraticArcMode(false);
    my2DActor->SetQuadraticArcMode(false);
    my1DActor->SetQuadraticArcMode(false);
    break;
  case SMESH_Actor::eArcs :
    myHighlitableActor->SetQuadraticArcMode(true);
    if(GetRepresentation() != SMESH_Actor::ePoint) {
      my2DActor->SetQuadraticArcMode(true);
      my1DActor->SetQuadraticArcMode(true);
    }
    break;
  default:
    break;
  }
}


SMESH_Actor::EQuadratic2DRepresentation SMESH_ActorDef::GetQuadratic2DRepresentation()
{
  if(myHighlitableActor->GetQuadraticArcMode())
    return SMESH_Actor::eArcs;
  else
    return SMESH_Actor::eLines;
}

void SMESH_ActorDef::SetMarkerStd( VTK::MarkerType theMarkerType, VTK::MarkerScale theMarkerScale )
{
  SALOME_Actor::SetMarkerStd( theMarkerType, theMarkerScale );
  myNodeActor->SetMarkerStd( theMarkerType, theMarkerScale );
  myNodeExtActor->SetMarkerStd( theMarkerType, theMarkerScale );
}

void SMESH_ActorDef::SetMarkerTexture( int theMarkerId, VTK::MarkerTexture theMarkerTexture )
{
  SALOME_Actor::SetMarkerTexture( theMarkerId, theMarkerTexture );
  myNodeActor->SetMarkerTexture( theMarkerId, theMarkerTexture );
  myNodeExtActor->SetMarkerTexture( theMarkerId, theMarkerTexture );
  myMarkerTexture = theMarkerTexture; // for deferred update of myHighlightActor
}
