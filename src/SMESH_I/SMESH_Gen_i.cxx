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
#include <TopTools_ListOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <gp_Pnt.hxx>
#include <BRep_Tool.hxx>
#include <TCollection_AsciiString.hxx>
#include <OSD.hxx>

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
#include "SMESH_PythonDump.hxx"

#include "SMESHDS_Document.hxx"
#include "SMESHDS_Group.hxx"
#include "SMESHDS_GroupOnGeom.hxx"
#include "SMESH_Mesh.hxx"
#include "SMESH_Hypothesis.hxx"
#include "SMESH_Group.hxx"
#include "SMESH_MeshEditor.hxx"

#include "SMDS_EdgePosition.hxx"
#include "SMDS_FacePosition.hxx"

#include CORBA_SERVER_HEADER(SMESH_Group)
#include CORBA_SERVER_HEADER(SMESH_Filter)

#include "DriverMED_W_SMESHDS_Mesh.h"
#include "DriverMED_R_SMESHDS_Mesh.h"

#include "SALOMEDS_Tool.hxx"
#include "SALOME_NamingService.hxx"
#include "SALOME_LifeCycleCORBA.hxx"
#include "Utils_SINGLETON.hxx"
#include "OpUtil.hxx"

#include CORBA_CLIENT_HEADER(SALOME_ModuleCatalog)

#include "GEOM_Client.hxx"
#include "Utils_ExceptHandlers.hxx"

#include <map>

using namespace std;
using SMESH::TPythonDump;

#define NUM_TMP_FILES 2

#ifdef _DEBUG_
static int MYDEBUG = 1;
#else
static int MYDEBUG = 0;
#endif

