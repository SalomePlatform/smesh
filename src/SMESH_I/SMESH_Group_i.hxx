//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's classes
//
//  Copyright (C) 2004  CEA
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
//  See http://www.salome-platform.org or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : SMESH_Group_i.hxx
//  Author : Sergey ANIKIN, OCC
//  Module : SMESH
//  $Header$


#ifndef SMESH_Group_i_HeaderFile
#define SMESH_Group_i_HeaderFile

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Group)
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_CLIENT_HEADER(GEOM_Gen)

#include "SALOME_GenericObj_i.hh"

class SMESH_Mesh_i;
class SMESH_Group;
class SMESHDS_GroupBase;

// ===========
// Group Base
// ===========
class SMESH_GroupBase_i:
  public virtual POA_SMESH::SMESH_GroupBase,
  public virtual SALOME::GenericObj_i
{
 public:
  SMESH_GroupBase_i(PortableServer::POA_ptr thePOA,
                    SMESH_Mesh_i* theMeshServant,
                    const int theLocalID );
  virtual ~SMESH_GroupBase_i();

  // CORBA interface implementation
  void SetName(const char* name);
  char* GetName();
  SMESH::ElementType GetType();
  CORBA::Long Size();
  CORBA::Boolean IsEmpty();
  CORBA::Boolean Contains(CORBA::Long elem_id);
  CORBA::Long GetID(CORBA::Long elem_index);
  SMESH::long_array* GetListOfID();
  SMESH::SMESH_Mesh_ptr GetMesh();

  // Inherited from SMESH_IDSource interface
  virtual SMESH::long_array* GetIDs();

  // Internal C++ interface
  int GetLocalID() const { return myLocalID; }
  SMESH_Mesh_i* GetMeshServant() const { return myMeshServant; }
  SMESH_Group* GetSmeshGroup() const;
  SMESHDS_GroupBase* GetGroupDS() const;

   void SetColorNumber(CORBA::Long color);
   CORBA::Long GetColorNumber();

private:
  SMESH_Mesh_i* myMeshServant;
  int myLocalID;
};

// ======
// Group
// ======

class SMESH_Group_i:
  public virtual POA_SMESH::SMESH_Group,
  public SMESH_GroupBase_i
{
 public:
  SMESH_Group_i( PortableServer::POA_ptr thePOA, SMESH_Mesh_i* theMeshServant, const int theLocalID );

  // CORBA interface implementation
  void Clear();
  CORBA::Long Add( const SMESH::long_array& theIDs );
  CORBA::Long Remove( const SMESH::long_array& theIDs );

  CORBA::Long AddByPredicate( SMESH::Predicate_ptr thePredicate );
  CORBA::Long RemoveByPredicate( SMESH::Predicate_ptr thePredicate );
};

// =========================
// Group linked to geometry
// =========================

class SMESH_GroupOnGeom_i:
  public virtual POA_SMESH::SMESH_GroupOnGeom,
  public SMESH_GroupBase_i
{
 public:
  SMESH_GroupOnGeom_i( PortableServer::POA_ptr thePOA, SMESH_Mesh_i* theMeshServant, const int theLocalID );

  // CORBA interface implementation
  GEOM::GEOM_Object_ptr GetShape();
};
#endif
