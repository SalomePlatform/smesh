//  SMESH SMESHGUI : GUI for SMESH component
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
//  File   : SMESHGUI_StudyAPI.h
//  Author : Yves FRICAUD 
//  Module : SMESH
//  $Header: 

#ifndef _INCLUDE_SMESHGUI_StudyAPI_
#define _INCLUDE_SMESHGUI_StudyAPI_

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Gen)
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Hypothesis)
#include CORBA_SERVER_HEADER(GEOM_Shape)
#include CORBA_SERVER_HEADER(SALOMEDS)
#include CORBA_SERVER_HEADER(SALOMEDS_Attributes)
#include CORBA_SERVER_HEADER(SALOME_GenericObj)
	
class SMESH_subMesh_ptr;

class SMESHGUI_StudyAPI {

  public:

  SMESHGUI_StudyAPI ();

  SMESHGUI_StudyAPI ( SALOMEDS::Study_ptr aStudy, SMESH::SMESH_Gen_ptr Gen);  

  ~SMESHGUI_StudyAPI ();

  // Adding in Study
  CORBA::Object_ptr     StringToIOR(const char* IOR);

  SALOMEDS::SObject_ptr AddNewMesh        (SMESH::SMESH_Mesh_ptr M);
  
  SALOMEDS::SObject_ptr AddNewHypothesis  (SMESH::SMESH_Hypothesis_ptr H);
  SALOMEDS::SObject_ptr AddNewAlgorithms  (SMESH::SMESH_Hypothesis_ptr H);

  SALOMEDS::SObject_ptr AddSubMeshOnShape (SALOMEDS::SObject_ptr SO_Mesh, SMESH::SMESH_subMesh_ptr SM, GEOM::shape_type ST);

  SALOMEDS::SObject_ptr AddSubMeshOnShape (SALOMEDS::SObject_ptr SO_Mesh, SALOMEDS::SObject_ptr SO_GeomShape, SMESH::SMESH_subMesh_ptr SM, GEOM::shape_type ST);

  void Update(SALOMEDS::Study_ptr aStudy);

  void ModifiedMesh( SALOMEDS::SObject_ptr MorSM, bool right);


  void SetShape  (SALOMEDS::SObject_ptr SO_Mesh_Or_SubMesh, SALOMEDS::SObject_ptr SO_GeomShape);
  GEOM::GEOM_Shape_ptr GetShapeOnMeshOrSubMesh(SALOMEDS::SObject_ptr SO_Mesh_Or_SubMesh);

  void SetHypothesis     (SALOMEDS::SObject_ptr SO_Mesh_Or_SubMesh, SALOMEDS::SObject_ptr SO_Hypothesis);
  void SetAlgorithms     (SALOMEDS::SObject_ptr SO_Mesh_Or_SubMesh, SALOMEDS::SObject_ptr SO_Algorithms);

  void UnSetHypothesis    (SALOMEDS::SObject_ptr SO_Applied_Hypothesis);
  void UnSetAlgorithm     (SALOMEDS::SObject_ptr SO_Applied_Algorithm);

  void SetTagHypothesisRoot();
  
  void SetName( SALOMEDS::SObject_ptr SO, const char* Name ) ;

  //Exploration
  
  SALOMEDS::SObject_ptr GetMeshOrSubmesh  (SALOMEDS::SObject_ptr SO);

  SALOMEDS::SObject_ptr FindObject(CORBA::Object_ptr theObject);

  static void setOrb();

  private:
  //fields
  SALOMEDS::Study_var        myStudy;
  SALOMEDS::StudyBuilder_var myStudyBuilder;
};
#endif

