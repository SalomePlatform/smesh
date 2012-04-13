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
//  File   : SMESH_CellLabelActor.cxx
//  Author : Roman NIKOLAEV
//  Module : SMESH
//
#include "SMESH_CellLabelActor.h"

#include <VTKViewer_TransformFilter.h>
#include <VTKViewer_CellCenters.h>

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
#include <vtkCellData.h>

vtkStandardNewMacro(SMESH_CellLabelActor);

/*!
  Constructor.
*/
SMESH_CellLabelActor::SMESH_CellLabelActor() {
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

  myClsLabeledDataMapper->SetLabelFormat("%d");
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

  vtkCallbackCommand* callBackCommand = vtkCallbackCommand::New();
  callBackCommand->SetClientData(this);
  callBackCommand->SetCallback(SMESH_CellLabelActor::ProcessEvents);

  myTransformFilter->AddObserver("VTKViewer_TransformFilter::TransformationFinished",
				 callBackCommand);
  callBackCommand->Delete();
}


/*!
  Destructor.
*/
SMESH_CellLabelActor::~SMESH_CellLabelActor() {
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
}


void SMESH_CellLabelActor::SetCellsLabeled(bool theIsCellsLabeled) {
  myTransformFilter->Update();
  vtkUnstructuredGrid* aGrid = vtkUnstructuredGrid::SafeDownCast(myTransformFilter->GetOutput());
  if(!aGrid)
    return;

  myIsCellsLabeled = theIsCellsLabeled && aGrid->GetNumberOfPoints();
  if(myIsCellsLabeled){
    myCellsNumDataSet->ShallowCopy(aGrid);
    vtkUnstructuredGrid *aDataSet = myCellsNumDataSet;
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

void SMESH_CellLabelActor::SetVisibility(int theMode)
{
  SMESH_DeviceActor::SetVisibility(theMode);
  myCellsLabels->VisibilityOff();
  if(myIsCellsLabeled && theMode)
    myCellsLabels->VisibilityOn();
}

void SMESH_CellLabelActor::AddToRender(vtkRenderer* theRenderer)
{
  SMESH_DeviceActor::AddToRender(theRenderer);
  myClsSelectVisiblePoints->SetRenderer(theRenderer);
  theRenderer->AddActor2D(myCellsLabels);
}

void SMESH_CellLabelActor::RemoveFromRender(vtkRenderer* theRenderer)
{
  theRenderer->RemoveActor(myCellsLabels);
  SMESH_DeviceActor::RemoveFromRender(theRenderer);
}

void SMESH_CellLabelActor::UpdateLabels() {
  if(myIsCellsLabeled)
    SetCellsLabeled(myIsCellsLabeled);
}


void SMESH_CellLabelActor::ProcessEvents(vtkObject* vtkNotUsed(theObject),
					 unsigned long theEvent,
					 void* theClientData,
					 void* vtkNotUsed(theCallData)) {
  SMESH_CellLabelActor* self = reinterpret_cast<SMESH_CellLabelActor*>(theClientData);
  if(self)
    self->UpdateLabels();
}
