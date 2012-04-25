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

//  SMESH OBJECT : interactive object for SMESH visualization
//  File   : SMESH_ActorDef.h
//  Author : Nicolas REJNERI
//  Module : SMESH
//
#ifndef SMESH_ACTORDEF_H
#define SMESH_ACTORDEF_H

#include "SMESH_Actor.h"
#include "SMESH_Object.h"

#include <vtkSmartPointer.h>

template <class T>
class TVTKSmartPtr: public vtkSmartPointer<T>
{
public:
  TVTKSmartPtr() {}
  TVTKSmartPtr(T* r, bool theIsOwner = false): vtkSmartPointer<T>(r) { 
    if(r && theIsOwner) 
      r->Delete();
  }
  TVTKSmartPtr& operator()(T* r, bool theIsOwner = false){ 
    vtkSmartPointer<T>::operator=(r); 
    if(r && theIsOwner) 
      r->Delete();
    return *this;
  }
  TVTKSmartPtr& operator=(T* r){ vtkSmartPointer<T>::operator=(r); return *this;}
  T* Get() const { return this->GetPointer();}
};


class vtkProperty;
class vtkShrinkFilter;
class vtkPolyDataMapper;
class vtkUnstructuredGrid;
class vtkMergeFilter;
class vtkPolyData;
class vtkMapper;
class vtkActor2D;
class vtkLookupTable;
class vtkPlane;
class vtkImplicitBoolean;
class vtkTimeStamp;

class SMESH_DeviceActor;
class SMESH_NodeLabelActor;
class SMESH_CellLabelActor;
class SMESH_ScalarBarActor;

#ifndef DISABLE_PLOT2DVIEWER
class SPlot2d_Histogram;
#endif


class SMESH_ActorDef : public SMESH_Actor
{
  friend class SMESH_VisualObj;
  friend class SMESH_Actor;

 public:
  vtkTypeMacro(SMESH_ActorDef,SMESH_Actor);
  
  virtual void Delete();

  virtual void ReleaseGraphicsResources(vtkWindow *renWin);
  virtual int RenderOpaqueGeometry(vtkViewport *viewport);
  virtual int RenderTranslucentGeometry(vtkViewport *viewport);
  virtual void Render(vtkRenderer *ren);

  virtual void AddToRender(vtkRenderer* theRenderer); 
  virtual void RemoveFromRender(vtkRenderer* theRenderer);

  virtual bool hasHighlight() { return true; }  
  virtual void highlight(bool theHighlight);  
  virtual void SetPreSelected(bool thePreselect = false);

  virtual bool IsInfinitive();  

  virtual void SetOpacity(vtkFloatingPointType theValue);
  virtual vtkFloatingPointType GetOpacity();

  virtual void SetSufaceColor(vtkFloatingPointType r,vtkFloatingPointType g,vtkFloatingPointType b, int delta );
  virtual void GetSufaceColor(vtkFloatingPointType& r,vtkFloatingPointType& g,vtkFloatingPointType& b, int& delta);

  virtual void SetEdgeColor(vtkFloatingPointType r,vtkFloatingPointType g,vtkFloatingPointType b);
  virtual void GetEdgeColor(vtkFloatingPointType& r,vtkFloatingPointType& g,vtkFloatingPointType& b);

  virtual void SetOutlineColor(vtkFloatingPointType r,vtkFloatingPointType g,vtkFloatingPointType b);
  virtual void GetOutlineColor(vtkFloatingPointType& r,vtkFloatingPointType& g,vtkFloatingPointType& b);


  virtual void SetNodeColor(vtkFloatingPointType r,vtkFloatingPointType g,vtkFloatingPointType b);
  virtual void GetNodeColor(vtkFloatingPointType& r,vtkFloatingPointType& g,vtkFloatingPointType& b);

  virtual void Set0DColor(vtkFloatingPointType r,vtkFloatingPointType g,vtkFloatingPointType b);
  virtual void Get0DColor(vtkFloatingPointType& r,vtkFloatingPointType& g,vtkFloatingPointType& b);

