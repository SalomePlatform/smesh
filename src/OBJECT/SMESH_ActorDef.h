//  Copyright (C) 2007-2008  CEA/DEN, EDF R&D, OPEN CASCADE
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
class vtkMaskPoints;
class vtkCellCenters;
class vtkLabeledDataMapper;
class vtkSelectVisiblePoints;

class vtkScalarBarActor;
class vtkLookupTable;

class vtkPlane;
class vtkImplicitBoolean;

class vtkTimeStamp;

class SMESH_DeviceActor;


class SMESH_ActorDef : public SMESH_Actor
{
  friend class SMESH_VisualObj;
  friend class SMESH_Actor;

 public:
  vtkTypeMacro(SMESH_ActorDef,SMESH_Actor);
  
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

  virtual void SetSufaceColor(vtkFloatingPointType r,vtkFloatingPointType g,vtkFloatingPointType b);
  virtual void GetSufaceColor(vtkFloatingPointType& r,vtkFloatingPointType& g,vtkFloatingPointType& b);

  virtual void SetBackSufaceColor(vtkFloatingPointType r,vtkFloatingPointType g,vtkFloatingPointType b);
  virtual void GetBackSufaceColor(vtkFloatingPointType& r,vtkFloatingPointType& g,vtkFloatingPointType& b);

  virtual void SetEdgeColor(vtkFloatingPointType r,vtkFloatingPointType g,vtkFloatingPointType b);
  virtual void GetEdgeColor(vtkFloatingPointType& r,vtkFloatingPointType& g,vtkFloatingPointType& b);

  virtual void SetNodeColor(vtkFloatingPointType r,vtkFloatingPointType g,vtkFloatingPointType b);
  virtual void GetNodeColor(vtkFloatingPointType& r,vtkFloatingPointType& g,vtkFloatingPointType& b);

  virtual void SetHighlightColor(vtkFloatingPointType r,vtkFloatingPointType g,vtkFloatingPointType b);
  virtual void GetHighlightColor(vtkFloatingPointType& r,vtkFloatingPointType& g,vtkFloatingPointType& b);

  virtual void SetPreHighlightColor(vtkFloatingPointType r,vtkFloatingPointType g,vtkFloatingPointType b);
  virtual void GetPreHighlightColor(vtkFloatingPointType& r,vtkFloatingPointType& g,vtkFloatingPointType& b);
 
  virtual vtkFloatingPointType GetLineWidth();
  virtual void SetLineWidth(vtkFloatingPointType theVal);

  virtual void SetNodeSize(vtkFloatingPointType size) ;
  virtual vtkFloatingPointType GetNodeSize() ;

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
  virtual bool GetPointsLabeled(){ return myIsPointsLabeled;}

  virtual void SetCellsLabeled(bool theIsCellsLabeled);
  virtual bool GetCellsLabeled(){ return myIsCellsLabeled;}

  virtual void SetFacesOriented(bool theIsFacesOriented);
  virtual bool GetFacesOriented();

  virtual void SetControlMode(eControl theMode);
  virtual eControl GetControlMode(){ return myControlMode;}

  virtual vtkScalarBarActor* GetScalarBarActor(){ return myScalarBarActor;}

  virtual void SetPlaneParam(vtkFloatingPointType theDir[3], vtkFloatingPointType theDist, vtkPlane* thePlane);
  virtual void GetPlaneParam(vtkFloatingPointType theDir[3], vtkFloatingPointType& theDist, vtkPlane* thePlane);

  virtual void RemoveAllClippingPlanes();
  virtual vtkIdType GetNumberOfClippingPlanes();
  virtual vtkPlane* GetClippingPlane(vtkIdType theID);
  virtual vtkIdType AddClippingPlane(vtkPlane* thePlane); 

  virtual TVisualObjPtr GetObject() { return myVisualObj;}

  virtual void SetControlsPrecision( const long p ) { myControlsPrecision = p; }
  virtual long GetControlsPrecision() const { return myControlsPrecision; }

  virtual void UpdateScalarBar();
  
 protected:
  void SetControlMode(eControl theMode, bool theCheckEntityMode);
  void SetImplicitFunctionUsed(bool theIsImplicitFunctionUsed);
  bool IsImplicitFunctionUsed() const;

  TVisualObjPtr myVisualObj;
  vtkTimeStamp* myTimeStamp;

  vtkScalarBarActor* myScalarBarActor;
  vtkLookupTable* myLookupTable;

  vtkProperty* mySurfaceProp;
  vtkProperty* myBackSurfaceProp;
  vtkProperty* myEdgeProp;
  vtkProperty* myNodeProp;

  SMESH_DeviceActor* myBaseActor;
  SMESH_DeviceActor* myNodeActor;
  SMESH_DeviceActor* myPickableActor;

  vtkProperty* myHighlightProp;
  vtkProperty* myPreselectProp;
  SMESH_DeviceActor* myHighlitableActor;

  eControl myControlMode;
  vtkProperty* my2DExtProp;
  SMESH_DeviceActor* my2DActor;
  SMESH_DeviceActor* my2DExtActor;
  SMESH_DeviceActor* my3DActor;
  SMESH_DeviceActor* myControlActor;

  vtkProperty* myNodeExtProp;
  SMESH_DeviceActor* myNodeExtActor;

  vtkProperty* my1DProp;
  SMESH_DeviceActor* my1DActor;
  vtkProperty* my1DExtProp;
  SMESH_DeviceActor* my1DExtActor;

  unsigned int myEntityMode;
  unsigned int myEntityState;
  bool myIsPointsVisible;

  bool myIsShrinkable;
  bool myIsShrunk;
  
  bool myIsPointsLabeled;
  vtkUnstructuredGrid* myPointsNumDataSet;
  vtkActor2D *myPointLabels;
  vtkMaskPoints* myPtsMaskPoints;
  vtkLabeledDataMapper* myPtsLabeledDataMapper;
  vtkSelectVisiblePoints* myPtsSelectVisiblePoints;

  bool myIsCellsLabeled;
  vtkUnstructuredGrid* myCellsNumDataSet;
  vtkActor2D *myCellsLabels;
  vtkMaskPoints* myClsMaskPoints;
  vtkCellCenters* myCellCenters;
  vtkLabeledDataMapper* myClsLabeledDataMapper;
  vtkSelectVisiblePoints* myClsSelectVisiblePoints;

  vtkImplicitBoolean* myImplicitBoolean;
  typedef TVTKSmartPtr<vtkPlane> TPlanePtr;
  typedef std::vector<TPlanePtr> TCippingPlaneCont;
  TCippingPlaneCont myCippingPlaneCont;
  long myControlsPrecision;

  bool myIsFacesOriented;

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