// Static variables definition
CORBA::ORB_var          SMESH_Gen_i::myOrb;
PortableServer::POA_var SMESH_Gen_i::myPoa;
SALOME_NamingService*   SMESH_Gen_i::myNS  = NULL;
SALOME_LifeCycleCORBA*  SMESH_Gen_i::myLCC = NULL;
SMESH_Gen_i*            SMESH_Gen_i::mySMESHGen = NULL;

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
    INFOS( "GetServant - Unknown exception was caught!!!" ); 
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
      INFOS( "SObjectToObject - Unknown exception was caught!!!" );
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
  INFOS( "SMESH_Gen_i::SMESH_Gen_i : default constructor" );
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
  INFOS( "SMESH_Gen_i::SMESH_Gen_i : standard constructor" );

  myOrb = CORBA::ORB::_duplicate(orb);
  myPoa = PortableServer::POA::_duplicate(poa);
  
  _thisObj = this ;
  _id = myPoa->activate_object( _thisObj );
  
  myIsEmbeddedMode = false;
  myShapeReader = NULL;  // shape reader
  mySMESHGen = this;

  OSD::SetSignal( true );
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
  INFOS( "SMESH_Gen_i::~SMESH_Gen_i" );

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
  if(MYDEBUG) MESSAGE( "Create Hypothesis <" << theHypName << "> from " << theLibName);

  // create a new hypothesis object servant
  SMESH_Hypothesis_i* myHypothesis_i = 0;
  SMESH::SMESH_Hypothesis_var hypothesis_i;

  try
  {
    // check, if creator for this hypothesis type already exists
    if (myHypCreatorMap.find(string(theHypName)) == myHypCreatorMap.end())
    {
      // load plugin library
      if(MYDEBUG) MESSAGE("Loading server meshers plugin library ...");
      void* libHandle = dlopen (theLibName, RTLD_LAZY);
      if (!libHandle)
      {
        // report any error, if occured
        const char* anError = dlerror();
        throw(SALOME_Exception(anError));
      }

      // get method, returning hypothesis creator
      if(MYDEBUG) MESSAGE("Find GetHypothesisCreator() method ...");
      typedef GenericHypothesisCreator_i* (*GetHypothesisCreator)(const char* theHypName);
      GetHypothesisCreator procHandle =
        (GetHypothesisCreator)dlsym( libHandle, "GetHypothesisCreator" );
      if (!procHandle)
      {
        throw(SALOME_Exception(LOCALIZED("bad hypothesis plugin library")));
        dlclose(libHandle);
      }

      // get hypothesis creator
      if(MYDEBUG) MESSAGE("Get Hypothesis Creator for " << theHypName);
      GenericHypothesisCreator_i* aCreator = procHandle(theHypName);
      if (!aCreator)
      {
        throw(SALOME_Exception(LOCALIZED("no such a hypothesis in this plugin")));
      }

      // map hypothesis creator to a hypothesis name
      myHypCreatorMap[string(theHypName)] = aCreator;
    }

    // create a new hypothesis object, store its ref. in studyContext
    if(MYDEBUG) MESSAGE("Create Hypothesis " << theHypName);
    myHypothesis_i =
      myHypCreatorMap[string(theHypName)]->Create (myPoa, GetCurrentStudyID(), &myGen);
    // _CS_gbo Explicit activation (no longer made in the constructor).
    myHypothesis_i->Activate();
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
  int nextId = RegisterObject( hypothesis_i );
  if(MYDEBUG) MESSAGE( "Add hypo to map with id = "<< nextId );

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
  if(MYDEBUG) MESSAGE( "SMESH_Gen_i::createMesh" );

  // Get or create the GEOM_Client instance
  try {
    // create a new mesh object servant, store it in a map in study context
    SMESH_Mesh_i* meshServant = new SMESH_Mesh_i( GetPOA(), this, GetCurrentStudyID() );
    // create a new mesh object
    meshServant->SetImpl( myGen.CreateMesh( GetCurrentStudyID(), myIsEmbeddedMode ));

    // activate the CORBA servant of Mesh
    SMESH::SMESH_Mesh_var mesh = SMESH::SMESH_Mesh::_narrow( meshServant->_this() );
    int nextId = RegisterObject( mesh );
    if(MYDEBUG) MESSAGE( "Add mesh to map with id = "<< nextId);
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
 *  SMESH_Gen_i::SetEmbeddedMode
 *
 *  Set current mode
 */
//=============================================================================

void SMESH_Gen_i::SetEmbeddedMode( CORBA::Boolean theMode )
{
  myIsEmbeddedMode = theMode;
}

//=============================================================================
/*!
 *  SMESH_Gen_i::IsEmbeddedMode
 *
 *  Get current mode
 */
//=============================================================================

CORBA::Boolean SMESH_Gen_i::IsEmbeddedMode()
{
  return myIsEmbeddedMode;
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
  //if(MYDEBUG)
  //MESSAGE( "SMESH_Gen_i::SetCurrentStudy" );
  myCurrentStudy = SALOMEDS::Study::_duplicate( theStudy );
  // create study context, if it doesn't exist and set current study
  int studyId = GetCurrentStudyID();
  if(MYDEBUG) MESSAGE( "SMESH_Gen_i::SetCurrentStudy: study Id = " << studyId );
  if ( myStudyContextMap.find( studyId ) == myStudyContextMap.end() ) {
    myStudyContextMap[ studyId ] = new StudyContext;      
  }

  SALOMEDS::StudyBuilder_var aStudyBuilder = myCurrentStudy->NewBuilder(); 
  if( !myCurrentStudy->FindComponent( "GEOM" )->_is_nil() )
    aStudyBuilder->LoadWith( myCurrentStudy->FindComponent( "GEOM" ), GetGeomEngine() );

  // set current study for geom engine
  //if ( !CORBA::is_nil( GetGeomEngine() ) )
  //  GetGeomEngine()->GetCurrentStudy( myCurrentStudy->StudyId() );
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
  if(MYDEBUG) MESSAGE( "SMESH_Gen_i::GetCurrentStudy: study Id = " << GetCurrentStudyID() );
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
  if ( !CORBA::is_nil( myCurrentStudy ) &&
      myStudyContextMap.find( GetCurrentStudyID() ) != myStudyContextMap.end() )
    return myStudyContextMap[ myCurrentStudy->StudyId() ];
  else
    return 0;
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
  // Create hypothesis/algorithm
  SMESH::SMESH_Hypothesis_var hyp = this->createHypothesis( theHypName, theLibName );

  // Publish hypothesis/algorithm in the study
  if ( CanPublishInStudy( hyp ) ) {
    SALOMEDS::SObject_var aSO = PublishHypothesis( myCurrentStudy, hyp );
    if ( !aSO->_is_nil() ) {
      // Update Python script
      TPythonDump() << aSO << " = " << this << ".CreateHypothesis('"
                    << theHypName << "', '" << theLibName << "')";
    }
  }

  return hyp._retn();
}

//================================================================================
/*!
 * \brief Return hypothesis of given type holding parameter values of the existing mesh
  * \param theHypType - hypothesis type name
  * \param theLibName - plugin library name
  * \param theMesh - The mesh of interest
  * \param theGeom - The shape to get parameter values from
  * \retval SMESH::SMESH_Hypothesis_ptr - The returned hypothesis may be the one existing
  *    in a study and used to compute the mesh, or a temporary one created just to pass
  *    parameter values
 */
//================================================================================

SMESH::SMESH_Hypothesis_ptr
SMESH_Gen_i::GetHypothesisParameterValues (const char*           theHypType,
                                           const char*           theLibName,
                                           SMESH::SMESH_Mesh_ptr theMesh,
                                           GEOM::GEOM_Object_ptr theGeom)
    throw ( SALOME::SALOME_Exception )
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( CORBA::is_nil( theMesh ) )
    THROW_SALOME_CORBA_EXCEPTION( "bad Mesh reference", SALOME::BAD_PARAM );
  if ( CORBA::is_nil( theGeom ) )
    THROW_SALOME_CORBA_EXCEPTION( "bad shape object reference", SALOME::BAD_PARAM );

  // -----------------------------------------------
  // find hypothesis used to mesh theGeom
  // -----------------------------------------------

  // get mesh and shape
  SMESH_Mesh_i* meshServant = SMESH::DownCast<SMESH_Mesh_i*>( theMesh );
  TopoDS_Shape shape = GeomObjectToShape( theGeom );
  if ( !meshServant || shape.IsNull() )
    return SMESH::SMESH_Hypothesis::_nil();
  ::SMESH_Mesh& mesh = meshServant->GetImpl();

  if ( mesh.NbNodes() == 0 ) // empty mesh
    return SMESH::SMESH_Hypothesis::_nil();

  // create a temporary hypothesis to know its dimention
  SMESH::SMESH_Hypothesis_var tmpHyp = this->createHypothesis( theHypType, theLibName );
  SMESH_Hypothesis_i* hypServant = SMESH::DownCast<SMESH_Hypothesis_i*>( tmpHyp );
  if ( !hypServant )
    return SMESH::SMESH_Hypothesis::_nil();
  ::SMESH_Hypothesis* hyp = hypServant->GetImpl();

  // look for a hypothesis of theHypType used to mesh the shape
  if ( myGen.GetShapeDim( shape ) == hyp->GetDim() )
  {
    // check local shape
    SMESH::ListOfHypothesis_var aHypList = theMesh->GetHypothesisList( theGeom );
    int nbLocalHyps = aHypList->length();
    for ( int i = 0; i < nbLocalHyps; i++ )
      if ( strcmp( theHypType, aHypList[i]->GetName() ) == 0 ) // FOUND local!
        return SMESH::SMESH_Hypothesis::_duplicate( aHypList[i] );
    // check super shapes
    TopTools_ListIteratorOfListOfShape itShape( mesh.GetAncestors( shape ));
    while ( nbLocalHyps == 0 && itShape.More() ) {
      GEOM::GEOM_Object_ptr geomObj = ShapeToGeomObject( itShape.Value() );
      if ( ! CORBA::is_nil( geomObj )) {
        SMESH::ListOfHypothesis_var aHypList = theMesh->GetHypothesisList( geomObj );
        nbLocalHyps = aHypList->length();
        for ( int i = 0; i < nbLocalHyps; i++ )
          if ( strcmp( theHypType, aHypList[i]->GetName() ) == 0 ) // FOUND global!
            return SMESH::SMESH_Hypothesis::_duplicate( aHypList[i] );
      }
      itShape.Next();
    }
  }

  // let the temporary hypothesis find out some how parameter values
  if ( hyp->SetParametersByMesh( &mesh, shape ))
    return SMESH::SMESH_Hypothesis::_duplicate( tmpHyp );
    
  return SMESH::SMESH_Hypothesis::_nil();
}

//=============================================================================
/*!
 *  SMESH_Gen_i::CreateMesh
 *
 *  Create empty mesh on a shape and publish it in the study
 */
//=============================================================================

SMESH::SMESH_Mesh_ptr SMESH_Gen_i::CreateMesh( GEOM::GEOM_Object_ptr theShapeObject )
     throw ( SALOME::SALOME_Exception )
{
  Unexpect aCatch(SALOME_SalomeException);
  if(MYDEBUG) MESSAGE( "SMESH_Gen_i::CreateMesh" );
  // create mesh
  SMESH::SMESH_Mesh_var mesh = this->createMesh();
  // set shape
  SMESH_Mesh_i* meshServant = SMESH::DownCast<SMESH_Mesh_i*>( mesh );
  ASSERT( meshServant );
  meshServant->SetShape( theShapeObject );

  // publish mesh in the study
  if ( CanPublishInStudy( mesh ) ) {
    SALOMEDS::StudyBuilder_var aStudyBuilder = myCurrentStudy->NewBuilder();
    aStudyBuilder->NewCommand();  // There is a transaction
    SALOMEDS::SObject_var aSO = PublishMesh( myCurrentStudy, mesh.in() );
    aStudyBuilder->CommitCommand();
    if ( !aSO->_is_nil() ) {
      // Update Python script
      TPythonDump() << aSO << " = " << this << ".CreateMesh(" << theShapeObject << ")";
    }
  }

  return mesh._retn();
}

//=============================================================================
/*!
 *  SMESH_Gen_i::CreateMeshFromUNV
 *
 *  Create mesh and import data from UNV file
 */
//=============================================================================

SMESH::SMESH_Mesh_ptr SMESH_Gen_i::CreateMeshesFromUNV( const char* theFileName )
  throw ( SALOME::SALOME_Exception )
{
  Unexpect aCatch(SALOME_SalomeException);
  if(MYDEBUG) MESSAGE( "SMESH_Gen_i::CreateMeshesFromUNV" );

  SMESH::SMESH_Mesh_var aMesh = createMesh();
  string aFileName;
  // publish mesh in the study
  if ( CanPublishInStudy( aMesh ) ) {
    SALOMEDS::StudyBuilder_var aStudyBuilder = myCurrentStudy->NewBuilder();
    aStudyBuilder->NewCommand();  // There is a transaction
    SALOMEDS::SObject_var aSO = PublishMesh( myCurrentStudy, aMesh.in(), aFileName.c_str() );
    aStudyBuilder->CommitCommand();
    if ( !aSO->_is_nil() ) {
      // Update Python script
      TPythonDump() << aSO << " = smeshgen.CreateMeshesFromUNV('" << theFileName << "')";
    }
  }

  SMESH_Mesh_i* aServant = dynamic_cast<SMESH_Mesh_i*>( GetServant( aMesh ).in() );
  ASSERT( aServant );
  aServant->ImportUNVFile( theFileName );
  return aMesh._retn();
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
  if(MYDEBUG) MESSAGE( "SMESH_Gen_i::CreateMeshFromMED" );

  // Python Dump
  TPythonDump aPythonDump;
  aPythonDump << "([";
  //TCollection_AsciiString aStr ("([");

  // Retrieve mesh names from the file
  DriverMED_R_SMESHDS_Mesh myReader;
  myReader.SetFile( theFileName );
  myReader.SetMeshId( -1 );
  Driver_Mesh::Status aStatus;
  list<string> aNames = myReader.GetMeshNames(aStatus);
  SMESH::mesh_array_var aResult = new SMESH::mesh_array();
  theStatus = (SMESH::DriverMED_ReadStatus)aStatus;
  if (theStatus == SMESH::DRS_OK) {
    SALOMEDS::StudyBuilder_var aStudyBuilder = myCurrentStudy->NewBuilder();
    aStudyBuilder->NewCommand();  // There is a transaction
    aResult->length( aNames.size() );
    int i = 0;
    
    // Iterate through all meshes and create mesh objects
    for ( list<string>::iterator it = aNames.begin(); it != aNames.end(); it++ ) {
      // Python Dump
      //if (i > 0) aStr += ", ";
      if (i > 0) aPythonDump << ", ";

      // create mesh
      SMESH::SMESH_Mesh_var mesh = createMesh();
      
      // publish mesh in the study
      SALOMEDS::SObject_var aSO;
      if ( CanPublishInStudy( mesh ) )
        aSO = PublishMesh( myCurrentStudy, mesh.in(), (*it).c_str() );
      if ( !aSO->_is_nil() ) {
        // Python Dump
        aPythonDump << aSO;
        //aStr += aSO->GetID();
      } else {
        // Python Dump
        aPythonDump << "mesh_" << i;
//         aStr += "mesh_";
//         aStr += TCollection_AsciiString(i);
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
    aStudyBuilder->CommitCommand();
  }

  // Update Python script
  aPythonDump << "], status) = " << this << ".CreateMeshesFromMED('" << theFileName << "')";

  return aResult._retn();
}

//=============================================================================
/*!
 *  SMESH_Gen_i::CreateMeshFromSTL
 *
 *  Create mesh and import data from STL file
 */
//=============================================================================

SMESH::SMESH_Mesh_ptr SMESH_Gen_i::CreateMeshesFromSTL( const char* theFileName )
  throw ( SALOME::SALOME_Exception )
{
  Unexpect aCatch(SALOME_SalomeException);
  if(MYDEBUG) MESSAGE( "SMESH_Gen_i::CreateMeshesFromSTL" );

  SMESH::SMESH_Mesh_var aMesh = createMesh();
  string aFileName;
  // publish mesh in the study
  if ( CanPublishInStudy( aMesh ) ) {
    SALOMEDS::StudyBuilder_var aStudyBuilder = myCurrentStudy->NewBuilder();
    aStudyBuilder->NewCommand();  // There is a transaction
    SALOMEDS::SObject_var aSO = PublishInStudy
      ( myCurrentStudy, SALOMEDS::SObject::_nil(), aMesh.in(), aFileName.c_str() );
    aStudyBuilder->CommitCommand();
    if ( !aSO->_is_nil() ) {
      // Update Python script
      TPythonDump() << aSO << " = " << this << ".CreateMeshesFromSTL('" << theFileName << "')";
    }
  }

  SMESH_Mesh_i* aServant = dynamic_cast<SMESH_Mesh_i*>( GetServant( aMesh ).in() );
  ASSERT( aServant );
  aServant->ImportSTLFile( theFileName );
  return aMesh._retn();
}

//=============================================================================
/*!
 *  SMESH_Gen_i::IsReadyToCompute
 *
 *  Returns true if mesh contains enough data to be computed
 */
//=============================================================================

CORBA::Boolean SMESH_Gen_i::IsReadyToCompute( SMESH::SMESH_Mesh_ptr theMesh,
                                              GEOM::GEOM_Object_ptr theShapeObject )
  throw ( SALOME::SALOME_Exception )
{
  Unexpect aCatch(SALOME_SalomeException);
  if(MYDEBUG) MESSAGE( "SMESH_Gen_i::IsReadyToCompute" );

  if ( CORBA::is_nil( theShapeObject ) )
    THROW_SALOME_CORBA_EXCEPTION( "bad shape object reference", 
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
      TopoDS_Shape myLocShape = GeomObjectToShape( theShapeObject );
      // call implementation
      ::SMESH_Mesh& myLocMesh = meshServant->GetImpl();
      return myGen.CheckAlgoState( myLocMesh, myLocShape );
    }
  }
  catch ( SALOME_Exception& S_ex ) {
    INFOS( "catch exception "<< S_ex.what() );
  }
  return false;
}

//================================================================================
/*!
 * \brief Returns errors of hypotheses definintion
  * \param theMesh - the mesh
  * \param theSubObject - the main or sub- shape
  * \retval SMESH::algo_error_array* - sequence of errors
 */
//================================================================================

SMESH::algo_error_array* SMESH_Gen_i::GetAlgoState( SMESH::SMESH_Mesh_ptr theMesh, 
                                                    GEOM::GEOM_Object_ptr theSubObject )
      throw ( SALOME::SALOME_Exception )
{
  Unexpect aCatch(SALOME_SalomeException);
  if(MYDEBUG) MESSAGE( "SMESH_Gen_i::GetAlgoState()" );

  if ( CORBA::is_nil( theSubObject ) )
    THROW_SALOME_CORBA_EXCEPTION( "bad shape object reference", SALOME::BAD_PARAM );

  if ( CORBA::is_nil( theMesh ) )
    THROW_SALOME_CORBA_EXCEPTION( "bad Mesh reference",SALOME::BAD_PARAM );

  SMESH::algo_error_array_var error_array = new SMESH::algo_error_array;
  try {
    SMESH_Mesh_i* meshServant = SMESH::DownCast<SMESH_Mesh_i*>( theMesh );
    ASSERT( meshServant );
    if ( meshServant ) {
      TopoDS_Shape myLocShape = GeomObjectToShape( theSubObject );
      ::SMESH_Mesh& myLocMesh = meshServant->GetImpl();
      list< ::SMESH_Gen::TAlgoStateError > error_list;
      list< ::SMESH_Gen::TAlgoStateError >::iterator error;
      // call ::SMESH_Gen::GetAlgoState()
      myGen.GetAlgoState( myLocMesh, myLocShape, error_list );
      error_array->length( error_list.size() );
      int i = 0;
      for ( error = error_list.begin(); error != error_list.end(); ++error )
      {
        // error name
        SMESH::AlgoStateErrorName errName;
        switch ( error->_name ) {
        case ::SMESH_Gen::MISSING_ALGO:     errName = SMESH::MISSING_ALGO; break;
        case ::SMESH_Gen::MISSING_HYPO:     errName = SMESH::MISSING_HYPO; break;
        case ::SMESH_Gen::NOT_CONFORM_MESH: errName = SMESH::NOT_CONFORM_MESH; break;
        default:
          THROW_SALOME_CORBA_EXCEPTION( "bad error name",SALOME::BAD_PARAM );
        }
        // algo name
        CORBA::String_var algoName;
        if ( error->_algo ) {
          if ( !myCurrentStudy->_is_nil() ) {
            // find algo in the study
            SALOMEDS::SComponent_var father = SALOMEDS::SComponent::_narrow
              ( myCurrentStudy->FindComponent( ComponentDataType() ) );
            if ( !father->_is_nil() ) {
              SALOMEDS::ChildIterator_var itBig = myCurrentStudy->NewChildIterator( father );
              for ( ; itBig->More(); itBig->Next() ) {
                SALOMEDS::SObject_var gotBranch = itBig->Value();
                if ( gotBranch->Tag() == GetAlgorithmsRootTag() ) {
                  SALOMEDS::ChildIterator_var algoIt = myCurrentStudy->NewChildIterator( gotBranch );
                  for ( ; algoIt->More(); algoIt->Next() ) {
                    SALOMEDS::SObject_var algoSO = algoIt->Value();
                    CORBA::Object_var    algoIOR = SObjectToObject( algoSO );
                    if ( !CORBA::is_nil( algoIOR )) {
                      SMESH_Hypothesis_i* myImpl = SMESH::DownCast<SMESH_Hypothesis_i*>( algoIOR );
                      if ( myImpl && myImpl->GetImpl() == error->_algo ) {
                        algoName = algoSO->GetName();
                        break;
                      }
                    }
                  } // loop on algo SO's
                  break;
                } // if algo tag
              } // SMESH component iterator
            }
          }
          if ( algoName.in() == 0 )
            // use algo type name
            algoName = CORBA::string_dup( error->_algo->GetName() );
        }
        // fill AlgoStateError structure
        SMESH::AlgoStateError & errStruct = error_array[ i++ ];
        errStruct.name         = errName;
        errStruct.algoName     = algoName;
        errStruct.algoDim      = error->_algoDim;
        errStruct.isGlobalAlgo = error->_isGlobalAlgo;
      }
    }
  }
  catch ( SALOME_Exception& S_ex ) {
    INFOS( "catch exception "<< S_ex.what() );
  }
  return error_array._retn();
}

//=============================================================================
/*!
 *  SMESH_Gen_i::GetSubShapesId
 *
 *  Get sub-shapes unique ID's list
 */
//=============================================================================

SMESH::long_array* SMESH_Gen_i::GetSubShapesId( GEOM::GEOM_Object_ptr theMainShapeObject,
					    const SMESH::object_array& theListOfSubShapeObject )
     throw ( SALOME::SALOME_Exception )
{
  Unexpect aCatch(SALOME_SalomeException);
  if(MYDEBUG) MESSAGE( "SMESH_Gen_i::GetSubShapesId" );

  SMESH::long_array_var shapesId = new SMESH::long_array;
  set<int> setId;

  if ( CORBA::is_nil( theMainShapeObject ) )
    THROW_SALOME_CORBA_EXCEPTION( "bad shape object reference",
                                  SALOME::BAD_PARAM );

  try
    {
      TopoDS_Shape myMainShape = GeomObjectToShape(theMainShapeObject);
      TopTools_IndexedMapOfShape myIndexToShape;      
      TopExp::MapShapes(myMainShape,myIndexToShape);

      for ( int i = 0; i < theListOfSubShapeObject.length(); i++ )
	{
	  GEOM::GEOM_Object_var aShapeObject
	    = GEOM::GEOM_Object::_narrow(theListOfSubShapeObject[i]);
	  if ( CORBA::is_nil( aShapeObject ) )
	    THROW_SALOME_CORBA_EXCEPTION ("bad shape object reference", \
				        SALOME::BAD_PARAM );

	  TopoDS_Shape locShape  = GeomObjectToShape(aShapeObject);
	  for (TopExp_Explorer exp(locShape,TopAbs_FACE); exp.More(); exp.Next())
	    {
	      const TopoDS_Face& F = TopoDS::Face(exp.Current());
	      setId.insert(myIndexToShape.FindIndex(F));
	      if(MYDEBUG) SCRUTE(myIndexToShape.FindIndex(F));
	    }
	  for (TopExp_Explorer exp(locShape,TopAbs_EDGE); exp.More(); exp.Next())
	    {
	      const TopoDS_Edge& E = TopoDS::Edge(exp.Current());
	      setId.insert(myIndexToShape.FindIndex(E));
	      if(MYDEBUG) SCRUTE(myIndexToShape.FindIndex(E));
	    }
	  for (TopExp_Explorer exp(locShape,TopAbs_VERTEX); exp.More(); exp.Next())
	    {
	      const TopoDS_Vertex& V = TopoDS::Vertex(exp.Current());
	      setId.insert(myIndexToShape.FindIndex(V));
	      if(MYDEBUG) SCRUTE(myIndexToShape.FindIndex(V));
	    }
	}
      shapesId->length(setId.size());
      set<int>::iterator iind;
      int i=0;
      for (iind = setId.begin(); iind != setId.end(); iind++)
	{
	  if(MYDEBUG) SCRUTE((*iind));
	  shapesId[i] = (*iind);
	  if(MYDEBUG) SCRUTE(shapesId[i]);
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
                                     GEOM::GEOM_Object_ptr theShapeObject )
     throw ( SALOME::SALOME_Exception )
{
  Unexpect aCatch(SALOME_SalomeException);
  if(MYDEBUG) MESSAGE( "SMESH_Gen_i::Compute" );

  if ( CORBA::is_nil( theShapeObject ) )
    THROW_SALOME_CORBA_EXCEPTION( "bad shape object reference", 
                                  SALOME::BAD_PARAM );

  if ( CORBA::is_nil( theMesh ) )
    THROW_SALOME_CORBA_EXCEPTION( "bad Mesh reference",
                                  SALOME::BAD_PARAM );

  // Update Python script
  TPythonDump() << "isDone = " << this << ".Compute( "
                << theMesh << ", " << theShapeObject << ")";
  TPythonDump() << "if not isDone: print 'Mesh " << theMesh << " : computation failed'";

  try {
    // get mesh servant
    SMESH_Mesh_i* meshServant = dynamic_cast<SMESH_Mesh_i*>( GetServant( theMesh ).in() );
    ASSERT( meshServant );
    if ( meshServant ) {
      // get local TopoDS_Shape
      TopoDS_Shape myLocShape = GeomObjectToShape( theShapeObject );
      // call implementation compute
      ::SMESH_Mesh& myLocMesh = meshServant->GetImpl();
      return myGen.Compute( myLocMesh, myLocShape);
    }
  }
  catch ( SALOME_Exception& S_ex ) {
    INFOS( "Compute(): catch exception "<< S_ex.what() );
  }
  catch ( ... ) {
    INFOS( "Compute(): unknown exception " );
  }
  return false;
}

//================================================================================
/*!
 * \brief Return geometrical object the given element is built on
 *  \param theMesh - the mesh the element is in
 *  \param theElementID - the element ID
 *  \param theGeomName - the name of the result geom object if it is not yet published
 *  \retval GEOM::GEOM_Object_ptr - the found or just published geom object
 */
//================================================================================

GEOM::GEOM_Object_ptr
SMESH_Gen_i::GetGeometryByMeshElement( SMESH::SMESH_Mesh_ptr  theMesh,
                                       CORBA::Long            theElementID,
                                       const char*            theGeomName)
  throw ( SALOME::SALOME_Exception )
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( CORBA::is_nil( theMesh ) )
    THROW_SALOME_CORBA_EXCEPTION( "bad Mesh reference", SALOME::BAD_PARAM );

  GEOM::GEOM_Object_var mainShape = theMesh->GetShapeToMesh();
  GEOM::GEOM_Gen_var    geomGen   = GetGeomEngine();

  // get a core mesh DS
  SMESH_Mesh_i* meshServant = SMESH::DownCast<SMESH_Mesh_i*>( theMesh );
  if ( meshServant && !geomGen->_is_nil() && !mainShape->_is_nil() )
  {
    ::SMESH_Mesh & mesh = meshServant->GetImpl();
    SMESHDS_Mesh* meshDS = mesh.GetMeshDS();
    // find the element in mesh
    if ( const SMDS_MeshElement * elem = meshDS->FindElement( theElementID ) )
      // find a shape id by the element
      if ( int shapeID = ::SMESH_MeshEditor( &mesh ).FindShape( elem )) {
        // get a geom object by the shape id
        GEOM::GEOM_Object_var geom = ShapeToGeomObject( meshDS->IndexToShape( shapeID ));
        if ( geom->_is_nil() ) {
          GEOM::GEOM_IShapesOperations_var op =
            geomGen->GetIShapesOperations( GetCurrentStudyID() );
          if ( !op->_is_nil() )
            geom = op->GetSubShape( mainShape, shapeID );
        }
        if ( !geom->_is_nil() ) {
          // try to find the corresponding SObject
          GeomObjectToShape( geom ); // geom client remembers the found shape
          SALOMEDS::SObject_var SObj = ObjectToSObject( myCurrentStudy, geom.in() );
          if ( SObj->_is_nil() )
            // publish a new subshape
            SObj = geomGen->AddInStudy( myCurrentStudy, geom, theGeomName, mainShape );
          // return only published geometry
          if ( !SObj->_is_nil() )
            return geom._retn();
        }
      }
  }
  return GEOM::GEOM_Object::_nil();
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

//  ASSERT( theComponent->GetStudy()->StudyId() == myCurrentStudy->StudyId() )
  // san -- in case <myCurrentStudy> differs from theComponent's study,
  // use that of the component
  if ( myCurrentStudy->_is_nil() || 
       theComponent->GetStudy()->StudyId() != myCurrentStudy->StudyId() )
    SetCurrentStudy( theComponent->GetStudy() );

  // Store study contents as a set of python commands
  SavePython(myCurrentStudy);

  StudyContext* myStudyContext = GetCurrentStudyContext();
  
  // Declare a byte stream
  SALOMEDS::TMPFile_var aStreamFile;
  
  // Obtain a temporary dir
  TCollection_AsciiString tmpDir =
    ( isMultiFile ) ? TCollection_AsciiString( ( char* )theURL ) : ( char* )SALOMEDS_Tool::GetTmpDir().c_str();

  // Create a sequence of files processed
  SALOMEDS::ListOfFileNames_var aFileSeq = new SALOMEDS::ListOfFileNames;
  aFileSeq->length( NUM_TMP_FILES );

  TCollection_AsciiString aStudyName( "" );
  if ( isMultiFile ) 
    aStudyName = ( (char*)SALOMEDS_Tool::GetNameFromPath( myCurrentStudy->URL() ).c_str() );

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


  //Remove the files if they exist: BugID: 11225
  TCollection_AsciiString cmd("rm -f \"");
  cmd+=filename;
  cmd+="\" \"";
  cmd+=meshfile;
  cmd+="\"";
  system(cmd.ToCString());

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
            bool shapeRefFound = false;
	    bool found = gotBranch->FindSubObject( GetRefOnShapeTag(), myRef );
	    if ( found ) {
	      SALOMEDS::SObject_var myShape;
	      bool ok = myRef->ReferencedObject( myShape );
	      if ( ok ) {
                shapeRefFound = (! CORBA::is_nil( myShape->GetObject() ));
		string myRefOnObject = myShape->GetID();
		if ( shapeRefFound && myRefOnObject.length() > 0 ) {
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
	    if ( found && !shapeRefFound ) { // remove applied hyps
              myCurrentStudy->NewBuilder()->RemoveObjectWithChildren( myHypBranch );
            }
	    if ( found && shapeRefFound ) {
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
	    if ( found && !shapeRefFound ) { // remove applied algos
              myCurrentStudy->NewBuilder()->RemoveObjectWithChildren( myAlgoBranch );
            }
	    if ( found && shapeRefFound ) {
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

              if ( found ) // check if there is shape reference in submeshes
              {
                bool hasShapeRef = false;
		SALOMEDS::ChildIterator_var itSM =
                  myCurrentStudy->NewChildIterator( mySubmeshBranch );
		for ( ; itSM->More(); itSM->Next() ) {
		  SALOMEDS::SObject_var mySubRef, myShape, mySObject = itSM->Value();
                  if ( mySObject->FindSubObject( GetRefOnShapeTag(), mySubRef ))
                    mySubRef->ReferencedObject( myShape );
                  if ( !CORBA::is_nil( myShape ) && !CORBA::is_nil( myShape->GetObject() ))
                    hasShapeRef = true;
                  else
                  { // remove one submesh
                    if ( shapeRefFound )
                    { // unassign hypothesis
                      SMESH::SMESH_subMesh_var mySubMesh =
                        SMESH::SMESH_subMesh::_narrow( SObjectToObject( mySObject ));
                      if ( !mySubMesh->_is_nil() ) {
                        int shapeID = mySubMesh->GetId();
                        TopoDS_Shape S = mySMESHDSMesh->IndexToShape( shapeID );
                        const list<const SMESHDS_Hypothesis*>& hypList =
                          mySMESHDSMesh->GetHypothesis( S );
                        list<const SMESHDS_Hypothesis*>::const_iterator hyp = hypList.begin();
                        while ( hyp != hypList.end() ) {
                          int hypID = (*hyp++)->GetID(); // goto next hyp here because
                          myLocMesh.RemoveHypothesis( S, hypID ); // hypList changes here
                        }
                      }
                    }
                    myCurrentStudy->NewBuilder()->RemoveObjectWithChildren( mySObject );
                  }
                } // loop on submeshes of a type
                if ( !shapeRefFound || !hasShapeRef ) { // remove the whole submeshes branch
                  myCurrentStudy->NewBuilder()->RemoveObjectWithChildren( mySubmeshBranch );
                  found = false;
                }
              }  // end check if there is shape reference in submeshes
	      if ( found ) {
		char name_meshgroup[ 30 ];
		if ( i == GetSubMeshOnVertexTag() )
		  strcpy( name_meshgroup, "SubMeshes On Vertex" );
		else if ( i == GetSubMeshOnEdgeTag() )
		  strcpy( name_meshgroup, "SubMeshes On Edge" );
		else if ( i == GetSubMeshOnWireTag() )
		  strcpy( name_meshgroup, "SubMeshes On Wire" );
		else if ( i == GetSubMeshOnFaceTag() )
		  strcpy( name_meshgroup, "SubMeshes On Face" );
		else if ( i == GetSubMeshOnShellTag() )
		  strcpy( name_meshgroup, "SubMeshes On Shell" );
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
		  if ( !CORBA::is_nil( anSubObject ))
                  {
		    SMESH::SMESH_subMesh_var mySubMesh = SMESH::SMESH_subMesh::_narrow( anSubObject ) ;
		    int subid = myStudyContext->findId( string( GetORB()->object_to_string( anSubObject ) ) );
		      
		    // for each mesh open the HDF group basing on its id
		    char submeshGrpName[ 30 ];
		    sprintf( submeshGrpName, "SubMesh %d", subid );
		    aSubGroup = new HDFgroup( submeshGrpName, aGroup );
		    aSubGroup->CreateOnDisk();

		    // write reference on a shape, already checked if it exists
                    SALOMEDS::SObject_var mySubRef, myShape;
                    if ( mySObject->FindSubObject( GetRefOnShapeTag(), mySubRef ))
                      mySubRef->ReferencedObject( myShape );
                    string myRefOnObject = myShape->GetID();
                    if ( myRefOnObject.length() > 0 ) {
                      aSize[ 0 ] = myRefOnObject.length() + 1;
                      aDataset = new HDFdataset( "Ref on shape", aSubGroup, HDF_STRING, aSize, 1 );
                      aDataset->CreateOnDisk();
                      aDataset->WriteOnDisk( ( char* )( myRefOnObject.c_str() ) );
                      aDataset->CloseOnDisk();
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
            if ( shapeRefFound )
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
		for ( ; it->More(); it->Next() ) {
		  SALOMEDS::SObject_var mySObject = it->Value();
		  CORBA::Object_var aSubObject = SObjectToObject( mySObject );
		  if ( !CORBA::is_nil( aSubObject ) ) {
		    SMESH_GroupBase_i* myGroupImpl =
                      dynamic_cast<SMESH_GroupBase_i*>( GetServant( aSubObject ).in() );
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

		      if(MYDEBUG) MESSAGE( "VSR - SMESH_Gen_i::Save(): saving group with StoreName = "
                              << grpName << " to MED file" );
		      SMESHDS_GroupBase* aGrpBaseDS =
                        myLocMesh.GetGroup( myGroupImpl->GetLocalID() )->GetGroupDS();
		      aGrpBaseDS->SetStoreName( grpName );

		      // Pass SMESHDS_Group to MED writer 
		      SMESHDS_Group* aGrpDS = dynamic_cast<SMESHDS_Group*>( aGrpBaseDS );
                      if ( aGrpDS )
                        myWriter.AddGroup( aGrpDS );

                      // write reference on a shape if exists
                      SMESHDS_GroupOnGeom* aGeomGrp =
                        dynamic_cast<SMESHDS_GroupOnGeom*>( aGrpBaseDS );
                      if ( aGeomGrp ) {
                        SALOMEDS::SObject_var mySubRef, myShape;
                        if (mySObject->FindSubObject( GetRefOnShapeTag(), mySubRef ) &&
                            mySubRef->ReferencedObject( myShape ) &&
                            !CORBA::is_nil( myShape->GetObject() ))
                        {
                          string myRefOnObject = myShape->GetID();
                          if ( myRefOnObject.length() > 0 ) {
                            char aRefName[ 30 ];
                            sprintf( aRefName, "Ref on shape %d", anId);
                            aSize[ 0 ] = myRefOnObject.length() + 1;
                            aDataset = new HDFdataset(aRefName, aGroup, HDF_STRING, aSize, 1);
                            aDataset->CreateOnDisk();
                            aDataset->WriteOnDisk( ( char* )( myRefOnObject.c_str() ) );
                            aDataset->CloseOnDisk();
                          }
                        }
                        else // shape ref is invalid:
                        {
                          // save a group on geometry as ordinary group
                          myWriter.AddGroup( aGeomGrp );
                        }
                      }
		    }
		  }
		}
		aGroup->CloseOnDisk();
	      }
	    } // loop on groups 

	    if ( strcmp( strHasData.c_str(), "1" ) == 0 )
            {
              // Flush current mesh information into MED file
	      myWriter.Perform();

              // maybe a shape was deleted in the study
              if ( !shapeRefFound && !mySMESHDSMesh->ShapeToMesh().IsNull() ) {
                TopoDS_Shape nullShape;
                myLocMesh.ShapeToMesh( nullShape ); // remove shape referring data
              }

              // Store node positions on sub-shapes (SMDS_Position):

              if ( !mySMESHDSMesh->SubMeshes().empty() )
              {
                aGroup = new HDFgroup( "Node Positions", aTopGroup );
                aGroup->CreateOnDisk();

                // in aGroup, create 5 datasets to contain:
                // "Nodes on Edges" - ID of node on edge
                // "Edge positions" - U parameter on node on edge
                // "Nodes on Faces" - ID of node on face
                // "Face U positions" - U parameter of node on face
                // "Face V positions" - V parameter of node on face

                // Find out nb of nodes on edges and faces
                // Collect corresponing sub-meshes
                int nbEdgeNodes = 0, nbFaceNodes = 0;
                list<SMESHDS_SubMesh*> aEdgeSM, aFaceSM;
                // loop on SMESHDS_SubMesh'es
                const map<int,SMESHDS_SubMesh*>& aSubMeshes = mySMESHDSMesh->SubMeshes();
                map<int,SMESHDS_SubMesh*>::const_iterator itSubM ( aSubMeshes.begin() );
                for ( ; itSubM != aSubMeshes.end() ; itSubM++ )
                {
                  SMESHDS_SubMesh* aSubMesh = (*itSubM).second;
                  if ( aSubMesh->IsComplexSubmesh() )
                    continue; // submesh containing other submeshs
                  int nbNodes = aSubMesh->NbNodes();
                  if ( nbNodes == 0 ) continue;

                  int aShapeID = (*itSubM).first;
                  int aShapeType = mySMESHDSMesh->IndexToShape( aShapeID ).ShapeType();
                  // write only SMDS_FacePosition and SMDS_EdgePosition
                  switch ( aShapeType ) {
                  case TopAbs_FACE:
                    nbFaceNodes += nbNodes;
                    aFaceSM.push_back( aSubMesh );
                    break;
                  case TopAbs_EDGE:
                    nbEdgeNodes += nbNodes;
                    aEdgeSM.push_back( aSubMesh );
                    break;
                  default:
                    continue;
                  }
                }
                // Treat positions on edges or faces
                for ( int onFace = 0; onFace < 2; onFace++ )
                {
                  // Create arrays to store in datasets
                  int iNode = 0, nbNodes = ( onFace ? nbFaceNodes : nbEdgeNodes );
                  if (!nbNodes) continue;
                  int* aNodeIDs = new int [ nbNodes ];
                  double* aUPos = new double [ nbNodes ];
                  double* aVPos = ( onFace ? new double[ nbNodes ] : 0 );

                  // Fill arrays
                  // loop on sub-meshes
                  list<SMESHDS_SubMesh*> * pListSM = ( onFace ? &aFaceSM : &aEdgeSM );
                  list<SMESHDS_SubMesh*>::iterator itSM = pListSM->begin();
                  for ( ; itSM != pListSM->end(); itSM++ )
                  {
                    SMESHDS_SubMesh* aSubMesh = (*itSM);
                    if ( aSubMesh->IsComplexSubmesh() )
                      continue; // submesh containing other submeshs

                    SMDS_NodeIteratorPtr itNode = aSubMesh->GetNodes();
                    // loop on nodes in aSubMesh
                    while ( itNode->more() )
                    {
                      //node ID
                      const SMDS_MeshNode* node = itNode->next();
                      aNodeIDs [ iNode ] = node->GetID();

                      // Position
                      const SMDS_PositionPtr pos = node->GetPosition();
                      if ( onFace ) { // on FACE
                        const SMDS_FacePosition* fPos =
                          dynamic_cast<const SMDS_FacePosition*>( pos.get() );
                        if ( fPos ) {
                          aUPos[ iNode ] = fPos->GetUParameter();
                          aVPos[ iNode ] = fPos->GetVParameter();
                          iNode++;
                        }
                        else
                          nbNodes--;
                      }
                      else { // on EDGE
                        const SMDS_EdgePosition* ePos =
                          dynamic_cast<const SMDS_EdgePosition*>( pos.get() );
                        if ( ePos ) {
                          aUPos[ iNode ] = ePos->GetUParameter();
                          iNode++;
                        }
                        else
                          nbNodes--;
                      }
                    } // loop on nodes in aSubMesh
                  } // loop on sub-meshes

                  // Write datasets
                  if ( nbNodes )
                  {
                    aSize[ 0 ] = nbNodes;
                    // IDS
                    string aDSName( onFace ? "Nodes on Faces" : "Nodes on Edges");
                    aDataset = new HDFdataset( (char*)aDSName.c_str(), aGroup, HDF_INT32, aSize, 1 );
                    aDataset->CreateOnDisk();
                    aDataset->WriteOnDisk( aNodeIDs );
                    aDataset->CloseOnDisk();

                    // U Positions
                    aDSName = ( onFace ? "Face U positions" : "Edge positions");
                    aDataset = new HDFdataset( (char*)aDSName.c_str(), aGroup, HDF_FLOAT64, aSize, 1);
                    aDataset->CreateOnDisk();
                    aDataset->WriteOnDisk( aUPos );
                    aDataset->CloseOnDisk();
                    // V Positions
                    if ( onFace ) {
                      aDataset = new HDFdataset( "Face V positions", aGroup, HDF_FLOAT64, aSize, 1);
                      aDataset->CreateOnDisk();
                      aDataset->WriteOnDisk( aVPos );
                      aDataset->CloseOnDisk();
                    }
                  }
                  delete [] aNodeIDs;
                  delete [] aUPos;
                  if ( aVPos ) delete [] aVPos;

                } // treat positions on edges or faces

                // close "Node Positions" group
                aGroup->CloseOnDisk(); 

              } // if ( there are submeshes in SMESHDS_Mesh )
            } // if ( hasData )

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
  if(MYDEBUG) MESSAGE( "SMESH_Gen_i::SaveASCII" );
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

  if ( myCurrentStudy->_is_nil() || 
       theComponent->GetStudy()->StudyId() != myCurrentStudy->StudyId() )
    SetCurrentStudy( theComponent->GetStudy() );

/*  if( !theComponent->_is_nil() )
  {
    //SALOMEDS::Study_var aStudy = SALOMEDS::Study::_narrow( theComponent->GetStudy() );
    if( !myCurrentStudy->FindComponent( "GEOM" )->_is_nil() )
      loadGeomData( myCurrentStudy->FindComponent( "GEOM" ) );
  }*/

  StudyContext* myStudyContext = GetCurrentStudyContext();
  
  // Get temporary files location
  TCollection_AsciiString tmpDir =
    isMultiFile ? TCollection_AsciiString( ( char* )theURL ) : ( char* )SALOMEDS_Tool::GetTmpDir().c_str();

  // Convert the stream into sequence of files to process
  SALOMEDS::ListOfFileNames_var aFileSeq = SALOMEDS_Tool::PutStreamToFiles( theStream,
                                                                            tmpDir.ToCString(),
									    isMultiFile );
  TCollection_AsciiString aStudyName( "" );
  if ( isMultiFile ) 
    aStudyName = ( (char*)SALOMEDS_Tool::GetNameFromPath( myCurrentStudy->URL() ).c_str() );

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
    INFOS( "Load(): " << filename << " not found!" );
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
	      delete [] hypname_str;
	      aDataset->CloseOnDisk();
	    }
	    // --> get hypothesis plugin library name
	    if ( strcmp( name_of_subgroup, "LibName"  ) == 0 ) {
	      aDataset = new HDFdataset( name_of_subgroup, aGroup );
	      aDataset->OpenOnDisk();
	      size = aDataset->GetSize();
	      char* libname_str = new char[ size ];
	      aDataset->ReadFromDisk( libname_str );
	      if(MYDEBUG) SCRUTE( libname_str );
	      libname = string( libname_str );
	      delete [] libname_str;
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
	      delete [] hypdata_str;
	      aDataset->CloseOnDisk();
	    }
	  }
	  // close hypothesis HDF group
	  aGroup->CloseOnDisk();

	  // --> restore hypothesis from data
	  if ( id > 0 && !hypname.empty()/* && !hypdata.empty()*/ ) { // VSR : persistent data can be empty
	    if(MYDEBUG) MESSAGE("VSR - load hypothesis : id = " << id <<
                    ", name = " << hypname.c_str() << ", persistent string = " << hypdata.c_str());
            SMESH::SMESH_Hypothesis_var myHyp;
	    
	    try { // protect persistence mechanism against exceptions
	      myHyp = this->createHypothesis( hypname.c_str(), libname.c_str() );
	    }
	    catch (...) {
	      INFOS( "Exception during hypothesis creation" );
	    }

	    SMESH_Hypothesis_i* myImpl = dynamic_cast<SMESH_Hypothesis_i*>( GetServant( myHyp ).in() );
	    if ( myImpl ) {
	      myImpl->LoadFrom( hypdata.c_str() );
	      string iorString = GetORB()->object_to_string( myHyp );
	      int newId = myStudyContext->findId( iorString );
	      myStudyContext->mapOldToNew( id, newId );
	    }
	    else
	      if(MYDEBUG) MESSAGE( "VSR - SMESH_Gen::Load - can't get servant" );
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
	      delete [] hypname_str;
	      aDataset->CloseOnDisk();
	    }
	    // --> get algorithm plugin library name
	    if ( strcmp( name_of_subgroup, "LibName"  ) == 0 ) {
	      aDataset = new HDFdataset( name_of_subgroup, aGroup );
	      aDataset->OpenOnDisk();
	      size = aDataset->GetSize();
	      char* libname_str = new char[ size ];
	      aDataset->ReadFromDisk( libname_str );
	      if(MYDEBUG) SCRUTE( libname_str );
	      libname = string( libname_str );
	      delete [] libname_str;
	      aDataset->CloseOnDisk();
	    }
	    // --> get algorithm data
	    if ( strcmp( name_of_subgroup, "Data"  ) == 0 ) {
	      aDataset = new HDFdataset( name_of_subgroup, aGroup );
	      aDataset->OpenOnDisk();
	      size = aDataset->GetSize();
	      char* hypdata_str = new char[ size ];
	      aDataset->ReadFromDisk( hypdata_str );
	      if(MYDEBUG) SCRUTE( hypdata_str );
	      hypdata = string( hypdata_str );
	      delete [] hypdata_str;
	      aDataset->CloseOnDisk();
	    }
	  }
	  // close algorithm HDF group
	  aGroup->CloseOnDisk();
	  
	  // --> restore algorithm from data
	  if ( id > 0 && !hypname.empty()/* && !hypdata.empty()*/ ) { // VSR : persistent data can be empty
	    if(MYDEBUG) MESSAGE("VSR - load algo : id = " << id <<
                    ", name = " << hypname.c_str() << ", persistent string = " << hypdata.c_str());
            SMESH::SMESH_Hypothesis_var myHyp;
	    	    
	    try { // protect persistence mechanism against exceptions
	      myHyp = this->createHypothesis( hypname.c_str(), libname.c_str() );
	    }
	    catch (...) {
	      INFOS( "Exception during hypothesis creation" );
	    }
	    
	    SMESH_Hypothesis_i* myImpl = dynamic_cast<SMESH_Hypothesis_i*>( GetServant( myHyp ).in() );
	    if ( myImpl ) {
	      myImpl->LoadFrom( hypdata.c_str() );
	      string iorString = GetORB()->object_to_string( myHyp );
	      int newId = myStudyContext->findId( iorString );
	      myStudyContext->mapOldToNew( id, newId );
	    }
	    else
	      if(MYDEBUG) MESSAGE( "VSR - SMESH_Gen::Load - can't get servant" );
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
	  if(MYDEBUG) MESSAGE( "VSR - load mesh : id = " << id );
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
	      myReader.Perform();
	      hasData = true;
	    }
	  }

	  // try to read and set reference to shape
	  GEOM::GEOM_Object_var aShapeObject;
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
	      //loadGeomData( shapeSO->GetFatherComponent() );

	      CORBA::Object_var shapeObject = SObjectToObject( shapeSO );
	      if ( !CORBA::is_nil( shapeObject ) ) {
		aShapeObject = GEOM::GEOM_Object::_narrow( shapeObject );
		if ( !aShapeObject->_is_nil() )
		  myNewMeshImpl->SetShape( aShapeObject );
	      }
	    }
	  }

	  // try to get applied algorithms
	  if ( aTopGroup->ExistInternalObject( "Applied Algorithms" ) ) {
	    aGroup = new HDFgroup( "Applied Algorithms", aTopGroup );
	    aGroup->OpenOnDisk();
	    // get number of applied algorithms
	    int aNbSubObjects = aGroup->nInternalObjects(); 
	    if(MYDEBUG) MESSAGE( "VSR - number of applied algos " << aNbSubObjects );
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
		    if ( !anHyp->_is_nil() && !aShapeObject->_is_nil() )
		      myNewMeshImpl->addHypothesis( aShapeObject, anHyp );
		  }
		}
	      }
	    }
	    aGroup->CloseOnDisk();
	  }

	  // try to get applied hypotheses
	  if ( aTopGroup->ExistInternalObject( "Applied Hypotheses" ) ) {
	    aGroup = new HDFgroup( "Applied Hypotheses", aTopGroup );
	    aGroup->OpenOnDisk();
	    // get number of applied hypotheses
	    int aNbSubObjects = aGroup->nInternalObjects(); 
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
		    if ( !anHyp->_is_nil() && !aShapeObject->_is_nil() )
		      myNewMeshImpl->addHypothesis( aShapeObject, anHyp );
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
	    else if ( j == GetSubMeshOnWireTag() )
	      strcpy( name_meshgroup, "SubMeshes On Wire" );
	    else if ( j == GetSubMeshOnFaceTag() )
	      strcpy( name_meshgroup, "SubMeshes On Face" );
	    else if ( j == GetSubMeshOnShellTag() )
	      strcpy( name_meshgroup, "SubMeshes On Shell" );
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
		  GEOM::GEOM_Object_var aSubShapeObject;
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
			aSubShapeObject = GEOM::GEOM_Object::_narrow( subShapeObject );
			if ( !aSubShapeObject->_is_nil() )
			  aSubMesh = SMESH::SMESH_subMesh::_duplicate
                            ( myNewMeshImpl->createSubMesh( aSubShapeObject ) );
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
//		    if(MYDEBUG) MESSAGE("VSR - SMESH_Gen_i::Load(): loading from MED file submesh with ID = " <<
//                            subid << " for subshape # " << anInternalSubmeshId);
//		    SMESHDS_SubMesh* aSubMeshDS =
//                      myNewMeshImpl->_mapSubMesh[anInternalSubmeshId]->CreateSubMeshDS();
//		    if ( !aSubMeshDS ) {
//		      if(MYDEBUG) MESSAGE("VSR - SMESH_Gen_i::Load(): FAILED to create a submesh for subshape # " <<
//                              anInternalSubmeshId << " in current mesh!");
//		    }
//		    else
//		      myReader.GetSubMesh( aSubMeshDS, subid );
//		  }
		    
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
			    if ( !anHyp->_is_nil() && !aShapeObject->_is_nil() )
			      myNewMeshImpl->addHypothesis( aSubShapeObject, anHyp );
			  }
			}
		      }
		    }
		    // close "applied algorithms" HDF group
		    aSubSubGroup->CloseOnDisk();
		  }
		  
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
			    if ( !anHyp->_is_nil() && !aShapeObject->_is_nil() )
			      myNewMeshImpl->addHypothesis( aSubShapeObject, anHyp );
			  }
			}
		      }
		    }
		    // close "applied hypotheses" HDF group
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

	  if(hasData) {
	    // Read sub-meshes from MED
	    if(MYDEBUG) MESSAGE("Create all sub-meshes");
	    myReader.CreateAllSubMeshes();


            // Read node positions on sub-shapes (SMDS_Position)

            if ( aTopGroup->ExistInternalObject( "Node Positions" ))
            {
              // There are 5 datasets to read:
              // "Nodes on Edges" - ID of node on edge
              // "Edge positions" - U parameter on node on edge
              // "Nodes on Faces" - ID of node on face
              // "Face U positions" - U parameter of node on face
              // "Face V positions" - V parameter of node on face
              char* aEid_DSName = "Nodes on Edges";
              char* aEu_DSName  = "Edge positions";
              char* aFu_DSName  = "Face U positions";
              //char* aFid_DSName = "Nodes on Faces";
              //char* aFv_DSName  = "Face V positions";

              // data to retrieve
              int nbEids = 0, nbFids = 0;
              int *aEids = 0, *aFids  = 0;
              double *aEpos = 0, *aFupos = 0, *aFvpos = 0;

              // open a group
              aGroup = new HDFgroup( "Node Positions", aTopGroup ); 
              aGroup->OpenOnDisk();

              // loop on 5 data sets
              int aNbObjects = aGroup->nInternalObjects();
              for ( int i = 0; i < aNbObjects; i++ )
              {
                // identify dataset
                char aDSName[ HDF_NAME_MAX_LEN+1 ];
                aGroup->InternalObjectIndentify( i, aDSName );
                // read data
                aDataset = new HDFdataset( aDSName, aGroup );
                aDataset->OpenOnDisk();
                if ( aDataset->GetType() == HDF_FLOAT64 ) // Positions
                {
                  double* pos = new double [ aDataset->GetSize() ];
                  aDataset->ReadFromDisk( pos );
                  // which one?
                  if ( strncmp( aDSName, aEu_DSName, strlen( aEu_DSName )) == 0 )
                    aEpos = pos;
                  else if ( strncmp( aDSName, aFu_DSName, strlen( aFu_DSName )) == 0 )
                    aFupos = pos;
                  else
                    aFvpos = pos;
                }
                else // NODE IDS
                {
		  int aSize = aDataset->GetSize();

                  // for reading files, created from 18.07.2005 till 10.10.2005
                  if (aDataset->GetType() == HDF_STRING)
                    aSize /= sizeof(int);

		  int* ids = new int [aSize];
                  aDataset->ReadFromDisk( ids );
                  // on face or nodes?
                  if ( strncmp( aDSName, aEid_DSName, strlen( aEid_DSName )) == 0 ) {
                    aEids = ids;
                    nbEids = aSize;
                  }
                  else {
                    aFids = ids;
                    nbFids = aSize;
                  }
                }
              } // loop on 5 datasets

              // Set node positions on edges or faces
              for ( int onFace = 0; onFace < 2; onFace++ )
              {
                int nbNodes = ( onFace ? nbFids : nbEids );
                if ( nbNodes == 0 ) continue;
                int* aNodeIDs = ( onFace ? aFids : aEids );
                double* aUPos = ( onFace ? aFupos : aEpos );
                double* aVPos = ( onFace ? aFvpos : 0 );
                // loop on node IDs
                for ( int iNode = 0; iNode < nbNodes; iNode++ )
                {
                  const SMDS_MeshNode* node = mySMESHDSMesh->FindNode( aNodeIDs[ iNode ]);
                  ASSERT( node );
                  SMDS_PositionPtr aPos = node->GetPosition();
                  ASSERT( aPos )
                  if ( onFace ) {
                    ASSERT( aPos->GetTypeOfPosition() == SMDS_TOP_FACE );
                    SMDS_FacePosition* fPos = const_cast<SMDS_FacePosition*>
                      ( static_cast<const SMDS_FacePosition*>( aPos.get() ));
                    fPos->SetUParameter( aUPos[ iNode ]);
                    fPos->SetVParameter( aVPos[ iNode ]);
                  }
                  else {
                    ASSERT( aPos->GetTypeOfPosition() == SMDS_TOP_EDGE );
                    SMDS_EdgePosition* fPos = const_cast<SMDS_EdgePosition*>
                      ( static_cast<const SMDS_EdgePosition*>( aPos.get() ));
                    fPos->SetUParameter( aUPos[ iNode ]);
                  }
                }
              }
              if ( aEids ) delete [] aEids;
              if ( aFids ) delete [] aFids;
              if ( aEpos ) delete [] aEpos;
              if ( aFupos ) delete [] aFupos;
              if ( aFvpos ) delete [] aFvpos;
              
              aGroup->CloseOnDisk();

            } // if ( aTopGroup->ExistInternalObject( "Node Positions" ) )
	  } // if ( hasData )

          // Recompute State (as computed sub-meshes are restored from MED)
	  if ( !aShapeObject->_is_nil() ) {
	    MESSAGE("Compute State Engine ...");
	    TopoDS_Shape myLocShape = GeomObjectToShape( aShapeObject );
	    myNewMeshImpl->GetImpl().GetSubMesh(myLocShape)->ComputeStateEngine
              (SMESH_subMesh::SUBMESH_RESTORED);
	    MESSAGE("Compute State Engine finished");
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

		  // Try to find a shape reference
                  TopoDS_Shape aShape;
                  char aRefName[ 30 ];
                  sprintf( aRefName, "Ref on shape %d", subid);
                  if ( aGroup->ExistInternalObject( aRefName ) ) {
                    // load mesh "Ref on shape" - it's an entry to SObject
                    aDataset = new HDFdataset( aRefName, aGroup );
                    aDataset->OpenOnDisk();
                    size = aDataset->GetSize();
                    char* refFromFile = new char[ size ];
                    aDataset->ReadFromDisk( refFromFile );
                    aDataset->CloseOnDisk();
                    if ( strlen( refFromFile ) > 0 ) {
                      SALOMEDS::SObject_var shapeSO = myCurrentStudy->FindObjectID( refFromFile );
                      CORBA::Object_var shapeObject = SObjectToObject( shapeSO );
                      if ( !CORBA::is_nil( shapeObject ) ) {
                        aShapeObject = GEOM::GEOM_Object::_narrow( shapeObject );
                        if ( !aShapeObject->_is_nil() )
                          aShape = GeomObjectToShape( aShapeObject );
                      }
                    }
                  }
		  // Create group servant
                  SMESH::ElementType type = (SMESH::ElementType)(ii - GetNodeGroupsTag() + 1);
		  SMESH::SMESH_GroupBase_var aNewGroup = SMESH::SMESH_GroupBase::_duplicate
                    ( myNewMeshImpl->createGroup( type, nameFromFile, aShape ) );
		  // Obtain a SMESHDS_Group object 
		  if ( aNewGroup->_is_nil() )
		    continue;

		  string iorSubString = GetORB()->object_to_string( aNewGroup );
		  int newSubId = myStudyContext->findId( iorSubString );
		  myStudyContext->mapOldToNew( subid, newSubId );

		  SMESH_GroupBase_i* aGroupImpl =
                    dynamic_cast<SMESH_GroupBase_i*>( GetServant( aNewGroup ).in() );
		  if ( !aGroupImpl )
		    continue;

		  SMESH_Group* aLocalGroup  = myLocMesh.GetGroup( aGroupImpl->GetLocalID() );
		  if ( !aLocalGroup )
		    continue;

		  SMESHDS_GroupBase* aGroupBaseDS = aLocalGroup->GetGroupDS();
		  aGroupBaseDS->SetStoreName( name_dataset );

		  // Fill group with contents from MED file
                  SMESHDS_Group* aGrp = dynamic_cast<SMESHDS_Group*>( aGroupBaseDS );
                  if ( aGrp )
                    myReader.GetGroup( aGrp );
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
  if(MYDEBUG) MESSAGE( "SMESH_Gen_i::LoadASCII" );
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
  if(MYDEBUG) MESSAGE( "SMESH_Gen_i::Close" );

  // Clear study contexts data
  int studyId = GetCurrentStudyID();
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
  if(MYDEBUG) MESSAGE( "SMESH_Gen_i::ComponentDataType" );
  return CORBA::string_dup( "SMESH" );
}

    
//=============================================================================
/*!
 *  SMESH_Gen_i::IORToLocalPersistentID
 *  
 *  Transform data from transient form to persistent
 */
