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
//  File   : SMESH_Actor.h
//  Author : Nicolas REJNERI
//  Module : SMESH
//
#ifndef SMESH_ACTOR_H
#define SMESH_ACTOR_H

#include <SALOMEconfig.h> // To fix some redefinition
#include <SALOME_Actor.h>
#include "SMESH_Object.h"

#include <vtkCommand.h>

class vtkUnstructuredGrid;

class SMESH_ScalarBarActor;

class vtkPlane;
class vtkImplicitBoolean;

#ifndef DISABLE_PLOT2DVIEWER
class SPlot2d_Histogram;
#endif

namespace SMESH
{
  const vtkIdType DeleteActorEvent = vtkCommand::UserEvent + 100;
}

class SMESHOBJECT_EXPORT SMESH_Actor: public SALOME_Actor
{
  static SMESH_Actor* New() { return NULL;}

 public:
  vtkTypeMacro(SMESH_Actor,SALOME_Actor);
  static SMESH_Actor* New(TVisualObjPtr theVisualObj, 
                          const char* theEntry, 
                          const char* theName,
                          int theIsClear);
  
  virtual void SetSufaceColor(vtkFloatingPointType r,vtkFloatingPointType g,vtkFloatingPointType b, int delta ) = 0;
  virtual void GetSufaceColor(vtkFloatingPointType& r,vtkFloatingPointType& g,vtkFloatingPointType& b, int& delta ) = 0;
    
  virtual void SetEdgeColor(vtkFloatingPointType r,vtkFloatingPointType g,vtkFloatingPointType b) = 0;
  virtual void GetEdgeColor(vtkFloatingPointType& r,vtkFloatingPointType& g,vtkFloatingPointType& b) = 0;

  virtual void SetNodeColor(vtkFloatingPointType r,vtkFloatingPointType g,vtkFloatingPointType b) = 0;
  virtual void GetNodeColor(vtkFloatingPointType& r,vtkFloatingPointType& g,vtkFloatingPointType& b) = 0;
  
  virtual void SetOutlineColor(vtkFloatingPointType r,vtkFloatingPointType g,vtkFloatingPointType b) = 0;
  virtual void GetOutlineColor(vtkFloatingPointType& r,vtkFloatingPointType& g,vtkFloatingPointType& b) = 0;  

  virtual void Set0DColor(vtkFloatingPointType r,vtkFloatingPointType g,vtkFloatingPointType b) = 0;
  virtual void Get0DColor(vtkFloatingPointType& r,vtkFloatingPointType& g,vtkFloatingPointType& b) = 0;

  virtual void SetHighlightColor(vtkFloatingPointType r,vtkFloatingPointType g,vtkFloatingPointType b) = 0;
  virtual void GetHighlightColor(vtkFloatingPointType& r,vtkFloatingPointType& g,vtkFloatingPointType& b) = 0;

  virtual void SetPreHighlightColor(vtkFloatingPointType r,vtkFloatingPointType g,vtkFloatingPointType b) = 0;
  virtual void GetPreHighlightColor(vtkFloatingPointType& r,vtkFloatingPointType& g,vtkFloatingPointType& b) = 0;
 
  virtual vtkFloatingPointType GetLineWidth() = 0;
  virtual void SetLineWidth(vtkFloatingPointType theVal) = 0;

  virtual void Set0DSize(vtkFloatingPointType size) = 0;
  virtual vtkFloatingPointType Get0DSize() = 0;

  enum EReperesent { ePoint, eEdge, eSurface};
  
  enum EEntityMode { e0DElements = 0x01, eEdges = 0x02, eFaces = 0x04, eVolumes = 0x08, eAllEntity = 0x0f};

  enum EQuadratic2DRepresentation { eLines = 0x01, eArcs = 0x02 };

  virtual void SetEntityMode(unsigned int theMode) = 0;
  virtual unsigned int GetEntityMode() const = 0;

  virtual void SetQuadratic2DRepresentation(EQuadratic2DRepresentation) = 0;
  virtual EQuadratic2DRepresentation GetQuadratic2DRepresentation() = 0;

  virtual void SetPointRepresentation(bool theIsPointsVisible) = 0;
  virtual bool GetPointRepresentation() = 0;

  virtual vtkUnstructuredGrid* GetUnstructuredGrid() = 0;

  virtual void SetShrinkFactor(vtkFloatingPointType theValue) = 0;

  virtual void SetPointsLabeled(bool theIsPointsLabeled) = 0;
  virtual bool GetPointsLabeled() = 0;

  virtual void SetCellsLabeled(bool theIsCellsLabeled) = 0;
  virtual bool GetCellsLabeled() = 0;

  virtual void SetFacesOriented(bool theIsFacesOriented) = 0;
  virtual bool GetFacesOriented() = 0;

  virtual void SetFacesOrientationColor(vtkFloatingPointType theColor[3]) = 0;
  virtual void GetFacesOrientationColor(vtkFloatingPointType theColor[3]) = 0;

  virtual void SetFacesOrientationScale(vtkFloatingPointType theScale) = 0;
  virtual vtkFloatingPointType GetFacesOrientationScale() = 0;

  virtual void SetFacesOrientation3DVectors(bool theState) = 0;
  virtual bool GetFacesOrientation3DVectors() = 0;

  enum eControl{eNone, eLength, eLength2D, eFreeBorders, eFreeEdges, eFreeNodes,
                eFreeFaces, eMultiConnection, eArea, eTaper, eAspectRatio,
                eMinimumAngle, eWarping, eSkew, eAspectRatio3D, eMultiConnection2D, eVolume3D,
                eMaxElementLength2D, eMaxElementLength3D, eBareBorderFace, eBareBorderVolume,
                eOverConstrainedFace, eOverConstrainedVolume, eCoincidentNodes,
                eCoincidentElems1D, eCoincidentElems2D, eCoincidentElems3D };
  virtual void SetControlMode(eControl theMode) = 0;
  virtual eControl GetControlMode() = 0;
  virtual SMESH::Controls::FunctorPtr GetFunctor() = 0;

  virtual SMESH_ScalarBarActor* GetScalarBarActor() = 0;

  virtual void RemoveAllClippingPlanes() = 0; 
  virtual vtkIdType GetNumberOfClippingPlanes() = 0; 
  virtual vtkPlane* GetClippingPlane(vtkIdType theID) = 0; 
  virtual vtkIdType AddClippingPlane(vtkPlane* thePlane) = 0; 

  virtual TVisualObjPtr GetObject() = 0;

  virtual void SetControlsPrecision( const long ) = 0;
  virtual long GetControlsPrecision() const = 0;

  virtual void UpdateScalarBar() = 0;
  virtual void UpdateDistribution() = 0;

#ifndef DISABLE_PLOT2DVIEWER
  virtual SPlot2d_Histogram* GetPlot2Histogram() = 0;
  virtual SPlot2d_Histogram* UpdatePlot2Histogram() = 0;
#endif
};


#endif //SMESH_ACTOR_H
