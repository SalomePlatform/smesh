// Copyright (C) 2007-2023  CEA/DEN, EDF R&D, OPEN CASCADE
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

// File   : SMESHGUI_InteractiveOp.cxx
// Author : Roman NIKOLAEV, Open CASCADE S.A.S.

// SMESH includes
//
#include "SMESHGUI.h"
#include "SMESHGUI_InteractiveOp.h"
#include "SMESHGUI_VTKUtils.h"

// GUI includes
#include <SVTK_ViewWindow.h>
#include <SVTK_RenderWindowInteractor.h>
#include <SVTK_Event.h>

// VTK includes
#include <vtkInteractorStyle.h>
#include <vtkCallbackCommand.h>
#include <vtkGenericRenderWindowInteractor.h>
#include <vtkInteractorObserver.h>


//================================================================================
/*!
 * \brief Constructor
*/
//================================================================================

SMESHGUI_InteractiveOp::SMESHGUI_InteractiveOp() :
  SMESHGUI_SelectionOp(),
  myInteractorStyle(0),
  myRWInteractor(0),
  myStyleEventCallbackCommand(vtkCallbackCommand::New()),
  myInteractorStypePriority(0.0),
  myInteractorEventCallbackCommand(vtkCallbackCommand::New()),
  myInteractorPriority(1.0)
{
  myStyleEventCallbackCommand->Delete();
  myStyleEventCallbackCommand->SetClientData(this);
  myStyleEventCallbackCommand->SetCallback(SMESHGUI_InteractiveOp::ProcessStyleEvents);

  myInteractorEventCallbackCommand->Delete();
  myInteractorEventCallbackCommand->SetClientData(this);
  myInteractorEventCallbackCommand->SetCallback(SMESHGUI_InteractiveOp::ProcessInteractorEvents);
}

//================================================================================
/*!
 * \brief Destructor
*/
//================================================================================
SMESHGUI_InteractiveOp::~SMESHGUI_InteractiveOp() 
{
}

//=======================================================================
// function : addObserver()
// purpose  : Add VTK observers to process SVTK_InteractorStyle and vtkGenericRenderWindowInteractor events
//======================================================================
void SMESHGUI_InteractiveOp::addObserver()
{
  if (myInteractorStyle && !myInteractorStyle->HasObserver(SVTK::InteractiveSelectionChanged, myStyleEventCallbackCommand.GetPointer())) {
    myInteractorStyle->AddObserver(SVTK::InteractiveSelectionChanged, myStyleEventCallbackCommand.GetPointer(),
      myInteractorStypePriority);
  }
  if (myRWInteractor && myRWInteractor->GetDevice() &&
    !myRWInteractor->GetDevice()->HasObserver(vtkCommand::LeftButtonPressEvent, myInteractorEventCallbackCommand.GetPointer())) {
    myRWInteractor->GetDevice()->AddObserver(vtkCommand::LeftButtonPressEvent, myInteractorEventCallbackCommand.GetPointer(),
      myInteractorPriority);
  }
}

//=======================================================================
// function : removeObserver()
// purpose  : Remove VTK observers
//======================================================================
void SMESHGUI_InteractiveOp::removeObserver() {
  if (myInteractorStyle && myInteractorStyle->HasObserver(SVTK::InteractiveSelectionChanged, myStyleEventCallbackCommand.GetPointer())) {
    myInteractorStyle->RemoveObserver(myStyleEventCallbackCommand.GetPointer());
  }
  if (myRWInteractor && myRWInteractor->GetDevice() &&
    myRWInteractor->GetDevice()->HasObserver(vtkCommand::LeftButtonPressEvent, myInteractorEventCallbackCommand.GetPointer())) {
    myRWInteractor->GetDevice()->RemoveObserver(myInteractorEventCallbackCommand.GetPointer());
  }
}

//=======================================================================
// function : startOperation()
// purpose  : Init dialog fields, connect signals and slots, show dialog
//=======================================================================

void SMESHGUI_InteractiveOp::startOperation()
{
  SVTK_ViewWindow* svtkViewWindow = SMESH::GetViewWindow(getSMESHGUI());
  if (svtkViewWindow) {
    myInteractorStyle = svtkViewWindow->GetInteractorStyle();
    myRWInteractor = svtkViewWindow->GetInteractor();
  }
}

//================================================================================
/*!
 * \brief Process interactor style events
          Static method. Used by vtkCallbackCommand->SetCallback method.
 */
 //===============================================================================
void SMESHGUI_InteractiveOp::ProcessStyleEvents(vtkObject* vtkNotUsed(theObject),
  unsigned long theEvent,
  void* theClientData,
  void* theCallData) {
  SMESHGUI_InteractiveOp* self = reinterpret_cast<SMESHGUI_InteractiveOp*>(theClientData);
  if (self)
    self->processStyleEvents(theEvent, theCallData);
}

//================================================================================
/*!
 * \brief Process Generic Render Window Interactor events.
          Static method. Used by vtkCallbackCommand->SetCallback method.
 */
 //===============================================================================
void SMESHGUI_InteractiveOp::ProcessInteractorEvents(vtkObject* vtkNotUsed(theObject),
  unsigned long theEvent,
  void* theClientData,
  void* theCallData) {
  SMESHGUI_InteractiveOp* self = reinterpret_cast<SMESHGUI_InteractiveOp*>(theClientData);
  if (self)
    self->processInteractorEvents(theEvent, theCallData);
}

//================================================================================
/*!
 * \brief Process interactor style events ()
          Virtual method. Should be overridden in inherited classes.
 */
 //===============================================================================

void SMESHGUI_InteractiveOp::processStyleEvents(unsigned long theEvent, void* theCallData) {
}

//================================================================================
/*!
 * \brief Process Generic Render Window Interactor events.
          Virtual method. Should be overridden in inherited classes.
 */
 //===============================================================================
void SMESHGUI_InteractiveOp::processInteractorEvents(unsigned long theEvent, void* theCallData)
{
}

//================================================================================
/*!
 * \brief Deactivate current operation in active VTK viewer
 */
 //===============================================================================
void SMESHGUI_InteractiveOp::deactivateCurrentViewOperation() {
  if (SVTK_ViewWindow* svtkViewWindow = SMESH::GetViewWindow(getSMESHGUI())) {
    svtkViewWindow->deactivateCurrectOperation();
  }
}