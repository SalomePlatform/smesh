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
//  File   : SMESH_DeviceActor.h
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$
//
#ifndef SMESH_DEVICE_ACTOR_H
#define SMESH_DEVICE_ACTOR_H

#include <VTKViewer_GeometryFilter.h>
#include <VTKViewer_MarkerDef.h>
#include "SMESH_Controls.hxx"
#include "SMESH_Object.h"

#include <vtkLODActor.h>
#include <vtkSmartPointer.h>

class vtkCell;
class vtkProperty;
class vtkMergeFilter;
class vtkShrinkFilter;
class vtkUnstructuredGrid;
class vtkLookupTable;
class vtkImplicitBoolean;
class vtkPassThroughFilter;

class VTKViewer_Transform;
class VTKViewer_TransformFilter;
class VTKViewer_ExtractUnstructuredGrid;
class VTKViewer_PolyDataMapper;

class SMESH_ExtractGeometry;
class SMESH_FaceOrientationFilter;
class SMESH_ScalarBarActor;


class SMESHOBJECT_EXPORT SMESH_DeviceActor: public vtkLODActor{
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

  virtual void SetFacesOriented(bool theIsFacesOriented);
  virtual bool GetFacesOriented() { return myIsFacesOriented; }

  virtual void SetFacesOrientationColor(vtkFloatingPointType theColor[3]);
  virtual void GetFacesOrientationColor(vtkFloatingPointType theColor[3]);

  virtual void SetFacesOrientationScale(vtkFloatingPointType theScale);
  virtual vtkFloatingPointType GetFacesOrientationScale();

  virtual void SetFacesOrientation3DVectors(bool theState);
  virtual bool GetFacesOrientation3DVectors();

  //----------------------------------------------------------------------------
  //! Setting for displaying quadratic elements
  virtual void SetQuadraticArcMode(bool theFlag);
  virtual bool GetQuadraticArcMode();
  
  virtual void SetQuadraticArcAngle(vtkFloatingPointType theMaxAngle);
  virtual vtkFloatingPointType GetQuadraticArcAngle();
  
  void UpdateFaceOrientation();

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

  virtual void AddToRender(vtkRenderer* theRenderer); 
  virtual void RemoveFromRender(vtkRenderer* theRenderer);

  VTKViewer_ExtractUnstructuredGrid* GetExtractUnstructuredGrid();
  vtkUnstructuredGrid* GetUnstructuredGrid();

  void SetControlMode(SMESH::Controls::FunctorPtr theFunctor,
                      SMESH_ScalarBarActor* theScalarBarActor,
                      vtkLookupTable* theLookupTable);
  void SetExtControlMode(SMESH::Controls::FunctorPtr theFunctor,
                         SMESH_ScalarBarActor* theScalarBarActor,
                         vtkLookupTable* theLookupTable);
  void SetExtControlMode(SMESH::Controls::FunctorPtr theFunctor);

  bool IsHighlited() { return myIsHighlited;}
  void SetHighlited(bool theIsHighlited);

  virtual void Render(vtkRenderer *, vtkMapper *);

  void SetImplicitFunctionUsed(bool theIsImplicitFunctionUsed);
  bool IsImplicitFunctionUsed() const{ return myIsImplicitFunctionUsed;}

  void SetMarkerEnabled( bool );
  void SetMarkerStd( VTK::MarkerType, VTK::MarkerScale );
  void SetMarkerTexture( int, VTK::MarkerTexture );
  VTK::MarkerType GetMarkerType();
  VTK::MarkerScale GetMarkerScale();
  int GetMarkerTexture();

 protected:
  void Init(TVisualObjPtr theVisualObj, vtkImplicitBoolean* theImplicitBoolean);
  void SetUnstructuredGrid(vtkUnstructuredGrid* theGrid);

  VTKViewer_PolyDataMapper *myMapper;
  TVisualObjPtr myVisualObj;

  vtkProperty *myProperty;
  EReperesent myRepresentation;

  SMESH_ExtractGeometry* myExtractGeometry;
  bool myIsImplicitFunctionUsed;

  vtkMergeFilter* myMergeFilter;
  VTKViewer_ExtractUnstructuredGrid* myExtractUnstructuredGrid;

  bool myIsFacesOriented;
  SMESH_FaceOrientationFilter* myFaceOrientationFilter;
  vtkPolyDataMapper* myFaceOrientationDataMapper;
  vtkActor* myFaceOrientation;

  bool myStoreClippingMapping;
  VTKViewer_GeometryFilter *myGeomFilter;
  VTKViewer_TransformFilter *myTransformFilter;
  std::vector<vtkPassThroughFilter*> myPassFilter;

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
