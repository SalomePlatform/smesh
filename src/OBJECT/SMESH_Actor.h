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

#ifndef SMESH_ACTOR_H
#define SMESH_ACTOR_H

#include "SALOME_Actor.h"
#include "SMESH_Object.h"

class vtkUnstructuredGrid;

class vtkScalarBarActor;

class vtkPlane;
class vtkImplicitBoolean;

class SMESH_Actor: public SALOME_Actor
{
  static SMESH_Actor* New() { return NULL;}

 public:
  vtkTypeMacro(SMESH_Actor,SALOME_Actor);
  static SMESH_Actor* New(TVisualObjPtr theVisualObj, 
			  const char* theEntry, 
			  const char* theName,
			  int theIsClear);
  
  virtual void SetSufaceColor(float r,float g,float b) = 0;
  virtual void GetSufaceColor(float& r,float& g,float& b) = 0;
  
  virtual void SetBackSufaceColor(float r,float g,float b) = 0;
  virtual void GetBackSufaceColor(float& r,float& g,float& b) = 0;
  
  virtual void SetEdgeColor(float r,float g,float b) = 0;
  virtual void GetEdgeColor(float& r,float& g,float& b) = 0;

  virtual void SetNodeColor(float r,float g,float b) = 0;
  virtual void GetNodeColor(float& r,float& g,float& b) = 0;

  virtual void SetHighlightColor(float r,float g,float b) = 0;
  virtual void GetHighlightColor(float& r,float& g,float& b) = 0;

  virtual void SetPreHighlightColor(float r,float g,float b) = 0;
  virtual void GetPreHighlightColor(float& r,float& g,float& b) = 0;
 
  virtual float GetLineWidth() = 0;
  virtual void SetLineWidth(float theVal) = 0;

  virtual void SetNodeSize(float size) = 0;
  virtual float GetNodeSize() = 0;

  enum EReperesent { ePoint, eEdge, eSurface};
  
  enum EEntityMode { eEdges = 0x01, eFaces = 0x02, eVolumes = 0x04, eAllEntity = 0x07};
  virtual void SetEntityMode(unsigned int theMode) = 0;
  virtual unsigned int GetEntityMode() const = 0;

  virtual void SetPointRepresentation(bool theIsPointsVisible) = 0;
  virtual bool GetPointRepresentation() = 0;

  virtual vtkUnstructuredGrid* GetUnstructuredGrid() = 0;

  virtual void SetShrinkFactor(float theValue) = 0;

  virtual void SetPointsLabeled(bool theIsPointsLabeled) = 0;
  virtual bool GetPointsLabeled() = 0;

  virtual void SetCellsLabeled(bool theIsCellsLabeled) = 0;
  virtual bool GetCellsLabeled() = 0;

  enum eControl{eNone, eLength, eLength2D, eFreeBorders, eFreeEdges, eMultiConnection, 
		eArea, eTaper, eAspectRatio, eMinimumAngle, eWarping, eSkew,
		eAspectRatio3D, eMultiConnection2D};
  virtual void SetControlMode(eControl theMode) = 0;
  virtual eControl GetControlMode() = 0;

  virtual vtkScalarBarActor* GetScalarBarActor() = 0;

  virtual void SetPlaneParam(float theDir[3], float theDist, vtkPlane* thePlane) = 0;
  virtual void GetPlaneParam(float theDir[3], float& theDist, vtkPlane* thePlane) = 0;

  virtual vtkImplicitBoolean* GetPlaneContainer() = 0; 

  virtual TVisualObjPtr GetObject() = 0;

  virtual void SetControlsPrecision( const long ) = 0;
  virtual long GetControlsPrecision() const = 0;
};


#endif //SMESH_ACTOR_H
