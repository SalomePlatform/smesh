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
//  File   : SMESH_NodeLabelActor.h
//  Author : Roman NIKOLAEV
//  Module : SMESH
//
#ifndef SMESH_NODE_LABEL_ACTOR_H
#define SMESH_NODE_LABEL_ACTOR_H

#include "SMESH_DeviceActor.h"

class vtkSelectVisiblePoints;
class vtkLabeledDataMapper;
class vtkActor2D;
class vtkMaskPoints;
class vtkUnstructuredGrid;


class SMESHOBJECT_EXPORT SMESH_NodeLabelActor : public SMESH_DeviceActor {
public:
  static SMESH_NodeLabelActor* New();

  static void ProcessEvents(vtkObject* theObject,
			    unsigned long theEvent,
			    void* theClientData,
			    void* theCallData);


  vtkTypeMacro(SMESH_NodeLabelActor, SMESH_DeviceActor);


  virtual void SetPointsLabeled(bool theIsPointsLabeled);
  virtual bool GetPointsLabeled(){ return myIsPointsLabeled;}  

  virtual void SetVisibility(int theMode);  

  virtual void AddToRender(vtkRenderer* theRenderer); 
  virtual void RemoveFromRender(vtkRenderer* theRenderer);

  void UpdateLabels();
  
protected:
  SMESH_NodeLabelActor();
  ~SMESH_NodeLabelActor();
  
  bool myIsPointsLabeled;
  vtkUnstructuredGrid* myPointsNumDataSet;
  vtkActor2D *myPointLabels;
  vtkMaskPoints* myPtsMaskPoints;
  vtkLabeledDataMapper* myPtsLabeledDataMapper;
  vtkSelectVisiblePoints* myPtsSelectVisiblePoints;

protected:
  // Not implemented.
  SMESH_NodeLabelActor(const SMESH_NodeLabelActor&);
  void operator=(const SMESH_NodeLabelActor&);
};

#endif //SMESH_NODE_LABEL_ACTOR_H
