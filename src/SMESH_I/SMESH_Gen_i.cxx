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
//  File   : SMESH_Gen_i.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

using namespace std;
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_CompSolid.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <gp_Pnt.hxx>
#include <BRep_Tool.hxx>
#include <TCollection_AsciiString.hxx>

#include "Utils_CorbaException.hxx"

#include "utilities.h"
#include <fstream>
#include <stdio.h>
#include <dlfcn.h>

#include <HDFOI.hxx>

#include "SMESH_Gen_i.hxx"
#include "SMESH_Mesh_i.hxx"
#include "SMESH_Hypothesis_i.hxx"
#include "SMESH_Algo_i.hxx"
#include "SMESH_Group_i.hxx"

#include "SMESHDS_Document.hxx"
#include "SMESHDS_Group.hxx"
#include "SMESH_topo.hxx"
#include "SMESH_Group.hxx"

#include CORBA_SERVER_HEADER(SMESH_Group)
#include CORBA_SERVER_HEADER(SMESH_Filter)
#include "SMESH_Filter_i.hxx"

#include "Document_Reader.h"
#include "DriverMED_W_SMESHDS_Mesh.h"
#include "DriverMED_R_SMESHDS_Mesh.h"
#include "DriverMED_R_SMESHDS_Document.h"
#include "DriverUNV_R_SMESHDS_Document.h"
#include "DriverDAT_R_SMESHDS_Document.h"

#include "SALOMEDS_Tool.hxx"
#include "SALOME_NamingService.hxx"
#include "SALOME_LifeCycleCORBA.hxx"
#include "Utils_SINGLETON.hxx"
#include "OpUtil.hxx"

#include CORBA_CLIENT_HEADER(SALOME_ModuleCatalog)

#include "GEOM_Client.hxx"
#include "Utils_ExceptHandlers.hxx"

#include <map>

#define NUM_TMP_FILES 2

// Tags definition ===========================================================
// Top level
long Tag_HypothesisRoot         = 1; // hypotheses root
long Tag_AlgorithmsRoot         = 2; // algorithms root
// Mesh/Submesh
long Tag_RefOnShape             = 1; // references to shape
long Tag_RefOnAppliedHypothesis = 2; // applied hypotheses root
long Tag_RefOnAppliedAlgorithms = 3; // applied algorithms root
// Mesh only
long Tag_SubMeshOnVertex        = 4; // sub-meshes roots by type
long Tag_SubMeshOnEdge          = 5; // ...
long Tag_SubMeshOnFace          = 6; // ...
long Tag_SubMeshOnSolid         = 7; // ...
long Tag_SubMeshOnCompound      = 8; // ...
long Tag_NodeGroups             = 9; // Group roots by type
long Tag_EdgeGroups             = 10; // ...
long Tag_FaceGroups             = 11; // ...
long Tag_VolumeGroups           = 12; // ...
// ===========================================================================

// Static variables definition
CORBA::ORB_var          SMESH_Gen_i::myOrb;
PortableServer::POA_var SMESH_Gen_i::myPoa;
SALOME_NamingService*   SMESH_Gen_i::myNS  = NULL;
SALOME_LifeCycleCORBA*  SMESH_Gen_i::myLCC = NULL;

//=============================================================================
/*!
 *  FindMaxChildTag [ static internal ]
 *
 *  Finds maximum child tag for the given object
 */
//=============================================================================

static long FindMaxChildTag( SALOMEDS::SObject_ptr theSObject )
{
  long aTag = 0;
  if ( !theSObject->_is_nil() ) {
    SALOMEDS::Study_var aStudy = theSObject->GetStudy();
    if ( !aStudy->_is_nil() ) {
      SALOMEDS::ChildIterator_var anIter = aStudy->NewChildIterator( theSObject );
      for ( ; anIter->More(); anIter->Next() ) {
	long nTag = anIter->Value()->Tag();
	if ( nTag > aTag )
	  aTag = nTag;
      }
    }
  }
  return aTag;
}

//=============================================================================
/*!
 *  Get...Tag [ static ]
 *
 *  Methods which determine SMESH data model structure
 */
//=============================================================================

long SMESH_Gen_i::GetHypothesisRootTag()
{
  return Tag_HypothesisRoot;
}

long SMESH_Gen_i::GetAlgorithmsRootTag()
{
  return Tag_AlgorithmsRoot;
}

long SMESH_Gen_i::GetRefOnShapeTag()
{
  return Tag_RefOnShape;
}

long SMESH_Gen_i::GetRefOnAppliedHypothesisTag()
{
  return Tag_RefOnAppliedHypothesis;
}

long SMESH_Gen_i::GetRefOnAppliedAlgorithmsTag()
{
  return Tag_RefOnAppliedAlgorithms;
}

long SMESH_Gen_i::GetSubMeshOnVertexTag()
{
  return Tag_SubMeshOnVertex;
}

long SMESH_Gen_i::GetSubMeshOnEdgeTag()
{
  return Tag_SubMeshOnEdge;
}

long SMESH_Gen_i::GetSubMeshOnFaceTag()
{
  return Tag_SubMeshOnFace;
}

long SMESH_Gen_i::GetSubMeshOnSolidTag()
{
  return Tag_SubMeshOnSolid;
}

long SMESH_Gen_i::GetSubMeshOnCompoundTag()
{
  return Tag_SubMeshOnCompound;
}

long SMESH_Gen_i::GetNodeGroupsTag()
{
  return Tag_NodeGroups;
}

long SMESH_Gen_i::GetEdgeGroupsTag()
{
  return Tag_EdgeGroups;
}

long SMESH_Gen_i::GetFaceGroupsTag()
{
  return Tag_FaceGroups;
}

long SMESH_Gen_i::GetVolumeGroupsTag()
{
  return Tag_VolumeGroups;
}

//=============================================================================
/*!
 *  GetServant [ static ]
 *
 *  Get servant of the CORBA object
 */
//=============================================================================

PortableServer::ServantBase_var SMESH_Gen_i::GetServant( CORBA::Object_ptr theObject )
{
  if( CORBA::is_nil( theObject ) || CORBA::is_nil( GetPOA() ) )
    return NULL;
  try {
    PortableServer::Servant aServant = GetPOA()->reference_to_servant( theObject );
    return aServant;
  } 
  catch (...) {
    MESSAGE( "GetServant - Unknown exception was caught!!!" ); 
    return NULL;
  }
}

//=============================================================================
/*!
 *  SObjectToObject [ static ]
 *
 *  Get CORBA object corresponding to the SALOMEDS::SObject
 */
//=============================================================================

CORBA::Object_var SMESH_Gen_i::SObjectToObject( SALOMEDS::SObject_ptr theSObject )
{
  SALOMEDS::GenericAttribute_var anAttr;
  CORBA::Object_var anObj;
  if ( !theSObject->_is_nil() ) {
    try {
      if( theSObject->FindAttribute( anAttr, "AttributeIOR" ) ) {
	SALOMEDS::AttributeIOR_var anIOR  = SALOMEDS::AttributeIOR::_narrow( anAttr );
	CORBA::String_var aValue = anIOR->Value();
	if( strcmp( aValue, "" ) != 0 )
	  anObj = GetORB()->string_to_object( aValue );
	}
    }
    catch( ... ) {
      MESSAGE( "SObjectToObject - Unknown exception was caught!!!" );
    }
  }
  return anObj;
}

//=============================================================================
/*!
 *  GetNS [ static ]
 *
 *  Get SALOME_NamingService object 
 */
//=============================================================================

SALOME_NamingService* SMESH_Gen_i::GetNS()
{
  if ( myNS == NULL ) {
    myNS = SINGLETON_<SALOME_NamingService>::Instance();
    ASSERT(SINGLETON_<SALOME_NamingService>::IsAlreadyExisting());
    myNS->init_orb( GetORB() );
  }
  return myNS;
}

//=============================================================================
/*!
 *  GetLCC [ static ]
 *
 *  Get SALOME_LifeCycleCORBA object
 */
//=============================================================================     
SALOME_LifeCycleCORBA*  SMESH_Gen_i::GetLCC() {
  if ( myLCC == NULL ) {
    myLCC = new SALOME_LifeCycleCORBA( GetNS() );
  }
  return myLCC;
}


//=============================================================================
/*!
 *  GetGeomEngine [ static ]
 *
 *  Get GEOM::GEOM_Gen reference
 */
//=============================================================================     
GEOM::GEOM_Gen_ptr SMESH_Gen_i::GetGeomEngine() {
  GEOM::GEOM_Gen_var aGeomEngine =
    GEOM::GEOM_Gen::_narrow( GetLCC()->FindOrLoad_Component("FactoryServer","GEOM") );
  return aGeomEngine._retn();
}

//=============================================================================
/*!
 *  SMESH_Gen_i::SMESH_Gen_i
 *
 *  Default constructor: not for use
 */
//=============================================================================

SMESH_Gen_i::SMESH_Gen_i()
{
  MESSAGE( "SMESH_Gen_i::SMESH_Gen_i : default constructor" );
}

//=============================================================================
/*!
 *  SMESH_Gen_i::SMESH_Gen_i 
 *
 *  Standard constructor, used with Container
 */
//=============================================================================

SMESH_Gen_i::SMESH_Gen_i( CORBA::ORB_ptr            orb,
			  PortableServer::POA_ptr   poa,
			  PortableServer::ObjectId* contId, 
			  const char*               instanceName, 
                          const char*               interfaceName )
     : Engines_Component_i( orb, poa, contId, instanceName, interfaceName )
{
  MESSAGE( "SMESH_Gen_i::SMESH_Gen_i : standard constructor" );

  myOrb = CORBA::ORB::_duplicate(orb);
  myPoa = PortableServer::POA::_duplicate(poa);
  
  _thisObj = this ;
  _id = myPoa->activate_object( _thisObj );
  
  myShapeReader = NULL;  // shape reader
}

//=============================================================================
/*!
 *  SMESH_Gen_i::~SMESH_Gen_i
 *
 *  Destructor
 */
//=============================================================================

SMESH_Gen_i::~SMESH_Gen_i()
{
  MESSAGE( "SMESH_Gen_i::~SMESH_Gen_i" );

  // delete hypothesis creators
  map<string, GenericHypothesisCreator_i*>::iterator itHyp;
  for (itHyp = myHypCreatorMap.begin(); itHyp != myHypCreatorMap.end(); itHyp++)
  {
    delete (*itHyp).second;
  }
  myHypCreatorMap.clear();

  // Clear study contexts data
  map<int, StudyContext*>::iterator it;
  for ( it = myStudyContextMap.begin(); it != myStudyContextMap.end(); ++it ) {
    delete it->second;
  }
  myStudyContextMap.clear();
  // delete shape reader
  if ( !myShapeReader ) 
    delete myShapeReader;
}
  
//=============================================================================
/*!
 *  SMESH_Gen_i::CreateFilterManager
 *
 *  Create filter manager
 */
//=============================================================================

SMESH::FilterManager_ptr SMESH_Gen_i::CreateFilterManager()
{
  SMESH::FilterManager_i* aFilter = new SMESH::FilterManager_i();
  SMESH::FilterManager_var anObj = aFilter->_this();
  return anObj._retn();
}

//=============================================================================
/*!
 *  SMESH_Gen_i::createHypothesis
 *
 *  Create hypothesis of given type
 */
//=============================================================================
SMESH::SMESH_Hypothesis_ptr SMESH_Gen_i::createHypothesis(const char* theHypName,
                                                          const char* theLibName)
     throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  MESSAGE( "Create Hypothesis <" << theHypName << "> from " << theLibName);

  // get study context
  StudyContext* myStudyContext = GetCurrentStudyContext();
  
  // create a new hypothesis object servant
  SMESH_Hypothesis_i* myHypothesis_i = 0;
  SMESH::SMESH_Hypothesis_var hypothesis_i;

  try
  {
    // check, if creator for this hypothesis type already exists
    if (myHypCreatorMap.find(string(theHypName)) == myHypCreatorMap.end())
    {
      // load plugin library
      MESSAGE("Loading server meshers plugin library ...");
      void* libHandle = dlopen (theLibName, RTLD_LAZY);
      if (!libHandle)
      {
        // report any error, if occured
        const char* anError = dlerror();
        throw(SALOME_Exception(anError));
      }

      // get method, returning hypothesis creator
      MESSAGE("Find GetHypothesisCreator() method ...");
      typedef GenericHypothesisCreator_i* (*GetHypothesisCreator)(const char* theHypName);
      GetHypothesisCreator procHandle =
        (GetHypothesisCreator)dlsym( libHandle, "GetHypothesisCreator" );
      if (!procHandle)
      {
        throw(SALOME_Exception(LOCALIZED("bad hypothesis plugin library")));
        dlclose(libHandle);
      }

      // get hypothesis creator
      MESSAGE("Get Hypothesis Creator for " << theHypName);
      GenericHypothesisCreator_i* aCreator = procHandle(theHypName);
      if (!aCreator)
      {
        throw(SALOME_Exception(LOCALIZED("no such a hypothesis in this plugin")));
      }

      // map hypothesis creator to a hypothesis name
      myHypCreatorMap[string(theHypName)] = aCreator;
    }

    // create a new hypothesis object, store its ref. in studyContext
    MESSAGE("Create Hypothesis " << theHypName);
    myHypothesis_i =
      myHypCreatorMap[string(theHypName)]->Create
        (myPoa, myCurrentStudy->StudyId(), &myGen);
    myHypothesis_i->SetLibName(theLibName); // for persistency assurance
  }
  catch (SALOME_Exception& S_ex)
  {
    THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
  }

  if (!myHypothesis_i)
    return hypothesis_i._retn();

  // activate the CORBA servant of hypothesis
  hypothesis_i = SMESH::SMESH_Hypothesis::_narrow( myHypothesis_i->_this() );
  string iorString = GetORB()->object_to_string( hypothesis_i );
  int nextId = myStudyContext->addObject( iorString );
  MESSAGE( "Add hypo to map with id = "<< nextId << " and IOR = " << iorString.c_str() );

  return hypothesis_i._retn();
}
  
