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


#include "SMESH_Actor.h"
#include "SMESH_DeviceActor.h"
#include "SALOME_ExtractUnstructuredGrid.h"

#include "QAD_Config.h"
#include "utilities.h"
#include <qstringlist.h>

#include <vtkObjectFactory.h>
#include <vtkShrinkPolyData.h>
#include <vtkMergeFilter.h>

#include <vtkActor2D.h>
#include <vtkPolyData.h>
#include <vtkMaskPoints.h>
#include <vtkCellCenters.h>
#include <vtkTextProperty.h>
#include <vtkLabeledDataMapper.h>
#include <vtkSelectVisiblePoints.h>

#include <vtkScalarBarActor.h>
#include <vtkLookupTable.h>
#include <vtkDoubleArray.h>

#ifdef _DEBUG_
static int MYDEBUG = 1;
static int MYDEBUGWITHFILES = 0;
#else
static int MYDEBUG = 0;
static int MYDEBUGWITHFILES = 0;
#endif

using namespace std;


inline float GetFloat(const QString& theValue, float theDefault = 0){
  if(theValue.isEmpty()) return theDefault;
  QString aValue = QAD_CONFIG->getSetting(theValue);
  if(aValue.isEmpty()) return theDefault;
  return aValue.toFloat();
}


SMESH_Actor* SMESH_Actor::New(){
  return new SMESH_Actor();
}


SMESH_Actor* SMESH_Actor::New(TVisualObjPtr theVisualObj, 
			      SMESH::FilterManager_ptr theFilterMgr,
			      const char* theEntry, 
			      const char* theName,
			      int theIsClear)
{
  SMESH_Actor* anActor = SMESH_Actor::New();
  anActor->Init(theVisualObj,theFilterMgr,theEntry,theName,theIsClear);
  return anActor;
}