  virtual void SetHighlightColor(vtkFloatingPointType r,vtkFloatingPointType g,vtkFloatingPointType b);
  virtual void GetHighlightColor(vtkFloatingPointType& r,vtkFloatingPointType& g,vtkFloatingPointType& b);

  virtual void SetPreHighlightColor(vtkFloatingPointType r,vtkFloatingPointType g,vtkFloatingPointType b);
  virtual void GetPreHighlightColor(vtkFloatingPointType& r,vtkFloatingPointType& g,vtkFloatingPointType& b);
 
  virtual vtkFloatingPointType GetLineWidth();
  virtual void SetLineWidth(vtkFloatingPointType theVal);

  virtual void Set0DSize(vtkFloatingPointType size);
  virtual vtkFloatingPointType Get0DSize();

  virtual int GetNodeObjId(int theVtkID);
  virtual vtkFloatingPointType* GetNodeCoord(int theObjID);

  virtual int GetElemObjId(int theVtkID);
  virtual vtkCell* GetElemCell(int theObjID);

  virtual int GetObjDimension( const int theObjId );

  virtual void SetVisibility(int theMode);
  void SetVisibility(int theMode, bool theIsUpdateRepersentation);

  virtual void SetRepresentation(int theMode);
  
  virtual unsigned int GetEntityMode() const { return myEntityMode;}
  virtual void SetEntityMode(unsigned int theMode);

  virtual void SetPointRepresentation(bool theIsPointsVisible);
  virtual bool GetPointRepresentation();

  virtual vtkFloatingPointType* GetBounds();
  virtual void SetTransform(VTKViewer_Transform* theTransform); 

  virtual vtkUnstructuredGrid* GetUnstructuredGrid();
  virtual vtkDataSet* GetInput();
  virtual vtkMapper* GetMapper();

  virtual vtkFloatingPointType GetShrinkFactor();
  virtual void SetShrinkFactor(vtkFloatingPointType theValue);

  virtual bool IsShrunkable() { return myIsShrinkable;}
  virtual bool IsShrunk() { return myIsShrunk;}
  virtual void SetShrink(); 
  virtual void UnShrink(); 

  virtual void SetPointsLabeled(bool theIsPointsLabeled);
  virtual bool GetPointsLabeled();

  virtual void SetCellsLabeled(bool theIsCellsLabeled);
  virtual bool GetCellsLabeled();

  virtual void SetFacesOriented(bool theIsFacesOriented);
  virtual bool GetFacesOriented();

  virtual void SetFacesOrientationColor(vtkFloatingPointType theColor[3]);
  virtual void GetFacesOrientationColor(vtkFloatingPointType theColor[3]);

  virtual void SetFacesOrientationScale(vtkFloatingPointType theScale);
  virtual vtkFloatingPointType GetFacesOrientationScale();

  virtual void SetFacesOrientation3DVectors(bool theState);
  virtual bool GetFacesOrientation3DVectors();

  virtual void SetControlMode(eControl theMode);
  virtual eControl GetControlMode(){ return myControlMode;}
  virtual SMESH::Controls::FunctorPtr GetFunctor() { return myFunctor; }

  virtual SMESH_ScalarBarActor* GetScalarBarActor(){ return myScalarBarActor;}

  virtual void RemoveAllClippingPlanes();
  virtual vtkIdType GetNumberOfClippingPlanes();
  virtual vtkPlane* GetClippingPlane(vtkIdType theID);
  virtual vtkIdType AddClippingPlane(vtkPlane* thePlane); 

  virtual TVisualObjPtr GetObject() { return myVisualObj;}

  virtual void SetControlsPrecision( const long p ) { myControlsPrecision = p; }
  virtual long GetControlsPrecision() const { return myControlsPrecision; }

