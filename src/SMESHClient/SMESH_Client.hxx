//  SMESH SMESHClient : tool to update client mesh structure by mesh from server
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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : SMESH_Client.hxx
//  Author : Pavel TELKOV
//  Module : SMESH

#ifndef _SMESH_Client_HeaderFile
#define _SMESH_Client_HeaderFile

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Gen)
#include CORBA_SERVER_HEADER(SMESH_Mesh)

#ifdef WNT
# ifdef SMESHCLIENT_EXPORTS
#  define SMESHCLIENT_EXPORT __declspec( dllexport )
# else
#  define SMESHCLIENT_EXPORT __declspec( dllimport )
# endif
#else
# define SMESHCLIENT_EXPORT
#endif

class SMESHDS_Mesh;
class SMDS_Mesh;


//=====================================================================
// SMESH_Client : class definition
//=====================================================================
class SMESHCLIENT_EXPORT SMESH_Client  
{
public:
  static 
  SMESH::SMESH_Gen_var
  GetSMESHGen(CORBA::ORB_ptr theORB,
	      CORBA::Boolean& theIsEmbeddedMode);

  SMESH_Client(CORBA::ORB_ptr theORB,
	       SMESH::SMESH_Mesh_ptr theMesh);
  ~SMESH_Client();

  bool 
  Update(bool theIsClear);

  SMDS_Mesh* 
  GetMesh() const;

  SMDS_Mesh*
  operator->() const;

  SMESH::SMESH_Mesh_ptr
  GetMeshServer();

protected: 
  SMESH::SMESH_Mesh_var myMeshServer;
  SMESHDS_Mesh* mySMESHDSMesh;
  SMDS_Mesh* mySMDSMesh;
};


#endif