//=============================================================================
/*!
 *  SMESH_Gen_i::createMesh
 *
 *  Create empty mesh on shape
 */
//=============================================================================
SMESH::SMESH_Mesh_ptr SMESH_Gen_i::createMesh()
     throw ( SALOME::SALOME_Exception )
{
  Unexpect aCatch(SALOME_SalomeException);
  MESSAGE( "SMESH_Gen_i::createMesh" );

  // get current study
  StudyContext* myStudyContext = GetCurrentStudyContext();

  // Get or create the GEOM_Client instance
  try {
    // create a new mesh object servant, store it in a map in study context
    SMESH_Mesh_i* meshServant = new SMESH_Mesh_i( GetPOA(),
						  this,
						  myCurrentStudy->StudyId() );
    // create a new mesh object
    meshServant->SetImpl( myGen.CreateMesh( myCurrentStudy->StudyId() ) );

    // activate the CORBA servant of Mesh
    SMESH::SMESH_Mesh_var mesh = meshServant->_this();
    string iorString = GetORB()->object_to_string( mesh );
    int nextId = myStudyContext->addObject( iorString );
    MESSAGE( "Add mesh to map with id = "<< nextId << " and IOR = " << iorString.c_str() );
    return mesh._retn();
  }
  catch (SALOME_Exception& S_ex) {
    THROW_SALOME_CORBA_EXCEPTION( S_ex.what(), SALOME::BAD_PARAM );
  }
  return SMESH::SMESH_Mesh::_nil();
}

//=============================================================================
/*!
 *  SMESH_Gen_i::GetShapeReader
 *
 *  Get shape reader
 */
//=============================================================================
GEOM_Client* SMESH_Gen_i::GetShapeReader()
{
  // create shape reader if necessary
  if ( !myShapeReader ) 
    myShapeReader = new GEOM_Client(GetContainerRef());
  ASSERT( myShapeReader );
  return myShapeReader;
}

//=============================================================================
/*!
 *  SMESH_Gen_i::SetCurrentStudy
 *
 *  Set current study
 */
//=============================================================================

void SMESH_Gen_i::SetCurrentStudy( SALOMEDS::Study_ptr theStudy )
{
  MESSAGE( "SMESH_Gen_i::SetCurrentStudy" );
  myCurrentStudy = SALOMEDS::Study::_duplicate( theStudy );
  // create study context, if it doesn't exist and set current study
  int studyId = myCurrentStudy->StudyId();
  MESSAGE( "SMESH_Gen_i::SetCurrentStudy: study Id = " << studyId );
  if ( myStudyContextMap.find( studyId ) == myStudyContextMap.end() ) {
    myStudyContextMap[ studyId ] = new StudyContext;      
  }
  // set current study for geom engine
  if ( !CORBA::is_nil( GetGeomEngine() ) )
    GetGeomEngine()->GetCurrentStudy( myCurrentStudy->StudyId() );
}

//=============================================================================
/*!
 *  SMESH_Gen_i::GetCurrentStudy
 *
 *  Get current study
 */
//=============================================================================

SALOMEDS::Study_ptr SMESH_Gen_i::GetCurrentStudy()
{
  MESSAGE( "SMESH_Gen_i::GetCurrentStudy: study Id = " << myCurrentStudy->StudyId() );
  return SALOMEDS::Study::_duplicate( myCurrentStudy );
}

//=============================================================================
/*!
 *  SMESH_Gen_i::GetCurrentStudyContext 
 *
 *  Get current study context
 */
//=============================================================================
StudyContext* SMESH_Gen_i::GetCurrentStudyContext()
{
  ASSERT( !CORBA::is_nil( myCurrentStudy ) )
  ASSERT( myStudyContextMap.find( myCurrentStudy->StudyId() ) != myStudyContextMap.end() );
  return myStudyContextMap[ myCurrentStudy->StudyId() ];
}

//=============================================================================
/*!
 *  SMESH_Gen_i::CreateHypothesis 
 *
 *  Create hypothesis/algorothm of given type and publish it in the study
 */
//=============================================================================

SMESH::SMESH_Hypothesis_ptr SMESH_Gen_i::CreateHypothesis( const char* theHypName,
                                                           const char* theLibName )
     throw ( SALOME::SALOME_Exception )
{
  Unexpect aCatch(SALOME_SalomeException);
  ASSERT( !CORBA::is_nil( myCurrentStudy ) );
  // Create hypothesis/algorithm
  SMESH::SMESH_Hypothesis_var hyp = this->createHypothesis( theHypName, theLibName );

  // Publish hypothesis/algorithm in the study
  if ( this->CanPublishInStudy( hyp ) ) {
    this->PublishInStudy( myCurrentStudy, SALOMEDS::SObject::_nil(), hyp, "" );
  }
  return hyp._retn();
}
  
//=============================================================================
/*!
 *  SMESH_Gen_i::CreateMesh
 *
 *  Create empty mesh on a shape and publish it in the study
 */
//=============================================================================

SMESH::SMESH_Mesh_ptr SMESH_Gen_i::CreateMesh( GEOM::GEOM_Shape_ptr theShape )
     throw ( SALOME::SALOME_Exception )
{
  Unexpect aCatch(SALOME_SalomeException);
  MESSAGE( "SMESH_Gen_i::CreateMesh" );
  ASSERT( !CORBA::is_nil( myCurrentStudy ) );
  // create mesh
  SMESH::SMESH_Mesh_var mesh = this->createMesh();
  // publish mesh in the study
  if ( this->CanPublishInStudy( mesh ) ) {
    this->PublishInStudy( myCurrentStudy, SALOMEDS::SObject::_nil(), mesh.in(), "" );
  }
  // set shape
  SMESH_Mesh_i* meshServant = dynamic_cast<SMESH_Mesh_i*>( GetServant( mesh ).in() );
  ASSERT( meshServant );
  meshServant->SetShape( theShape );
  return mesh._retn();
}
  
//=============================================================================
/*!
 *  SMESH_Gen_i::CreateMeshFromMED
 *
 *  Create mesh and import data from MED file
 */
//=============================================================================

SMESH::mesh_array* SMESH_Gen_i::CreateMeshesFromMED( const char* theFileName,
                                                     SMESH::DriverMED_ReadStatus& theStatus)
     throw ( SALOME::SALOME_Exception )
{
  Unexpect aCatch(SALOME_SalomeException);
  MESSAGE( "SMESH_Gen_i::CreateMeshFromMED" );
  ASSERT( !CORBA::is_nil( myCurrentStudy ) );

  // Retrieve mesh names from the file
  DriverMED_R_SMESHDS_Mesh myReader;
  myReader.SetFile( theFileName );
  myReader.SetMeshId( -1 );
  list<string> aNames = myReader.GetMeshNames();

  SMESH::mesh_array_var aResult = new SMESH::mesh_array();
  aResult->length( aNames.size() );
  int i = 0;

  // Iterate through all meshes and create mesh objects
  theStatus = SMESH::DRS_OK;
  for ( list<string>::iterator it = aNames.begin(); it != aNames.end(); it++ ) {
    // create mesh
    SMESH::SMESH_Mesh_var mesh = createMesh();
    
    // publish mesh in the study
    if ( CanPublishInStudy( mesh ) ) {
      PublishInStudy( myCurrentStudy, SALOMEDS::SObject::_nil(), mesh.in(), (*it).c_str() );
    }

    // Read mesh data (groups are published automatically by ImportMEDFile())
    SMESH_Mesh_i* meshServant = dynamic_cast<SMESH_Mesh_i*>( GetServant( mesh ).in() );
    ASSERT( meshServant );
    SMESH::DriverMED_ReadStatus status1 =
      meshServant->ImportMEDFile( theFileName, (*it).c_str() );
    if (status1 > theStatus)
      theStatus = status1;

    aResult[i++] = SMESH::SMESH_Mesh::_duplicate( mesh );
  }

  return aResult._retn();
}

//=============================================================================
/*!
 *  SMESH_Gen_i::IsReadyToCompute
 *
 *  Returns true if mesh contains enough data to be computed
 */
//=============================================================================

CORBA::Boolean SMESH_Gen_i::IsReadyToCompute( SMESH::SMESH_Mesh_ptr theMesh,
                                              GEOM::GEOM_Shape_ptr  theShape )
  throw ( SALOME::SALOME_Exception )
{
  Unexpect aCatch(SALOME_SalomeException);
  MESSAGE( "SMESH_Gen_i::IsReadyToCompute" );

  if ( CORBA::is_nil( theShape ) )
    THROW_SALOME_CORBA_EXCEPTION( "bad shape reference", 
                                  SALOME::BAD_PARAM );

  if ( CORBA::is_nil( theMesh ) )
    THROW_SALOME_CORBA_EXCEPTION( "bad Mesh reference",
                                  SALOME::BAD_PARAM );

  try {
    // get mesh servant
    SMESH_Mesh_i* meshServant = dynamic_cast<SMESH_Mesh_i*>( GetServant( theMesh ).in() );
    ASSERT( meshServant );
    if ( meshServant ) {
      // get local TopoDS_Shape
      TopoDS_Shape myLocShape = GetShapeReader()->GetShape( GetGeomEngine(), theShape );
      // call implementation
      ::SMESH_Mesh& myLocMesh = meshServant->GetImpl();
      return myGen.CheckAlgoState( myLocMesh, myLocShape );
    }
  }
  catch ( SALOME_Exception& S_ex ) {
    MESSAGE( "catch exception "<< S_ex.what() );
  }
  return false;
}

//=============================================================================
/*!
 *  SMESH_Gen_i::GetSubShapesId
 *
 *  Get sub-shapes unique ID's list
 */
//=============================================================================

SMESH::long_array* SMESH_Gen_i::GetSubShapesId( GEOM::GEOM_Shape_ptr      theMainShape,
					        const SMESH::shape_array& theListOfSubShape )
     throw ( SALOME::SALOME_Exception )
{
  Unexpect aCatch(SALOME_SalomeException);
  MESSAGE( "SMESH_Gen_i::GetSubShapesId" );

  SMESH::long_array_var shapesId = new SMESH::long_array;
  set<int> setId;

  if ( CORBA::is_nil( theMainShape ) )
    THROW_SALOME_CORBA_EXCEPTION( "bad shape reference",
                                  SALOME::BAD_PARAM );

  try
    {
      if (! myShapeReader) myShapeReader = new GEOM_Client(GetContainerRef());
      ASSERT(myShapeReader);
      TopoDS_Shape myMainShape  = GetShapeReader()->GetShape(GetGeomEngine(),theMainShape);
      TopTools_IndexedMapOfShape myIndexToShape;      
      TopExp::MapShapes(myMainShape,myIndexToShape);

      for (int i=0; i<theListOfSubShape.length(); i++)
	{
	  GEOM::GEOM_Shape_var aShape
	    = GEOM::GEOM_Shape::_narrow(theListOfSubShape[i]);
	  if (CORBA::is_nil(aShape))
	    THROW_SALOME_CORBA_EXCEPTION("bad shape reference", \
					 SALOME::BAD_PARAM);
	  TopoDS_Shape locShape  = GetShapeReader()->GetShape(GetGeomEngine(),aShape);
	  for (TopExp_Explorer exp(locShape,TopAbs_FACE); exp.More(); exp.Next())
	    {
	      const TopoDS_Face& F = TopoDS::Face(exp.Current());
	      setId.insert(myIndexToShape.FindIndex(F));
	      SCRUTE(myIndexToShape.FindIndex(F));
	    }
	  for (TopExp_Explorer exp(locShape,TopAbs_EDGE); exp.More(); exp.Next())
	    {
	      const TopoDS_Edge& E = TopoDS::Edge(exp.Current());
	      setId.insert(myIndexToShape.FindIndex(E));
	      SCRUTE(myIndexToShape.FindIndex(E));
	    }
	  for (TopExp_Explorer exp(locShape,TopAbs_VERTEX); exp.More(); exp.Next())
	    {
	      const TopoDS_Vertex& V = TopoDS::Vertex(exp.Current());
	      setId.insert(myIndexToShape.FindIndex(V));
	      SCRUTE(myIndexToShape.FindIndex(V));
	    }
	}
      shapesId->length(setId.size());
      set<int>::iterator iind;
      int i=0;
      for (iind = setId.begin(); iind != setId.end(); iind++)
	{
	  SCRUTE((*iind));
	  shapesId[i] = (*iind);
	  SCRUTE(shapesId[i]);
	  i++;
	}
    }
  catch (SALOME_Exception& S_ex)
    {
      THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
    }

  return shapesId._retn();
}

//=============================================================================
/*!
 *  SMESH_Gen_i::Compute
 *
 *  Compute mesh on a shape
 */
//=============================================================================

