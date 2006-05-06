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
//  File   : SMESH_DeviceActor.h
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#ifndef SMESH_DEVICE_ACTOR_H
#define SMESH_DEVICE_ACTOR_H

#include <VTKViewer_GeometryFilter.h>
#include "SMESH_Controls.hxx"
#include "SMESH_Object.h"

#include <vtkLODActor.h>
#include <vtkSmartPointer.h>

class vtkCell;
class vtkProperty;
class vtkMergeFilter;
class vtkShrinkFilter;
class vtkPolyDataMapper;
class vtkUnstructuredGrid;
class vtkScalarBarActor;
class vtkLookupTable;
class vtkImplicitBoolean;

class VTKViewer_Transform;
class VTKViewer_TransformFilter;
class VTKViewer_PassThroughFilter;
class VTKViewer_ExtractUnstructuredGrid;

class SMESH_ExtractGeometry;


class SMESH_DeviceActor: public vtkLODActor{
  friend class SMESH_ActorDef;

 public:
  vtkTypeMacro(SMESH_DeviceActor,vtkLODActor);
  static SMESH_DeviceActor* New();

  void SetStoreClippingMapping(bool theStoreMapping);
  void SetStoreGemetryMapping(bool theStoreMapping);
  void SetStoreIDMapping(bool theStoreMapping);

  virtual int GetNodeObjId(int theVtkID);
  virtual vtkFloatingPointType* GetNodeCoord(int theObjID);

  virtual int GetElemObjId(int theVtkID);
  virtual vtkCell* GetElemCell(int theObjID);

  virtual void SetTransform(VTKViewer_Transform* theTransform); 
  virtual unsigned long int GetMTime();

  vtkFloatingPointType GetShrinkFactor();
  void  SetShrinkFactor(vtkFloatingPointType value);

  bool IsShrunkable() { return myIsShrinkable;}
  bool IsShrunk() { return myIsShrunk;}
  void SetShrink(); 
  void UnShrink(); 

  enum EReperesent { ePoint, eWireframe, eSurface, eInsideframe};
  EReperesent GetRepresentation(){ return myRepresentation;}
  void SetRepresentation(EReperesent theMode);

  virtual void SetVisibility(int theMode);
  virtual int GetVisibility();

  VTKViewer_ExtractUnstructuredGrid* GetExtractUnstructuredGrid();
  vtkUnstructuredGrid* GetUnstructuredGrid();

  void SetControlMode(SMESH::Controls::FunctorPtr theFunctor,
		      vtkScalarBarActor* theScalarBarActor,
		      vtkLookupTable* theLookupTable);
  void SetExtControlMode(SMESH::Controls::FunctorPtr theFunctor,
			 vtkScalarBarActor* theScalarBarActor,
			 vtkLookupTable* theLookupTable);
  void SetExtControlMode(SMESH::Controls::FunctorPtr theFunctor);

  bool IsHighlited() { return myIsHighlited;}
  void SetHighlited(bool theIsHighlited);

  virtual void Render(vtkRenderer *, vtkMapper *);

  void SetImplicitFunctionUsed(bool theIsImplicitFunctionUsed);
  bool IsImplicitFunctionUsed() const{ return myIsImplicitFunctionUsed;}

 protected:
  void Init(TVisualObjPtr theVisualObj, vtkImplicitBoolean* theImplicitBoolean);
  void SetUnstructuredGrid(vtkUnstructuredGrid* theGrid);

  vtkPolyDataMapper *myMapper;
  TVisualObjPtr myVisualObj;

  vtkProperty *myProperty;
  EReperesent myRepresentation;

  SMESH_ExtractGeometry* myExtractGeometry;
  bool myIsImplicitFunctionUsed;

  vtkMergeFilter* myMergeFilter;
  VTKViewer_ExtractUnstructuredGrid* myExtractUnstructuredGrid;

  bool myStoreClippingMapping;
  VTKViewer_GeometryFilter *myGeomFilter;
  VTKViewer_TransformFilter *myTransformFilter;
  std::vector<VTKViewer_PassThroughFilter*> myPassFilter;

  vtkShrinkFilter* myShrinkFilter;
  bool myIsShrinkable;
  bool myIsShrunk;
  
  bool myIsHighlited;

  vtkFloatingPointType myPolygonOffsetFactor;
  vtkFloatingPointType myPolygonOffsetUnits;

  void
  SetPolygonOffsetParameters(vtkFloatingPointType factor, 
			     vtkFloatingPointType units);

  void
  GetPolygonOffsetParameters(vtkFloatingPointType& factor, 
			     vtkFloatingPointType& units)
  {
    factor = myPolygonOffsetFactor;
    units = myPolygonOffsetUnits;
  }

  SMESH_DeviceActor();
  ~SMESH_DeviceActor();
  SMESH_DeviceActor(const SMESH_DeviceActor&);
  void operator=(const SMESH_DeviceActor&);

};


#endif //SMESH_DEVICE_ACTOR_H