SMESH_Actor::SMESH_Actor(){
  if(MYDEBUG) MESSAGE("SMESH_Actor");

  myIsPointsVisible = false;

  myIsShrinkable = false;
  myIsShrunk = false;

  float aPointSize = GetFloat("SMESH:SettingsNodesSize",3);
  float aLineWidth = GetFloat("SMESH:SettingsWidth",1);

  vtkMatrix4x4 *aMatrix = vtkMatrix4x4::New();
  SALOME_ExtractUnstructuredGrid* aFilter = NULL;

  //Definition 2D and 3D divices of the actor
  //-----------------------------------------
  float anRGB[3] = {1,1,1};
  mySurfaceProp = vtkProperty::New();
  anRGB[0] = GetFloat("SMESH:SettingsFillColorRed", 0)/255.;
  anRGB[1] = GetFloat("SMESH:SettingsFillColorGreen", 170)/255.;
  anRGB[2] = GetFloat("SMESH:SettingsFillColorBlue", 255)/255.;
  mySurfaceProp->SetColor(anRGB[0],anRGB[1],anRGB[2]);

  myBackSurfaceProp = vtkProperty::New();
  anRGB[0] = GetFloat("SMESH:SettingsBackFaceColorRed", 0)/255.;
  anRGB[1] = GetFloat("SMESH:SettingsBackFaceColorGreen", 0)/255.;
  anRGB[2] = GetFloat("SMESH:SettingsBackFaceColorBlue", 255)/255.;
  myBackSurfaceProp->SetColor(anRGB[0],anRGB[1],anRGB[2]);

  my2DActor = SMESH_DeviceActor::New();
  my2DActor->SetUserMatrix(aMatrix);
  my2DActor->SetStoreMapping(true);
  my2DActor->PickableOff();
  my2DActor->SetProperty(mySurfaceProp);
  my2DActor->SetBackfaceProperty(myBackSurfaceProp);
  my2DActor->SetRepresentation(SMESH_DeviceActor::eSurface);
  aFilter = my2DActor->GetExtractUnstructuredGrid();
  aFilter->SetModeOfChanging(SALOME_ExtractUnstructuredGrid::eAdding);
  aFilter->RegisterCellsWithType(VTK_TRIANGLE);
  aFilter->RegisterCellsWithType(VTK_POLYGON);
  aFilter->RegisterCellsWithType(VTK_QUAD);

  my3DActor = SMESH_DeviceActor::New();
  my3DActor->SetUserMatrix(aMatrix);
  my3DActor->SetStoreMapping(true);
  my3DActor->PickableOff();
  my3DActor->SetProperty(mySurfaceProp);
  my3DActor->SetBackfaceProperty(myBackSurfaceProp);
  my3DActor->SetRepresentation(SMESH_DeviceActor::eSurface);
  aFilter = my3DActor->GetExtractUnstructuredGrid();
  aFilter->SetModeOfChanging(SALOME_ExtractUnstructuredGrid::eAdding);
  aFilter->RegisterCellsWithType(VTK_TETRA);
  aFilter->RegisterCellsWithType(VTK_VOXEL);
  aFilter->RegisterCellsWithType(VTK_HEXAHEDRON);
  aFilter->RegisterCellsWithType(VTK_WEDGE);
  aFilter->RegisterCellsWithType(VTK_PYRAMID);


  //Definition 1D divice of the actor
  //---------------------------------
  myEdgeProp = vtkProperty::New();
  myEdgeProp->SetAmbient(1.0);
  myEdgeProp->SetDiffuse(0.0);
  myEdgeProp->SetSpecular(0.0);
  anRGB[0] = GetFloat("SMESH:SettingsOutlineColorRed", 0)/255.;
  anRGB[1] = GetFloat("SMESH:SettingsOutlineColorGreen", 170)/255.;
  anRGB[2] = GetFloat("SMESH:SettingsOutlineColorBlue", 255)/255.;
  myEdgeProp->SetColor(anRGB[0],anRGB[1],anRGB[2]);
  myEdgeProp->SetLineWidth(aLineWidth);

  my1DActor = SMESH_DeviceActor::New();
  my1DActor->SetUserMatrix(aMatrix);
  my1DActor->SetStoreMapping(true);
  my1DActor->PickableOff();
  my1DActor->SetProperty(myEdgeProp);
  my1DActor->SetRepresentation(SMESH_DeviceActor::eSurface);
  aFilter = my1DActor->GetExtractUnstructuredGrid();
  aFilter->SetModeOfChanging(SALOME_ExtractUnstructuredGrid::eAdding);
  aFilter->RegisterCellsWithType(VTK_LINE);

  my1DProp = vtkProperty::New();
  my1DProp->DeepCopy(myEdgeProp);
  static int aCotnrolLineWidth = 3;
  my1DProp->SetLineWidth(aCotnrolLineWidth);
  my1DProp->SetPointSize(aPointSize);

  my1DExtProp = vtkProperty::New();
  my1DExtProp->DeepCopy(myEdgeProp);
  anRGB[0] = 1 - anRGB[0];
  anRGB[1] = 1 - anRGB[1];
  anRGB[2] = 1 - anRGB[2];
  my1DExtProp->SetColor(anRGB[0],anRGB[1],anRGB[2]);
  my1DExtProp->SetLineWidth(aCotnrolLineWidth);

  my1DExtActor = SMESH_DeviceActor::New();
  my1DExtActor->SetUserMatrix(aMatrix);
  my1DExtActor->SetStoreMapping(true);
  my1DExtActor->PickableOff();
  my1DExtActor->SetVisibility(false);
  my1DExtActor->SetProperty(my1DExtProp);
  my1DExtActor->SetRepresentation(SMESH_DeviceActor::eInsideframe);
  aFilter = my1DExtActor->GetExtractUnstructuredGrid();
  aFilter->SetModeOfChanging(SALOME_ExtractUnstructuredGrid::eAdding);
  aFilter->RegisterCellsWithType(VTK_LINE);


  //Definition 0D divice of the actor
  //---------------------------------
  myNodeProp = vtkProperty::New();
  anRGB[0] = GetFloat("SMESH:SettingsNodeColorRed",255)/255.;
  anRGB[1] = GetFloat("SMESH:SettingsNodeColorGreen",0)/255.;
  anRGB[2] = GetFloat("SMESH:SettingsNodeColorBlue",0)/255.;
  myNodeProp->SetColor(anRGB[0],anRGB[1],anRGB[2]);
  myNodeProp->SetPointSize(aPointSize);

  myNodeActor = SMESH_DeviceActor::New();
  myNodeActor->SetUserMatrix(aMatrix);
  myNodeActor->SetStoreMapping(true);
  myNodeActor->PickableOff();
  myNodeActor->SetVisibility(false);
  myNodeActor->SetProperty(myNodeProp);
  myNodeActor->SetRepresentation(SMESH_DeviceActor::ePoint);
  aFilter = myNodeActor->GetExtractUnstructuredGrid();
  aFilter->SetModeOfExtraction(SALOME_ExtractUnstructuredGrid::ePoints);


  //Definition of Pickable and Highlitable engines
  //----------------------------------------------

  myBaseActor = SMESH_DeviceActor::New();
  myBaseActor->SetUserMatrix(aMatrix);
  myBaseActor->SetStoreMapping(true);
  myBaseActor->GetProperty()->SetOpacity(0.0);

  myPickableActor = myBaseActor;

  myHighlightProp = vtkProperty::New();
  myHighlightProp->SetAmbient(1.0);
  myHighlightProp->SetDiffuse(0.0);
  myHighlightProp->SetSpecular(0.0);
  anRGB[0] = GetFloat("SMESH:SettingsSelectColorRed", 255)/255.;   // 1;
  anRGB[1] = GetFloat("SMESH:SettingsSelectColorGreen", 255)/255.; // 1;
  anRGB[2] = GetFloat("SMESH:SettingsSelectColorBlue", 255)/255.;  // 1;
  myHighlightProp->SetColor(anRGB[0],anRGB[1],anRGB[2]);
  myHighlightProp->SetPointSize(aPointSize);
  myHighlightProp->SetRepresentation(1);
 
  myPreselectProp = vtkProperty::New();
  myPreselectProp->SetAmbient(1.0);
  myPreselectProp->SetDiffuse(0.0);
  myPreselectProp->SetSpecular(0.0);
  anRGB[0] = GetFloat("SMESH:SettingsPreSelectColorRed", 0)/255.;     // 0;
  anRGB[1] = GetFloat("SMESH:SettingsPreSelectColorGreen", 255)/255.; // 1;
  anRGB[2] = GetFloat("SMESH:SettingsPreSelectColorBlue", 255)/255.;  // 1;
  myPreselectProp->SetColor(anRGB[0],anRGB[1],anRGB[2]);
  myPreselectProp->SetPointSize(aPointSize);
  myPreselectProp->SetRepresentation(1);

  myHighlitableActor = SMESH_DeviceActor::New();
  myHighlitableActor->SetUserMatrix(aMatrix);
  myHighlitableActor->SetStoreMapping(false);
  myHighlitableActor->PickableOff();
  myHighlitableActor->SetRepresentation(SMESH_DeviceActor::eWireframe);


  SetShrinkFactor(GetFloat("SMESH:SettingsShrinkCoeff", 75)/100.);

  myName = "";
  myIO = NULL;
  myDisplayMode = 0;

  ishighlighted = false;
  ispreselected = false;

  myColorMode = eNone;
  my1DColorMode = e1DNone;
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

  vtkTextProperty* aScalarBarTitleProp = vtkTextProperty::New();

  if ( QAD_CONFIG->hasSetting( "SMESH:ScalarBarTitleColor" ) ) {
    QStringList aTColor = QStringList::split(  ":", QAD_CONFIG->getSetting( "SMESH:ScalarBarTitleColor" ), false );
    aScalarBarTitleProp->SetColor( ( aTColor.count() > 0 ? aTColor[0].toInt()/255. : 1.0 ),
				   ( aTColor.count() > 1 ? aTColor[1].toInt()/255. : 1.0 ), 
				   ( aTColor.count() > 2 ? aTColor[2].toInt()/255. : 1.0 ) );
  }
  else
    aScalarBarTitleProp->SetColor( 1.0, 1.0, 1.0 );

  aScalarBarTitleProp->SetFontFamilyToArial();
  if( QAD_CONFIG->hasSetting( "SMESH:ScalarBarTitleFont" ) ){
    if ( QAD_CONFIG->getSetting( "SMESH:ScalarBarTitleFont" ) == "Arial" )
      aScalarBarTitleProp->SetFontFamilyToArial();
    else if ( QAD_CONFIG->getSetting( "SMESH:ScalarBarTitleFont" ) == "Courier" )
      aScalarBarTitleProp->SetFontFamilyToCourier();
    else if ( QAD_CONFIG->getSetting( "SMESH:ScalarBarTitleFont" ) == "Times" )
      aScalarBarTitleProp->SetFontFamilyToTimes();
  }

  if ( QAD_CONFIG->getSetting( "SMESH:ScalarBarTitleBold" ) == "true" )
    aScalarBarTitleProp->BoldOn();
  else
    aScalarBarTitleProp->BoldOff();

  if ( QAD_CONFIG->getSetting( "SMESH:ScalarBarTitleItalic" ) == "true" )
    aScalarBarTitleProp->ItalicOn();
  else
    aScalarBarTitleProp->ItalicOff();

  if ( QAD_CONFIG->getSetting( "SMESH:ScalarBarTitleShadow" ) == "true" )
    aScalarBarTitleProp->ShadowOn();
  else
    aScalarBarTitleProp->ShadowOff();

  myScalarBarActor->SetTitleTextProperty( aScalarBarTitleProp );
  aScalarBarTitleProp->Delete();

  vtkTextProperty* aScalarBarLabelProp = vtkTextProperty::New();

  if ( QAD_CONFIG->hasSetting( "SMESH:ScalarBarLabelColor" ) ) {
    QStringList aTColor = QStringList::split(  ":", QAD_CONFIG->getSetting( "SMESH:ScalarBarLabelColor" ), false );
    aScalarBarLabelProp->SetColor( ( aTColor.count() > 0 ? aTColor[0].toInt()/255. : 1.0 ),
				   ( aTColor.count() > 1 ? aTColor[1].toInt()/255. : 1.0 ), 
				   ( aTColor.count() > 2 ? aTColor[2].toInt()/255. : 1.0 ) );
  }
  else
    aScalarBarLabelProp->SetColor( 1.0, 1.0, 1.0 );

  aScalarBarLabelProp->SetFontFamilyToArial();
  if( QAD_CONFIG->hasSetting( "SMESH:ScalarBarLabelFont" ) ){
    if ( QAD_CONFIG->getSetting( "SMESH:ScalarBarLabelFont" ) == "Arial" )
      aScalarBarLabelProp->SetFontFamilyToArial();
    else if ( QAD_CONFIG->getSetting( "SMESH:ScalarBarLabelFont" ) == "Courier" )
      aScalarBarLabelProp->SetFontFamilyToCourier();
    else if ( QAD_CONFIG->getSetting( "SMESH:ScalarBarLabelFont" ) == "Times" )
      aScalarBarLabelProp->SetFontFamilyToTimes();
  }

  if ( QAD_CONFIG->getSetting( "SMESH:ScalarBarLabelBold" ) == "true" )
    aScalarBarLabelProp->BoldOn();
  else
    aScalarBarLabelProp->BoldOff();

  if ( QAD_CONFIG->getSetting( "SMESH:ScalarBarLabelItalic" ) == "true" )
    aScalarBarLabelProp->ItalicOn();
  else
    aScalarBarLabelProp->ItalicOff();

  if ( QAD_CONFIG->getSetting( "SMESH:ScalarBarLabelShadow" ) == "true" )
    aScalarBarLabelProp->ShadowOn();
  else
    aScalarBarLabelProp->ShadowOff();

  myScalarBarActor->SetLabelTextProperty( aScalarBarLabelProp );
  aScalarBarLabelProp->Delete();

  if ( QAD_CONFIG->getSetting("SMESH:ScalarBarOrientation") == "Horizontal" )
    myScalarBarActor->SetOrientationToHorizontal();
  else
    myScalarBarActor->SetOrientationToVertical();

  float aXVal = QAD_CONFIG->getSetting("SMESH:ScalarBarOrientation") == "Horizontal" ? 0.20 : 0.01;
  if ( QAD_CONFIG->hasSetting( "SMESH:ScalarBarXPosition" ) )
    aXVal = QAD_CONFIG->getSetting( "SMESH:ScalarBarXPosition" ).toFloat();
  float aYVal = QAD_CONFIG->getSetting("SMESH:ScalarBarOrientation") == "Horizontal" ? 0.01 : 0.1;
  if ( QAD_CONFIG->hasSetting( "SMESH:ScalarBarYPosition" ) )
    aYVal = QAD_CONFIG->getSetting( "SMESH:ScalarBarYPosition" ).toFloat();
  myScalarBarActor->SetPosition( aXVal, aYVal );

  float aWVal = QAD_CONFIG->getSetting("SMESH:ScalarBarOrientation") == "Horizontal" ? 0.60 : 0.10;
  if ( QAD_CONFIG->hasSetting( "SMESH:ScalarBarWidth" ) )
    aWVal = QAD_CONFIG->getSetting( "SMESH:ScalarBarWidth" ).toFloat();
  myScalarBarActor->SetWidth( aWVal );

  float aHVal = QAD_CONFIG->getSetting("SMESH:ScalarBarOrientation") == "Horizontal" ? 0.12 : 0.80;
  if ( QAD_CONFIG->hasSetting( "SMESH:ScalarBarHeight" ) )
    aHVal = QAD_CONFIG->getSetting( "SMESH:ScalarBarHeight" ).toFloat();
  myScalarBarActor->SetHeight( aHVal );

  int anIntVal = 5;
  if ( QAD_CONFIG->hasSetting( "SMESH:ScalarBarNbOfLabels" ) )
    anIntVal = QAD_CONFIG->getSetting("SMESH:ScalarBarNbOfLabels").toInt();
  myScalarBarActor->SetNumberOfLabels(anIntVal == 0? 5: anIntVal);

  anIntVal = 64;
  if ( QAD_CONFIG->hasSetting( "SMESH:ScalarBarNbOfColors" ) )
    anIntVal = QAD_CONFIG->getSetting("SMESH:ScalarBarNbOfColors").toInt();
  myScalarBarActor->SetMaximumNumberOfColors(anIntVal == 0? 64: anIntVal);


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
}


