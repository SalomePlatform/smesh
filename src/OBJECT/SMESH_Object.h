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
//  File   : SMESH_Object.h
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#ifndef SMESH_OBJECT_H
#define SMESH_OBJECT_H

#include <boost/shared_ptr.hpp>
#include <vtkSystemIncludes.h>

#include "SMESH_Controls.hxx"
#include "SMDSAbs_ElementType.hxx"

class SMDS_Mesh;
class vtkUnstructuredGrid;

/*
  Class       : SMESH_VisualObj
  Description : Base class for all mesh objects to be visuilised
*/
class SMESH_VisualObj
{
public:
  virtual void Update( int theIsClear = true ) = 0;
  virtual void UpdateFunctor( const SMESH::Controls::FunctorPtr& theFunctor ) = 0;
  virtual int GetElemDimension( const int theObjId ) = 0;

  virtual int GetNbEntities( const SMDSAbs_ElementType theType) const = 0;
  virtual SMDS_Mesh* GetMesh() const = 0;

  virtual bool GetEdgeNodes( const int theElemId,
			     const int theEdgeNum,
			     int&      theNodeId1,
			     int&      theNodeId2 ) const = 0;
  
  virtual vtkUnstructuredGrid* GetUnstructuredGrid() = 0;
  
  virtual vtkIdType GetNodeObjId( int theVTKID ) = 0;
  virtual vtkIdType GetNodeVTKId( int theObjID ) = 0;
  virtual vtkIdType GetElemObjId( int theVTKID ) = 0;
  virtual vtkIdType GetElemVTKId( int theObjID ) = 0;
};

typedef boost::shared_ptr<SMESH_VisualObj> TVisualObjPtr;

#endif
