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

#ifndef SMESH_OBJECTDEF_H
#define SMESH_OBJECTDEF_H

// IDL Headers
#include "SALOMEconfig.h"
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Group)

#include <map>
#include <list>

#include "SMESH_Controls.hxx"
#include "SMESH_Object.h"

class vtkPoints;
class SALOME_ExtractUnstructuredGrid;

class SMESH_Actor;
class SMDS_MeshNode;
class SMDS_MeshElement;

/*
  Class       : SMESH_VisualObj
  Description : Base class for all mesh objects to be visuilised
*/
class SMESH_VisualObjDef: public SMESH_VisualObj
{
public:
  typedef std::list<const SMDS_MeshElement*>   TEntityList;
  typedef std::map<vtkIdType,vtkIdType>  TMapOfIds;
  
                            SMESH_VisualObjDef();
  virtual                   ~SMESH_VisualObjDef();
  
  virtual void              Update( int theIsClear = true ) = 0;
  virtual void              UpdateFunctor( const SMESH::Controls::FunctorPtr& theFunctor ) = 0;
  virtual int               GetElemDimension( const int theObjId ) = 0;

  virtual int               GetNbEntities( const SMDSAbs_ElementType theType) const = 0;
  virtual int               GetEntities( const SMDSAbs_ElementType, TEntityList& ) const = 0;
  virtual bool              IsNodePrs() const = 0;
  virtual SMDS_Mesh*        GetMesh() const = 0;

  virtual bool              GetEdgeNodes( const int theElemId,
                                          const int theEdgeNum,
                                          int&      theNodeId1,
                                          int&      theNodeId2 ) const;

  virtual vtkUnstructuredGrid* GetUnstructuredGrid() { return myGrid; }
  
  virtual vtkIdType         GetNodeObjId( int theVTKID );
  virtual vtkIdType         GetNodeVTKId( int theObjID );
  virtual vtkIdType         GetElemObjId( int theVTKID );
  virtual vtkIdType         GetElemVTKId( int theObjID );
  
protected:

  void                      createPoints( vtkPoints* );
  void                      buildPrs();
  void                      buildNodePrs();
  void                      buildElemPrs();
  
private:                                   

  TMapOfIds                 mySMDS2VTKNodes;
  TMapOfIds                 myVTK2SMDSNodes;
  TMapOfIds                 mySMDS2VTKElems;
  TMapOfIds                 myVTK2SMDSElems;

  vtkUnstructuredGrid*      myGrid;
};


/*
  Class       : SMESH_MeshObj
  Description : Class for visualisation of mesh
*/

class SMESH_MeshObj: public SMESH_VisualObjDef
{
public:

                            SMESH_MeshObj( SMESH::SMESH_Mesh_ptr );
  virtual                   ~SMESH_MeshObj();
  
  virtual void              Update( int theIsClear = true );
  
  virtual int               GetNbEntities( const SMDSAbs_ElementType) const;
  virtual int               GetEntities( const SMDSAbs_ElementType, TEntityList& ) const;
  virtual bool              IsNodePrs() const;

  virtual int               GetElemDimension( const int theObjId );

  virtual void              UpdateFunctor( const SMESH::Controls::FunctorPtr& theFunctor );
  
  SMESH::SMESH_Mesh_ptr     GetMeshServer() { return myMeshServer.in(); }
  SMDS_Mesh*                GetMesh() const { return myMesh; }

protected:

  SMESH::SMESH_Mesh_var     myMeshServer;
  SMDS_Mesh*                myMesh;
};


/*
  Class       : SMESH_SubMeshObj
  Description : Base class for visualisation of submeshes and groups
*/

class SMESH_SubMeshObj: public SMESH_VisualObjDef
{
public:

                            SMESH_SubMeshObj(SMESH_MeshObj* theMeshObj);
  virtual                   ~SMESH_SubMeshObj();

  virtual void              Update( int theIsClear = true );
  
  virtual void              UpdateFunctor( const SMESH::Controls::FunctorPtr& theFunctor );
  virtual int               GetElemDimension( const int theObjId );
  virtual SMDS_Mesh*        GetMesh() const { return myMeshObj->GetMesh(); }
  
protected:

  SMESH_MeshObj*            myMeshObj;
};


/*
  Class       : SMESH_GroupObj
  Description : Class for visualisation of groups
*/

class SMESH_GroupObj: public SMESH_SubMeshObj
{
public:
                            SMESH_GroupObj( SMESH::SMESH_GroupBase_ptr, SMESH_MeshObj* );
  virtual                   ~SMESH_GroupObj();

  virtual int               GetNbEntities( const SMDSAbs_ElementType) const;
  virtual int               GetEntities( const SMDSAbs_ElementType, TEntityList& ) const;
  virtual bool              IsNodePrs() const;

private:

  SMESH::SMESH_GroupBase_var    myGroupServer;
};


/*
  Class       : SMESH_subMeshObj
  Description : Class for visualisation of submeshes
*/

class SMESH_subMeshObj : public SMESH_SubMeshObj
{
public:

                            SMESH_subMeshObj( SMESH::SMESH_subMesh_ptr, 
                                              SMESH_MeshObj* );
  virtual                   ~SMESH_subMeshObj();

  virtual int               GetNbEntities( const SMDSAbs_ElementType) const;
  virtual int               GetEntities( const SMDSAbs_ElementType, TEntityList& ) const;
  virtual bool              IsNodePrs() const;    
  
protected:

  SMESH::SMESH_subMesh_var  mySubMeshServer;
};


#endif