CORBA::Boolean SMESH_Gen_i::Compute( SMESH::SMESH_Mesh_ptr theMesh,
                                     GEOM::GEOM_Shape_ptr  theShape )
     throw ( SALOME::SALOME_Exception )
{
  Unexpect aCatch(SALOME_SalomeException);
  MESSAGE( "SMESH_Gen_i::Compute" );

  if ( CORBA::is_nil( theShape ) )
    THROW_SALOME_CORBA_EXCEPTION( "bad shape reference", 
                                  SALOME::BAD_PARAM );

  if ( CORBA::is_nil( theMesh ) )
    THROW_SALOME_CORBA_EXCEPTION( "bad Mesh reference",
                                  SALOME::BAD_PARAM );

  try {
    // get mesh servant
    SMESH_Mesh_i* meshServant = dynamic_cast<SMESH_Mesh_i*>( GetServant( theMesh ).in() );
    ASSERT( meshServant );
    if ( meshServant ) {
      // get local TopoDS_Shape
      TopoDS_Shape myLocShape = GetShapeReader()->GetShape( GetGeomEngine(), theShape );
      // call implementarion compute
      ::SMESH_Mesh& myLocMesh = meshServant->GetImpl();
      return myGen.Compute( myLocMesh, myLocShape);
    }
  }
  catch ( SALOME_Exception& S_ex ) {
    MESSAGE( "Compute(): catch exception "<< S_ex.what() );
  }
  catch ( ... ) {
    MESSAGE( "Compute(): unknown exception " );
  }
  return false;
}

//=============================================================================
/*!
 *  SMESH_Gen_i::Save
 *
 *  Save SMESH module's data
 */