SMESH_Actor::~SMESH_Actor(){
  if(MYDEBUG) MESSAGE("~SMESH_Actor");

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
}


void SMESH_Actor::SetPointsLabeled( bool theIsPointsLabeled )
{
  myIsPointsLabeled = theIsPointsLabeled;

  if ( myIsPointsLabeled )
  {
    myPointsNumDataSet->ShallowCopy( GetUnstructuredGrid() );
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
  
  SetRepresentation( GetRepresentation() );
}


void SMESH_Actor::SetCellsLabeled(bool theIsCellsLabeled){
  myIsCellsLabeled = theIsCellsLabeled;
  if(myIsCellsLabeled){
    myCellsNumDataSet->ShallowCopy(GetUnstructuredGrid());
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
}


void SMESH_Actor::SetControlMode(eControl theMode){
  myColorMode = eNone;
  my1DColorMode = e1DNone;

  my1DActor->GetMapper()->SetScalarVisibility(false);
  my2DActor->GetMapper()->SetScalarVisibility(false);
  my3DActor->GetMapper()->SetScalarVisibility(false);
  myScalarBarActor->SetVisibility(false);

  bool anIsScalarVisible = theMode > eNone;

  if(anIsScalarVisible){
    SMESH::NumericalFunctor_var aFunctor;
    SMESH::Predicate_var aPredicate;
      
    switch(theMode){
    case eLengthEdges:
      aFunctor = myFilterMgr->CreateLength();
      myControlActor = my1DActor;
      break;
    case eFreeBorders:
      aPredicate = myFilterMgr->CreateFreeBorders();
      myControlActor = my1DActor;
      break;
    case eMultiConnection:
      aFunctor = myFilterMgr->CreateMultiConnection();
      myControlActor = my1DActor;
      break;
    case eArea:
      aFunctor = myFilterMgr->CreateArea();
      myControlActor = my2DActor;
      break;
    case eTaper:
      aFunctor = myFilterMgr->CreateTaper();
      myControlActor = my2DActor;
      break;
    case eAspectRatio:
      aFunctor = myFilterMgr->CreateAspectRatio();
      myControlActor = my2DActor;
      break;
    case eMinimumAngle:
      aFunctor = myFilterMgr->CreateMinimumAngle();
      myControlActor = my2DActor;
      break;
    case eWarping:
      aFunctor = myFilterMgr->CreateWarping();
      myControlActor = my2DActor;
      break;
    case eSkew:
      aFunctor = myFilterMgr->CreateSkew();
      myControlActor = my2DActor;
      break;
    default:
      return;
    }
    
    vtkUnstructuredGrid* aGrid = myControlActor->GetUnstructuredGrid();
    vtkIdType aNbCells = aGrid->GetNumberOfCells();
    if(aNbCells){
      if(theMode == eFreeBorders){
	if(!aPredicate->_is_nil()){
	  myVisualObj->UpdateFunctor(aPredicate);
	  SALOME_ExtractUnstructuredGrid* aFilter = 
	    my1DExtActor->GetExtractUnstructuredGrid();
	  aFilter->SetModeOfChanging(SALOME_ExtractUnstructuredGrid::eAdding);
	  aFilter->ClearRegisteredCells();
	  for( vtkIdType i = 0; i < aNbCells; i++ ){
	    vtkIdType anObjId = myControlActor->GetElemObjId( i );
	    CORBA::Boolean aValue = aPredicate->IsSatisfy(anObjId);
	    if(aValue)
	      aFilter->RegisterCell(i);
	  }
	  if(!aFilter->IsCellsRegistered())
	    aFilter->RegisterCell(-1);
	  aPredicate->Destroy();
	}
	myColorMode = theMode;
	my1DColorMode = e1DHighlited;
      }else{
	myColorMode = theMode;
	if(myControlActor == my1DActor)
	  my1DColorMode = e1DColored;
	
	vtkUnstructuredGrid* aDataSet = vtkUnstructuredGrid::New();
	aDataSet->ShallowCopy(aGrid);
	myScalarBarActor->SetVisibility(true);
	myControlActor->GetMapper()->SetScalarVisibility(true);
	
	vtkDoubleArray *aScalars = vtkDoubleArray::New();
	aScalars->SetNumberOfComponents( 1 );
	aScalars->SetNumberOfTuples(aNbCells);
	
	if(!aFunctor->_is_nil()){
	  myVisualObj->UpdateFunctor(aFunctor);
	  for( vtkIdType i = 0; i < aNbCells; i++ ){
	    vtkIdType anObjId = myControlActor->GetElemObjId( i );
	    CORBA::Double aValue = aFunctor->GetValue(anObjId);
	    aScalars->SetValue( i, aValue );
	  }
	  aFunctor->Destroy();
	}else if(!aPredicate->_is_nil()){
	  myVisualObj->UpdateFunctor(aPredicate);
	  for( vtkIdType i = 0; i < aNbCells; i++ ){
	    vtkIdType anObjId = myControlActor->GetElemObjId( i );
	    CORBA::Boolean aValue = aPredicate->IsSatisfy(anObjId);
	    aScalars->SetValue( i, aValue );
	  }
	  aPredicate->Destroy();
	}
	aDataSet->GetCellData()->SetScalars(aScalars);
	aScalars->Delete();
	
	float aRange[2];
	aScalars->GetRange(aRange);
	myLookupTable->SetRange(aRange);
	myLookupTable->Build();
	
	myControlActor->GetMergeFilter()->SetScalars(aDataSet);
	aDataSet->Delete();
	if(MYDEBUGWITHFILES) 
	  WriteUnstructuredGrid(aDataSet,"/tmp/SetControlMode.vtk");
      }
    }
  }
  SetRepresentation(GetRepresentation());
  
  Modified();
}


void SMESH_Actor::AddToRender(vtkRenderer* theRenderer){
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

void SMESH_Actor::RemoveFromRender(vtkRenderer* theRenderer){
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


void SMESH_Actor::Init(TVisualObjPtr theVisualObj, 
		       SMESH::FilterManager_ptr theFilterMgr,
		       const char* theEntry, 
		       const char* theName,
		       int theIsClear)
{
  Handle(SALOME_InteractiveObject) anIO =
    new SALOME_InteractiveObject(strdup(theEntry),strdup("MESH"),strdup(theName));
  setIO(anIO);
  setName(strdup(theName));

  myVisualObj = theVisualObj;
  myNodeActor->myVisualObj = myVisualObj;
  myBaseActor->myVisualObj = myVisualObj;
  myHighlitableActor->myVisualObj = myVisualObj;

  my1DActor->myVisualObj = myVisualObj;
  my1DExtActor->myVisualObj = myVisualObj;

  my2DActor->myVisualObj = myVisualObj;
  my3DActor->myVisualObj = myVisualObj;

  myVisualObj->Update(theIsClear);
  SetUnstructuredGrid(myVisualObj->GetUnstructuredGrid());
  
  myFilterMgr = SMESH::FilterManager::_duplicate(theFilterMgr);

  QString aMode = QAD_CONFIG->getSetting("SMESH:DisplayMode");
  SetRepresentation(-1);
  if(aMode.compare("Wireframe") == 0){
    SetRepresentation(eEdge);
  }else if(aMode.compare("Shading") == 0){
    SetRepresentation(eSurface);
  }else if(aMode.compare("Nodes") == 0){
    SetRepresentation(ePoint);
  }

  aMode = QAD_CONFIG->getSetting("SMESH:Shrink");
  if(aMode == "yes"){
    SetShrink();
  }

  Modified();
}


vtkPolyData* SMESH_Actor::GetPolyDataInput(){
  return myPickableActor->GetPolyDataInput();
}


void SMESH_Actor::SetTransform(SALOME_Transform* theTransform){
  myNodeActor->SetTransform(theTransform);
  myBaseActor->SetTransform(theTransform);
  myHighlitableActor->SetTransform(theTransform);

  my1DActor->SetTransform(theTransform);
  my1DExtActor->SetTransform(theTransform);

  my2DActor->SetTransform(theTransform);
  my3DActor->SetTransform(theTransform);

  Modified();
}


void SMESH_Actor::SetUnstructuredGrid(vtkUnstructuredGrid* theGrid){
  if(theGrid){
    myNodeActor->SetUnstructuredGrid(theGrid);
    myBaseActor->SetUnstructuredGrid(theGrid);
    myHighlitableActor->SetUnstructuredGrid(theGrid);

    my1DActor->SetUnstructuredGrid(theGrid);
    my1DExtActor->SetUnstructuredGrid(theGrid);

    my2DActor->SetUnstructuredGrid(theGrid);
    my3DActor->SetUnstructuredGrid(theGrid);

    my1DActor->GetMapper()->SetLookupTable(myLookupTable);
    my2DActor->GetMapper()->SetLookupTable(myLookupTable);
    my3DActor->GetMapper()->SetLookupTable(myLookupTable);
    
    float aFactor, aUnits;
    my2DActor->GetPolygonOffsetParameters(aFactor,aUnits);
    my2DActor->SetPolygonOffsetParameters(aFactor,aUnits*0.75);

    //SetIsShrunkable(theGrid->GetNumberOfCells() > 10);
    SetIsShrunkable(true);

    Modified();
  }
}


void SMESH_Actor::SetMapper(vtkMapper* theMapper){
  vtkLODActor::SetMapper(theMapper);
}


void SMESH_Actor::ShallowCopy(vtkProp *prop){
  SALOME_Actor::ShallowCopy(prop);
}


vtkMapper* SMESH_Actor::GetMapper(){
  return myPickableActor->GetMapper();
}


vtkUnstructuredGrid* SMESH_Actor::GetUnstructuredGrid(){ 
  return myVisualObj->GetUnstructuredGrid();
}


bool SMESH_Actor::IsInfinitive(){
  vtkDataSet *aDataSet = myPickableActor->GetUnstructuredGrid();
  aDataSet->Update();
  myIsInfinite = aDataSet->GetNumberOfCells() == 0;
  return SALOME_Actor::IsInfinitive();
}


void SMESH_Actor::SetIsShrunkable(bool theShrunkable){
  myIsShrinkable = theShrunkable;
  Modified();
}

float SMESH_Actor::GetShrinkFactor(){
  return myBaseActor->GetShrinkFactor();
}

void SMESH_Actor::SetShrinkFactor(float theValue){
  myBaseActor->SetShrinkFactor(theValue);

  my1DActor->SetShrinkFactor(theValue);
  my1DExtActor->SetShrinkFactor(theValue);

  my2DActor->SetShrinkFactor(theValue);
  my3DActor->SetShrinkFactor(theValue);

  Modified();
}

void SMESH_Actor::SetShrink(){
  if(!myIsShrinkable) return;

  myBaseActor->SetShrink();

  my1DActor->SetShrink();
  my1DExtActor->SetShrink();

  my2DActor->SetShrink();
  my3DActor->SetShrink();

  myIsShrunk = true;
  Modified();
}

void SMESH_Actor::UnShrink(){
  if(!myIsShrunk) return;

  myBaseActor->UnShrink();

  my1DActor->UnShrink();
  my1DExtActor->UnShrink();

  my2DActor->UnShrink();
  my3DActor->UnShrink();

  myIsShrunk = false;
  Modified();
}


int SMESH_Actor::GetObjId(int theVtkID){
  if (GetRepresentation() == 0){
    return GetNodeObjId(theVtkID);
  }else{
    return GetElemObjId(theVtkID);
  }
}

SALOME_Actor::TVectorId SMESH_Actor::GetVtkId(int theObjID){
  if (GetRepresentation() == 0){
    return GetNodeVtkId(theObjID);
  }else{
    return GetElemVtkId(theObjID);
  }
}


int SMESH_Actor::GetNodeObjId(int theVtkID){
  return myPickableActor->GetNodeObjId(theVtkID);
}

SALOME_Actor::TVectorId SMESH_Actor::GetNodeVtkId(int theObjID){
  return myPickableActor->GetNodeVtkId(theObjID);
}


int SMESH_Actor::GetElemObjId(int theVtkID){
  return myPickableActor->GetElemObjId(theVtkID);
}

SALOME_Actor::TVectorId SMESH_Actor::GetElemVtkId(int theObjID){
  return myPickableActor->GetElemVtkId(theObjID);
}


void SMESH_Actor::SetVisibility(int theMode){
  SALOME_Actor::SetVisibility(theMode);
  if(GetVisibility()){
    SetRepresentation(GetRepresentation());

    if(myColorMode != eNone){
      if(my1DColorMode == e1DHighlited)
	my1DExtActor->VisibilityOn();
      else if(myControlActor->GetUnstructuredGrid()->GetNumberOfCells())
	myScalarBarActor->VisibilityOn();
    }

    if(myRepresentation != ePoint)
      myPickableActor->VisibilityOn();

    my1DActor->VisibilityOn();

    my2DActor->VisibilityOn();
    my3DActor->VisibilityOn();

    if(myIsPointsLabeled) myPointLabels->VisibilityOn();
    if(myIsCellsLabeled) myCellsLabels->VisibilityOn();
  }else{
    myNodeActor->VisibilityOff();
    myBaseActor->VisibilityOff();

    my1DActor->VisibilityOff();
    my1DExtActor->VisibilityOff();

    my2DActor->VisibilityOff();
    my3DActor->VisibilityOff();

    myScalarBarActor->VisibilityOff();
    myPointLabels->VisibilityOff();
    myCellsLabels->VisibilityOff();
  }
  Modified();
}


void SMESH_Actor::SetRepresentation(int theMode){ 
  int aNbEdges = myVisualObj->GetNbEntities(SMESH::EDGE);
  int aNbFaces = myVisualObj->GetNbEntities(SMESH::FACE);
  int aNbVolumes = myVisualObj->GetNbEntities(SMESH::VOLUME);
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
  switch(my1DColorMode){
  case e1DColored: 
    aProp = aBackProp = my1DProp;
    if(myRepresentation != ePoint)
      aReperesent = SMESH_DeviceActor::eInsideframe;
    break;
  case e1DHighlited: 
    my1DExtActor->SetVisibility(true);
    break;
  }
  
  my1DActor->SetProperty(aProp);
  my1DActor->SetBackfaceProperty(aBackProp);
  my1DActor->SetRepresentation(aReperesent);
  
  if ( GetPointRepresentation() || !GetUnstructuredGrid()->GetNumberOfCells() || myIsPointsLabeled )
  {
    myPickableActor = myNodeActor;
    myNodeActor->SetVisibility(true);
  }

  SetMapper(myPickableActor->GetMapper());

  Modified();
}


void SMESH_Actor::SetPointRepresentation(int theIsPointsVisible){
  myIsPointsVisible = theIsPointsVisible;
  SetRepresentation(GetRepresentation());
}


void SMESH_Actor::UpdateHighlight(){
  myHighlitableActor->SetVisibility(false);
  myNodeActor->SetProperty(myNodeProp);
  if(myPickableActor != myNodeActor)
    myNodeActor->SetVisibility(false);

  if(ishighlighted){
    myHighlitableActor->SetProperty(myHighlightProp);
  }else if(ispreselected){
    myHighlitableActor->SetProperty(myPreselectProp);
  }

  if(ishighlighted || ispreselected){
    if(!GetUnstructuredGrid()->GetNumberOfCells()){
      myNodeActor->SetProperty(myHighlitableActor->GetProperty());
      myNodeActor->SetRepresentation(SMESH_DeviceActor::ePoint);
      myNodeActor->SetVisibility(GetVisibility());
    }else{
      myHighlitableActor->SetRepresentation(SMESH_DeviceActor::eWireframe);
      myHighlitableActor->SetVisibility(GetVisibility());
    }
  }
}


void SMESH_Actor::highlight(Standard_Boolean highlight){
  ishighlighted = highlight;
  UpdateHighlight();
}


void SMESH_Actor::SetPreSelected(Standard_Boolean presel){ 
  ispreselected = presel; 
  UpdateHighlight();
}


// From vtkFollower
int SMESH_Actor::RenderOpaqueGeometry(vtkViewport *vp)
{
  if (myPickableActor->GetIsOpaque())
    {
    vtkRenderer *ren = static_cast<vtkRenderer *>(vp);
    this->Render(ren);
    return 1;
    }
  return 0;
}


int SMESH_Actor::RenderTranslucentGeometry(vtkViewport *vp)
{
  if (!myPickableActor->GetIsOpaque())
    {
    vtkRenderer *ren = static_cast<vtkRenderer *>(vp);
    this->Render(ren);
    return 1;
    }
  return 0;
}


void SMESH_Actor::Render(vtkRenderer *ren){}


void SMESH_Actor::ReleaseGraphicsResources(vtkWindow *renWin){
  SALOME_Actor::ReleaseGraphicsResources(renWin);

  myPickableActor->ReleaseGraphicsResources(renWin);
}


static void GetColor(vtkProperty *theProperty, float& r,float& g,float& b){
  float* aColor = theProperty->GetColor();
  r = aColor[0];
  g = aColor[1];
  b = aColor[2];
}


void SMESH_Actor::SetOpacity(float theValue){
  mySurfaceProp->SetOpacity(theValue);
  myBackSurfaceProp->SetOpacity(theValue);
  myEdgeProp->SetOpacity(theValue);
  myNodeProp->SetOpacity(theValue);

  my1DProp->SetOpacity(theValue);
}


float SMESH_Actor::GetOpacity(){
  return mySurfaceProp->GetOpacity();
}


void SMESH_Actor::SetSufaceColor(float r,float g,float b){
  mySurfaceProp->SetColor(r,g,b);
  Modified();
}

void SMESH_Actor::GetSufaceColor(float& r,float& g,float& b){
  ::GetColor(mySurfaceProp,r,g,b);
}

void SMESH_Actor::SetBackSufaceColor(float r,float g,float b){
  myBackSurfaceProp->SetColor(r,g,b);
  Modified();
}

void SMESH_Actor::GetBackSufaceColor(float& r,float& g,float& b){
  ::GetColor(myBackSurfaceProp,r,g,b);
}

void SMESH_Actor::SetEdgeColor(float r,float g,float b){
  myEdgeProp->SetColor(r,g,b);
  my1DProp->SetColor(r,g,b);
  my1DExtProp->SetColor(1.0-r,1.0-g,1.0-b);
  Modified();
}

void SMESH_Actor::GetEdgeColor(float& r,float& g,float& b){
  ::GetColor(myEdgeProp,r,g,b);
}

void SMESH_Actor::SetNodeColor(float r,float g,float b){ 
  myNodeProp->SetColor(r,g,b);
  Modified();
}

void SMESH_Actor::GetNodeColor(float& r,float& g,float& b){ 
  ::GetColor(myNodeProp,r,g,b);
}

void SMESH_Actor::SetHighlightColor(float r,float g,float b){ 
  myHighlightProp->SetColor(r,g,b);
  Modified();
}

void SMESH_Actor::GetHighlightColor(float& r,float& g,float& b){ 
  ::GetColor(myHighlightProp,r,g,b);
}

void SMESH_Actor::SetPreHighlightColor(float r,float g,float b){ 
  myPreselectProp->SetColor(r,g,b);
  Modified();
}

void SMESH_Actor::GetPreHighlightColor(float& r,float& g,float& b){ 
  ::GetColor(myPreselectProp,r,g,b);
}


float SMESH_Actor::GetLineWidth(){
  return myEdgeProp->GetLineWidth();
}


void SMESH_Actor::SetLineWidth(float theVal){
  myEdgeProp->SetLineWidth(theVal);
  Modified();
}


void SMESH_Actor::SetNodeSize(float theSize){
  myNodeProp->SetPointSize(theSize);
  myHighlightProp->SetPointSize(theSize);
  myPreselectProp->SetPointSize(theSize);

  my1DExtProp->SetPointSize(theSize);
  my1DProp->SetPointSize(theSize);

  Modified();
}

float SMESH_Actor::GetNodeSize(){
  return myNodeProp->GetPointSize();
}

int SMESH_Actor::GetObjDimension( const int theObjId )
{
  return myVisualObj->GetElemDimension( theObjId );
}