  virtual void UpdateScalarBar();
  virtual void UpdateDistribution();

#ifndef DISABLE_PLOT2DVIEWER
  virtual SPlot2d_Histogram* GetPlot2Histogram() { return my2dHistogram; }
  virtual SPlot2d_Histogram* UpdatePlot2Histogram();
#endif


  virtual void SetQuadratic2DRepresentation(EQuadratic2DRepresentation);
  virtual EQuadratic2DRepresentation GetQuadratic2DRepresentation();
  
  virtual void SetMarkerStd( VTK::MarkerType, VTK::MarkerScale );
  virtual void SetMarkerTexture( int, VTK::MarkerTexture );

 protected:
  void SetControlMode(eControl theMode, bool theCheckEntityMode);
  void SetImplicitFunctionUsed(bool theIsImplicitFunctionUsed);
  bool IsImplicitFunctionUsed() const;

  TVisualObjPtr myVisualObj;
  vtkTimeStamp* myTimeStamp;

  SMESH_ScalarBarActor* myScalarBarActor;
  vtkLookupTable* myLookupTable;

  vtkProperty* mySurfaceProp;
  vtkProperty* myBackSurfaceProp;
  vtkProperty* myEdgeProp;
  vtkProperty* myNodeProp;

  SMESH_DeviceActor* myBaseActor;
  SMESH_NodeLabelActor* myNodeActor;
  SMESH_DeviceActor* myPickableActor;

  vtkProperty* myHighlightProp;
  vtkProperty* myOutLineProp;
  vtkProperty* myPreselectProp;
          
  SMESH_DeviceActor* myHighlitableActor;

  eControl myControlMode;
  SMESH::Controls::FunctorPtr myFunctor;
  vtkProperty* my2DExtProp;
  SMESH_CellLabelActor* my2DActor;
  SMESH_DeviceActor* my2DExtActor;
  SMESH_CellLabelActor* my3DActor;
  SMESH_DeviceActor* my3DExtActor;
  SMESH_DeviceActor* myControlActor;

  vtkProperty* myNodeExtProp;
  SMESH_DeviceActor* myNodeExtActor;

  vtkProperty* my1DProp;
  SMESH_CellLabelActor* my1DActor;
  vtkProperty* my1DExtProp;
  SMESH_DeviceActor* my1DExtActor;

  vtkProperty* my0DProp;
  SMESH_CellLabelActor* my0DActor;
  vtkProperty* my0DExtProp;
  SMESH_DeviceActor* my0DExtActor;

  unsigned int myEntityMode;
  unsigned int myEntityState;
  unsigned int myEntityModeCache;
  bool myIsEntityModeCache;
  bool myIsPointsVisible;

  bool myIsShrinkable;
  bool myIsShrunk;
  
  vtkImplicitBoolean* myImplicitBoolean;
  typedef TVTKSmartPtr<vtkPlane> TPlanePtr;
  typedef std::vector<TPlanePtr> TCippingPlaneCont;
  TCippingPlaneCont myCippingPlaneCont;
  long myControlsPrecision;

#ifndef DISABLE_PLOT2DVIEWER
  SPlot2d_Histogram* my2dHistogram;
#endif

  bool myIsFacesOriented;
  
  int myDeltaBrightness;

  VTK::MarkerTexture myMarkerTexture;

  SMESH_ActorDef();
  ~SMESH_ActorDef();

  bool Init(TVisualObjPtr theVisualObj, 
            const char* theEntry, 
            const char* theName,
            int theIsClear);

  void SetIsShrunkable(bool theShrunkable);
  void UpdateHighlight();
  void Update();

 private:
  // hide the two parameter Render() method from the user and the compiler.
  virtual void Render(vtkRenderer *, vtkMapper *) {};
  virtual void ShallowCopy(vtkProp *prop);
  virtual void SetMapper(vtkMapper *);
  static SMESH_ActorDef* New();

  // Not implemented.
  SMESH_ActorDef(const SMESH_ActorDef&);  
  void operator=(const SMESH_ActorDef&);
};


#endif //SMESH_ACTORDEF_H