//=============================================================================
SALOMEDS::TMPFile* SMESH_Gen_i::Save( SALOMEDS::SComponent_ptr theComponent,
				      const char*              theURL,
				      bool                     isMultiFile )
{
  INFOS( "SMESH_Gen_i::Save" );

  ASSERT( theComponent->GetStudy()->StudyId() == myCurrentStudy->StudyId() )
  StudyContext* myStudyContext = GetCurrentStudyContext();
  
  // Declare a byte stream
  SALOMEDS::TMPFile_var aStreamFile;
  
  // Obtain a temporary dir
  TCollection_AsciiString tmpDir =
    ( isMultiFile ) ? TCollection_AsciiString( ( char* )theURL ) : SALOMEDS_Tool::GetTmpDir();

  // Create a sequence of files processed
  SALOMEDS::ListOfFileNames_var aFileSeq = new SALOMEDS::ListOfFileNames;
  aFileSeq->length( NUM_TMP_FILES );

  TCollection_AsciiString aStudyName( "" );
  if ( isMultiFile ) 
    aStudyName = ( SALOMEDS_Tool::GetNameFromPath( myCurrentStudy->URL() ) );

  // Set names of temporary files
  TCollection_AsciiString filename =
    aStudyName + TCollection_AsciiString( "_SMESH.hdf" );        // for SMESH data itself
  TCollection_AsciiString meshfile =
    aStudyName + TCollection_AsciiString( "_SMESH_Mesh.med" );   // for mesh data to be stored in MED file
  aFileSeq[ 0 ] = CORBA::string_dup( filename.ToCString() );
  aFileSeq[ 1 ] = CORBA::string_dup( meshfile.ToCString() );
  filename = tmpDir + filename;
  meshfile = tmpDir + meshfile;

  HDFfile*    aFile;
  HDFdataset* aDataset;
  HDFgroup*   aTopGroup;
  HDFgroup*   aGroup;
  HDFgroup*   aSubGroup;
  HDFgroup*   aSubSubGroup;
  hdf_size    aSize[ 1 ];

  // MED writer to be used by storage process
  DriverMED_W_SMESHDS_Mesh myWriter;
  myWriter.SetFile( meshfile.ToCString() );

  // Write data
  // ---> create HDF file
  aFile = new HDFfile( filename.ToCString() );
  aFile->CreateOnDisk();
  
  // --> iterator for top-level objects
  SALOMEDS::ChildIterator_var itBig = myCurrentStudy->NewChildIterator( theComponent );
  for ( ; itBig->More(); itBig->Next() ) {
    SALOMEDS::SObject_var gotBranch = itBig->Value();

    // --> hypotheses root branch (only one for the study)
    if ( gotBranch->Tag() == GetHypothesisRootTag() ) {
      // create hypotheses root HDF group
      aTopGroup = new HDFgroup( "Hypotheses", aFile );
      aTopGroup->CreateOnDisk();

      // iterator for all hypotheses
      SALOMEDS::ChildIterator_var it = myCurrentStudy->NewChildIterator( gotBranch );
      for ( ; it->More(); it->Next() ) {
        SALOMEDS::SObject_var mySObject = it->Value();
	CORBA::Object_var anObject = SObjectToObject( mySObject );
	if ( !CORBA::is_nil( anObject ) ) {
          SMESH::SMESH_Hypothesis_var myHyp = SMESH::SMESH_Hypothesis::_narrow( anObject );
          if ( !myHyp->_is_nil() ) {
	    SMESH_Hypothesis_i* myImpl = dynamic_cast<SMESH_Hypothesis_i*>( GetServant( myHyp ).in() );
	    if ( myImpl ) {
	      string hypname = string( myHyp->GetName() );
	      string libname = string( myHyp->GetLibName() );
	      int    id      = myStudyContext->findId( string( GetORB()->object_to_string( anObject ) ) );
	      string hypdata = string( myImpl->SaveTo() );

	      // for each hypothesis create HDF group basing on its id
	      char hypGrpName[30];
	      sprintf( hypGrpName, "Hypothesis %d", id );
	      aGroup = new HDFgroup( hypGrpName, aTopGroup );
	      aGroup->CreateOnDisk();
	      // --> type name of hypothesis
	      aSize[ 0 ] = hypname.length() + 1;
	      aDataset = new HDFdataset( "Name", aGroup, HDF_STRING, aSize, 1 );
	      aDataset->CreateOnDisk();
	      aDataset->WriteOnDisk( ( char* )( hypname.c_str() ) );
	      aDataset->CloseOnDisk();
	      // --> server plugin library name of hypothesis
	      aSize[ 0 ] = libname.length() + 1;
	      aDataset = new HDFdataset( "LibName", aGroup, HDF_STRING, aSize, 1 );
	      aDataset->CreateOnDisk();
	      aDataset->WriteOnDisk( ( char* )( libname.c_str() ) );
	      aDataset->CloseOnDisk();
	      // --> persistent data of hypothesis
	      aSize[ 0 ] = hypdata.length() + 1;
	      aDataset = new HDFdataset( "Data", aGroup, HDF_STRING, aSize, 1 );
	      aDataset->CreateOnDisk();
	      aDataset->WriteOnDisk( ( char* )( hypdata.c_str() ) );
	      aDataset->CloseOnDisk();
	      // close hypothesis HDF group
	      aGroup->CloseOnDisk();
	    }
	  }
	}
      }
      // close hypotheses root HDF group
      aTopGroup->CloseOnDisk();
    }
    // --> algorithms root branch (only one for the study)
    else if ( gotBranch->Tag() == GetAlgorithmsRootTag() ) {
      // create algorithms root HDF group
      aTopGroup = new HDFgroup( "Algorithms", aFile );
      aTopGroup->CreateOnDisk();

      // iterator for all algorithms
      SALOMEDS::ChildIterator_var it = myCurrentStudy->NewChildIterator( gotBranch );
      for ( ; it->More(); it->Next() ) {
        SALOMEDS::SObject_var mySObject = it->Value();
	CORBA::Object_var anObject = SObjectToObject( mySObject );
	if ( !CORBA::is_nil( anObject ) ) {
          SMESH::SMESH_Hypothesis_var myHyp = SMESH::SMESH_Hypothesis::_narrow( anObject );
          if ( !myHyp->_is_nil() ) {
	    SMESH_Hypothesis_i* myImpl = dynamic_cast<SMESH_Hypothesis_i*>( GetServant( myHyp ).in() );
	    if ( myImpl ) {
	      string hypname = string( myHyp->GetName() );
	      string libname = string( myHyp->GetLibName() );
	      int    id      = myStudyContext->findId( string( GetORB()->object_to_string( anObject ) ) );
	      string hypdata = string( myImpl->SaveTo() );

	      // for each algorithm create HDF group basing on its id
	      char hypGrpName[30];
	      sprintf( hypGrpName, "Algorithm %d", id );
	      aGroup = new HDFgroup( hypGrpName, aTopGroup );
	      aGroup->CreateOnDisk();
	      // --> type name of algorithm
	      aSize[0] = hypname.length() + 1;
	      aDataset = new HDFdataset( "Name", aGroup, HDF_STRING, aSize, 1 );
	      aDataset->CreateOnDisk();
	      aDataset->WriteOnDisk( ( char* )( hypname.c_str() ) );
	      aDataset->CloseOnDisk();
	      // --> server plugin library name of hypothesis
	      aSize[0] = libname.length() + 1;
	      aDataset = new HDFdataset( "LibName", aGroup, HDF_STRING, aSize, 1 );
	      aDataset->CreateOnDisk();
	      aDataset->WriteOnDisk( ( char* )( libname.c_str() ) );
	      aDataset->CloseOnDisk();
	      // --> persistent data of algorithm
	      aSize[0] = hypdata.length() + 1;
	      aDataset = new HDFdataset( "Data", aGroup, HDF_STRING, aSize, 1 );
	      aDataset->CreateOnDisk();
	      aDataset->WriteOnDisk( ( char* )( hypdata.c_str() ) );
	      aDataset->CloseOnDisk();
	      // close algorithm HDF group
	      aGroup->CloseOnDisk();
	    }
	  }
	}
      }
      // close algorithms root HDF group
      aTopGroup->CloseOnDisk();
    }
    // --> mesh objects roots branches
    else if ( gotBranch->Tag() > GetAlgorithmsRootTag() ) {
      CORBA::Object_var anObject = SObjectToObject( gotBranch );
      if ( !CORBA::is_nil( anObject ) ) {
	SMESH::SMESH_Mesh_var myMesh = SMESH::SMESH_Mesh::_narrow( anObject ) ;
        if ( !myMesh->_is_nil() ) {
	  SMESH_Mesh_i* myImpl = dynamic_cast<SMESH_Mesh_i*>( GetServant( myMesh ).in() );
	  if ( myImpl ) {
	    int id = myStudyContext->findId( string( GetORB()->object_to_string( anObject ) ) );
	    ::SMESH_Mesh& myLocMesh = myImpl->GetImpl();
	    SMESHDS_Mesh* mySMESHDSMesh = myLocMesh.GetMeshDS();

	    // for each mesh open the HDF group basing on its id
	    char meshGrpName[ 30 ];
	    sprintf( meshGrpName, "Mesh %d", id );
	    aTopGroup = new HDFgroup( meshGrpName, aFile );
	    aTopGroup->CreateOnDisk();

	    // --> put dataset to hdf file which is a flag that mesh has data
	    string strHasData = "0";
	    // check if the mesh is not empty
	    if ( mySMESHDSMesh->NbNodes() > 0 ) {
	      // write mesh data to med file
	      myWriter.SetMesh( mySMESHDSMesh );
	      myWriter.SetMeshId( id );
	      strHasData = "1";
	    }
	    aSize[ 0 ] = strHasData.length() + 1;
	    aDataset = new HDFdataset( "Has data", aTopGroup, HDF_STRING, aSize, 1 );
	    aDataset->CreateOnDisk();
	    aDataset->WriteOnDisk( ( char* )( strHasData.c_str() ) );
	    aDataset->CloseOnDisk();
	    
	    // write reference on a shape if exists
	    SALOMEDS::SObject_var myRef;
	    bool found = gotBranch->FindSubObject( GetRefOnShapeTag(), myRef );
	    if ( found ) {
	      SALOMEDS::SObject_var myShape;
	      bool ok = myRef->ReferencedObject( myShape );
	      if ( ok ) {
		string myRefOnObject = myShape->GetID();
		if ( myRefOnObject.length() > 0 ) {
		  aSize[ 0 ] = myRefOnObject.length() + 1;
		  aDataset = new HDFdataset( "Ref on shape", aTopGroup, HDF_STRING, aSize, 1 );
		  aDataset->CreateOnDisk();
		  aDataset->WriteOnDisk( ( char* )( myRefOnObject.c_str() ) );
		  aDataset->CloseOnDisk();
		}
	      }
	    }

	    // write applied hypotheses if exist
	    SALOMEDS::SObject_var myHypBranch;
	    found = gotBranch->FindSubObject( GetRefOnAppliedHypothesisTag(), myHypBranch );
	    if ( found ) {
	      aGroup = new HDFgroup( "Applied Hypotheses", aTopGroup );
	      aGroup->CreateOnDisk();

	      SALOMEDS::ChildIterator_var it = myCurrentStudy->NewChildIterator( myHypBranch );
	      int hypNb = 0;
	      for ( ; it->More(); it->Next() ) {
		SALOMEDS::SObject_var mySObject = it->Value();
		SALOMEDS::SObject_var myRefOnHyp;
		bool ok = mySObject->ReferencedObject( myRefOnHyp );
		if ( ok ) {
		  // san - it is impossible to recover applied hypotheses
                  //       using their entries within Load() method,
		  // for there are no AttributeIORs in the study when Load() is working. 
		  // Hence, it is better to store persistent IDs of hypotheses as references to them

		  //string myRefOnObject = myRefOnHyp->GetID();
		  CORBA::Object_var anObject = SObjectToObject( myRefOnHyp );
		  int id = myStudyContext->findId( string( GetORB()->object_to_string( anObject ) ) );
		  //if ( myRefOnObject.length() > 0 ) {
		  //aSize[ 0 ] = myRefOnObject.length() + 1;
		  char hypName[ 30 ], hypId[ 30 ];
		  sprintf( hypName, "Hyp %d", ++hypNb );
		  sprintf( hypId, "%d", id );
		  aSize[ 0 ] = strlen( hypId ) + 1;
		  aDataset = new HDFdataset( hypName, aGroup, HDF_STRING, aSize, 1 );
		  aDataset->CreateOnDisk();
		  //aDataset->WriteOnDisk( ( char* )( myRefOnObject.c_str() ) );
		  aDataset->WriteOnDisk( hypId );
		  aDataset->CloseOnDisk();
		  //}
		}
	      }
	      aGroup->CloseOnDisk();
	    }

	    // write applied algorithms if exist
	    SALOMEDS::SObject_var myAlgoBranch;
	    found = gotBranch->FindSubObject( GetRefOnAppliedAlgorithmsTag(), myAlgoBranch );
	    if ( found ) {
	      aGroup = new HDFgroup( "Applied Algorithms", aTopGroup );
	      aGroup->CreateOnDisk();

	      SALOMEDS::ChildIterator_var it = myCurrentStudy->NewChildIterator( myAlgoBranch );
	      int algoNb = 0;
	      for ( ; it->More(); it->Next() ) {
		SALOMEDS::SObject_var mySObject = it->Value();
		SALOMEDS::SObject_var myRefOnAlgo;
		bool ok = mySObject->ReferencedObject( myRefOnAlgo );
		if ( ok ) {
		  // san - it is impossible to recover applied algorithms
                  //       using their entries within Load() method,
		  // for there are no AttributeIORs in the study when Load() is working. 
		  // Hence, it is better to store persistent IDs of algorithms as references to them

		  //string myRefOnObject = myRefOnAlgo->GetID();
		  CORBA::Object_var anObject = SObjectToObject( myRefOnAlgo );
		  int id = myStudyContext->findId( string( GetORB()->object_to_string( anObject ) ) );
		  //if ( myRefOnObject.length() > 0 ) {
		  //aSize[ 0 ] = myRefOnObject.length() + 1;
		  char algoName[ 30 ], algoId[ 30 ];
		  sprintf( algoName, "Algo %d", ++algoNb );
		  sprintf( algoId, "%d", id );
		  aSize[ 0 ] = strlen( algoId ) + 1;
		  aDataset = new HDFdataset( algoName, aGroup, HDF_STRING, aSize, 1 );
		  aDataset->CreateOnDisk();
		  //aDataset->WriteOnDisk( ( char* )( myRefOnObject.c_str() ) );
		  aDataset->WriteOnDisk( algoId );
		  aDataset->CloseOnDisk();
		  //}
		}
	      }
	      aGroup->CloseOnDisk();
	    }

	    // --> submesh objects sub-branches
	    for ( int i = GetSubMeshOnVertexTag(); i <= GetSubMeshOnCompoundTag(); i++ ) {
	      SALOMEDS::SObject_var mySubmeshBranch;
	      found = gotBranch->FindSubObject( i, mySubmeshBranch );
	      if ( found ) {
		char name_meshgroup[ 30 ];
		if ( i == GetSubMeshOnVertexTag() )
		  strcpy( name_meshgroup, "SubMeshes On Vertex" );
		else if ( i == GetSubMeshOnEdgeTag() )
		  strcpy( name_meshgroup, "SubMeshes On Edge" );
		else if ( i == GetSubMeshOnFaceTag() )
		  strcpy( name_meshgroup, "SubMeshes On Face" );
		else if ( i == GetSubMeshOnSolidTag() )
		  strcpy( name_meshgroup, "SubMeshes On Solid" );
		else if ( i == GetSubMeshOnCompoundTag() )
		  strcpy( name_meshgroup, "SubMeshes On Compound" );
		
		// for each type of submeshes create container HDF group
		aGroup = new HDFgroup( name_meshgroup, aTopGroup );
		aGroup->CreateOnDisk();
	    
		// iterator for all submeshes of given type
		SALOMEDS::ChildIterator_var itSM = myCurrentStudy->NewChildIterator( mySubmeshBranch );
		for ( ; itSM->More(); itSM->Next() ) {
		  SALOMEDS::SObject_var mySObject = itSM->Value();
		  CORBA::Object_var anSubObject = SObjectToObject( mySObject );
		  if ( !CORBA::is_nil( anSubObject ) ) {
		    SMESH::SMESH_subMesh_var mySubMesh = SMESH::SMESH_subMesh::_narrow( anSubObject ) ;
		    int subid = myStudyContext->findId( string( GetORB()->object_to_string( anSubObject ) ) );
		      
		    // for each mesh open the HDF group basing on its id
		    char submeshGrpName[ 30 ];
		    sprintf( submeshGrpName, "SubMesh %d", subid );
		    aSubGroup = new HDFgroup( submeshGrpName, aGroup );
		    aSubGroup->CreateOnDisk();

//		    // Put submesh data to MED convertor
//		    if ( myImpl->_mapSubMesh.find( mySubMesh->GetId() ) != myImpl->_mapSubMesh.end() ) {
//		      MESSAGE( "VSR - SMESH_Gen_i::Save(): saving submesh with ID = "
//                              << mySubMesh->GetId() << " to MED file" );
//		      ::SMESH_subMesh* aLocalSubmesh = myImpl->_mapSubMesh[mySubMesh->GetId()];
//		      myWriter.AddSubMesh( aLocalSubmesh->GetSubMeshDS(), subid );
//		    }
		    
		    // write reference on a shape if exists
		    SALOMEDS::SObject_var mySubRef;
		    found = mySObject->FindSubObject( GetRefOnShapeTag(), mySubRef );
		    if ( found ) {
		      SALOMEDS::SObject_var myShape;
		      bool ok = mySubRef->ReferencedObject( myShape );
		      if ( ok ) {
			string myRefOnObject = myShape->GetID();
			if ( myRefOnObject.length() > 0 ) {
			  aSize[ 0 ] = myRefOnObject.length() + 1;
			  aDataset = new HDFdataset( "Ref on shape", aSubGroup, HDF_STRING, aSize, 1 );
			  aDataset->CreateOnDisk();
			  aDataset->WriteOnDisk( ( char* )( myRefOnObject.c_str() ) );
			  aDataset->CloseOnDisk();
			}
		      }
		    }

		    // write applied hypotheses if exist
		    SALOMEDS::SObject_var mySubHypBranch;
		    found = mySObject->FindSubObject( GetRefOnAppliedHypothesisTag(), mySubHypBranch );
		    if ( found ) {
		      aSubSubGroup = new HDFgroup( "Applied Hypotheses", aSubGroup );
		      aSubSubGroup->CreateOnDisk();

		      SALOMEDS::ChildIterator_var it = myCurrentStudy->NewChildIterator( mySubHypBranch );
		      int hypNb = 0;
		      for ( ; it->More(); it->Next() ) {
			SALOMEDS::SObject_var mySubSObject = it->Value();
			SALOMEDS::SObject_var myRefOnHyp;
			bool ok = mySubSObject->ReferencedObject( myRefOnHyp );
			if ( ok ) {
			  //string myRefOnObject = myRefOnHyp->GetID();
			  CORBA::Object_var anObject = SObjectToObject( myRefOnHyp );
			  int id = myStudyContext->findId( string( GetORB()->object_to_string( anObject ) ) );
			  //if ( myRefOnObject.length() > 0 ) {
			  //aSize[ 0 ] = myRefOnObject.length() + 1;
			  char hypName[ 30 ], hypId[ 30 ];
			  sprintf( hypName, "Hyp %d", ++hypNb );
			  sprintf( hypId, "%d", id );
			  aSize[ 0 ] = strlen( hypId ) + 1;
			  aDataset = new HDFdataset( hypName, aSubSubGroup, HDF_STRING, aSize, 1 );
			  aDataset->CreateOnDisk();
			  //aDataset->WriteOnDisk( ( char* )( myRefOnObject.c_str() ) );
			  aDataset->WriteOnDisk( hypId );
			  aDataset->CloseOnDisk();
			  //}
			}
		      }
		      aSubSubGroup->CloseOnDisk();
		    }
		    
		    // write applied algorithms if exist
		    SALOMEDS::SObject_var mySubAlgoBranch;
		    found = mySObject->FindSubObject( GetRefOnAppliedAlgorithmsTag(), mySubAlgoBranch );
		    if ( found ) {
		      aSubSubGroup = new HDFgroup( "Applied Algorithms", aSubGroup );
		      aSubSubGroup->CreateOnDisk();

		      SALOMEDS::ChildIterator_var it = myCurrentStudy->NewChildIterator( mySubAlgoBranch );
		      int algoNb = 0;
		      for ( ; it->More(); it->Next() ) {
			SALOMEDS::SObject_var mySubSObject = it->Value();
			SALOMEDS::SObject_var myRefOnAlgo;
			bool ok = mySubSObject->ReferencedObject( myRefOnAlgo );
			if ( ok ) {
			  //string myRefOnObject = myRefOnAlgo->GetID();
			  CORBA::Object_var anObject = SObjectToObject( myRefOnAlgo );
			  int id = myStudyContext->findId( string( GetORB()->object_to_string( anObject ) ) );
			  //if ( myRefOnObject.length() > 0 ) {
			  //aSize[ 0 ] = myRefOnObject.length() + 1;
			  char algoName[ 30 ], algoId[ 30 ];
			  sprintf( algoName, "Algo %d", ++algoNb );
			  sprintf( algoId, "%d", id );
			  aSize[ 0 ] = strlen( algoId ) + 1;
			  aDataset = new HDFdataset( algoName, aSubSubGroup, HDF_STRING, aSize, 1 );
			  aDataset->CreateOnDisk();
			  //aDataset->WriteOnDisk( ( char* )( myRefOnObject.c_str() ) );
			  aDataset->WriteOnDisk( algoId );
			  aDataset->CloseOnDisk();
			  //}
			}
		      }
		      aSubSubGroup->CloseOnDisk();
		    }
		    // close submesh HDF group
		    aSubGroup->CloseOnDisk();
		  }
		}
		// close container of submeshes by type HDF group
		aGroup->CloseOnDisk();
	      }
	    }
            // All sub-meshes will be stored in MED file
            myWriter.AddAllSubMeshes();

	    // groups root sub-branch
	    SALOMEDS::SObject_var myGroupsBranch;
	    for ( int i = GetNodeGroupsTag(); i <= GetVolumeGroupsTag(); i++ ) {
	      found = gotBranch->FindSubObject( i, myGroupsBranch );
	      if ( found ) {
		char name_group[ 30 ];
		if ( i == GetNodeGroupsTag() )
		  strcpy( name_group, "Groups of Nodes" );
		else if ( i == GetEdgeGroupsTag() )
		  strcpy( name_group, "Groups of Edges" );
		else if ( i == GetFaceGroupsTag() )
		  strcpy( name_group, "Groups of Faces" );
		else if ( i == GetVolumeGroupsTag() )
		  strcpy( name_group, "Groups of Volumes" );

		aGroup = new HDFgroup( name_group, aTopGroup );
		aGroup->CreateOnDisk();

		SALOMEDS::ChildIterator_var it = myCurrentStudy->NewChildIterator( myGroupsBranch );
		int grpNb = 0;
		for ( ; it->More(); it->Next() ) {
		  SALOMEDS::SObject_var mySObject = it->Value();
		  CORBA::Object_var aSubObject = SObjectToObject( mySObject );
		  if ( !CORBA::is_nil( aSubObject ) ) {
		    SMESH_Group_i* myGroupImpl = dynamic_cast<SMESH_Group_i*>( GetServant( aSubObject ).in() );
		    if ( !myGroupImpl )
		      continue;

		    int anId = myStudyContext->findId( string( GetORB()->object_to_string( aSubObject ) ) );
		    
		    // For each group, create a dataset named "Group <group_persistent_id>"
                    // and store the group's user name into it
		    char grpName[ 30 ];
		    sprintf( grpName, "Group %d", anId );
		    char* aUserName = myGroupImpl->GetName();
		    aSize[ 0 ] = strlen( aUserName ) + 1;

		    aDataset = new HDFdataset( grpName, aGroup, HDF_STRING, aSize, 1 );
		    aDataset->CreateOnDisk();
		    aDataset->WriteOnDisk( aUserName );
		    aDataset->CloseOnDisk();

		    // Store the group contents into MED file
		    if ( myLocMesh.GetGroup( myGroupImpl->GetLocalID() ) ) {
		      MESSAGE( "VSR - SMESH_Gen_i::Save(): saving group with StoreName = "
                              << grpName << " to MED file" );
		      SMESHDS_Group* aGrpDS = myLocMesh.GetGroup( myGroupImpl->GetLocalID() )->GetGroupDS();
		      aGrpDS->SetStoreName( grpName );

		      // Pass SMESHDS_Group to MED writer 
		      myWriter.AddGroup( aGrpDS );
		    }
		  }
		}
		aGroup->CloseOnDisk();
	      }
	    }

	    // Flush current mesh information into MED file
	    if ( strcmp( strHasData.c_str(), "1" ) == 0 ) 	      
	      myWriter.Add();

	    // close mesh HDF group
	    aTopGroup->CloseOnDisk();
	  }
	}
      }
    }
  }

  // close HDF file
  aFile->CloseOnDisk();
  delete aFile;

  // Convert temporary files to stream
  aStreamFile = SALOMEDS_Tool::PutFilesToStream( tmpDir.ToCString(), aFileSeq.in(), isMultiFile );

  // Remove temporary files and directory
  if ( !isMultiFile ) 
    SALOMEDS_Tool::RemoveTemporaryFiles( tmpDir.ToCString(), aFileSeq.in(), true );

  INFOS( "SMESH_Gen_i::Save() completed" );
  return aStreamFile._retn();
}

