// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

#include "SMESHGUI_IdPreview.h"

#include <SALOME_Actor.h>
#include <SMDS_Mesh.hxx>
#include <SVTK_ViewWindow.h>

#include <TColStd_MapIteratorOfMapOfInteger.hxx>

#include <vtkActor2D.h>
#include <vtkDataSetMapper.h>
#include <vtkLabeledDataMapper.h>
#include <vtkMaskPoints.h>
#include <vtkPointData.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkSelectVisiblePoints.h>
#include <vtkTextProperty.h>
#include <vtkUnstructuredGrid.h>

// Extracted from SMESHGUI_MergeDlg.cxx

SMESHGUI_IdPreview::SMESHGUI_IdPreview(SVTK_ViewWindow* theViewWindow):
  myViewWindow(theViewWindow)
{
  myIdGrid = vtkUnstructuredGrid::New();

  // Create and display actor
  vtkDataSetMapper* aMapper = vtkDataSetMapper::New();
  aMapper->SetInputData( myIdGrid );

  myIdActor = SALOME_Actor::New();
  myIdActor->SetInfinitive(true);
  myIdActor->VisibilityOff();
  myIdActor->PickableOff();

  myIdActor->SetMapper( aMapper );
  aMapper->Delete();

  myViewWindow->AddActor(myIdActor);

  //Definition of points numbering pipeline
  myPointsNumDataSet = vtkUnstructuredGrid::New();

  myPtsMaskPoints = vtkMaskPoints::New();
  myPtsMaskPoints->SetInputData(myPointsNumDataSet);
  myPtsMaskPoints->SetOnRatio(1);

  myPtsSelectVisiblePoints = vtkSelectVisiblePoints::New();
  myPtsSelectVisiblePoints->SetInputConnection(myPtsMaskPoints->GetOutputPort());
  myPtsSelectVisiblePoints->SelectInvisibleOff();
  myPtsSelectVisiblePoints->SetTolerance(0.1);
    
  myPtsLabeledDataMapper = vtkLabeledDataMapper::New();
  myPtsLabeledDataMapper->SetInputConnection(myPtsSelectVisiblePoints->GetOutputPort());
  myPtsLabeledDataMapper->SetLabelModeToLabelScalars();
    
  vtkTextProperty* aPtsTextProp = vtkTextProperty::New();
  aPtsTextProp->SetFontFamilyToTimes();
  static int aPointsFontSize = 12;
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

  AddToRender(myViewWindow->getRenderer());
}

void SMESHGUI_IdPreview::SetPointsData ( SMDS_Mesh*                   theMesh,
                                         const TColStd_MapOfInteger & theNodesIdMap )
{
  vtkPoints* aPoints = vtkPoints::New();
  aPoints->SetNumberOfPoints(theNodesIdMap.Extent());
  myIDs.clear();

  TColStd_MapIteratorOfMapOfInteger idIter( theNodesIdMap );
  for( int i = 0; idIter.More(); idIter.Next(), i++ )
  {
    const SMDS_MeshNode* aNode = theMesh->FindNode(idIter.Key());
    aPoints->SetPoint( i, aNode->X(), aNode->Y(), aNode->Z() );
    myIDs.push_back(idIter.Key());
  }

  myIdGrid->SetPoints(aPoints);

  aPoints->Delete();

  myIdActor->GetMapper()->Update();
}

void SMESHGUI_IdPreview::SetElemsData( const std::vector<int> & theElemsIdMap,
                                       const std::list<gp_XYZ> & aGrCentersXYZ )
{
  vtkPoints* aPoints = vtkPoints::New();
  aPoints->SetNumberOfPoints( theElemsIdMap.size() );
  myIDs = theElemsIdMap;

  std::list<gp_XYZ>::const_iterator coordIt = aGrCentersXYZ.begin();
  for( int i = 0; coordIt != aGrCentersXYZ.end(); coordIt++, i++ )
    aPoints->SetPoint( i, coordIt->X(), coordIt->Y(), coordIt->Z() );

  myIdGrid->SetPoints(aPoints);
  aPoints->Delete();

  myIdActor->GetMapper()->Update();
}

void SMESHGUI_IdPreview::AddToRender(vtkRenderer* theRenderer)
{
  myIdActor->AddToRender(theRenderer);

  myPtsSelectVisiblePoints->SetRenderer(theRenderer);
  theRenderer->AddActor2D(myPointLabels);
}

void SMESHGUI_IdPreview::RemoveFromRender(vtkRenderer* theRenderer)
{
  myIdActor->RemoveFromRender(theRenderer);

  myPtsSelectVisiblePoints->SetRenderer(theRenderer);
  theRenderer->RemoveActor(myPointLabels);
}

void SMESHGUI_IdPreview::SetPointsLabeled( bool theIsPointsLabeled, bool theIsActorVisible )
{
  myIsPointsLabeled = theIsPointsLabeled && myIdGrid->GetNumberOfPoints();

  if ( myIsPointsLabeled ) {
    myPointsNumDataSet->ShallowCopy(myIdGrid);
    vtkDataSet *aDataSet = myPointsNumDataSet;
    int aNbElem = myIDs.size();
    vtkIntArray *anArray = vtkIntArray::New();
    anArray->SetNumberOfValues( aNbElem );
    for ( int i = 0; i < aNbElem; i++ )
      anArray->SetValue( i, myIDs[i] );
    aDataSet->GetPointData()->SetScalars( anArray );
    anArray->Delete();
    myPtsMaskPoints->SetInputData( aDataSet );
    myPointLabels->SetVisibility( theIsActorVisible );
  }
  else {
    myPointLabels->SetVisibility( false );
  }
}

SMESHGUI_IdPreview::~SMESHGUI_IdPreview()
{
  RemoveFromRender(myViewWindow->getRenderer());

  myIdGrid->Delete();

  myViewWindow->RemoveActor(myIdActor);
  myIdActor->Delete();

  //Deleting of points numbering pipeline
  //---------------------------------------
  myPointsNumDataSet->Delete();

  //myPtsLabeledDataMapper->RemoveAllInputs();        //vtk 5.0 porting
  myPtsLabeledDataMapper->Delete();

  //myPtsSelectVisiblePoints->UnRegisterAllOutputs(); //vtk 5.0 porting
  myPtsSelectVisiblePoints->Delete();

  //myPtsMaskPoints->UnRegisterAllOutputs();          //vtk 5.0 porting
  myPtsMaskPoints->Delete();

  myPointLabels->Delete();

  //       myTimeStamp->Delete();
}
