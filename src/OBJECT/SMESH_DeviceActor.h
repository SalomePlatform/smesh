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
//  File   : SMESH_Actor.h
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#ifndef SMESH_DEVICE_ACTOR_H
#define SMESH_DEVICE_ACTOR_H

#include "SALOME_GeometryFilter.h"
#include "SMESH_Object.h"

#include <vtkLODActor.h>

class vtkProperty;
class vtkMergeFilter;
class vtkShrinkFilter;
class vtkPolyDataMapper;
class vtkUnstructuredGrid;

class SALOME_Transform;
class SALOME_TransformFilter;
class SALOME_PassThroughFilter;
class SALOME_ExtractUnstructuredGrid;


class SMESH_DeviceActor: public vtkLODActor{
  friend class SMESH_Actor;

 public:
  vtkTypeMacro(SMESH_DeviceActor,vtkLODActor);
  static SMESH_DeviceActor* New();

  void SetStoreMapping(int theStoreMapping);
  int GetStoreMapping(){ return myStoreMapping;}

  typedef SALOME_GeometryFilter::TVectorId TVectorId;
  int GetObjId(int theVtkID);
  TVectorId GetVtkId(int theObjID);

  int GetNodeObjId(int theVtkID);
  TVectorId GetNodeVtkId(int theObjID);

  int GetElemObjId(int theVtkID);
  TVectorId GetElemVtkId(int theObjID);

  vtkPolyData* GetPolyDataInput(); 

  virtual void SetTransform(SALOME_Transform* theTransform); 
  virtual unsigned long int GetMTime();

  float GetShrinkFactor();
  void  SetShrinkFactor(float value);

  bool IsShrunkable() { return myIsShrinkable;}
  bool IsShrunk() { return myIsShrunk;}
  void SetShrink(); 
  void UnShrink(); 

  enum EReperesent { ePoint, eWireframe, eSurface, eInsideframe};
  EReperesent GetRepresentation(){ return myRepresentation;}
  void SetRepresentation(EReperesent theMode);

  virtual void SetVisibility(int theMode);
  virtual int GetVisibility();

  SALOME_ExtractUnstructuredGrid* GetExtractUnstructuredGrid();
  vtkUnstructuredGrid* GetUnstructuredGrid();
  vtkMergeFilter* GetMergeFilter();

  virtual void Render(vtkRenderer *, vtkMapper *);

 protected:
  void SetUnstructuredGrid(vtkUnstructuredGrid* theGrid);

  vtkPolyDataMapper *myMapper;
  TVisualObjPtr myVisualObj;

  vtkProperty *myProperty;
  EReperesent myRepresentation;

  vtkMergeFilter* myMergeFilter;
  SALOME_ExtractUnstructuredGrid* myExtractUnstructuredGrid;

  bool myStoreMapping;
  SALOME_GeometryFilter *myGeomFilter;
  SALOME_TransformFilter *myTransformFilter;
  std::vector<SALOME_PassThroughFilter*> myPassFilter;

  vtkShrinkFilter* myShrinkFilter;
  bool myIsShrinkable;
  bool myIsShrunk;
  
  float myPolygonOffsetFactor;
  float myPolygonOffsetUnits;

  void SetPolygonOffsetParameters(float factor, float units);
  void GetPolygonOffsetParameters(float& factor, float& units){
    factor = myPolygonOffsetFactor;
    units = myPolygonOffsetUnits;
  }

  SMESH_DeviceActor();
  ~SMESH_DeviceActor();
  SMESH_DeviceActor(const SMESH_DeviceActor&) {};
  void operator=(const SMESH_DeviceActor&) {};

};


#endif //SMESH_ACTOR_H