//=============================================================================
/*!
 *  SMESH_Gen_i::SaveASCII
 *
 *  Save SMESH module's data in ASCII format (not implemented yet)
 */
//=============================================================================

SALOMEDS::TMPFile* SMESH_Gen_i::SaveASCII( SALOMEDS::SComponent_ptr theComponent,
					   const char*              theURL,
					   bool                     isMultiFile ) {
  MESSAGE( "SMESH_Gen_i::SaveASCII" );
  SALOMEDS::TMPFile_var aStreamFile = Save( theComponent, theURL, isMultiFile );
  return aStreamFile._retn();
}

//=============================================================================
/*!
 *  SMESH_Gen_i::loadGeomData
 *
 *  Load GEOM module data
 */
//=============================================================================

void SMESH_Gen_i::loadGeomData( SALOMEDS::SComponent_ptr theCompRoot )
{
  if ( theCompRoot->_is_nil() )
    return;

  SALOMEDS::Study_var aStudy = SALOMEDS::Study::_narrow( theCompRoot->GetStudy() );
  if ( aStudy->_is_nil() )
    return;

  SALOMEDS::StudyBuilder_var aStudyBuilder = aStudy->NewBuilder(); 
  aStudyBuilder->LoadWith( theCompRoot, GetGeomEngine() );
}

//=============================================================================
/*!
 *  SMESH_Gen_i::Load
 *
 *  Load SMESH module's data
 */
//=============================================================================

