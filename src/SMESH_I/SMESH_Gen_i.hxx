//=============================================================================
// File      : SMESH_Gen_i.hxx
// Created   : lun mai  6 13:41:30 CEST 2002
// Author    : Paul RASCLE, EDF
// Project   : SALOME
// Copyright : EDF 2002
// $Header$
//=============================================================================

#ifndef _SMESH_GEN_I_HXX_
#define _SMESH_GEN_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Gen)
#include CORBA_SERVER_HEADER(SMESH_Mesh)
#include CORBA_SERVER_HEADER(SMESH_Hypothesis)
#include CORBA_CLIENT_HEADER(GEOM_Gen)
#include CORBA_CLIENT_HEADER(GEOM_Shape)
#include CORBA_CLIENT_HEADER(SALOMEDS)
#include CORBA_CLIENT_HEADER(SALOMEDS_Attributes)


class SMESH_Mesh_i;

#include "SMESH_HypothesisFactory_i.hxx"
#include "SMESH_Mesh_i.hxx"
#include "SALOME_Component_i.hxx"
#include "SALOME_NamingService.hxx"

#include "SMESH_Gen.hxx"
#include "SMESH_topo.hxx"
#include "GEOM_Client.hxx"

#include <HDFOI.hxx>

#include <map>

typedef struct studyContext_iStruct
{
  map<int,SMESH_Mesh_i*> mapMesh_i;
} StudyContext_iStruct;

class SMESH_Gen_i:
  public POA_SMESH::SMESH_Gen,
  public Engines_Component_i 
{
public:

  SMESH_Gen_i();
  SMESH_Gen_i(CORBA::ORB_ptr orb,
              PortableServer::POA_ptr poa,
              PortableServer::ObjectId * contId, 
              const char *instanceName, 
              const char *interfaceName);
  virtual ~SMESH_Gen_i();
  
  SMESH::SMESH_Hypothesis_ptr CreateHypothesis(const char* anHyp,
                                               CORBA::Long studyId)
    throw (SALOME::SALOME_Exception);
  
  SMESH::SMESH_Mesh_ptr Init(GEOM::GEOM_Gen_ptr geomEngine,
                             CORBA::Long studyId,
                             GEOM::GEOM_Shape_ptr aShape)
    throw (SALOME::SALOME_Exception);

  CORBA::Boolean Compute(SMESH::SMESH_Mesh_ptr aMesh,
                         GEOM::GEOM_Shape_ptr aShape)
    throw (SALOME::SALOME_Exception);

  CORBA::Boolean IsReadyToCompute(SMESH::SMESH_Mesh_ptr aMesh,
                                  GEOM::GEOM_Shape_ptr aShape)
    throw (SALOME::SALOME_Exception);

  SMESH::long_array* GetSubShapesId(GEOM::GEOM_Gen_ptr geomEngine,
                                   CORBA::Long studyId,
                                   GEOM::GEOM_Shape_ptr mainShape,
                                   const SMESH::shape_array& listOfSubShape)
    throw (SALOME::SALOME_Exception);


  // inherited methods from SALOMEDS::Driver

  SALOMEDS::TMPFile* Save(SALOMEDS::SComponent_ptr theComponent,
			  const char* theURL,
			  bool isMultiFile);
  bool Load(SALOMEDS::SComponent_ptr theComponent,
	    const SALOMEDS::TMPFile& theStream,
	    const char* theURL,
	    bool isMultiFile);
  void Close(SALOMEDS::SComponent_ptr theComponent);
  char* ComponentDataType();
    
  char* IORToLocalPersistentID(SALOMEDS::SObject_ptr theSObject,
			       const char* IORString,
			       CORBA::Boolean isMultiFile);
  char* LocalPersistentIDToIOR(SALOMEDS::SObject_ptr theSObject,
			       const char* aLocalPersistentID,
			       CORBA::Boolean isMultiFile);

  bool CanPublishInStudy(CORBA::Object_ptr theIOR) { return false; }
  SALOMEDS::SObject_ptr PublishInStudy(SALOMEDS::Study_ptr theStudy,
		                       SALOMEDS::SObject_ptr theSObject,
		                       CORBA::Object_ptr theObject,
		                       const char* theName) throw (SALOME::SALOME_Exception) {
    SALOMEDS::SObject_var aResultSO;
    return aResultSO._retn();
  }

  CORBA::Boolean CanCopy(SALOMEDS::SObject_ptr theObject) {return false;}
  SALOMEDS::TMPFile* CopyFrom(SALOMEDS::SObject_ptr theObject, CORBA::Long& theObjectID) {return false;}
  CORBA::Boolean CanPaste(const char* theComponentName, CORBA::Long theObjectID) {return false;}
  SALOMEDS::SObject_ptr PasteInto(const SALOMEDS::TMPFile& theStream,
				  CORBA::Long theObjectID,
				  SALOMEDS::SObject_ptr theObject) {
    SALOMEDS::SObject_var aResultSO;
    return aResultSO._retn();
  }

  GEOM_Client* _ShapeReader;
protected:
  SMESH_topo* ExploreMainShape(GEOM::GEOM_Gen_ptr geomEngine,
                               CORBA::Long studyId,
                               GEOM::GEOM_Shape_ptr aShape);
  
private:
  SMESH_HypothesisFactory_i _hypothesisFactory_i;
  ::SMESH_Gen _impl;  // no namespace here
  int _localId; // unique Id of created objects, within SMESH_Gen_i entity

  map<int, StudyContext_iStruct*> _mapStudyContext_i;
  map <string, string> _SMESHCorbaObj;
};

#endif
