// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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
//  File   : SMESH_NodeLabelActor.cxx
//  Author : Roman NIKOLAEV
//  Module : SMESH
//
#include "SMESH_NodeLabelActor.h"

#include <VTKViewer_TransformFilter.h>

#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkMaskPoints.h>
#include <vtkSelectVisiblePoints.h>
#include <vtkLabeledDataMapper.h>
#include <vtkActor2D.h>
#include <vtkTextProperty.h>
#include <vtkPointData.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkUnstructuredGrid.h>

vtkStandardNewMacro(SMESH_NodeLabelActor);

/*!
  Constructor.
*/
SMESH_NodeLabelActor::SMESH_NodeLabelActor() {
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
  myPtsLabeledDataMapper->SetLabelFormat("%d");
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

  vtkCallbackCommand* callBackCommand = vtkCallbackCommand::New();
  callBackCommand->SetClientData(this);
  callBackCommand->SetCallback(SMESH_NodeLabelActor::ProcessEvents);

  myTransformFilter->AddObserver("VTKViewer_TransformFilter::TransformationFinished",
				 callBackCommand);
  callBackCommand->Delete();
}

/*!
  Destructor
*/
SMESH_NodeLabelActor::~SMESH_NodeLabelActor() {
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

}

void SMESH_NodeLabelActor::SetPointsLabeled(bool theIsPointsLabeled) {
  myTransformFilter->Update();
  vtkDataSet* aGrid = vtkUnstructuredGrid::SafeDownCast(myTransformFilter->GetOutput());

  if(!aGrid)
    return;
    
  myIsPointsLabeled = theIsPointsLabeled && aGrid->GetNumberOfPoints();

  if ( myIsPointsLabeled )
  {
    myPointsNumDataSet->ShallowCopy(aGrid);
    vtkUnstructuredGrid *aDataSet = myPointsNumDataSet;
    
    int aNbElem = aDataSet->GetNumberOfPoints();
    
    vtkIntArray *anArray = vtkIntArray::New();
    anArray->SetNumberOfValues( aNbElem );
    
    for ( vtkIdType anId = 0; anId < aNbElem; anId++ )
    {
      int aSMDSId = myVisualObj->GetNodeObjId( anId );
      anArray->SetValue( anId, aSMDSId );
    }
    
    aDataSet->GetPointData()->SetScalars( anArray );
    myPtsMaskPoints->SetInput( aDataSet );
    myPointLabels->SetVisibility( GetVisibility() );
    anArray->Delete();
  }
  else
  {
    myPointLabels->SetVisibility( false );
  } 
}


void SMESH_NodeLabelActor::SetVisibility(int theMode)
{
  SMESH_DeviceActor::SetVisibility(theMode);
  myPointLabels->VisibilityOff();
  if(myIsPointsLabeled && theMode)
    myPointLabels->VisibilityOn();
}


void SMESH_NodeLabelActor::AddToRender(vtkRenderer* theRenderer)
{
  SMESH_DeviceActor::AddToRender(theRenderer);
  myPtsSelectVisiblePoints->SetRenderer(theRenderer);
  theRenderer->AddActor2D(myPointLabels);
}

void SMESH_NodeLabelActor::RemoveFromRender(vtkRenderer* theRenderer)
{
  theRenderer->RemoveActor(myPointLabels);
  SMESH_DeviceActor::RemoveFromRender(theRenderer);
}

void SMESH_NodeLabelActor::UpdateLabels() {
  if(myIsPointsLabeled)
    SetPointsLabeled(myIsPointsLabeled);
}


void SMESH_NodeLabelActor::ProcessEvents(vtkObject* vtkNotUsed(theObject),
					 unsigned long theEvent,
					 void* theClientData,
					 void* vtkNotUsed(theCallData)) {
  SMESH_NodeLabelActor* self = reinterpret_cast<SMESH_NodeLabelActor*>(theClientData);    
  if(self)
    self->UpdateLabels();
}