bool SMESH_Gen_i::Load( SALOMEDS::SComponent_ptr theComponent,
		        const SALOMEDS::TMPFile& theStream,
		        const char*              theURL,
		        bool                     isMultiFile )
{
  INFOS( "SMESH_Gen_i::Load" );

  ASSERT( theComponent->GetStudy()->StudyId() == myCurrentStudy->StudyId() )
  StudyContext* myStudyContext = GetCurrentStudyContext();
  
  // Get temporary files location
  TCollection_AsciiString tmpDir =
    isMultiFile ? TCollection_AsciiString( ( char* )theURL ) : SALOMEDS_Tool::GetTmpDir();

  // Convert the stream into sequence of files to process
  SALOMEDS::ListOfFileNames_var aFileSeq = SALOMEDS_Tool::PutStreamToFiles( theStream,
                                                                            tmpDir.ToCString(),
									    isMultiFile );
  TCollection_AsciiString aStudyName( "" );
  if ( isMultiFile ) 
    aStudyName = ( SALOMEDS_Tool::GetNameFromPath( myCurrentStudy->URL() ) );

  // Set names of temporary files
  TCollection_AsciiString filename = tmpDir + aStudyName + TCollection_AsciiString( "_SMESH.hdf" );
  TCollection_AsciiString meshfile = tmpDir + aStudyName + TCollection_AsciiString( "_SMESH_Mesh.med" );

  int size;
  HDFfile*    aFile;
  HDFdataset* aDataset;
  HDFgroup*   aTopGroup;
  HDFgroup*   aGroup;
  HDFgroup*   aSubGroup;
  HDFgroup*   aSubSubGroup;

  // Read data
  // ---> open HDF file
  aFile = new HDFfile( filename.ToCString() );
  try {
    aFile->OpenOnDisk( HDF_RDONLY );
  }
  catch ( HDFexception ) {
    MESSAGE( "Load(): " << filename << " not found!" );
    return false;
  }

  DriverMED_R_SMESHDS_Mesh myReader;
  myReader.SetFile( meshfile.ToCString() );

  // get total number of top-level groups
  int aNbGroups = aFile->nInternalObjects(); 
  if ( aNbGroups > 0 ) {
    // --> in first turn we should read&create hypotheses
    if ( aFile->ExistInternalObject( "Hypotheses" ) ) {
      // open hypotheses root HDF group
      aTopGroup = new HDFgroup( "Hypotheses", aFile ); 
      aTopGroup->OpenOnDisk();

      // get number of hypotheses
      int aNbObjects = aTopGroup->nInternalObjects(); 
      for ( int j = 0; j < aNbObjects; j++ ) {
	// try to identify hypothesis
	char hypGrpName[ HDF_NAME_MAX_LEN+1 ];
        aTopGroup->InternalObjectIndentify( j, hypGrpName );

	if ( string( hypGrpName ).substr( 0, 10 ) == string( "Hypothesis" ) ) {
	  // open hypothesis group
	  aGroup = new HDFgroup( hypGrpName, aTopGroup ); 
	  aGroup->OpenOnDisk();

	  // --> get hypothesis id
	  int    id = atoi( string( hypGrpName ).substr( 10 ).c_str() );
	  string hypname;
	  string libname;
	  string hypdata;

	  // get number of datasets
	  int aNbSubObjects = aGroup->nInternalObjects();
	  for ( int k = 0; k < aNbSubObjects; k++ ) {
	    // identify dataset
	    char name_of_subgroup[ HDF_NAME_MAX_LEN+1 ];
	    aGroup->InternalObjectIndentify( k, name_of_subgroup );
	    // --> get hypothesis name
	    if ( strcmp( name_of_subgroup, "Name"  ) == 0 ) {
	      aDataset = new HDFdataset( name_of_subgroup, aGroup );
	      aDataset->OpenOnDisk();
	      size = aDataset->GetSize();
	      char* hypname_str = new char[ size ];
	      aDataset->ReadFromDisk( hypname_str );
	      hypname = string( hypname_str );
	      delete hypname_str;
	      aDataset->CloseOnDisk();
	    }
	    // --> get hypothesis plugin library name
	    if ( strcmp( name_of_subgroup, "LibName"  ) == 0 ) {
	      aDataset = new HDFdataset( name_of_subgroup, aGroup );
	      aDataset->OpenOnDisk();
	      size = aDataset->GetSize();
	      char* libname_str = new char[ size ];
	      aDataset->ReadFromDisk( libname_str );
	      SCRUTE( libname_str );
	      libname = string( libname_str );
	      delete libname_str;
	      aDataset->CloseOnDisk();
	    }
	    // --> get hypothesis data
	    if ( strcmp( name_of_subgroup, "Data"  ) == 0 ) {
	      aDataset = new HDFdataset( name_of_subgroup, aGroup );
	      aDataset->OpenOnDisk();
	      size = aDataset->GetSize();
	      char* hypdata_str = new char[ size ];
	      aDataset->ReadFromDisk( hypdata_str );
	      hypdata = string( hypdata_str );
	      delete hypdata_str;
	      aDataset->CloseOnDisk();
	    }
	  }
	  // close hypothesis HDF group
	  aGroup->CloseOnDisk();

	  // --> restore hypothesis from data
	  if ( id > 0 && !hypname.empty()/* && !hypdata.empty()*/ ) { // VSR : persistent data can be empty
	    MESSAGE("VSR - load hypothesis : id = " << id <<
                    ", name = " << hypname.c_str() << ", persistent string = " << hypdata.c_str());
            SMESH::SMESH_Hypothesis_var myHyp;
	    
	    try { // protect persistence mechanism against exceptions
	      myHyp = this->createHypothesis( hypname.c_str(), libname.c_str() );
	    }
	    catch (...) {
	      MESSAGE( "Exception during hypothesis creation" );
	    }

	    SMESH_Hypothesis_i* myImpl = dynamic_cast<SMESH_Hypothesis_i*>( GetServant( myHyp ).in() );
	    if ( myImpl ) {
	      myImpl->LoadFrom( hypdata.c_str() );
	      string iorString = GetORB()->object_to_string( myHyp );
	      int newId = myStudyContext->findId( iorString );
	      myStudyContext->mapOldToNew( id, newId );
	    }
	    else
	      MESSAGE( "VSR - SMESH_Gen::Load - can't get servant" );
          }
        }
      }
      // close hypotheses root HDF group
      aTopGroup->CloseOnDisk();
    }

    // --> then we should read&create algorithms
    if ( aFile->ExistInternalObject( "Algorithms" ) ) {
      // open algorithms root HDF group
      aTopGroup = new HDFgroup( "Algorithms", aFile ); 
      aTopGroup->OpenOnDisk();

      // get number of algorithms
      int aNbObjects = aTopGroup->nInternalObjects(); 
      for ( int j = 0; j < aNbObjects; j++ ) {
	// try to identify algorithm
	char hypGrpName[ HDF_NAME_MAX_LEN+1 ];
        aTopGroup->InternalObjectIndentify( j, hypGrpName );

	if ( string( hypGrpName ).substr( 0, 9 ) == string( "Algorithm" ) ) {
	  // open algorithm group
	  aGroup = new HDFgroup( hypGrpName, aTopGroup ); 
	  aGroup->OpenOnDisk();

	  // --> get algorithm id
	  int    id = atoi( string( hypGrpName ).substr( 9 ).c_str() );
	  string hypname;
	  string libname;
	  string hypdata;

	  // get number of datasets
	  int aNbSubObjects = aGroup->nInternalObjects();
	  for ( int k = 0; k < aNbSubObjects; k++ ) {
	    // identify dataset
	    char name_of_subgroup[ HDF_NAME_MAX_LEN+1 ];
	    aGroup->InternalObjectIndentify( k, name_of_subgroup );
	    // --> get algorithm name
	    if ( strcmp( name_of_subgroup, "Name"  ) == 0 ) {
	      aDataset = new HDFdataset( name_of_subgroup, aGroup );
	      aDataset->OpenOnDisk();
	      size = aDataset->GetSize();
	      char* hypname_str = new char[ size ];
	      aDataset->ReadFromDisk( hypname_str );
	      hypname = string( hypname_str );
	      delete hypname_str;
	      aDataset->CloseOnDisk();
	    }
	    // --> get algorithm plugin library name
	    if ( strcmp( name_of_subgroup, "LibName"  ) == 0 ) {
	      aDataset = new HDFdataset( name_of_subgroup, aGroup );
	      aDataset->OpenOnDisk();
	      size = aDataset->GetSize();
	      char* libname_str = new char[ size ];
	      aDataset->ReadFromDisk( libname_str );
	      SCRUTE( libname_str );
	      libname = string( libname_str );
	      delete libname_str;
	      aDataset->CloseOnDisk();
	    }
	    // --> get algorithm data
	    if ( strcmp( name_of_subgroup, "Data"  ) == 0 ) {
	      aDataset = new HDFdataset( name_of_subgroup, aGroup );
	      aDataset->OpenOnDisk();
	      size = aDataset->GetSize();
	      char* hypdata_str = new char[ size ];
	      aDataset->ReadFromDisk( hypdata_str );
	      SCRUTE( hypdata_str );
	      hypdata = string( hypdata_str );
	      delete hypdata_str;
	      aDataset->CloseOnDisk();
	    }
	  }
	  // close algorithm HDF group
	  aGroup->CloseOnDisk();
	  
	  // --> restore algorithm from data
	  if ( id > 0 && !hypname.empty()/* && !hypdata.empty()*/ ) { // VSR : persistent data can be empty
	    MESSAGE("VSR - load algo : id = " << id <<
                    ", name = " << hypname.c_str() << ", persistent string = " << hypdata.c_str());
            SMESH::SMESH_Hypothesis_var myHyp;
	    	    
	    try { // protect persistence mechanism against exceptions
	      myHyp = this->createHypothesis( hypname.c_str(), libname.c_str() );
	    }
	    catch (...) {
	      MESSAGE( "Exception during hypothesis creation" );
	    }
	    
	    SMESH_Hypothesis_i* myImpl = dynamic_cast<SMESH_Hypothesis_i*>( GetServant( myHyp ).in() );
	    if ( myImpl ) {
	      myImpl->LoadFrom( hypdata.c_str() );
	      string iorString = GetORB()->object_to_string( myHyp );
	      int newId = myStudyContext->findId( iorString );
	      myStudyContext->mapOldToNew( id, newId );
	    }
	    else
	      MESSAGE( "VSR - SMESH_Gen::Load - can't get servant" );
          }
        }
      }
      // close algorithms root HDF group
      aTopGroup->CloseOnDisk();
    }

    // --> the rest groups should be meshes
    for ( int i = 0; i < aNbGroups; i++ ) {
      // identify next group
      char meshName[ HDF_NAME_MAX_LEN+1 ];
      aFile->InternalObjectIndentify( i, meshName );

      if ( string( meshName ).substr( 0, 4 ) == string( "Mesh" ) ) {
	// --> get mesh id
	int id = atoi( string( meshName ).substr( 4 ).c_str() );
	if ( id <= 0 )
	  continue;

	bool hasData = false;

	// open mesh HDF group
	aTopGroup = new HDFgroup( meshName, aFile ); 
	aTopGroup->OpenOnDisk();

	// get number of child HDF objects
	int aNbObjects = aTopGroup->nInternalObjects(); 
	if ( aNbObjects > 0 ) {
	  // create mesh
	  MESSAGE( "VSR - load mesh : id = " << id );
	  SMESH::SMESH_Mesh_var myNewMesh = this->createMesh();
	  SMESH_Mesh_i* myNewMeshImpl = dynamic_cast<SMESH_Mesh_i*>( GetServant( myNewMesh ).in() );
	  if ( !myNewMeshImpl )
	    continue;
	  string iorString = GetORB()->object_to_string( myNewMesh );
	  int newId = myStudyContext->findId( iorString );
	  myStudyContext->mapOldToNew( id, newId );
	  
	  ::SMESH_Mesh& myLocMesh = myNewMeshImpl->GetImpl();
	  SMESHDS_Mesh* mySMESHDSMesh = myLocMesh.GetMeshDS();

	  // try to find mesh data dataset
	  if ( aTopGroup->ExistInternalObject( "Has data" ) ) {
	    // load mesh "has data" flag
	    aDataset = new HDFdataset( "Has data", aTopGroup );
	    aDataset->OpenOnDisk();
	    size = aDataset->GetSize();
	    char* strHasData = new char[ size ];
	    aDataset->ReadFromDisk( strHasData );
	    aDataset->CloseOnDisk();
	    if ( strcmp( strHasData, "1") == 0 ) {
	      // read mesh data from MED file
	      myReader.SetMesh( mySMESHDSMesh );
	      myReader.SetMeshId( id );
	      myReader.ReadMySelf();
	      hasData = true;
	    }
	  }

	  // try to read and set reference to shape
	  GEOM::GEOM_Shape_var aShape;
	  if ( aTopGroup->ExistInternalObject( "Ref on shape" ) ) {
	    // load mesh "Ref on shape" - it's an entry to SObject
	    aDataset = new HDFdataset( "Ref on shape", aTopGroup );
	    aDataset->OpenOnDisk();
	    size = aDataset->GetSize();
	    char* refFromFile = new char[ size ];
	    aDataset->ReadFromDisk( refFromFile );
	    aDataset->CloseOnDisk();
	    if ( strlen( refFromFile ) > 0 ) {
	      SALOMEDS::SObject_var shapeSO = myCurrentStudy->FindObjectID( refFromFile );

	      // Make sure GEOM data are loaded first
	      loadGeomData( shapeSO->GetFatherComponent() );

	      CORBA::Object_var shapeObject = SObjectToObject( shapeSO );
	      if ( !CORBA::is_nil( shapeObject ) ) {
		aShape = GEOM::GEOM_Shape::_narrow( shapeObject );
		if ( !aShape->_is_nil() )
		  myNewMeshImpl->setShape( aShape );
	      }
	    }
	  }

	  // try to get applied hypotheses
	  if ( aTopGroup->ExistInternalObject( "Applied Hypotheses" ) ) {
	    aGroup = new HDFgroup( "Applied Hypotheses", aTopGroup );
	    aGroup->OpenOnDisk();
	    // get number of applied hypotheses
	    int aNbSubObjects = aGroup->nInternalObjects(); 
	    MESSAGE( "VSR - number of applied hypotheses = " << aNbSubObjects );
	    for ( int j = 0; j < aNbSubObjects; j++ ) {
	      char name_dataset[ HDF_NAME_MAX_LEN+1 ];
	      aGroup->InternalObjectIndentify( j, name_dataset );
	      // check if it is a hypothesis
	      if ( string( name_dataset ).substr( 0, 3 ) == string( "Hyp" ) ) {
		aDataset = new HDFdataset( name_dataset, aGroup );
		aDataset->OpenOnDisk();
		size = aDataset->GetSize();
		char* refFromFile = new char[ size ];
		aDataset->ReadFromDisk( refFromFile );
		aDataset->CloseOnDisk();

		// san - it is impossible to recover applied hypotheses using their entries within Load() method
		
		//SALOMEDS::SObject_var hypSO = myCurrentStudy->FindObjectID( refFromFile );
		//CORBA::Object_var hypObject = SObjectToObject( hypSO );
		int id = atoi( refFromFile );
		string anIOR = myStudyContext->getIORbyOldId( id );
		if ( !anIOR.empty() ) {
		  CORBA::Object_var hypObject = GetORB()->string_to_object( anIOR.c_str() );
		  if ( !CORBA::is_nil( hypObject ) ) {
		    SMESH::SMESH_Hypothesis_var anHyp = SMESH::SMESH_Hypothesis::_narrow( hypObject );
		    if ( !anHyp->_is_nil() && !aShape->_is_nil() )
		      myNewMeshImpl->addHypothesis( aShape, anHyp );
		  }
		}
	      }
	    }
	    aGroup->CloseOnDisk();
	  }
	
	  // try to get applied algorithms
	  if ( aTopGroup->ExistInternalObject( "Applied Algorithms" ) ) {
	    aGroup = new HDFgroup( "Applied Algorithms", aTopGroup );
	    aGroup->OpenOnDisk();
	    // get number of applied algorithms
	    int aNbSubObjects = aGroup->nInternalObjects(); 
	    MESSAGE( "VSR - number of applied algos " << aNbSubObjects );
	    for ( int j = 0; j < aNbSubObjects; j++ ) {
	      char name_dataset[ HDF_NAME_MAX_LEN+1 ];
	      aGroup->InternalObjectIndentify( j, name_dataset );
	      // check if it is an algorithm
	      if ( string( name_dataset ).substr( 0, 4 ) == string( "Algo" ) ) {
		aDataset = new HDFdataset( name_dataset, aGroup );
		aDataset->OpenOnDisk();
		size = aDataset->GetSize();
		char* refFromFile = new char[ size ];
		aDataset->ReadFromDisk( refFromFile );
		aDataset->CloseOnDisk();

		// san - it is impossible to recover applied algorithms using their entries within Load() method
		
		//SALOMEDS::SObject_var hypSO = myCurrentStudy->FindObjectID( refFromFile );
		//CORBA::Object_var hypObject = SObjectToObject( hypSO );
		int id = atoi( refFromFile );
		string anIOR = myStudyContext->getIORbyOldId( id );
		if ( !anIOR.empty() ) {
		  CORBA::Object_var hypObject = GetORB()->string_to_object( anIOR.c_str() );
		  if ( !CORBA::is_nil( hypObject ) ) {
		    SMESH::SMESH_Hypothesis_var anHyp = SMESH::SMESH_Hypothesis::_narrow( hypObject );
		    if ( !anHyp->_is_nil() && !aShape->_is_nil() )
		      myNewMeshImpl->addHypothesis( aShape, anHyp );
		  }
		}
	      }
	    }
	    aGroup->CloseOnDisk();
	  }

	  // --> try to find submeshes containers for each type of submesh
	  for ( int j = GetSubMeshOnVertexTag(); j <= GetSubMeshOnCompoundTag(); j++ ) {
	    char name_meshgroup[ 30 ];
	    if ( j == GetSubMeshOnVertexTag() )
	      strcpy( name_meshgroup, "SubMeshes On Vertex" );
	    else if ( j == GetSubMeshOnEdgeTag() )
	      strcpy( name_meshgroup, "SubMeshes On Edge" );
	    else if ( j == GetSubMeshOnFaceTag() )
	      strcpy( name_meshgroup, "SubMeshes On Face" );
	    else if ( j == GetSubMeshOnSolidTag() )
	      strcpy( name_meshgroup, "SubMeshes On Solid" );
	    else if ( j == GetSubMeshOnCompoundTag() )
	      strcpy( name_meshgroup, "SubMeshes On Compound" );
	    
	    // try to get submeshes container HDF group
	    if ( aTopGroup->ExistInternalObject( name_meshgroup ) ) {
	      // open submeshes containers HDF group
	      aGroup = new HDFgroup( name_meshgroup, aTopGroup );
	      aGroup->OpenOnDisk();
	      
	      // get number of submeshes
	      int aNbSubMeshes = aGroup->nInternalObjects(); 
	      for ( int k = 0; k < aNbSubMeshes; k++ ) {
		// identify submesh
		char name_submeshgroup[ HDF_NAME_MAX_LEN+1 ];
		aGroup->InternalObjectIndentify( k, name_submeshgroup );
		if ( string( name_submeshgroup ).substr( 0, 7 ) == string( "SubMesh" )  ) {
		  // --> get submesh id
		  int subid = atoi( string( name_submeshgroup ).substr( 7 ).c_str() );
		  if ( subid <= 0 )
		    continue;
		  // open submesh HDF group
		  aSubGroup = new HDFgroup( name_submeshgroup, aGroup );
		  aSubGroup->OpenOnDisk();
		  
		  // try to read and set reference to subshape
		  GEOM::GEOM_Shape_var     aSubShape;
		  SMESH::SMESH_subMesh_var aSubMesh;
		    
		  if ( aSubGroup->ExistInternalObject( "Ref on shape" ) ) {
		    // load submesh "Ref on shape" - it's an entry to SObject
		    aDataset = new HDFdataset( "Ref on shape", aSubGroup );
		    aDataset->OpenOnDisk();
		    size = aDataset->GetSize();
		    char* refFromFile = new char[ size ];
		    aDataset->ReadFromDisk( refFromFile );
		    aDataset->CloseOnDisk();
		    if ( strlen( refFromFile ) > 0 ) {
		      SALOMEDS::SObject_var subShapeSO = myCurrentStudy->FindObjectID( refFromFile );
		      CORBA::Object_var subShapeObject = SObjectToObject( subShapeSO );
		      if ( !CORBA::is_nil( subShapeObject ) ) {
			aSubShape = GEOM::GEOM_Shape::_narrow( subShapeObject );
			if ( !aSubShape->_is_nil() )
			  aSubMesh = SMESH::SMESH_subMesh::_duplicate
                            ( myNewMeshImpl->createSubMesh( aSubShape ) );
			if ( aSubMesh->_is_nil() )
			  continue;
			string iorSubString = GetORB()->object_to_string( aSubMesh );
			int newSubId = myStudyContext->findId( iorSubString );
			myStudyContext->mapOldToNew( subid, newSubId );
		      }
		    }
		  }
		  
		  if ( aSubMesh->_is_nil() )
		    continue;

		  // VSR: Get submesh data from MED convertor
//		  int anInternalSubmeshId = aSubMesh->GetId(); // this is not a persistent ID, it's an internal one computed from sub-shape
//		  if (myNewMeshImpl->_mapSubMesh.find(anInternalSubmeshId) != myNewMeshImpl->_mapSubMesh.end()) {
//		    MESSAGE("VSR - SMESH_Gen_i::Load(): loading from MED file submesh with ID = " <<
//                            subid << " for subshape # " << anInternalSubmeshId);
//		    SMESHDS_SubMesh* aSubMeshDS =
//                      myNewMeshImpl->_mapSubMesh[anInternalSubmeshId]->CreateSubMeshDS();
//		    if ( !aSubMeshDS ) {
//		      MESSAGE("VSR - SMESH_Gen_i::Load(): FAILED to create a submesh for subshape # " <<
//                              anInternalSubmeshId << " in current mesh!");
//		    }
//		    else
//		      myReader.GetSubMesh( aSubMeshDS, subid );
//		  }
		    
		  // try to get applied hypotheses
		  if ( aSubGroup->ExistInternalObject( "Applied Hypotheses" ) ) {
		    // open "applied hypotheses" HDF group
		    aSubSubGroup = new HDFgroup( "Applied Hypotheses", aSubGroup );
		    aSubSubGroup->OpenOnDisk();
		    // get number of applied hypotheses
		    int aNbSubObjects = aSubSubGroup->nInternalObjects(); 
		    for ( int l = 0; l < aNbSubObjects; l++ ) {
		      char name_dataset[ HDF_NAME_MAX_LEN+1 ];
		      aSubSubGroup->InternalObjectIndentify( l, name_dataset );
		      // check if it is a hypothesis
		      if ( string( name_dataset ).substr( 0, 3 ) == string( "Hyp" ) ) {
			aDataset = new HDFdataset( name_dataset, aSubSubGroup );
			aDataset->OpenOnDisk();
			size = aDataset->GetSize();
			char* refFromFile = new char[ size ];
			aDataset->ReadFromDisk( refFromFile );
			aDataset->CloseOnDisk();
			
			//SALOMEDS::SObject_var hypSO = myCurrentStudy->FindObjectID( refFromFile );
			//CORBA::Object_var hypObject = SObjectToObject( hypSO );
			int id = atoi( refFromFile );
			string anIOR = myStudyContext->getIORbyOldId( id );
			if ( !anIOR.empty() ) {
			  CORBA::Object_var hypObject = GetORB()->string_to_object( anIOR.c_str() );
			  if ( !CORBA::is_nil( hypObject ) ) {
			    SMESH::SMESH_Hypothesis_var anHyp = SMESH::SMESH_Hypothesis::_narrow( hypObject );
			    if ( !anHyp->_is_nil() && !aShape->_is_nil() )
			      myNewMeshImpl->addHypothesis( aSubShape, anHyp );
			  }
			}
		      }
		    }
		    // close "applied hypotheses" HDF group
		    aSubSubGroup->CloseOnDisk();
		  }
	
		  // try to get applied algorithms
		  if ( aSubGroup->ExistInternalObject( "Applied Algorithms" ) ) {
		    // open "applied algorithms" HDF group
		    aSubSubGroup = new HDFgroup( "Applied Algorithms", aSubGroup );
		    aSubSubGroup->OpenOnDisk();
		    // get number of applied algorithms
		    int aNbSubObjects = aSubSubGroup->nInternalObjects(); 
		    for ( int l = 0; l < aNbSubObjects; l++ ) {
		      char name_dataset[ HDF_NAME_MAX_LEN+1 ];
		      aSubSubGroup->InternalObjectIndentify( l, name_dataset );
		      // check if it is an algorithm
		      if ( string( name_dataset ).substr( 0, 4 ) == string( "Algo" ) ) {
			aDataset = new HDFdataset( name_dataset, aSubSubGroup );
			aDataset->OpenOnDisk();
			size = aDataset->GetSize();
			char* refFromFile = new char[ size ];
			aDataset->ReadFromDisk( refFromFile );
			aDataset->CloseOnDisk();

			//SALOMEDS::SObject_var hypSO = myCurrentStudy->FindObjectID( refFromFile );
			//CORBA::Object_var hypObject = SObjectToObject( hypSO );
			int id = atoi( refFromFile );
			string anIOR = myStudyContext->getIORbyOldId( id );
			if ( !anIOR.empty() ) {
			  CORBA::Object_var hypObject = GetORB()->string_to_object( anIOR.c_str() );
			  if ( !CORBA::is_nil( hypObject ) ) {
			    SMESH::SMESH_Hypothesis_var anHyp = SMESH::SMESH_Hypothesis::_narrow( hypObject );
			    if ( !anHyp->_is_nil() && !aShape->_is_nil() )
			      myNewMeshImpl->addHypothesis( aSubShape, anHyp );
			  }
			}
		      }
		    }
		    // close "applied algorithms" HDF group
		    aSubSubGroup->CloseOnDisk();
		  }
		  
		  // close submesh HDF group
		  aSubGroup->CloseOnDisk();
		}
	      }
	      // close submeshes containers HDF group
	      aGroup->CloseOnDisk();
	    }
	  }

	  if(hasData){
	    // Read sub-meshes from MED
	    MESSAGE("JFA - Create all sub-meshes");
	    myReader.CreateAllSubMeshes();
	  }

          // Recompute State (as computed sub-meshes are restored from MED)
	  if ( !aShape->_is_nil() ) {
	    MESSAGE("JFA - Compute State Engine ...");
//          map<int, ::SMESH_subMesh*>::iterator anAllSubMeshes = myNewMeshImpl->_mapSubMesh.begin();
//          for (; anAllSubMeshes != myNewMeshImpl->_mapSubMesh.end(); anAllSubMeshes++)
//          {
//            (*anAllSubMeshes).second->GetSubMeshDS(); // init SMESH_subMesh::_meshDS
//            (*anAllSubMeshes).second->ComputeStateEngine(SMESH_subMesh::SUBMESH_RESTORED);
//          }
	    TopoDS_Shape myLocShape = GetShapeReader()->GetShape( GetGeomEngine(), aShape );
	    myNewMeshImpl->GetImpl().GetSubMesh(myLocShape)->ComputeStateEngine(SMESH_subMesh::SUBMESH_RESTORED);
	    MESSAGE("JFA - Compute State Engine finished");
	  }

	  // try to get groups
	  for ( int ii = GetNodeGroupsTag(); ii <= GetVolumeGroupsTag(); ii++ ) {
	    char name_group[ 30 ];
	    if ( ii == GetNodeGroupsTag() )
	      strcpy( name_group, "Groups of Nodes" );
	    else if ( ii == GetEdgeGroupsTag() )
	      strcpy( name_group, "Groups of Edges" );
	    else if ( ii == GetFaceGroupsTag() )
	      strcpy( name_group, "Groups of Faces" );
	    else if ( ii == GetVolumeGroupsTag() )
	      strcpy( name_group, "Groups of Volumes" );

	    if ( aTopGroup->ExistInternalObject( name_group ) ) {
	      aGroup = new HDFgroup( name_group, aTopGroup );
	      aGroup->OpenOnDisk();
	      // get number of groups
	      int aNbSubObjects = aGroup->nInternalObjects(); 
	      for ( int j = 0; j < aNbSubObjects; j++ ) {
		char name_dataset[ HDF_NAME_MAX_LEN+1 ];
		aGroup->InternalObjectIndentify( j, name_dataset );
		// check if it is an group
		if ( string( name_dataset ).substr( 0, 5 ) == string( "Group" ) ) {
		  // --> get group id
		  int subid = atoi( string( name_dataset ).substr( 5 ).c_str() );
		  if ( subid <= 0 )
		    continue;
		  aDataset = new HDFdataset( name_dataset, aGroup );
		  aDataset->OpenOnDisk();

		  // Retrieve actual group name
		  size = aDataset->GetSize();
		  char* nameFromFile = new char[ size ];
		  aDataset->ReadFromDisk( nameFromFile );
		  aDataset->CloseOnDisk();
		  
		  // Create group servant
		  SMESH::SMESH_Group_var aNewGroup = SMESH::SMESH_Group::_duplicate
                    ( myNewMeshImpl->createGroup( (SMESH::ElementType)(ii - GetNodeGroupsTag() + 1),
                                                 nameFromFile ) );
		  // Obtain a SMESHDS_Group object 
		  if ( aNewGroup->_is_nil() )
		    continue;

		  string iorSubString = GetORB()->object_to_string( aNewGroup );
		  int newSubId = myStudyContext->findId( iorSubString );
		  myStudyContext->mapOldToNew( subid, newSubId );

		  SMESH_Group_i* aGroupImpl = dynamic_cast<SMESH_Group_i*>( GetServant( aNewGroup ).in() );
		  if ( !aGroupImpl )
		    continue;

		  SMESH_Group* aLocalGroup  = myLocMesh.GetGroup( aGroupImpl->GetLocalID() );
		  if ( !aLocalGroup )
		    continue;

		  SMESHDS_Group* aGroupDS = aLocalGroup->GetGroupDS();
		  aGroupDS->SetStoreName( name_dataset );

		  // Fill group with contents from MED file
		  myReader.GetGroup( aGroupDS );
		}
	      }
	      aGroup->CloseOnDisk();
	    }
	  }
	}
	// close mesh group
	aTopGroup->CloseOnDisk();	
      }
    }
  }
  // close HDF file
  aFile->CloseOnDisk();
  delete aFile;

  // Remove temporary files created from the stream
  if ( !isMultiFile ) 
    SALOMEDS_Tool::RemoveTemporaryFiles( tmpDir.ToCString(), aFileSeq.in(), true );

  INFOS( "SMESH_Gen_i::Load completed" );
  return true;
}

