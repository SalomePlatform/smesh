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
//  File   : SMESHGUI_PreVisualObj.h
//  Module : SMESH
//
#ifndef SMESHGUI_PreVisualObj_H
#define SMESHGUI_PreVisualObj_H

#include "SMESH_SMESHGUI.hxx"

#include "SMESH_Object.h"

class SMESHDS_Mesh;

/*!
 * \brief Incarnation of SMESH_VisualObj allowing usage of SMESH_Actor
 *        to show arbitrary mesh data. SMESHGUI_PreVisualObj encapsulates
 *        a instance of SMDS_Mesh that can be filled by its user.
 *        Main usage: to initialize a SMESH_Actor to display some preview
 */
class SMESHGUI_EXPORT SMESHGUI_PreVisualObj : public SMESH_VisualObj
{
  mutable SMESHDS_Mesh* myMesh;
  bool                  myEntitiesFlag;
  unsigned int          myEntitiesState;

 public:
  SMESHGUI_PreVisualObj();
  virtual SMDS_Mesh* GetMesh() const;
  SMESHDS_Mesh* GetMeshDS() const { return myMesh; }

  virtual bool Update( int theIsClear );
  virtual bool NulData() { return false; }
  virtual void UpdateFunctor( const SMESH::Controls::FunctorPtr& theFunctor );
  virtual int  GetElemDimension( const smIdType theObjId );
  virtual smIdType  GetNbEntities( const SMDSAbs_ElementType theType) const;
  virtual bool IsValid() const;
  virtual bool GetEdgeNodes( const smIdType theElemId,
                             const int      theEdgeNum,
                             smIdType&      theNodeId1,
                             smIdType&      theNodeId2 ) const;

  virtual vtkIdType GetNodeObjId( vtkIdType theVTKID );
  virtual vtkIdType GetNodeVTKId( vtkIdType theObjID );
  virtual vtkIdType GetElemObjId( vtkIdType theVTKID );
  virtual vtkIdType GetElemVTKId( vtkIdType theObjID );
  virtual void                  ClearEntitiesFlags();
  virtual bool                  GetEntitiesFlag();
  virtual unsigned int          GetEntitiesState();

  virtual SMESH::SMESH_Mesh_ptr GetMeshServer();
  virtual vtkUnstructuredGrid*  GetUnstructuredGrid();
};

#endif
