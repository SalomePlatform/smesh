//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
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
//  File   : SMESH_Mesh_i.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

#ifndef _SMESH_MESH_I_HXX_
#define _SMESH_MESH_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Hypothesis)
#include CORBA_CLIENT_HEADER(GEOM_Gen)
#include CORBA_CLIENT_HEADER(GEOM_Shape)
#include CORBA_CLIENT_HEADER(MED)

class SMESH_Gen_i;

#include "SMESH_Mesh.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_subMesh_i.hxx"
#include "SMESH_subMesh.hxx"
#include "SMESH_topo.hxx"

#include <map>

class SMESH_Mesh_i:
  public POA_SMESH::SMESH_Mesh
{
public:
  SMESH_Mesh_i();
  SMESH_Mesh_i(SMESH_Gen_i* myGen_i,
	       GEOM::GEOM_Gen_ptr geomEngine,
	       CORBA::Long studyId,
	       int localId);

  virtual ~SMESH_Mesh_i();

  // --- CORBA

  CORBA::Boolean AddHypothesis(GEOM::GEOM_Shape_ptr aSubShape,
			       SMESH::SMESH_Hypothesis_ptr anHyp)
    throw (SALOME::SALOME_Exception);

  CORBA::Boolean  RemoveHypothesis(GEOM::GEOM_Shape_ptr aSubShape,
				   SMESH::SMESH_Hypothesis_ptr anHyp)
    throw (SALOME::SALOME_Exception);

  SMESH::ListOfHypothesis* GetHypothesisList(GEOM::GEOM_Shape_ptr aSubShape)
    throw (SALOME::SALOME_Exception);

  SMESH::SMESH_subMesh_ptr GetElementsOnShape(GEOM::GEOM_Shape_ptr aSubShape)
    throw (SALOME::SALOME_Exception);

//    SMESH::string_array* GetLog(CORBA::Boolean clearAfterGet)
//      throw (SALOME::SALOME_Exception);

  SMESH::log_array* GetLog(CORBA::Boolean clearAfterGet)
    throw (SALOME::SALOME_Exception);

  SMESH::SMESH_MeshEditor_ptr GetMeshEditor();

  void ClearLog()
    throw (SALOME::SALOME_Exception);

  CORBA::Long GetId()
    throw (SALOME::SALOME_Exception);

  CORBA::Long GetStudyId()
    throw (SALOME::SALOME_Exception);

  void Export(const char* fileName, const char* fileType)
	throw (SALOME::SALOME_Exception);

  // --- C++ interface

  void SetImpl(::SMESH_Mesh* impl);

  ::SMESH_Mesh& GetImpl();         // :: force no namespace here
  GEOM::GEOM_Gen_ptr GetGeomEngine();
  void SetIor(SMESH::SMESH_Mesh_ptr myIor);
  SMESH::SMESH_Mesh_ptr GetIor();

  SALOME_MED::MESH_ptr GetMEDMesh()
    throw (SALOME::SALOME_Exception);
  
  CORBA::Long NbNodes()
    throw (SALOME::SALOME_Exception);
  
  CORBA::Long NbEdges()
    throw (SALOME::SALOME_Exception);
  
  CORBA::Long NbFaces()
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbTriangles()
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbQuadrangles()
    throw (SALOME::SALOME_Exception);
  
  CORBA::Long NbVolumes()
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbTetras()
    throw (SALOME::SALOME_Exception);

  CORBA::Long NbHexas()
    throw (SALOME::SALOME_Exception);
  
  CORBA::Long NbSubMesh()
    throw (SALOME::SALOME_Exception);
  

  map<int, SMESH_subMesh_i*> _mapSubMesh_i; //NRI
  map<int, ::SMESH_subMesh*> _mapSubMesh;   //NRI

private:
  SMESH::log_array_var createUpdateAllCommand(SMESH::log_array_var log, int * index);
  ::SMESH_Mesh* _impl;  // :: force no namespace here
  SMESH_Gen_i* _gen_i;
  //   CORBA::ORB_ptr _orb;
//   SMESH_topo* _topo;   // all local TopoDS_Shape of subShapes
  int _id;          // id given by creator (unique within the creator instance)
  GEOM::GEOM_Gen_var _geom;
  int _studyId;
  //  int _localId; // id attributed to all objects created by Mesh_i
  map<int, SMESH::SMESH_subMesh_ptr> _mapSubMeshIor;
  SMESH::SMESH_Mesh_var _myIor;
};

#endif