//=============================================================================
/*!
 *  SMESH_Gen_i::LoadASCII
 *
 *  Load SMESH module's data in ASCII format (not implemented yet)
 */
//=============================================================================

bool SMESH_Gen_i::LoadASCII( SALOMEDS::SComponent_ptr theComponent,
			     const SALOMEDS::TMPFile& theStream,
			     const char*              theURL,
			     bool                     isMultiFile ) {
  MESSAGE( "SMESH_Gen_i::LoadASCII" );
  return Load( theComponent, theStream, theURL, isMultiFile );
}

//=============================================================================
/*!
 *  SMESH_Gen_i::Close
 *
 *  Clears study-connected data when it is closed
 */
//=============================================================================

void SMESH_Gen_i::Close( SALOMEDS::SComponent_ptr theComponent )
{
  MESSAGE( "SMESH_Gen_i::Close" );

  // Clear study contexts data
  int studyId = myCurrentStudy->StudyId();
  if ( myStudyContextMap.find( studyId ) != myStudyContextMap.end() ) {
    delete myStudyContextMap[ studyId ];
    myStudyContextMap.erase( studyId );
  }
  return;
}

//=============================================================================
/*!
 *  SMESH_Gen_i::ComponentDataType
 * 
 *  Get component data type
 */
//=============================================================================

char* SMESH_Gen_i::ComponentDataType()
{
  MESSAGE( "SMESH_Gen_i::ComponentDataType" );
  return strdup( "SMESH" );
}

    
//=============================================================================
/*!
 *  SMESH_Gen_i::IORToLocalPersistentID
 *  
 *  Transform data from transient form to persistent
 */
//=============================================================================

char* SMESH_Gen_i::IORToLocalPersistentID( SALOMEDS::SObject_ptr theSObject,
					   const char*           IORString,
					   CORBA::Boolean        isMultiFile,
					   CORBA::Boolean        isASCII )
{
  MESSAGE( "SMESH_Gen_i::IORToLocalPersistentID" );
  StudyContext* myStudyContext = GetCurrentStudyContext();
  
  if ( strcmp( IORString, "" ) != 0 ) {
    int anId = myStudyContext->findId( IORString );
    if ( anId ) {
      MESSAGE( "VSR " << anId )
      char strId[ 20 ];
      sprintf( strId, "%d", anId );
      return  CORBA::string_dup( strId );
    }
  }
  return strdup( "" );
}

//=============================================================================
/*!
 *  SMESH_Gen_i::LocalPersistentIDToIOR
 *
 *  Transform data from persistent form to transient
 */
//=============================================================================

char* SMESH_Gen_i::LocalPersistentIDToIOR( SALOMEDS::SObject_ptr theSObject,
					   const char*           aLocalPersistentID,
					   CORBA::Boolean        isMultiFile,
					   CORBA::Boolean        isASCII )
{
  MESSAGE( "SMESH_Gen_i::LocalPersistentIDToIOR(): id = " << aLocalPersistentID );
  StudyContext* myStudyContext = GetCurrentStudyContext();

  if ( strcmp( aLocalPersistentID, "" ) != 0 ) {
    int anId = atoi( aLocalPersistentID );
    return CORBA::string_dup( myStudyContext->getIORbyOldId( anId ).c_str() );
  }
  return strdup( "" );
}

//=============================================================================
/*!
 *  SMESH_Gen_i::CanPublishInStudy
 *
 *  Returns true if object can be published in the study
 */
//=============================================================================

bool SMESH_Gen_i::CanPublishInStudy(CORBA::Object_ptr theIOR)
{
  SMESH::SMESH_Mesh_var aMesh       = SMESH::SMESH_Mesh::_narrow(theIOR);
  if( !aMesh->_is_nil() )
    return true;

  SMESH::SMESH_subMesh_var aSubMesh = SMESH::SMESH_subMesh::_narrow(theIOR);
  if( !aSubMesh->_is_nil() )
    return true;

  SMESH::SMESH_Hypothesis_var aHyp  = SMESH::SMESH_Hypothesis::_narrow(theIOR);
  if( !aHyp->_is_nil() )
    return true;

  SMESH::SMESH_Group_var aGroup     = SMESH::SMESH_Group::_narrow(theIOR);
  if( !aGroup->_is_nil() )
    return true;

  return false;
}

//=============================================================================
/*!
 *  SMESH_Gen_i::PublishInStudy
 *
 *  Publish object in the study
 */
//=============================================================================

SALOMEDS::SObject_ptr SMESH_Gen_i::PublishInStudy(SALOMEDS::Study_ptr theStudy,
						  SALOMEDS::SObject_ptr theSObject,
						  CORBA::Object_ptr theIOR,
						  const char* theName) 
throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  MESSAGE( "********** SMESH_Gen_i::PublishInStudy()" );
  SALOMEDS::SObject_var aSO;

  // san - first try to find SObject corresponding to SMESH component in theStudy
  // It is dangerous to use FindComponent("MESH") for this, as some other component
  // of type "MESH" might be present in theStudy.
  // So component's user name obtained from ModuleCatalog is passed to FindObject()...
  SALOME_ModuleCatalog::ModuleCatalog_var aCat =
    SALOME_ModuleCatalog::ModuleCatalog::_narrow( GetNS()->Resolve("/Kernel/ModulCatalog") );
  if ( CORBA::is_nil( aCat ) )
    return aSO._retn();

  SALOME_ModuleCatalog::Acomponent_var   aComp = aCat->GetComponent( "SMESH" );
  if ( CORBA::is_nil( aComp ) )
    return aSO._retn();

  SALOMEDS::SComponent_var father =
    SALOMEDS::SComponent::_narrow( theStudy->FindObject( strdup( aComp->componentusername() ) ) );
  SALOMEDS::StudyBuilder_var aStudyBuilder = theStudy->NewBuilder(); 

  SALOMEDS::GenericAttribute_var anAttr;
  SALOMEDS::AttributeName_var    aName;
  SALOMEDS::AttributePixMap_var  aPixmap;
  
  if ( father->_is_nil() ) {
    father  = aStudyBuilder->NewComponent( "MESH" );
    anAttr  = aStudyBuilder->FindOrCreateAttribute( father, "AttributeName" );
    aName   = SALOMEDS::AttributeName::_narrow( anAttr );
    aName   ->SetValue( strdup( aComp->componentusername() ) );
    anAttr  = aStudyBuilder->FindOrCreateAttribute( father, "AttributePixMap" );
    aPixmap = SALOMEDS::AttributePixMap::_narrow( anAttr );
    aPixmap ->SetPixMap( "ICON_OBJBROWSER_SMESH" );
    aStudyBuilder->DefineComponentInstance( father, SMESH_Gen::_this() );
  }

  if ( father->_is_nil() )  
    return aSO._retn();

  SALOMEDS::AttributeIOR_var        anIOR;
  SALOMEDS::AttributeSelectable_var aSelAttr;
  TCollection_AsciiString anObjName("obj");

  // Publishing a mesh
  SMESH::SMESH_Mesh_var aMesh = SMESH::SMESH_Mesh::_narrow( theIOR );
  if( !aMesh->_is_nil() ) {
    // Find correct free tag
    long aTag = FindMaxChildTag( father.in() );
    if ( aTag <= GetAlgorithmsRootTag() )
      aTag = GetAlgorithmsRootTag() + 1;
    else
      aTag++;
    // Add New Mesh
    SALOMEDS::SObject_var newMesh = aStudyBuilder->NewObjectToTag( father, aTag );
    anAttr  = aStudyBuilder->FindOrCreateAttribute( newMesh, "AttributePixMap" );
    aPixmap = SALOMEDS::AttributePixMap::_narrow( anAttr );
    aPixmap ->SetPixMap( "ICON_SMESH_TREE_MESH" );
    anAttr  = aStudyBuilder->FindOrCreateAttribute( newMesh, "AttributeIOR" );
    anIOR   = SALOMEDS::AttributeIOR::_narrow(anAttr);
    anIOR   ->SetValue( GetORB()->object_to_string( aMesh ) );
    aSO     = SALOMEDS::SObject::_narrow( newMesh );
    anObjName = TCollection_AsciiString( "Mesh" );
  }

  // Publishing a sub-mesh
  SMESH::SMESH_subMesh_var aSubMesh = SMESH::SMESH_subMesh::_narrow( theIOR );
  if( aSO->_is_nil() && !aSubMesh->_is_nil() ) {
    // try to obtain a parent mesh's SObject
    MESSAGE( "********** SMESH_Gen_i::PublishInStudy(): publishing submesh..." );
    SALOMEDS::SObject_var aParentSO;
    SMESH::SMESH_Mesh_var aParentMesh;
    SMESH_subMesh_i* aServant = dynamic_cast<SMESH_subMesh_i*>( GetServant( aSubMesh ).in() );
    if ( aServant != NULL ) {
      aParentMesh = aServant->_mesh_i->_this();
      if ( !aParentMesh->_is_nil() ) {
	aParentSO = theStudy->FindObjectIOR( GetORB()->object_to_string( aParentMesh ) );
      }
    }

    // Find submesh sub-tree tag
    if ( !aParentSO->_is_nil() ) {
      long aRootTag = GetSubMeshOnVertexTag();
      char* aRootName = "";

      SMESH_Mesh_i* aMeshServant = aServant->_mesh_i;
      if ( aMeshServant->_mapSubMesh.find( aServant->GetId() ) != aMeshServant->_mapSubMesh.end() ) {
	MESSAGE( "********** SMESH_Gen_i::PublishInStudy(): local submesh found" )
	SMESH_subMesh* aLocalSubMesh = aMeshServant->_mapSubMesh[aServant->GetId()];
	switch ( aLocalSubMesh->GetSubShape().ShapeType() ) {
	case TopAbs_VERTEX:
	  aRootTag  = GetSubMeshOnVertexTag();
	  aRootName = "SubMeshes on Vertex";
	  break;
	case TopAbs_EDGE:
	  aRootTag  = GetSubMeshOnEdgeTag();
	  aRootName = "SubMeshes on Edge";	  
	  break;
	case TopAbs_FACE:
	  aRootTag  = GetSubMeshOnFaceTag();
	  aRootName = "SubMeshes on Face";	  
	  break;
	case TopAbs_SOLID:
	  aRootTag  = GetSubMeshOnSolidTag();
	  aRootName = "SubMeshes on Solid";
	  break;
	default:
	  aRootTag  = GetSubMeshOnCompoundTag();
	  aRootName = "SubMeshes on Compound";
	  break;
	}
      }

      // Find or create submesh root
      SALOMEDS::SObject_var aRootSO;
      if ( !aParentSO->FindSubObject ( aRootTag, aRootSO ) ) {
	    MESSAGE( "********** SMESH_Gen_i::PublishInStudy(): creating submesh root..." )
	aRootSO  = aStudyBuilder->NewObjectToTag( aParentSO, aRootTag );
	anAttr   = aStudyBuilder->FindOrCreateAttribute( aRootSO, "AttributeName" );
	aName    = SALOMEDS::AttributeName::_narrow( anAttr );
	aName    ->SetValue( aRootName );
	anAttr   = aStudyBuilder->FindOrCreateAttribute( aRootSO, "AttributeSelectable" );
	aSelAttr = SALOMEDS::AttributeSelectable::_narrow( anAttr );
	aSelAttr ->SetSelectable( false );
      }

      // Add new submesh to corresponding sub-tree
      MESSAGE( "********** SMESH_Gen_i::PublishInStudy(): adding submesh to study..." )
      SALOMEDS::SObject_var newMesh = aStudyBuilder->NewObject( aRootSO );
      anAttr  = aStudyBuilder->FindOrCreateAttribute( newMesh, "AttributePixMap" );
      aPixmap = SALOMEDS::AttributePixMap::_narrow( anAttr );
      aPixmap ->SetPixMap( "ICON_SMESH_TREE_MESH" );
      anAttr  = aStudyBuilder->FindOrCreateAttribute( newMesh, "AttributeIOR" );
      anIOR   = SALOMEDS::AttributeIOR::_narrow(anAttr);
      anIOR   ->SetValue( GetORB()->object_to_string( aSubMesh ) );
      aSO     = SALOMEDS::SObject::_narrow( newMesh );
      anObjName = TCollection_AsciiString( "SubMesh" );
    }
  }

  // Publishing a hypothesis or algorithm
  SMESH::SMESH_Hypothesis_var aHyp = SMESH::SMESH_Hypothesis::_narrow( theIOR );
  if( aSO->_is_nil() && !aHyp->_is_nil() ) {
    //Find or Create Hypothesis root
    SALOMEDS::SObject_var  HypothesisRoot;
    Standard_Integer aRootTag =
      SMESH::SMESH_Algo::_narrow( theIOR )->_is_nil() ? GetHypothesisRootTag() : GetAlgorithmsRootTag();

    if ( !father->FindSubObject ( aRootTag, HypothesisRoot ) ) {
      HypothesisRoot = aStudyBuilder->NewObjectToTag( father, aRootTag );
      anAttr   = aStudyBuilder->FindOrCreateAttribute( HypothesisRoot, "AttributeName" );
      aName    = SALOMEDS::AttributeName::_narrow( anAttr );
      aName    ->SetValue( aRootTag ==  GetHypothesisRootTag()  ? "Hypotheses" : "Algorithms" );
      anAttr   = aStudyBuilder->FindOrCreateAttribute( HypothesisRoot, "AttributeSelectable" );
      aSelAttr = SALOMEDS::AttributeSelectable::_narrow( anAttr );
      aSelAttr ->SetSelectable( false );
      anAttr   = aStudyBuilder->FindOrCreateAttribute( HypothesisRoot, "AttributePixMap" );
      aPixmap  = SALOMEDS::AttributePixMap::_narrow( anAttr );
      aPixmap  ->SetPixMap( aRootTag == GetHypothesisRootTag()  ? "ICON_SMESH_TREE_HYPO" : "ICON_SMESH_TREE_ALGO" );
    }

    // Add New Hypothesis
    string aPmName;
    SALOMEDS::SObject_var newHypo = aStudyBuilder->NewObject( HypothesisRoot );
    anAttr  = aStudyBuilder->FindOrCreateAttribute( newHypo, "AttributePixMap" );
    aPixmap = SALOMEDS::AttributePixMap::_narrow( anAttr );
    aPmName = ( aRootTag == GetHypothesisRootTag()  ? "ICON_SMESH_TREE_HYPO_" : "ICON_SMESH_TREE_ALGO_" );
    aPmName += aHyp->GetName();
    aPixmap ->SetPixMap( aPmName.c_str() );
    anAttr  = aStudyBuilder->FindOrCreateAttribute( newHypo, "AttributeIOR" );
    anIOR   = SALOMEDS::AttributeIOR::_narrow(anAttr);
    anIOR   ->SetValue( GetORB()->object_to_string( aHyp ) );
    aSO     = SALOMEDS::SObject::_narrow( newHypo );
    anObjName = TCollection_AsciiString( aHyp->GetName() );
  }

  // Publishing a group
  SMESH::SMESH_Group_var aGroup = SMESH::SMESH_Group::_narrow(theIOR);
  if( aSO->_is_nil() && !aGroup->_is_nil() ) {
    // try to obtain a parent mesh's SObject
    MESSAGE( "********** SMESH_Gen_i::PublishInStudy(): publishing group..." );
    SALOMEDS::SObject_var aParentSO;
    SMESH::SMESH_Mesh_var aParentMesh;
    SMESH_Group_i* aServant = dynamic_cast<SMESH_Group_i*>( GetServant( aGroup ).in() );
    if ( aServant != NULL ) {
      aParentMesh = SMESH::SMESH_Mesh::_narrow( GetPOA()->servant_to_reference( aServant->GetMeshServant() ) );
      if ( !aParentMesh->_is_nil() ) {
	MESSAGE( "********** SMESH_Gen_i::PublishInStudy(): publishing group: refernce to mesh is OK" );
	string anIOR = GetORB()->object_to_string( aParentMesh );
	MESSAGE( "********** SMESH_Gen_i::PublishInStudy(): publishing group: mesh IOR = "<<anIOR.c_str() );
	aParentSO = theStudy->FindObjectIOR( anIOR.c_str() );
      }
    }

    // Find proper group sub-tree tag
    if ( !aParentSO->_is_nil() ) {
      MESSAGE( "********** SMESH_Gen_i::PublishInStudy(): publishing group: parent mesh found" );
      int aType = (int)aGroup->GetType();
      const char* aRootNames[] = { "Compound Groups", "Groups of Nodes", "Groups of Edges", "Groups of Faces", "Groups of Volumes" };

      // Currently, groups with heterogenous content are not supported
      if ( aType != SMESH::ALL ) {
	MESSAGE( "********** SMESH_Gen_i::PublishInStudy(): publishing group: group type OK" );
	long aRootTag = GetNodeGroupsTag() + aType - 1;

	// Find or create groups root
	SALOMEDS::SObject_var aRootSO;
	if ( !aParentSO->FindSubObject ( aRootTag, aRootSO ) ) {
	  MESSAGE( "********** SMESH_Gen_i::PublishInStudy(): creating groups root..." )
	  aRootSO  = aStudyBuilder->NewObjectToTag( aParentSO, aRootTag );
	  anAttr   = aStudyBuilder->FindOrCreateAttribute( aRootSO, "AttributeName" );
	  aName    = SALOMEDS::AttributeName::_narrow( anAttr );
	  aName    ->SetValue( aRootNames[aType] );
	  anAttr   = aStudyBuilder->FindOrCreateAttribute( aRootSO, "AttributeSelectable" );
	  aSelAttr = SALOMEDS::AttributeSelectable::_narrow( anAttr );
	  aSelAttr ->SetSelectable( false );
	}

	// Add new group to corresponding sub-tree
	MESSAGE( "********** SMESH_Gen_i::PublishInStudy(): adding group to study..." )
	SALOMEDS::SObject_var aGroupSO = aStudyBuilder->NewObject( aRootSO );
	anAttr  = aStudyBuilder->FindOrCreateAttribute( aGroupSO, "AttributePixMap" );
	aPixmap = SALOMEDS::AttributePixMap::_narrow( anAttr );
	aPixmap ->SetPixMap( "ICON_SMESH_TREE_GROUP" );
	anAttr  = aStudyBuilder->FindOrCreateAttribute( aGroupSO, "AttributeIOR" );
	anIOR   = SALOMEDS::AttributeIOR::_narrow( anAttr );
	anIOR   ->SetValue( GetORB()->object_to_string( aGroup ) );
	aSO     = SALOMEDS::SObject::_narrow( aGroupSO );
	anObjName = TCollection_AsciiString( "Group" );
      }
    }
  }

  // Setting SObject's name
  if ( !aSO->_is_nil() ) {
    if ( strlen( theName ) == 0 ) 
      anObjName += TCollection_AsciiString( "_" ) + TCollection_AsciiString( aSO->Tag() );
    else 
      anObjName = TCollection_AsciiString( strdup( theName ) );
    anAttr  = aStudyBuilder->FindOrCreateAttribute( aSO, "AttributeName" );
    aName   = SALOMEDS::AttributeName::_narrow( anAttr );
    aName   ->SetValue( anObjName.ToCString() );    
  }

  MESSAGE( "********** SMESH_Gen_i::PublishInStudy(): COMPLETED" )
  return aSO._retn();
}
      
//=============================================================================
/*! 
 *  SMESHEngine_factory
 *
 *  C factory, accessible with dlsym, after dlopen  
 */
//=============================================================================

extern "C"
{
  PortableServer::ObjectId* SMESHEngine_factory( CORBA::ORB_ptr            orb,
						 PortableServer::POA_ptr   poa, 
						 PortableServer::ObjectId* contId,
						 const char*               instanceName, 
						 const char*               interfaceName )
  {
    MESSAGE( "PortableServer::ObjectId* SMESHEngine_factory()" );
    SCRUTE(interfaceName);
    SMESH_Gen_i * mySMESH_Gen 
      = new SMESH_Gen_i(orb, poa, contId, instanceName, interfaceName);
    return mySMESH_Gen->getId() ;
  }
}
