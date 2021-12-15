// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
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

// File   : SMESHGUI_InteractiveOp.h
// Author : Roman NIKOLAEV, Open CASCADE S.A.S.
//

#pragma once

// SMESH includes
#include "SMESH_SMESHGUI.hxx"
#include "SMESHGUI_SelectionOp.h"

// VTK includes
#include <vtkSmartPointer.h>

/*
  Class       : SMESHGUI_InteractiveOp
  Description : Base operation for all interactive operations in the VTK 3D Viewer
*/

class vtkInteractorStyle;
class vtkCallbackCommand;
class vtkObject;
class SVTK_RenderWindowInteractor;

class SMESHGUI_EXPORT SMESHGUI_InteractiveOp: public SMESHGUI_SelectionOp {
  Q_OBJECT
public:
  SMESHGUI_InteractiveOp();
  virtual ~SMESHGUI_InteractiveOp();

  virtual void deactivateCurrentViewOperation();

protected:
  virtual void                   startOperation() override;

  static void                    ProcessStyleEvents(vtkObject* object,
                                                    unsigned long event,
                                                    void* clientdata,
                                                    void* calldata);
  virtual void                   processStyleEvents(unsigned long event,
                                                    void* calldata);

  static void                    ProcessInteractorEvents(vtkObject* object,
                                                         unsigned long event,
                                                         void* clientdata,
                                                          void* calldata);
  virtual void                   processInteractorEvents(unsigned long event,
                                                         void* calldata);

  virtual void addObserver();
  virtual void removeObserver();


protected:
  vtkInteractorStyle*           myInteractorStyle;
  SVTK_RenderWindowInteractor*  myRWInteractor;

  // Priority at which  events are processed:
  //   Interactor Stype events
  double myInteractorStypePriority;
  //   Interactor events
  double myInteractorPriority;

  // Used to process events
  vtkSmartPointer<vtkCallbackCommand> myStyleEventCallbackCommand;
  vtkSmartPointer<vtkCallbackCommand> myInteractorEventCallbackCommand;
};
