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

#include "SALOME_GenericObj_i.hh"

class SMESH_Mesh_i;

class SMESH_Group_i:
  public virtual POA_SMESH::SMESH_Group,
  public virtual SALOME::GenericObj_i
{
public:
  SMESH_Group_i( PortableServer::POA_ptr thePOA, SMESH_Mesh_i* theMeshServant, const int theLocalID );
  virtual ~SMESH_Group_i();

  // CORBA interface implementation
  void SetName( const char* theName );

  char* GetName();

  SMESH::ElementType GetType();

  CORBA::Long Size();

  CORBA::Boolean IsEmpty();

  void Clear();

  CORBA::Boolean Contains( CORBA::Long theID );

  CORBA::Long Add( const SMESH::long_array& theIDs );

  CORBA::Long GetID( CORBA::Long theIndex );

  SMESH::long_array* GetListOfID();

  CORBA::Long Remove( const SMESH::long_array& theIDs );

  SMESH::SMESH_Mesh_ptr GetMesh();

  // Internal C++ interface
  int GetLocalID();

  SMESH_Mesh_i* GetMeshServant() { return myMeshServant; }

private:
  SMESH_Mesh_i* myMeshServant;
  int myLocalID;
};

#endif