//=============================================================================

char* SMESH_Gen_i::IORToLocalPersistentID( SALOMEDS::SObject_ptr /*theSObject*/,
					   const char*           IORString,
					   CORBA::Boolean        /*isMultiFile*/,
					   CORBA::Boolean        /*isASCII*/ )
{
  if(MYDEBUG) MESSAGE( "SMESH_Gen_i::IORToLocalPersistentID" );
  StudyContext* myStudyContext = GetCurrentStudyContext();
  
  if ( myStudyContext && strcmp( IORString, "" ) != 0 ) {
    int anId = myStudyContext->findId( IORString );
    if ( anId ) {
      if(MYDEBUG) MESSAGE( "VSR " << anId )
      char strId[ 20 ];
      sprintf( strId, "%d", anId );
      return  CORBA::string_dup( strId );
    }
  }
  return CORBA::string_dup( "" );
}

//=============================================================================
/*!
 *  SMESH_Gen_i::LocalPersistentIDToIOR
 *
 *  Transform data from persistent form to transient
 */
//=============================================================================

char* SMESH_Gen_i::LocalPersistentIDToIOR( SALOMEDS::SObject_ptr /*theSObject*/,
					   const char*           aLocalPersistentID,
					   CORBA::Boolean        /*isMultiFile*/,
					   CORBA::Boolean        /*isASCII*/ )
{
  if(MYDEBUG) MESSAGE( "SMESH_Gen_i::LocalPersistentIDToIOR(): id = " << aLocalPersistentID );
  StudyContext* myStudyContext = GetCurrentStudyContext();

  if ( myStudyContext && strcmp( aLocalPersistentID, "" ) != 0 ) {
    int anId = atoi( aLocalPersistentID );
    return CORBA::string_dup( myStudyContext->getIORbyOldId( anId ).c_str() );
  }
  return CORBA::string_dup( "" );
}

//=======================================================================
//function : RegisterObject
//purpose  : 
//=======================================================================

int SMESH_Gen_i::RegisterObject(CORBA::Object_ptr theObject)
{
  StudyContext* myStudyContext = GetCurrentStudyContext();
  if ( myStudyContext && !CORBA::is_nil( theObject )) {
    string iorString = GetORB()->object_to_string( theObject );
    return myStudyContext->addObject( iorString );
  }
  return 0;
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
    if(MYDEBUG) MESSAGE( "PortableServer::ObjectId* SMESHEngine_factory()" );
    if(MYDEBUG) SCRUTE(interfaceName);
    SMESH_Gen_i* aSMESHGen = new SMESH_Gen_i(orb, poa, contId, instanceName, interfaceName);
    return aSMESHGen->getId() ;
  }
}
