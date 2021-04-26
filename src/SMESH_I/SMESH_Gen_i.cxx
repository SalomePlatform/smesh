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
//  File   : SMESH_Gen_i.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRep_Tool.hxx>
#include <OSD.hxx>
#include <TColStd_MapOfAsciiString.hxx>
#include <TCollection_AsciiString.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_CompSolid.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <gp_Pnt.hxx>

#ifdef WIN32
 #include <windows.h>
 #include <process.h>
#else
 #include <dlfcn.h>
 #include <libgen.h> // for basename function
#endif

#ifdef WIN32
 #define LibHandle HMODULE
 #define LoadLib( name ) LoadLibrary( name )
 #define GetProc GetProcAddress
 #define UnLoadLib( handle ) FreeLibrary( handle );
#else // WIN32
 #define LibHandle void*
 #ifdef DYNLOAD_LOCAL
  #define LoadLib( name ) dlopen( name, RTLD_LAZY | RTLD_LOCAL )
 #else // DYNLOAD_LOCAL
  #define LoadLib( name ) dlopen( name, RTLD_LAZY | RTLD_GLOBAL )
 #endif // DYNLOAD_LOCAL
 #define GetProc dlsym
 #define UnLoadLib( handle ) dlclose( handle );
#endif // WIN32

#include "SMESH_Gen_i.hxx"
#include "SMESH_version.h"

#include "DriverMED_W_SMESHDS_Mesh.h"
#include "DriverMED_R_SMESHDS_Mesh.h"
#ifdef WITH_CGNS
#include "DriverCGNS_Read.hxx"
#endif
#include "MED_Factory.hxx"
#include "SMDS_EdgePosition.hxx"
#include "SMDS_FacePosition.hxx"
#include "SMDS_SetIterator.hxx"
#include "SMDS_SpacePosition.hxx"
#include "SMDS_VertexPosition.hxx"
#include "SMESHDS_Document.hxx"
#include "SMESHDS_Group.hxx"
#include "SMESHDS_GroupOnGeom.hxx"
#include "SMESH_Algo_i.hxx"
#include "SMESH_File.hxx"
#include "SMESH_Group.hxx"
#include "SMESH_Group_i.hxx"
#include "SMESH_Hypothesis.hxx"
#include "SMESH_Hypothesis_i.hxx"
#include "SMESH_Mesh.hxx"
#include "SMESH_MeshEditor.hxx"
#include "SMESH_Mesh_i.hxx"
#include "SMESH_PreMeshInfo.hxx"
#include "SMESH_PythonDump.hxx"
#include "SMESH_ControlsDef.hxx"

// to pass CORBA exception through SMESH_TRY
#define SMY_OWN_CATCH catch( SALOME::SALOME_Exception& se ) { throw se; }
#include "SMESH_TryCatch.hxx" // to include after OCC headers!

#include CORBA_SERVER_HEADER(SMESH_Group)
#include CORBA_SERVER_HEADER(SMESH_Filter)
#include CORBA_SERVER_HEADER(SMESH_MeshEditor)


#include <GEOMImpl_Types.hxx>
#include <GEOM_Client.hxx>

#include <Basics_Utils.hxx>
#include <Basics_DirUtils.hxx>
#include <HDFOI.hxx>
#include <OpUtil.hxx>
#include <SALOMEDS_Tool.hxx>
#include <SALOME_Container_i.hxx>
#include <SALOME_LifeCycleCORBA.hxx>
#include <SALOME_NamingService.hxx>
#include <Utils_CorbaException.hxx>
#include <Utils_ExceptHandlers.hxx>
#include <Utils_SINGLETON.hxx>
#include <utilities.h>

#include CORBA_CLIENT_HEADER(SALOME_ModuleCatalog)
#include CORBA_CLIENT_HEADER(SALOME_Session)

// helpers about SALOME::GenericObj
#include <SALOMEDS_wrap.hxx>
#include <SALOMEDS_Attributes_wrap.hxx>
#include <GEOM_wrap.hxx>

#include <map>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <memory>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>

using namespace std;
using SMESH::TPythonDump;
using SMESH::TVar;

#define NUM_TMP_FILES 2

#ifdef _DEBUG_
static int MYDEBUG = 0;
#else
static int MYDEBUG = 0;
#endif

// Static variables definition
GEOM::GEOM_Gen_var      SMESH_Gen_i::myGeomGen;
CORBA::ORB_var          SMESH_Gen_i::myOrb;
PortableServer::POA_var SMESH_Gen_i::myPoa;
SALOME_NamingService_Abstract*   SMESH_Gen_i::myNS  = nullptr;
SALOME_LifeCycleCORBA*  SMESH_Gen_i::myLCC = nullptr;
SMESH_Gen_i*            SMESH_Gen_i::mySMESHGen = nullptr;


const int nbElemPerDiagonal = 10;

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
  catch (PortableServer::POA::ObjectNotActive &ex)
  {
    INFOS("GetServant: ObjectNotActive");
    return NULL;
  }
  catch (PortableServer::POA::WrongAdapter &ex)
  {
    INFOS("GetServant: WrongAdapter: OK when several servants used to build several mesh in parallel...");
    return NULL;
  }
  catch (PortableServer::POA::WrongPolicy &ex)
  {
    INFOS("GetServant: WrongPolicy");
    return NULL;
  }
  catch (...)
  {
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
  SALOMEDS::GenericAttribute_wrap anAttr;
  CORBA::Object_var anObj;
  if ( !theSObject->_is_nil() ) {
    try {
      if( theSObject->FindAttribute( anAttr.inout(), "AttributeIOR" ) ) {
        SALOMEDS::AttributeIOR_wrap anIOR  = anAttr;
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

SALOME_NamingService_Abstract* SMESH_Gen_i::GetNS()
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

SALOME_LifeCycleCORBA*  SMESH_Gen_i::GetLCC()
{
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

GEOM::GEOM_Gen_var SMESH_Gen_i::GetGeomEngine( GEOM::GEOM_Object_ptr go )
{
  GEOM::GEOM_Gen_ptr gen = GEOM::GEOM_Gen::_nil();
  if ( !CORBA::is_nil( go ))
    gen = go->GetGen();
  return gen;
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
                          const char*               interfaceName,
                          bool                      checkNS)
  : Engines_Component_i( orb, poa, contId, instanceName, interfaceName, false, checkNS )
{

  myOrb = CORBA::ORB::_duplicate(orb);
  myPoa = PortableServer::POA::_duplicate(poa);

  _thisObj = this ;
  _id = myPoa->activate_object( _thisObj );

  myStudyContext = new StudyContext;

  myIsEmbeddedMode = false;
  myIsEnablePublish = true;
  myShapeReader = NULL;  // shape reader
  mySMESHGen = this;
  myIsHistoricalPythonDump = true;
  myToForgetMeshDataOnHypModif = false;

  // set it in standalone mode only
  //OSD::SetSignal( true );

  // 0020605: EDF 1190 SMESH: Display performance. 80 seconds for 52000 cells.
  // find out mode (embedded or standalone) here else
  // meshes created before calling SMESH_Client::GetSMESHGen(), which calls
  // SMESH_Gen_i::SetEmbeddedMode(), have wrong IsEmbeddedMode flag
  if(checkNS)
  {
    if ( SALOME_NamingService_Abstract* ns = GetNS() )
    {
      CORBA::Object_var obj = ns->Resolve( "/Kernel/Session" );
      SALOME::Session_var session = SALOME::Session::_narrow( obj ) ;
      if ( !session->_is_nil() )
      {
        CORBA::String_var str_host = session->getHostname();
        CORBA::Long        s_pid = session->getPID();
        string my_host = Kernel_Utils::GetHostname();
#ifdef WIN32
        long    my_pid = (long)_getpid();
#else
        long    my_pid = (long) getpid();
#endif
        SetEmbeddedMode( s_pid == my_pid && my_host == str_host.in() );
      }
    }
  }
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
  // delete hypothesis creators
  map<string, GenericHypothesisCreator_i*>::iterator itHyp, itHyp2;
  for (itHyp = myHypCreatorMap.begin(); itHyp != myHypCreatorMap.end(); itHyp++)
  {
    // same creator can be mapped under different names
    GenericHypothesisCreator_i* creator = (*itHyp).second;
    if ( !creator )
      continue;
    delete creator;
    for (itHyp2 = itHyp; itHyp2 != myHypCreatorMap.end(); itHyp2++)
      if ( creator == (*itHyp2).second )
        (*itHyp2).second = 0;
  }
  myHypCreatorMap.clear();

  // Clear study contexts data
  delete myStudyContext;

  // delete shape reader
  if ( myShapeReader )
    delete myShapeReader;
}

//=============================================================================
/*!
 *  SMESH_Gen_i::getHypothesisCreator
 *
 *  Get hypothesis creator
 */
//=============================================================================

GenericHypothesisCreator_i* SMESH_Gen_i::getHypothesisCreator(const char* theHypName,
                                                              const char* theLibName,
                                                              std::string& thePlatformLibName)
{
  std::string aPlatformLibName;
  /* It's Need to translate lib name for WIN32 or X platform */
  if ( theLibName && theLibName[0] != '\0'  )
  {
    int libNameLen = strlen(theLibName);
    //check for old format "libXXXXXXX.so"
    if (libNameLen > 7 &&
        !strncmp( theLibName, "lib", 3 ) &&
        !strcmp( theLibName+libNameLen-3, ".so" ))
    {
      //the old format
#if defined(WIN32)
      aPlatformLibName = std::string( theLibName+3, libNameLen-6 ) + ".dll";
#elif defined(__APPLE__)
      aPlatformLibName = std::string( theLibName, libNameLen-3 ) + ".dylib";
#else
      aPlatformLibName = theLibName;
#endif
    }
    else
    {
      //try to use new format
#if defined(WIN32)
      aPlatformLibName = theLibName;
      aPlatformLibName += ".dll";
#elif defined(__APPLE__)
      aPlatformLibName = std::string( "lib" ) + std::string( theLibName ) + ".dylib";
#else
      aPlatformLibName = std::string( "lib" ) + std::string( theLibName ) + ".so";
#endif
    }
  }
  thePlatformLibName = aPlatformLibName;

  Unexpect aCatch(SALOME_SalomeException);
  if(MYDEBUG) MESSAGE( "Create Hypothesis <" << theHypName << "> from " << aPlatformLibName);

  typedef GenericHypothesisCreator_i* (*GetHypothesisCreator)(const char* );
  GenericHypothesisCreator_i* aCreator;
  try
  {
    // check, if creator for this hypothesis type already exists
    if (myHypCreatorMap.find(string(theHypName)) == myHypCreatorMap.end())
    {
      // load plugin library
      if(MYDEBUG) MESSAGE("Loading server meshers plugin library ...");
#ifdef WIN32
#ifdef UNICODE
      const wchar_t* path = Kernel_Utils::decode_s(aPlatformLibName);
#else
      const char* path = aPlatformLibName.c_str();
#endif
#else
      const char* path = aPlatformLibName.c_str();
#endif
      LibHandle libHandle = LoadLib( path );
#if defined(WIN32) && defined(UNICODE)
      delete path;
#endif
      if (!libHandle)
      {
        // report any error, if occurred
#ifndef WIN32
        const char* anError = dlerror();
        throw(SALOME_Exception( anError ));
#else
        throw(SALOME_Exception ( SMESH_Comment("Can't load meshers plugin library " )
                                 << aPlatformLibName));
#endif
      }

      // get method, returning hypothesis creator
      if(MYDEBUG) MESSAGE("Find GetHypothesisCreator() method ...");
      GetHypothesisCreator procHandle =
        (GetHypothesisCreator)GetProc( libHandle, "GetHypothesisCreator" );
      if (!procHandle)
      {
        throw(SALOME_Exception(SMESH_Comment("bad hypothesis plugin library")
                               << aPlatformLibName ));
        UnLoadLib(libHandle);
      }

      // get hypothesis creator
      if(MYDEBUG) MESSAGE("Get Hypothesis Creator for " << theHypName);
      aCreator = procHandle(theHypName);
      if (!aCreator)
      {
        throw(SALOME_Exception( SMESH_Comment( theHypName ) << " is missing from "
                                << aPlatformLibName));
      }
      // map hypothesis creator to a hypothesis name
      myHypCreatorMap[string(theHypName)] = aCreator;
      return aCreator;
    }
    else
    {
      return myHypCreatorMap[string(theHypName)];
    }
  }
  catch (SALOME_Exception& S_ex)
  {
    THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
  }
  return aCreator;
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
{
  SMESH_Hypothesis_i* myHypothesis_i = 0;
  SMESH::SMESH_Hypothesis_var hypothesis_i;
  std::string aPlatformLibName;
  GenericHypothesisCreator_i* aCreator =
    getHypothesisCreator(theHypName, theLibName, aPlatformLibName);

  // create a new hypothesis object, store its ref. in studyContext
  myHypothesis_i = aCreator->Create(myPoa, &myGen);
  if (myHypothesis_i)
  {
    myHypothesis_i->SetLibName( aPlatformLibName.c_str() ); // for persistency assurance
    CORBA::String_var hypName = myHypothesis_i->GetName();
    myHypCreatorMap[ hypName.in() ] = aCreator;

    // activate the CORBA servant of hypothesis
    hypothesis_i = myHypothesis_i->_this();
    int nextId = RegisterObject( hypothesis_i );
    if(MYDEBUG) { MESSAGE( "Add hypo to map with id = "<< nextId ); }
    else        { (void)nextId; } // avoid "unused variable" warning in release mode
  }
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
{
  Unexpect aCatch(SALOME_SalomeException);
  if(MYDEBUG) MESSAGE( "SMESH_Gen_i::createMesh" );

  // Get or create the GEOM_Client instance
  try {
    // create a new mesh object servant, store it in a map in study context
    SMESH_Mesh_i* meshServant = new SMESH_Mesh_i( GetPOA(), this );
    // create a new mesh object
    if(MYDEBUG) MESSAGE("myIsEmbeddedMode " << myIsEmbeddedMode);
    meshServant->SetImpl( myGen.CreateMesh( myIsEmbeddedMode ));

    // activate the CORBA servant of Mesh
    SMESH::SMESH_Mesh_var mesh = SMESH::SMESH_Mesh::_narrow( meshServant->_this() );
    int nextId = RegisterObject( mesh );
    if(MYDEBUG) { MESSAGE( "Add mesh to map with id = "<< nextId); }
    else        { (void)nextId; } // avoid "unused variable" warning in release mode
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
 *  SMESH_Gen_i::SetGeomEngine
 *
 *  Set GEOM::GEOM_Gen reference
 */
//=============================================================================

void SMESH_Gen_i::SetGeomEngine( GEOM::GEOM_Gen_ptr geomcompo )
{
  myGeomGen = GEOM::GEOM_Gen::_duplicate( geomcompo );
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

  if ( !myIsEmbeddedMode ) {
    //PAL10867: disable signals catching with "noexcepthandler" option
    char* envNoCatchSignals = getenv("NOT_INTERCEPT_SIGNALS");
    if (!envNoCatchSignals || !atoi(envNoCatchSignals))
    {
      bool raiseFPE;
#ifdef _DEBUG_
      raiseFPE = true;
      char* envDisableFPE = getenv("DISABLE_FPE");
      if (envDisableFPE && atoi(envDisableFPE))
        raiseFPE = false;
#else
      raiseFPE = false;
#endif
      OSD::SetSignal( raiseFPE );
    }
    // else OSD::SetSignal() is called in GUI
  }
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
 *  SMESH_Gen_i::SetEnablePublish
 *
 *  Set enable publishing in the study
 */
//=============================================================================

void SMESH_Gen_i::SetEnablePublish( CORBA::Boolean theIsEnablePublish )
{
  myIsEnablePublish = theIsEnablePublish;
}

//=============================================================================
/*!
 *  SMESH_Gen_i::IsEnablePublish
 *
 *  Check enable publishing
 */
//=============================================================================

CORBA::Boolean SMESH_Gen_i::IsEnablePublish()
{
  return myIsEnablePublish;
}

//=============================================================================
/*!
 *  SMESH_Gen_i::UpdateStudy
 *
 *  Update study (needed at switching GEOM->SMESH)
 */
//=============================================================================

void SMESH_Gen_i::UpdateStudy()
{
  if ( !myStudyContext )
    myStudyContext = new StudyContext;

  SALOMEDS::Study_var aStudy = getStudyServant();
  if ( !CORBA::is_nil( aStudy ) )
  {
    SALOMEDS::StudyBuilder_var aStudyBuilder = aStudy->NewBuilder();

    SALOMEDS::SComponent_wrap GEOM_var = aStudy->FindComponent( "GEOM" );
    if( !GEOM_var->_is_nil() )
      aStudyBuilder->LoadWith( GEOM_var, GetGeomEngine( /*isShaper=*/false ) );

    GEOM_var = aStudy->FindComponent( "SHAPERSTUDY" );
    if( !GEOM_var->_is_nil() )
      aStudyBuilder->LoadWith( GEOM_var, GetGeomEngine( /*isShaper=*/true ) );

    // NPAL16168, issue 0020210
    // Let meshes update their data depending on GEOM groups that could change
    CORBA::String_var compDataType = ComponentDataType();
    SALOMEDS::SComponent_wrap me = aStudy->FindComponent( compDataType.in() );
    if ( !me->_is_nil() ) {
      SALOMEDS::ChildIterator_wrap anIter = aStudy->NewChildIterator( me );
      for ( ; anIter->More(); anIter->Next() ) {
        SALOMEDS::SObject_wrap so = anIter->Value();
        CORBA::Object_var     ior = SObjectToObject( so );
        if ( SMESH_Mesh_i*   mesh = SMESH::DownCast<SMESH_Mesh_i*>( ior ))
          mesh->CheckGeomModif();
      }
    }
  }
}

//================================================================================
/*!
 * \brief Return true if mesh has ICON_SMESH_TREE_GEOM_MODIF icon
 */
//================================================================================

bool SMESH_Gen_i::isGeomModifIcon( SMESH::SMESH_Mesh_ptr mesh )
{
  SALOMEDS::SObject_wrap so = ObjectToSObject( mesh );
  SALOMEDS::GenericAttribute_wrap attr;
  if ( ! so->_is_nil() && so->FindAttribute( attr.inout(), "AttributePixMap" ))
  {
    SALOMEDS::AttributePixMap_wrap pm = attr;
    CORBA::String_var             ico = pm->GetPixMap();
    return ( strcmp( ico.in(), "ICON_SMESH_TREE_GEOM_MODIF" ) == 0 );
  }
  return false;
}

//=================================================================================
// function : hasObjectInfo()
// purpose  : shows if module provides information for its objects
//=================================================================================

bool SMESH_Gen_i::hasObjectInfo()
{
  return true;
}

//=================================================================================
// function : getObjectInfo()
// purpose  : returns an information for a given object by its entry
//=================================================================================

char* SMESH_Gen_i::getObjectInfo( const char* entry )
{
  // for a mesh with icon == ICON_SMESH_TREE_GEOM_MODIF show a warning;
  // for the rest, "module 'SMESH', ID=0:1:2:*"

  SMESH_Comment txt;

  SALOMEDS::SObject_wrap  so = getStudyServant()->FindObjectID( entry );
  CORBA::Object_var      obj = SObjectToObject( so );
  SMESH::SMESH_Mesh_var mesh = SMESH::SMESH_Mesh::_narrow( obj );
  if ( isGeomModifIcon( mesh ))
  {
    txt << "The geometry was changed and the mesh needs to be recomputed";
  }

  if ( txt.empty() )
  {
    CORBA::String_var compType = ComponentDataType();
    txt << "module '" << compType << "', ID=" << entry;
  }
  return CORBA::string_dup( txt );
}

//=============================================================================
/*!
 *  SMESH_Gen_i::GetStudyContext
 *
 *  Get study context
 */
//=============================================================================

StudyContext* SMESH_Gen_i::GetStudyContext()
{
  return myStudyContext;
}

//=============================================================================
/*!
 *  SMESH_Gen_i::CreateHypothesis
 *
 *  Create hypothesis/algorithm of given type and publish it in the study
 */
//=============================================================================

SMESH::SMESH_Hypothesis_ptr SMESH_Gen_i::CreateHypothesis( const char* theHypName,
                                                           const char* theLibName )
{
  Unexpect aCatch(SALOME_SalomeException);
  // Create hypothesis/algorithm
  SMESH::SMESH_Hypothesis_var hyp = this->createHypothesis( theHypName, theLibName );

  // Publish hypothesis/algorithm in the study
  if ( CanPublishInStudy( hyp ) ) {
    SALOMEDS::SObject_wrap aSO = PublishHypothesis( hyp );
    if ( !aSO->_is_nil() ) {
      // Update Python script
      TPythonDump(this) << aSO << " = " << this << ".CreateHypothesis('"
                    << theHypName << "', '" << theLibName << "')";
    }
  }

  return hyp._retn();
}

//================================================================================
/*!
 * \brief Return a hypothesis initialized by given average length.
 *  \param theHypType - hypothesis type name
 *  \param theLibName - plugin library name
 *  \param theAverageLength - average length
 *  \param theQuadDominated - is quad-dominated flag
 *  \retval SMESH::SMESH_Hypothesis_ptr - the new hypothesis
 */
//================================================================================

SMESH::SMESH_Hypothesis_ptr
SMESH_Gen_i::CreateHypothesisByAverageLength( const char*    theHypType,
                                              const char*    theLibName,
                                              CORBA::Double  theAverageLength,
                                              CORBA::Boolean theQuadDominated)
{
  SMESH::HypInitParams initParams = { ::SMESH_Hypothesis::BY_AVERAGE_LENGTH,
                                      theAverageLength, theQuadDominated };

  SMESH::SMESH_Hypothesis_var hyp =
    GetHypothesisParameterValues( theHypType, theLibName,
                                  SMESH::SMESH_Mesh::_nil(),
                                  GEOM::GEOM_Object::_nil(),
                                  initParams );
  SALOMEDS::SObject_wrap so = PublishHypothesis( hyp );

  TPythonDump(this) << hyp << " = " << this << ".CreateHypothesisByAverageLength( '"
                << theHypType << "', '"
                << theLibName << "', "
                << theAverageLength << ", "
                << theQuadDominated << " )";

  return hyp._retn();
}

//================================================================================
/*!
 * \brief Return a hypothesis holding parameter values corresponding either to the mesh
 * existing on the given geometry or to size of the geometry.
 *  \param theHypType - hypothesis type name
 *  \param theLibName - plugin library name
 *  \param theMesh - The mesh of interest
 *  \param theGeom - The shape to get parameter values from
 *  \retval SMESH::SMESH_Hypothesis_ptr - The returned hypothesis may be the one existing
 *     in a study and used to compute the mesh, or a temporary one created just to pass
 *     parameter values
 */
//================================================================================

SMESH::SMESH_Hypothesis_ptr
SMESH_Gen_i::GetHypothesisParameterValues( const char*                 theHypType,
                                           const char*                 theLibName,
                                           SMESH::SMESH_Mesh_ptr       theMesh,
                                           GEOM::GEOM_Object_ptr       theGeom,
                                           const SMESH::HypInitParams& theParams)
{
  Unexpect aCatch(SALOME_SalomeException);

  const bool byMesh = ( theParams.way == ::SMESH_Hypothesis::BY_MESH );
  if ( byMesh && CORBA::is_nil( theMesh ) )
    return SMESH::SMESH_Hypothesis::_nil();
  if ( byMesh && CORBA::is_nil( theGeom ) )
    return SMESH::SMESH_Hypothesis::_nil();

  // -----------------------------------------------
  // find hypothesis used to mesh theGeom
  // -----------------------------------------------

  // get mesh and shape
  SMESH_Mesh_i* meshServant = SMESH::DownCast<SMESH_Mesh_i*>( theMesh );
  TopoDS_Shape shape = GeomObjectToShape( theGeom );
  if ( byMesh && ( !meshServant || meshServant->NbNodes()==0 || shape.IsNull() ))
    return SMESH::SMESH_Hypothesis::_nil();
  ::SMESH_Mesh* mesh = meshServant ? &meshServant->GetImpl() : (::SMESH_Mesh*)0;

  // create a temporary hypothesis to know its dimension
  SMESH::SMESH_Hypothesis_var tmpHyp = this->createHypothesis( theHypType, theLibName );
  SMESH_Hypothesis_i* hypServant = SMESH::DownCast<SMESH_Hypothesis_i*>( tmpHyp );
  if ( !hypServant )
    return SMESH::SMESH_Hypothesis::_nil();
  ::SMESH_Hypothesis* hyp = hypServant->GetImpl();

  if ( byMesh ) {
    // look for a hypothesis of theHypType used to mesh the shape
    if ( myGen.GetShapeDim( shape ) == hyp->GetDim() )
    {
      // check local shape
      SMESH::ListOfHypothesis_var aHypList = theMesh->GetHypothesisList( theGeom );
      int nbLocalHyps = aHypList->length();
      for ( int i = 0; i < nbLocalHyps; i++ ) {
        CORBA::String_var hypName = aHypList[i]->GetName();
        if ( strcmp( theHypType, hypName.in() ) == 0 ) // FOUND local!
          return SMESH::SMESH_Hypothesis::_duplicate( aHypList[i] );
      }
      // check super shapes
      TopTools_ListIteratorOfListOfShape itShape( mesh->GetAncestors( shape ));
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

    // let the temporary hypothesis find out somehow parameter values by mesh
    if ( hyp->SetParametersByMesh( mesh, shape ))
      return SMESH::SMESH_Hypothesis::_duplicate( tmpHyp );
  }
  else
  {
    ::SMESH_Hypothesis::TDefaults dflts;
    dflts._way           = ( ::SMESH_Hypothesis::InitWay) theParams.way;
    dflts._nbSegments    = myGen.GetDefaultNbSegments();
    dflts._elemLength    = theParams.averageLength;
    dflts._quadDominated = theParams.quadDominated;
    if ( theParams.way == ::SMESH_Hypothesis::BY_GEOM )
    {
      if ( mesh )
        dflts._diagonal  = mesh->GetShapeDiagonalSize();
      else
        dflts._diagonal  = ::SMESH_Mesh::GetShapeDiagonalSize( shape );
      dflts._elemLength  = dflts._diagonal / myGen.GetBoundaryBoxSegmentation();
      dflts._shape       = &shape;
    }

    // let the hypothesis initialize it's values
    if ( hyp->SetParametersByDefaults( dflts, mesh ))
      return SMESH::SMESH_Hypothesis::_duplicate( tmpHyp );
  }

  return SMESH::SMESH_Hypothesis::_nil();
}

//=============================================================================
/*!
 * Returns \c True if a hypothesis is assigned to a sole sub-mesh in a current Study
 *  \param [in] theHyp - the hypothesis of interest
 *  \param [out] theMesh - the sole mesh using \a theHyp
 *  \param [out] theShape - the sole geometry \a theHyp is assigned to
 *  \return boolean - \c True if \a theMesh and \a theShape are sole using \a theHyp
 *
 * If two meshes on same shape have theHyp assigned to the same sub-shape, they are
 * considered as SAME sub-mesh => result is \c true.
 * This method ids used to initialize SMESHGUI_GenericHypothesisCreator with
 * a shape to which an hyp being edited is assigned.
 */
//=============================================================================

CORBA::Boolean SMESH_Gen_i::GetSoleSubMeshUsingHyp( SMESH::SMESH_Hypothesis_ptr theHyp,
                                                    SMESH::SMESH_Mesh_out       theMesh,
                                                    GEOM::GEOM_Object_out       theShape)
{
  if ( CORBA::is_nil( theHyp ))
    return false;

  // get Mesh component SO
  CORBA::String_var compDataType = ComponentDataType();
  SALOMEDS::SComponent_wrap comp = getStudyServant()->FindComponent( compDataType.in() );
  if ( CORBA::is_nil( comp ))
    return false;

  // look for child SO of meshes
  SMESH::SMESH_Mesh_var foundMesh;
  TopoDS_Shape          foundShape;
  bool                  isSole = true;
  SALOMEDS::ChildIterator_wrap meshIter = getStudyServant()->NewChildIterator( comp );
  for ( ; meshIter->More() && isSole; meshIter->Next() )
  {
    SALOMEDS::SObject_wrap curSO = meshIter->Value();
    CORBA::Object_var        obj = SObjectToObject( curSO );
    SMESH_Mesh_i*         mesh_i = SMESH::DownCast< SMESH_Mesh_i* >( obj );
    if ( ! mesh_i )
      continue;

    // look for a sole shape where theHyp is assigned
    bool isHypFound = false;
    const ShapeToHypothesis & s2hyps = mesh_i->GetImpl().GetMeshDS()->GetHypotheses();
    ShapeToHypothesis::Iterator s2hypsIt( s2hyps );
    for ( ; s2hypsIt.More() && isSole; s2hypsIt.Next() )
    {
      const THypList& hyps = s2hypsIt.Value();
      THypList::const_iterator h = hyps.begin();
      for ( ; h != hyps.end(); ++h )
        if ( (*h)->GetID() == theHyp->GetId() )
          break;
      if ( h != hyps.end()) // theHyp found
      {
        isHypFound = true;
        if ( ! foundShape.IsNull() &&
             ! foundShape.IsSame( s2hypsIt.Key() )) // not a sole sub-shape
        {
          foundShape.Nullify();
          isSole = false;
          break;
        }
        foundShape = s2hypsIt.Key();
      }
    } // loop on assigned hyps

    if ( isHypFound && !foundShape.IsNull() ) // a mesh using theHyp is found
    {
      if ( !foundMesh->_is_nil() ) // not a sole mesh
      {
        if ( !foundMesh->HasShapeToMesh() ||
             !mesh_i   ->HasShapeToMesh() )
        {
          isSole = ( foundMesh->HasShapeToMesh() == mesh_i->HasShapeToMesh() );
        }
        else
        {
          GEOM::GEOM_Object_var s1 = mesh_i   ->GetShapeToMesh();
          GEOM::GEOM_Object_var s2 = foundMesh->GetShapeToMesh();
          isSole = s1->IsSame( s2 );
        }
      }
      foundMesh = SMESH::SMESH_Mesh::_narrow( obj );
    }

  } // loop on meshes

  if ( isSole &&
       ! foundMesh->_is_nil() &&
       ! foundShape.IsNull() )
  {
    theMesh  = foundMesh._retn();
    theShape = ShapeToGeomObject( foundShape );
    return ( !theMesh->_is_nil() && !theShape->_is_nil() );
  }
  return false;
}

//=============================================================================
/*!
 * Set number of segments per diagonal of boundary box of geometry by which
 * default segment length of appropriate 1D hypotheses is defined
 */
//=============================================================================

void SMESH_Gen_i::SetBoundaryBoxSegmentation( CORBA::Long theNbSegments )
{
  if ( theNbSegments > 0 )
    myGen.SetBoundaryBoxSegmentation( int( theNbSegments ));
  else
    THROW_SALOME_CORBA_EXCEPTION( "non-positive number of segments", SALOME::BAD_PARAM );
}

//=============================================================================
/*!
 * \brief Set default number of segments per edge
 */
//=============================================================================

void SMESH_Gen_i::SetDefaultNbSegments(CORBA::Long theNbSegments)
{
  if ( theNbSegments > 0 )
    myGen.SetDefaultNbSegments( int(theNbSegments) );
  else
    THROW_SALOME_CORBA_EXCEPTION( "non-positive number of segments", SALOME::BAD_PARAM );
}

//=============================================================================
/*!
 * Set an option value
 */
//=============================================================================

void SMESH_Gen_i::SetOption(const char* name, const char* value)
{
  if ( name && value && strlen( value ) > 0 )
  {
    string msgToGUI;
    if ( strcmp(name, "historical_python_dump") == 0 )
    {
      myIsHistoricalPythonDump = ( value[0] == '1' || toupper(value[0]) == 'T' ); // 1 || true
      msgToGUI = "preferences/SMESH/historical_python_dump/";
      msgToGUI += myIsHistoricalPythonDump ? "true" : "false";
    }
    else if ( strcmp(name, "forget_mesh_on_hyp_modif") == 0 )
    {
      myToForgetMeshDataOnHypModif = ( value[0] == '1' || toupper(value[0]) == 'T' ); // 1 || true
      msgToGUI = "preferences/SMESH/forget_mesh_on_hyp_modif/";
      msgToGUI += myToForgetMeshDataOnHypModif ? "true" : "false";
    }
    else if ( strcmp(name, "default_grp_color") == 0 )
    {
      vector<int> color;
      string str = value;
      // color must be presented as a string of following form:
      if ( str.at(0) == '#' && str.length() == 7 ) { // hexadecimal color ("#ffaa00", for example)
        str = str.substr(1);
        for ( size_t i = 0; i < str.length()/2; i++ )
          if ( str.at(i*2) >= '0' && str.at(i*2) <= 'f' && str.at(i*2+1) >= '0' && str.at(i*2+1) <= 'f' )
            color.push_back( strtol( str.substr( i*2, 2 ).c_str(), NULL, 16 ) );
      }
      else if ( value ) { // rgb color ("255,170,0", for example)
        string tempValue( value );
        char* colorValue = strtok( &tempValue[0], "," );
        while ( colorValue != NULL ) {
          int c_value = atoi( colorValue );
          if ( c_value >= 0 && c_value <= 255 )
            color.push_back( c_value );
          colorValue = strtok( NULL, "," );
        }
      }
      if ( color.size() == 3 ) { // color must have three valid component
        SMESHDS_GroupBase::SetDefaultColor( Quantity_Color( color[0]/255., color[1]/255., color[2]/255., Quantity_TOC_RGB ) );
        myDefaultGroupColor = value;
        msgToGUI = "preferences/SMESH/default_grp_color/";
        msgToGUI += value;
      }
    }

    // update preferences in case if SetOption() is invoked from python console
    if ( !msgToGUI.empty() )
    {
      CORBA::Object_var obj = SMESH_Gen_i::GetNS()->Resolve( "/Kernel/Session" );
      SALOME::Session_var session = SALOME::Session::_narrow( obj );
      if ( !CORBA::is_nil( session ) )
        session->emitMessageOneWay(msgToGUI.c_str());
    }
  }
}

//=============================================================================
/*!
 *  Return an option value
 */
//=============================================================================

char* SMESH_Gen_i::GetOption(const char* name)
{
  if ( name )
  {
    if ( strcmp(name, "historical_python_dump") == 0 )
    {
      return CORBA::string_dup( myIsHistoricalPythonDump ? "true" : "false" );
    }
    if ( strcmp(name, "forget_mesh_on_hyp_modif") == 0 )
    {
      return CORBA::string_dup( myToForgetMeshDataOnHypModif ? "true" : "false" );
    }
    if ( strcmp(name, "default_grp_color") == 0 )
    {
      return CORBA::string_dup( myDefaultGroupColor.c_str() );
    }
  }
  return CORBA::string_dup( "" );
}

//=============================================================================
/*!
 *  SMESH_Gen_i::CreateMesh
 *
 *  Create empty mesh on a shape and publish it in the study
 */
//=============================================================================

SMESH::SMESH_Mesh_ptr SMESH_Gen_i::CreateMesh( GEOM::GEOM_Object_ptr theShapeObject )
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
    SALOMEDS::StudyBuilder_var aStudyBuilder = getStudyServant()->NewBuilder();
    aStudyBuilder->NewCommand();  // There is a transaction
    SALOMEDS::SObject_wrap aSO = PublishMesh( mesh.in() );
    aStudyBuilder->CommitCommand();
    if ( !aSO->_is_nil() ) {
      // Update Python script
      TPythonDump(this) << aSO << " = " << this << ".CreateMesh(" << theShapeObject << ")";
    }
  }

  return mesh._retn();
}

//=============================================================================
/*!
 *  SMESH_Gen_i::CreateEmptyMesh
 *
 *  Create empty mesh
 */
//=============================================================================

SMESH::SMESH_Mesh_ptr SMESH_Gen_i::CreateEmptyMesh()
{
  Unexpect aCatch(SALOME_SalomeException);
  if(MYDEBUG) MESSAGE( "SMESH_Gen_i::CreateMesh" );
  // create mesh
  SMESH::SMESH_Mesh_var mesh = this->createMesh();

  // publish mesh in the study
  if ( CanPublishInStudy( mesh ) ) {
    SALOMEDS::StudyBuilder_var aStudyBuilder = getStudyServant()->NewBuilder();
    aStudyBuilder->NewCommand();  // There is a transaction
    SALOMEDS::SObject_wrap aSO = PublishMesh( mesh.in() );
    aStudyBuilder->CommitCommand();
    if ( !aSO->_is_nil() ) {
      // Update Python script
      TPythonDump(this) << aSO << " = " << this << ".CreateEmptyMesh()";
    }
  }

  return mesh._retn();
}

namespace
{
  //================================================================================
  /*!
   * \brief Throw an exception in case if the file can't be read
   */
  //================================================================================

  void checkFileReadable( const char* theFileName )
  {
    SMESH_File f ( theFileName );
    if ( !f )
    {
      if ( !f.error().empty() )
        THROW_SALOME_CORBA_EXCEPTION( f.error().c_str(), SALOME::BAD_PARAM);

      THROW_SALOME_CORBA_EXCEPTION
        (( SMESH_Comment("Can't open for reading the file ") << theFileName ).c_str(),
         SALOME::BAD_PARAM );
    }
  }
}

//=============================================================================
/*!
 *  SMESH_Gen_i::CreateMeshFromUNV
 *
 *  Create mesh and import data from UNV file
 */
//=============================================================================

SMESH::SMESH_Mesh_ptr SMESH_Gen_i::CreateMeshesFromUNV( const char* theFileName )
{
  Unexpect aCatch(SALOME_SalomeException);

  checkFileReadable( theFileName );

  SMESH::SMESH_Mesh_var aMesh = createMesh();
  string aFileName;
  // publish mesh in the study
  if ( CanPublishInStudy( aMesh ) ) {
    SALOMEDS::StudyBuilder_var aStudyBuilder = getStudyServant()->NewBuilder();
    aStudyBuilder->NewCommand();  // There is a transaction
    SALOMEDS::SObject_wrap aSO = PublishMesh( aMesh.in(), aFileName.c_str() );
    aStudyBuilder->CommitCommand();
    if ( !aSO->_is_nil() ) {
      // Update Python script
      TPythonDump(this) << aSO << " = " << this << ".CreateMeshesFromUNV(r'" << theFileName << "')";
    }
  }

  SMESH_Mesh_i* aServant = dynamic_cast<SMESH_Mesh_i*>( GetServant( aMesh ).in() );
  ASSERT( aServant );
  aServant->ImportUNVFile( theFileName );

  // Dump creation of groups
  SMESH::ListOfGroups_var groups = aServant->GetGroups();

  aServant->GetImpl().GetMeshDS()->Modified();
  return aMesh._retn();
}

//=============================================================================
/*!
 *  SMESH_Gen_i::CreateMeshFromMED
 *
 *  Create mesh and import data from MED file
 */
//=============================================================================

SMESH::mesh_array* SMESH_Gen_i::CreateMeshesFromMEDorSAUV( const char* theFileName,
                                                           SMESH::DriverMED_ReadStatus& theStatus,
                                                           const char* theCommandNameForPython,
                                                           const char* theFileNameForPython)
{
#ifdef WIN32
  char bname[ _MAX_FNAME ];
  _splitpath( theFileNameForPython, NULL, NULL, bname, NULL );
  string aFileName = bname;
#else
  string aFileName = basename( const_cast<char *>(theFileNameForPython) );
#endif
  // Retrieve mesh names from the file
  DriverMED_R_SMESHDS_Mesh myReader;
  myReader.SetFile( theFileName );
  myReader.SetMeshId( -1 );
  Driver_Mesh::Status aStatus;
  list<string> aNames = myReader.GetMeshNames(aStatus);
  SMESH::mesh_array_var aResult = new SMESH::mesh_array();
  theStatus = (SMESH::DriverMED_ReadStatus)aStatus;

  { // open a new scope to make aPythonDump die before PythonDump in SMESH_Mesh::GetGroups()

  // Python Dump
  TPythonDump aPythonDump(this);
  aPythonDump << "([";

  if (theStatus == SMESH::DRS_OK) {
    SALOMEDS::StudyBuilder_var aStudyBuilder;
    aStudyBuilder = getStudyServant()->NewBuilder();
    aStudyBuilder->NewCommand();  // There is a transaction

    aResult->length( aNames.size() );
    int i = 0;

    // Iterate through all meshes and create mesh objects
    for ( list<string>::iterator it = aNames.begin(); it != aNames.end(); it++ )
    {
      // Python Dump
      if (i > 0) aPythonDump << ", ";

      // create mesh
      SMESH::SMESH_Mesh_var mesh = createMesh();

      // publish mesh in the study
      SALOMEDS::SObject_wrap aSO;
      if ( CanPublishInStudy( mesh ) )
        // little trick: for MED file theFileName and theFileNameForPython are the same, but they are different for SAUV
        // - as names of meshes are stored in MED file, we use them for data publishing
        // - as mesh name is not stored in UNV file, we use file name as name of mesh when publishing data
        aSO = PublishMesh( mesh.in(), ( theFileName == theFileNameForPython ) ? (*it).c_str() : aFileName.c_str() );

      // Python Dump
      if ( !aSO->_is_nil() ) {
        aPythonDump << aSO;
      } else {
        aPythonDump << "mesh_" << i;
      }

      // Read mesh data (groups are published automatically by ImportMEDFile())
      SMESH_Mesh_i* meshServant = dynamic_cast<SMESH_Mesh_i*>( GetServant( mesh ).in() );
      ASSERT( meshServant );
      SMESH::DriverMED_ReadStatus status1 =
        meshServant->ImportMEDFile( theFileName, (*it).c_str() );
      if (status1 > theStatus)
        theStatus = status1;

      aResult[i++] = SMESH::SMESH_Mesh::_duplicate( mesh );
      meshServant->GetImpl().GetMeshDS()->Modified();
    }
    if ( !aStudyBuilder->_is_nil() )
      aStudyBuilder->CommitCommand();
  }

  // Update Python script
  aPythonDump << "], status) = " << this << "." << theCommandNameForPython << "(r'" << theFileNameForPython << "')";
  }
  // Dump creation of groups
  for ( CORBA::ULong  i = 0; i < aResult->length(); ++i )
    SMESH::ListOfGroups_var groups = aResult[ i ]->GetGroups();

  return aResult._retn();
}

//================================================================================
/*!
 * \brief Create meshes by reading a MED file
 */
//================================================================================

SMESH::mesh_array* SMESH_Gen_i::CreateMeshesFromMED( const char*                  theFileName,
                                                     SMESH::DriverMED_ReadStatus& theStatus)
{
  Unexpect aCatch(SALOME_SalomeException);
  checkFileReadable( theFileName );

  SMESH::mesh_array* result = CreateMeshesFromMEDorSAUV(theFileName, theStatus,
                                                        "CreateMeshesFromMED", theFileName);
  return result;
}

//=============================================================================
/*!
 *  SMESH_Gen_i::CreateMeshFromSAUV
 *
 *  Create mesh and import data from SAUV file
 */
//=============================================================================

SMESH::mesh_array* SMESH_Gen_i::CreateMeshesFromSAUV( const char*                  theFileName,
                                                      SMESH::DriverMED_ReadStatus& theStatus)
{
  Unexpect aCatch(SALOME_SalomeException);
  checkFileReadable( theFileName );

  std::string sauvfilename(theFileName);
  std::string medfilename(theFileName);
  medfilename += ".med";
  std::string cmd;
#ifdef WIN32
  cmd = "%PYTHONBIN% ";
#else
  cmd = "python3 ";
#endif
  cmd += "-c \"";
  cmd += "from medutilities import convert ; convert(r'" + sauvfilename + "', 'GIBI', 'MED', 1, r'" + medfilename + "')";
  cmd += "\"";
  system(cmd.c_str());
  SMESH::mesh_array* result = CreateMeshesFromMEDorSAUV(medfilename.c_str(),
                                                        theStatus,
                                                        "CreateMeshesFromSAUV",
                                                        sauvfilename.c_str());
#ifdef WIN32
  cmd = "%PYTHONBIN% ";
#else
  cmd = "python3 ";
#endif
  cmd += "-c \"";
  cmd += "from medutilities import my_remove ; my_remove(r'" + medfilename + "')";
  cmd += "\"";
  system(cmd.c_str());
  return result;
}

//=============================================================================
/*!
 *  SMESH_Gen_i::CreateMeshFromSTL
 *
 *  Create mesh and import data from STL file
 */
//=============================================================================

SMESH::SMESH_Mesh_ptr SMESH_Gen_i::CreateMeshesFromSTL( const char* theFileName )
{
  Unexpect aCatch(SALOME_SalomeException);
  checkFileReadable( theFileName );

  SMESH::SMESH_Mesh_var aMesh = createMesh();
  //string aFileName;
#ifdef WIN32
  char bname[ _MAX_FNAME ];
  _splitpath( theFileName, NULL, NULL, bname, NULL );
  string aFileName = bname;
#else
  string aFileName = basename( const_cast<char *>(theFileName) );
#endif
  // publish mesh in the study
  if ( CanPublishInStudy( aMesh ) ) {
    SALOMEDS::StudyBuilder_var aStudyBuilder = getStudyServant()->NewBuilder();
    aStudyBuilder->NewCommand();  // There is a transaction
    SALOMEDS::SObject_wrap aSO = PublishInStudy( SALOMEDS::SObject::_nil(), aMesh.in(), aFileName.c_str() );
    aStudyBuilder->CommitCommand();
    if ( !aSO->_is_nil() ) {
      // Update Python script
      TPythonDump(this) << aSO << " = " << this << ".CreateMeshesFromSTL(r'" << theFileName << "')";
    }
  }

  SMESH_Mesh_i* aServant = dynamic_cast<SMESH_Mesh_i*>( GetServant( aMesh ).in() );
  ASSERT( aServant );
  aServant->ImportSTLFile( theFileName );
  aServant->GetImpl().GetMeshDS()->Modified();
  return aMesh._retn();
}

//================================================================================
/*!
 * \brief Create meshes and import data from the CGSN file
 */
//================================================================================

SMESH::mesh_array* SMESH_Gen_i::CreateMeshesFromCGNS( const char*                  theFileName,
                                                      SMESH::DriverMED_ReadStatus& theStatus)
{
  Unexpect aCatch(SALOME_SalomeException);
  checkFileReadable( theFileName );

  SMESH::mesh_array_var aResult = new SMESH::mesh_array();

#ifdef WITH_CGNS
  // Retrieve nb meshes from the file
  DriverCGNS_Read myReader;
  myReader.SetFile( theFileName );
  Driver_Mesh::Status aStatus;
  int nbMeshes = myReader.GetNbMeshes(aStatus);
  theStatus = (SMESH::DriverMED_ReadStatus)aStatus;

  aResult->length( nbMeshes );

  { // open a new scope to make aPythonDump die before PythonDump in SMESH_Mesh::GetGroups()

    // Python Dump
    TPythonDump aPythonDump(this);
    aPythonDump << "([";

    if (theStatus == SMESH::DRS_OK)
    {
      SALOMEDS::StudyBuilder_var aStudyBuilder = getStudyServant()->NewBuilder();
      aStudyBuilder->NewCommand();  // There is a transaction

      int i = 0;

      // Iterate through all meshes and create mesh objects
      for ( ; i < nbMeshes; ++i )
      {
        // Python Dump
        if (i > 0) aPythonDump << ", ";

        // create mesh
        SMESH::SMESH_Mesh_var mesh = createMesh();
        aResult[i] = SMESH::SMESH_Mesh::_duplicate( mesh );

        // Read mesh data (groups are published automatically by ImportMEDFile())
        SMESH_Mesh_i* meshServant = dynamic_cast<SMESH_Mesh_i*>( GetServant( mesh ).in() );
        ASSERT( meshServant );
        string meshName;
        SMESH::DriverMED_ReadStatus status1 =
          meshServant->ImportCGNSFile( theFileName, i, meshName );
        if (status1 > theStatus)
          theStatus = status1;

        meshServant->GetImpl().GetMeshDS()->Modified();
        // publish mesh in the study
        SALOMEDS::SObject_wrap aSO;
        if ( CanPublishInStudy( mesh ) )
          aSO = PublishMesh( mesh.in(), meshName.c_str() );

        // Python Dump
        if ( !aSO->_is_nil() ) {
          aPythonDump << aSO;
        }
        else {
          aPythonDump << "mesh_" << i;
        }
      }
      aStudyBuilder->CommitCommand();
    }

    aPythonDump << "], status) = " << this << ".CreateMeshesFromCGNS(r'" << theFileName << "')";
  }
  // Dump creation of groups
  for ( CORBA::ULong i = 0; i < aResult->length(); ++i )
    SMESH::ListOfGroups_var groups = aResult[ i ]->GetGroups();
#else
  THROW_SALOME_CORBA_EXCEPTION("CGNS library is unavailable", SALOME::INTERNAL_ERROR);
#endif

  return aResult._retn();
}

//================================================================================
/*!
 * \brief Create a mesh and import data from a GMF file
 */
//================================================================================

SMESH::SMESH_Mesh_ptr
SMESH_Gen_i::CreateMeshesFromGMF( const char*             theFileName,
                                  CORBA::Boolean          theMakeRequiredGroups,
                                  SMESH::ComputeError_out theError)
{
  Unexpect aCatch(SALOME_SalomeException);
  checkFileReadable( theFileName );

  SMESH::SMESH_Mesh_var aMesh = createMesh();
#ifdef WIN32
  char bname[ _MAX_FNAME ];
  _splitpath( theFileName, NULL, NULL, bname, NULL );
  string aFileName = bname;
#else
  string aFileName = basename( const_cast<char *>(theFileName) );
#endif
  // publish mesh in the study
  if ( CanPublishInStudy( aMesh ) ) {
    SALOMEDS::StudyBuilder_var aStudyBuilder = getStudyServant()->NewBuilder();
    aStudyBuilder->NewCommand();  // There is a transaction
    SALOMEDS::SObject_wrap aSO = PublishInStudy( SALOMEDS::SObject::_nil(), aMesh.in(), aFileName.c_str() );
    aStudyBuilder->CommitCommand();
    if ( !aSO->_is_nil() ) {
      // Update Python script
      TPythonDump(this) << "("<< aSO << ", error) = " << this << ".CreateMeshesFromGMF(r'"
                    << theFileName << "', "
                    << theMakeRequiredGroups << " )";
    }
  }
  SMESH_Mesh_i* aServant = dynamic_cast<SMESH_Mesh_i*>( GetServant( aMesh ).in() );
  ASSERT( aServant );
  theError = aServant->ImportGMFFile( theFileName, theMakeRequiredGroups );
  aServant->GetImpl().GetMeshDS()->Modified();
  return aMesh._retn();
}


//=============================================================================
/*!
 *  SMESH_Gen_i::IsReadyToCompute
 *
 *  Return true if mesh contains enough data to be computed
 */
//=============================================================================

CORBA::Boolean SMESH_Gen_i::IsReadyToCompute( SMESH::SMESH_Mesh_ptr theMesh,
                                              GEOM::GEOM_Object_ptr theShapeObject )
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
 * \brief  Find SObject for an algo
 */
//================================================================================

SALOMEDS::SObject_ptr SMESH_Gen_i::GetAlgoSO(const ::SMESH_Algo* algo)
{
  if ( algo ) {
    SALOMEDS::Study_var aStudy = getStudyServant();
    if ( !aStudy->_is_nil() ) {
      // find algo in the study
      CORBA::String_var compDataType  = ComponentDataType();
      SALOMEDS::SComponent_wrap father = aStudy->FindComponent( compDataType.in() );
      if ( !father->_is_nil() ) {
        SALOMEDS::ChildIterator_wrap itBig = aStudy->NewChildIterator( father );
        for ( ; itBig->More(); itBig->Next() ) {
          SALOMEDS::SObject_wrap gotBranch = itBig->Value();
          if ( gotBranch->Tag() == GetAlgorithmsRootTag() ) {
            SALOMEDS::ChildIterator_wrap algoIt = aStudy->NewChildIterator( gotBranch );
            for ( ; algoIt->More(); algoIt->Next() ) {
              SALOMEDS::SObject_wrap algoSO = algoIt->Value();
              CORBA::Object_var     algoIOR = SObjectToObject( algoSO );
              if ( !CORBA::is_nil( algoIOR )) {
                SMESH_Hypothesis_i* impl = SMESH::DownCast<SMESH_Hypothesis_i*>( algoIOR );
                if ( impl && impl->GetImpl() == algo )
                  return algoSO._retn();
              }
            } // loop on algo SO's
            break;
          } // if algo tag
        } // SMESH component iterator
      }
    }
  }
  return SALOMEDS::SObject::_nil();
}

//================================================================================
/*!
 * \brief Return errors of mesh computation
 */
//================================================================================

SMESH::compute_error_array* SMESH_Gen_i::GetComputeErrors( SMESH::SMESH_Mesh_ptr theMesh,
                                                           GEOM::GEOM_Object_ptr theSubObject )
{
  Unexpect aCatch(SALOME_SalomeException);
  if(MYDEBUG) MESSAGE( "SMESH_Gen_i::GetComputeErrors()" );

  if ( CORBA::is_nil( theSubObject ) && theMesh->HasShapeToMesh())
    THROW_SALOME_CORBA_EXCEPTION( "bad shape object reference", SALOME::BAD_PARAM );

  if ( CORBA::is_nil( theMesh ) )
    THROW_SALOME_CORBA_EXCEPTION( "bad Mesh reference",SALOME::BAD_PARAM );

  SMESH::compute_error_array_var error_array = new SMESH::compute_error_array;
  try {
    if ( SMESH_Mesh_i* meshServant = SMESH::DownCast<SMESH_Mesh_i*>( theMesh ))
    {
      TopoDS_Shape shape;
      if(theMesh->HasShapeToMesh())
        shape = GeomObjectToShape( theSubObject );
      else
        shape = SMESH_Mesh::PseudoShape();

      ::SMESH_Mesh& mesh = meshServant->GetImpl();

      error_array->length( mesh.GetMeshDS()->MaxShapeIndex() );
      int nbErr = 0;

      SMESH_subMesh *sm = mesh.GetSubMesh(shape);
      const bool includeSelf = true, complexShapeFirst = true;
      SMESH_subMeshIteratorPtr smIt = sm->getDependsOnIterator(includeSelf,
                                                               complexShapeFirst);
      while ( smIt->more() )
      {
        sm = smIt->next();
        // if ( sm->GetSubShape().ShapeType() == TopAbs_VERTEX )
        //   break;
        SMESH_ComputeErrorPtr error = sm->GetComputeError();
        if ( error && !error->IsOK() )
        {
          if ( !( error->myAlgo ) &&
               !( error->myAlgo = sm->GetAlgo() ))
            continue;
          SMESH::ComputeError & errStruct = error_array[ nbErr++ ];
          errStruct.code       = -( error->myName < 0 ? error->myName + 1: error->myName ); // -1 -> 0
          errStruct.comment    = error->myComment.c_str();
          errStruct.subShapeID = sm->GetId();
          SALOMEDS::SObject_wrap algoSO = GetAlgoSO( error->myAlgo );
          if ( !algoSO->_is_nil() ) {
            CORBA::String_var algoName = algoSO->GetName();
            errStruct.algoName = algoName;
          }
          else {
            errStruct.algoName = error->myAlgo->GetName();
          }
          errStruct.hasBadMesh = error->HasBadElems();
        }
      }
      error_array->length( nbErr );
    }
  }
  catch ( SALOME_Exception& S_ex ) {
    INFOS( "catch exception "<< S_ex.what() );
  }

  return error_array._retn();
}

//================================================================================
/*!
 * \brief Return mesh elements preventing computation of a subshape
 */
//================================================================================

SMESH::MeshPreviewStruct*
SMESH_Gen_i::GetBadInputElements( SMESH::SMESH_Mesh_ptr theMesh,
                                  CORBA::Short          theSubShapeID )
{
  Unexpect aCatch(SALOME_SalomeException);
  if(MYDEBUG) MESSAGE( "SMESH_Gen_i::GetBadInputElements()" );

  if ( CORBA::is_nil( theMesh ) )
    THROW_SALOME_CORBA_EXCEPTION( "bad Mesh reference",SALOME::BAD_PARAM );

  SMESH::MeshPreviewStruct_var result = new SMESH::MeshPreviewStruct;
  try {
    // mesh servant
    if ( SMESH_Mesh_i* meshServant = SMESH::DownCast<SMESH_Mesh_i*>( theMesh ))
    {
      // mesh implementation
      ::SMESH_Mesh& mesh = meshServant->GetImpl();
      // submesh by subshape id
      if ( SMESH_subMesh * sm = mesh.GetSubMeshContaining( theSubShapeID ))
      {
        // compute error
        SMESH_ComputeErrorPtr error = sm->GetComputeError();
        if ( error && error->HasBadElems() )
        {
          typedef map<const SMDS_MeshElement*, int > TNode2LocalIDMap;
          typedef TNode2LocalIDMap::iterator         TNodeLocalID;

          // get nodes of elements and count elements
          TNode2LocalIDMap mapNode2LocalID;
          list< TNodeLocalID > connectivity;
          int i, nbElements = 0, nbConnNodes = 0;

          const list<const SMDS_MeshElement*>& badElems =
            static_cast<SMESH_BadInputElements*>( error.get() )->myBadElements;
          list<const SMDS_MeshElement*>::const_iterator elemIt  = badElems.begin();
          list<const SMDS_MeshElement*>::const_iterator elemEnd = badElems.end();
          for ( ; elemIt != elemEnd; ++elemIt, ++nbElements )
          {
            SMDS_ElemIteratorPtr nIt = (*elemIt)->nodesIterator();
            while ( nIt->more() )
              connectivity.push_back
                ( mapNode2LocalID.insert( make_pair( nIt->next(), ++nbConnNodes)).first );
          }
          // fill node coords and assign local ids to the nodes
          int nbNodes = mapNode2LocalID.size();
          result->nodesXYZ.length( nbNodes );
          TNodeLocalID node2ID = mapNode2LocalID.begin();
          for ( i = 0; i < nbNodes; ++i, ++node2ID ) {
            node2ID->second = i;
            const SMDS_MeshNode* node = (const SMDS_MeshNode*) node2ID->first;
            result->nodesXYZ[i].x = node->X();
            result->nodesXYZ[i].y = node->Y();
            result->nodesXYZ[i].z = node->Z();
          }
          // fill connectivity
          result->elementConnectivities.length( nbConnNodes );
          list< TNodeLocalID >::iterator connIt = connectivity.begin();
          for ( i = 0; i < nbConnNodes; ++i, ++connIt ) {
            result->elementConnectivities[i] = (*connIt)->second;
          }
          // fill element types
          result->elementTypes.length( nbElements );
          for ( i = 0, elemIt = badElems.begin(); i <nbElements; ++i, ++elemIt )
          {
            const SMDS_MeshElement* elem = *elemIt;
            result->elementTypes[i].SMDS_ElementType = (SMESH::ElementType) elem->GetType();
            result->elementTypes[i].isPoly           = elem->IsPoly();
            result->elementTypes[i].nbNodesInElement = elem->NbNodes();
          }
        }
      }
    }
  }
  catch ( SALOME_Exception& S_ex ) {
    INFOS( "catch exception "<< S_ex.what() );
  }

  return result._retn();
}

//================================================================================
/*!
 * \brief Create a group of elements preventing computation of a sub-shape
 */
//================================================================================

SMESH::ListOfGroups*
SMESH_Gen_i::MakeGroupsOfBadInputElements( SMESH::SMESH_Mesh_ptr theMesh,
                                           CORBA::Short          theSubShapeID,
                                           const char*           theGroupName )
{
  Unexpect aCatch(SALOME_SalomeException);

  SMESH::ListOfGroups_var groups;

  if ( CORBA::is_nil( theMesh ) )
    THROW_SALOME_CORBA_EXCEPTION( "bad Mesh reference",SALOME::BAD_PARAM );

  try {
    if ( SMESH_Mesh_i* meshServant = SMESH::DownCast<SMESH_Mesh_i*>( theMesh ))
    {
      groups = meshServant->MakeGroupsOfBadInputElements( theSubShapeID, theGroupName );
      TPythonDump(this) << groups << " = " << this
                    << ".MakeGroupsOfBadInputElements( "
                    << theMesh << ", " << theSubShapeID << ", '" << theGroupName << "' )";
    }
  }
  catch ( SALOME_Exception& S_ex ) {
    INFOS( "catch exception "<< S_ex.what() );
  }
  return groups._retn();
}

//================================================================================
/*!
 * \brief Returns errors of hypotheses definition
 *  \param theMesh - the mesh
 *  \param theSubObject - the main or sub- shape
 *  \retval SMESH::algo_error_array* - sequence of errors
 */
//================================================================================

SMESH::algo_error_array* SMESH_Gen_i::GetAlgoState( SMESH::SMESH_Mesh_ptr theMesh,
                                                    GEOM::GEOM_Object_ptr theSubObject )
{
  Unexpect aCatch(SALOME_SalomeException);
  if(MYDEBUG) MESSAGE( "SMESH_Gen_i::GetAlgoState()" );

  if ( CORBA::is_nil( theSubObject ) && theMesh->HasShapeToMesh())
    THROW_SALOME_CORBA_EXCEPTION( "bad shape object reference", SALOME::BAD_PARAM );

  if ( CORBA::is_nil( theMesh ) )
    THROW_SALOME_CORBA_EXCEPTION( "bad Mesh reference",SALOME::BAD_PARAM );

  SMESH::algo_error_array_var error_array = new SMESH::algo_error_array;
  try {
    SMESH_Mesh_i* meshServant = SMESH::DownCast<SMESH_Mesh_i*>( theMesh );
    ASSERT( meshServant );
    if ( meshServant ) {
      TopoDS_Shape myLocShape;
      if(theMesh->HasShapeToMesh())
        myLocShape = GeomObjectToShape( theSubObject );
      else
        myLocShape = SMESH_Mesh::PseudoShape();

      ::SMESH_Mesh& myLocMesh = meshServant->GetImpl();
      list< ::SMESH_Gen::TAlgoStateError > error_list;
      list< ::SMESH_Gen::TAlgoStateError >::iterator error;
      // call ::SMESH_Gen::GetAlgoState()
      myGen.GetAlgoState( myLocMesh, myLocShape, error_list );
      error_array->length( error_list.size() );
      int i = 0;
      for ( error = error_list.begin(); error != error_list.end(); ++error )
      {
        // fill AlgoStateError structure
        SMESH::AlgoStateError & errStruct = error_array[ i++ ];
        errStruct.state        = SMESH_Mesh_i::ConvertHypothesisStatus( error->_name );
        errStruct.algoDim      = error->_algoDim;
        errStruct.isGlobalAlgo = error->_isGlobalAlgo;
        errStruct.algoName     = "";
        SALOMEDS::SObject_wrap algoSO = GetAlgoSO( error->_algo );
        if ( !algoSO->_is_nil() ) {
          CORBA::String_var algoName = algoSO->GetName();
          errStruct.algoName = algoName.in();
        }
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

SMESH::long_array*
SMESH_Gen_i::GetSubShapesId( GEOM::GEOM_Object_ptr      theMainShapeObject,
                             const SMESH::object_array& theListOfSubShapeObject )
{
  Unexpect aCatch(SALOME_SalomeException);
  if(MYDEBUG) MESSAGE( "SMESH_Gen_i::GetSubShapesId" );

  SMESH::long_array_var shapesId = new SMESH::long_array;
  set<int> setId;

  if ( CORBA::is_nil( theMainShapeObject ) )
    THROW_SALOME_CORBA_EXCEPTION( "bad shape object reference", SALOME::BAD_PARAM );

  try
  {
    TopoDS_Shape myMainShape = GeomObjectToShape(theMainShapeObject);
    TopTools_IndexedMapOfShape myIndexToShape;
    TopExp::MapShapes(myMainShape,myIndexToShape);

    for ( CORBA::ULong i = 0; i < theListOfSubShapeObject.length(); i++ )
    {
      GEOM::GEOM_Object_var aShapeObject
        = GEOM::GEOM_Object::_narrow(theListOfSubShapeObject[i]);
      if ( CORBA::is_nil( aShapeObject ) )
        THROW_SALOME_CORBA_EXCEPTION ("bad shape object reference",     \
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
{
  //MEMOSTAT;
  Unexpect aCatch(SALOME_SalomeException);
  if(MYDEBUG) MESSAGE( "SMESH_Gen_i::Compute" );

  if ( CORBA::is_nil( theShapeObject ) && theMesh->HasShapeToMesh())
    THROW_SALOME_CORBA_EXCEPTION( "bad shape object reference",
                                  SALOME::BAD_PARAM );

  if ( CORBA::is_nil( theMesh ) )
    THROW_SALOME_CORBA_EXCEPTION( "bad Mesh reference",
                                  SALOME::BAD_PARAM );

  // Update Python script
  TPythonDump(this) << "isDone = " << this << ".Compute( "
                << theMesh << ", " << theShapeObject << ")";

  try {
    // get mesh servant
    SMESH_Mesh_i* meshServant = SMESH::DownCast<SMESH_Mesh_i*>( theMesh );
    ASSERT( meshServant );
    if ( meshServant ) {
      if ( isGeomModifIcon( theMesh ))
        meshServant->Clear();
      else
        meshServant->Load();
      // NPAL16168: "geometrical group edition from a submesh don't modify mesh computation"
      meshServant->CheckGeomModif();
      // get local TopoDS_Shape
      TopoDS_Shape myLocShape;
      if(theMesh->HasShapeToMesh())
        myLocShape = GeomObjectToShape( theShapeObject );
      else
        myLocShape = SMESH_Mesh::PseudoShape();
      // call implementation compute
      ::SMESH_Mesh& myLocMesh = meshServant->GetImpl();
      myGen.PrepareCompute( myLocMesh, myLocShape );
      int how = ::SMESH_Gen::COMPACT_MESH;
      if ( myLocShape != myLocMesh.GetShapeToMesh() ) // compute a sub-mesh
        how |= ::SMESH_Gen::SHAPE_ONLY;
      bool ok = myGen.Compute( myLocMesh, myLocShape, how );
      meshServant->CreateGroupServants(); // algos can create groups (issue 0020918)
      myLocMesh.GetMeshDS()->Modified();
      UpdateIcons( theMesh );
      if ( ok )
        HighLightInvalid( theMesh, /*isInvalid=*/false );
      return ok;
    }
  }
  catch ( std::bad_alloc& ) {
    INFOS( "Compute(): lack of memory" );
  }
  catch ( SALOME_Exception& S_ex ) {
    INFOS( "Compute(): catch exception "<< S_ex.what() );
  }
  catch ( ... ) {
    INFOS( "Compute(): unknown exception " );
  }
  return false;
}

//=============================================================================
/*!
 *  SMESH_Gen_i::CancelCompute
 *
 *  Cancel Compute mesh on a shape
 */
//=============================================================================

void SMESH_Gen_i::CancelCompute( SMESH::SMESH_Mesh_ptr theMesh,
                                 GEOM::GEOM_Object_ptr theShapeObject )
{
  if ( SMESH_Mesh_i* meshServant = dynamic_cast<SMESH_Mesh_i*>( GetServant( theMesh ).in() ))
  {
    ::SMESH_Mesh& myLocMesh = meshServant->GetImpl();
    TopoDS_Shape myLocShape;
    if(theMesh->HasShapeToMesh())
      myLocShape = GeomObjectToShape( theShapeObject );
    else
      myLocShape = SMESH_Mesh::PseudoShape();
    myGen.CancelCompute( myLocMesh, myLocShape);
  }
}

//=============================================================================
/*!
 *  SMESH_Gen_i::Precompute
 *
 *  Compute mesh as preview till indicated dimension on shape
 */
//=============================================================================

SMESH::MeshPreviewStruct* SMESH_Gen_i::Precompute( SMESH::SMESH_Mesh_ptr theMesh,
                                                   GEOM::GEOM_Object_ptr theShapeObject,
                                                   SMESH::Dimension      theDimension,
                                                   SMESH::long_array&    theShapesId)
{
  Unexpect aCatch(SALOME_SalomeException);
  if(MYDEBUG) MESSAGE( "SMESH_Gen_i::Precompute" );

  if ( CORBA::is_nil( theShapeObject ) && theMesh->HasShapeToMesh())
    THROW_SALOME_CORBA_EXCEPTION( "bad shape object reference",
                                  SALOME::BAD_PARAM );

  if ( CORBA::is_nil( theMesh ) )
    THROW_SALOME_CORBA_EXCEPTION( "bad Mesh reference",
                                  SALOME::BAD_PARAM );

  SMESH::MeshPreviewStruct_var result = new SMESH::MeshPreviewStruct;
  try {
    // get mesh servant
    SMESH_Mesh_i* meshServant = dynamic_cast<SMESH_Mesh_i*>( GetServant( theMesh ).in() );
    meshServant->Load();
    ASSERT( meshServant );
    if ( meshServant ) {
      // NPAL16168: "geometrical group edition from a submesh don't modify mesh computation"
      meshServant->CheckGeomModif();
      // get local TopoDS_Shape
      TopoDS_Shape myLocShape;
      if(theMesh->HasShapeToMesh())
        myLocShape = GeomObjectToShape( theShapeObject );
      else
        return result._retn();

      // call implementation compute
      ::SMESH_Mesh& myLocMesh = meshServant->GetImpl();
      TSetOfInt shapeIds;
      ::MeshDimension aDim = (MeshDimension)theDimension;
      if ( myGen.Compute( myLocMesh, myLocShape, ::SMESH_Gen::COMPACT_MESH, aDim, &shapeIds ) )
      {
        int nbShapeId = shapeIds.size();
        theShapesId.length( nbShapeId );
        // iterates on shapes and collect mesh entities into mesh preview
        TSetOfInt::const_iterator idIt = shapeIds.begin();
        TSetOfInt::const_iterator idEnd = shapeIds.end();
        std::map< int, int > mapOfShIdNb;
        std::set< SMESH_TLink > setOfEdge;
        std::list< SMDSAbs_ElementType > listOfElemType;
        typedef map<const SMDS_MeshElement*, int > TNode2LocalIDMap;
        typedef TNode2LocalIDMap::iterator         TNodeLocalID;
        TNode2LocalIDMap mapNode2LocalID;
        list< TNodeLocalID > connectivity;
        int i, nbConnNodes = 0;
        std::set< const SMESH_subMesh* > setOfVSubMesh;
        // iterates on shapes
        for ( ; idIt != idEnd; idIt++ )
        {
          if ( mapOfShIdNb.find( *idIt ) != mapOfShIdNb.end() )
            continue;
          SMESH_subMesh* sm = myLocMesh.GetSubMeshContaining(*idIt);
          if ( !sm || !sm->IsMeshComputed() )
            continue;

          const TopoDS_Shape& aSh = sm->GetSubShape();
          const int shDim = myGen.GetShapeDim( aSh );
          if ( shDim < 1 || shDim > theDimension )
            continue;

          mapOfShIdNb[ *idIt ] = 0;
          theShapesId[ mapOfShIdNb.size() - 1 ] = *idIt;

          SMESHDS_SubMesh* smDS = sm->GetSubMeshDS();
          if ( !smDS ) continue;

          if ( theDimension == SMESH::DIM_2D )
          {
            SMDS_ElemIteratorPtr faceIt = smDS->GetElements();
            while ( faceIt->more() )
            {
              const SMDS_MeshElement* face = faceIt->next();
              int aNbNode = face->NbNodes();
              if ( aNbNode > 4 )
                aNbNode /= 2; // do not take into account additional middle nodes

              SMDS_MeshNode* node1 = (SMDS_MeshNode*)face->GetNode( 0 );
              for ( int nIndx = 0; nIndx < aNbNode; nIndx++ )
              {
                SMDS_MeshNode* node2 = (SMDS_MeshNode*)face->GetNode( nIndx+1 < aNbNode ? nIndx+1 : 0 );
                if ( setOfEdge.insert( SMESH_TLink ( node1, node2 ) ).second )
                {
                  listOfElemType.push_back( SMDSAbs_Edge );
                  connectivity.push_back
                    ( mapNode2LocalID.insert( make_pair( node1, ++nbConnNodes)).first );
                  connectivity.push_back
                    ( mapNode2LocalID.insert( make_pair( node2, ++nbConnNodes)).first );
                }
                node1 = node2;
              }
            }
          }
          else if ( theDimension == SMESH::DIM_1D )
          {
            SMDS_NodeIteratorPtr nodeIt = smDS->GetNodes();
            while ( nodeIt->more() )
            {
              listOfElemType.push_back( SMDSAbs_Node );
              connectivity.push_back
                ( mapNode2LocalID.insert( make_pair( nodeIt->next(), ++nbConnNodes)).first );
            }
            // add corner nodes by first vertex from edge
            SMESH_subMeshIteratorPtr edgeSmIt =
              sm->getDependsOnIterator(/*includeSelf*/false,
                                       /*complexShapeFirst*/false);
            while ( edgeSmIt->more() )
            {
              SMESH_subMesh* vertexSM = edgeSmIt->next();
              // check that vertex is not already treated
              if ( !setOfVSubMesh.insert( vertexSM ).second )
                continue;
              if ( vertexSM->GetSubShape().ShapeType() != TopAbs_VERTEX )
                continue;

              const SMESHDS_SubMesh* vertexSmDS = vertexSM->GetSubMeshDS();
              SMDS_NodeIteratorPtr nodeIt = vertexSmDS->GetNodes();
              while ( nodeIt->more() )
              {
                listOfElemType.push_back( SMDSAbs_Node );
                connectivity.push_back
                  ( mapNode2LocalID.insert( make_pair( nodeIt->next(), ++nbConnNodes)).first );
              }
            }
          }
        }

        // fill node coords and assign local ids to the nodes
        int nbNodes = mapNode2LocalID.size();
        result->nodesXYZ.length( nbNodes );
        TNodeLocalID node2ID = mapNode2LocalID.begin();
        for ( i = 0; i < nbNodes; ++i, ++node2ID ) {
          node2ID->second = i;
          const SMDS_MeshNode* node = (const SMDS_MeshNode*) node2ID->first;
          result->nodesXYZ[i].x = node->X();
          result->nodesXYZ[i].y = node->Y();
          result->nodesXYZ[i].z = node->Z();
        }
        // fill connectivity
        result->elementConnectivities.length( nbConnNodes );
        list< TNodeLocalID >::iterator connIt = connectivity.begin();
        for ( i = 0; i < nbConnNodes; ++i, ++connIt ) {
          result->elementConnectivities[i] = (*connIt)->second;
        }

        // fill element types
        result->elementTypes.length( listOfElemType.size() );
        std::list< SMDSAbs_ElementType >::const_iterator typeIt = listOfElemType.begin();
        std::list< SMDSAbs_ElementType >::const_iterator typeEnd = listOfElemType.end();
        for ( i = 0; typeIt != typeEnd; ++i, ++typeIt )
        {
          SMDSAbs_ElementType elemType = *typeIt;
          result->elementTypes[i].SMDS_ElementType = (SMESH::ElementType)elemType;
          result->elementTypes[i].isPoly           = false;
          result->elementTypes[i].nbNodesInElement = elemType == SMDSAbs_Edge ? 2 : 1;
        }

        // correct number of shapes
        theShapesId.length( mapOfShIdNb.size() );
      }
    }
  }
  catch ( std::bad_alloc& ) {
    INFOS( "Precompute(): lack of memory" );
  }
  catch ( SALOME_Exception& S_ex ) {
    INFOS( "Precompute(): catch exception "<< S_ex.what() );
  }
  catch ( ... ) {
    INFOS( "Precompute(): unknown exception " );
  }
  return result._retn();
}


//=============================================================================
/*!
 *  SMESH_Gen_i::Evaluate
 *
 *  Evaluate mesh on a shape
 */
//=============================================================================

SMESH::smIdType_array* SMESH_Gen_i::Evaluate(SMESH::SMESH_Mesh_ptr theMesh,
                                             GEOM::GEOM_Object_ptr theShapeObject)
{
  Unexpect aCatch(SALOME_SalomeException);
  if(MYDEBUG) MESSAGE( "SMESH_Gen_i::Evaluate" );

  if ( CORBA::is_nil( theShapeObject ) && theMesh->HasShapeToMesh())
    THROW_SALOME_CORBA_EXCEPTION( "bad shape object reference", SALOME::BAD_PARAM );

  if ( CORBA::is_nil( theMesh ) )
    THROW_SALOME_CORBA_EXCEPTION( "bad Mesh reference", SALOME::BAD_PARAM );

  SMESH::smIdType_array_var nbels = new SMESH::smIdType_array;
  nbels->length(SMESH::Entity_Last);
  int i = SMESH::Entity_Node;
  for (; i < SMESH::Entity_Last; i++)
    nbels[i] = 0;

  // Update Python script
  TPythonDump(this) << "theNbElems = " << this << ".Evaluate( "
                << theMesh << ", " << theShapeObject << ")";

  try {
    // get mesh servant
    SMESH_Mesh_i* meshServant = dynamic_cast<SMESH_Mesh_i*>( GetServant( theMesh ).in() );
    ASSERT( meshServant );
    if ( meshServant ) {
      meshServant->Load();
      // NPAL16168: "geometrical group edition from a submesh don't modify mesh computation"
      meshServant->CheckGeomModif();
      // get local TopoDS_Shape
      TopoDS_Shape myLocShape;
      if(theMesh->HasShapeToMesh())
        myLocShape = GeomObjectToShape( theShapeObject );
      else
        myLocShape = SMESH_Mesh::PseudoShape();
      // call implementation compute
      ::SMESH_Mesh& myLocMesh = meshServant->GetImpl();
      MapShapeNbElems aResMap;
      /*CORBA::Boolean ret =*/ myGen.Evaluate( myLocMesh, myLocShape, aResMap);
      MapShapeNbElemsItr anIt = aResMap.begin();
      for(; anIt!=aResMap.end(); anIt++) {
        const vector<smIdType>& aVec = (*anIt).second;
        for ( i = SMESH::Entity_Node; i < (int)aVec.size(); i++ ) {
          smIdType nbElem = aVec[i];
          if ( nbElem < 0 ) // algo failed, check that it has reported a message
          {
            SMESH_subMesh*            sm = anIt->first;
            SMESH_ComputeErrorPtr& error = sm->GetComputeError();
            const SMESH_Algo*       algo = sm->GetAlgo();
            if ( (algo && !error.get()) || error->IsOK() )
              error.reset( new SMESH_ComputeError( COMPERR_ALGO_FAILED,"Failed to evaluate",algo));
          }
          else
          {
            nbels[i] += aVec[i];
          }
        }
      }
      return nbels._retn();
    }
  }
  catch ( std::bad_alloc& ) {
    INFOS( "Evaluate(): lack of memory" );
  }
  catch ( SALOME_Exception& S_ex ) {
    INFOS( "Evaluate(): catch exception "<< S_ex.what() );
  }
  catch ( ... ) {
    INFOS( "Evaluate(): unknown exception " );
  }

  return nbels._retn();
}

//================================================================================
/*!
 * \brief Return geometrical object the given element is built on
 *  \param theMesh - the mesh the element is in
 *  \param theElementID - the element ID
 *  \param theGeomName - the name of the result geom object if it is not yet published
 *  \retval GEOM::GEOM_Object_ptr - the found or just published geom object (no need to UnRegister())
 */
//================================================================================

GEOM::GEOM_Object_ptr
SMESH_Gen_i::GetGeometryByMeshElement( SMESH::SMESH_Mesh_ptr  theMesh,
                                       SMESH::smIdType        theElementID,
                                       const char*            theGeomName)
{
  Unexpect aCatch(SALOME_SalomeException);

  GEOM::GEOM_Object_wrap geom = FindGeometryByMeshElement(theMesh, theElementID);
  if ( !geom->_is_nil() ) {
    GEOM::GEOM_Object_var mainShape = theMesh->GetShapeToMesh();
    GEOM::GEOM_Gen_var    geomGen   = GetGeomEngine( geom );

    // try to find the corresponding SObject
    SALOMEDS::SObject_wrap SObj = ObjectToSObject( geom.in() );
    if ( SObj->_is_nil() ) // submesh can be not found even if published
    {
      // try to find published submesh
      GEOM::ListOfLong_var list = geom->GetSubShapeIndices();
      if ( !geom->IsMainShape() && list->length() == 1 ) {
        SALOMEDS::SObject_wrap mainSO = ObjectToSObject( mainShape );
        SALOMEDS::ChildIterator_wrap it;
        if ( !mainSO->_is_nil() ) {
          it = getStudyServant()->NewChildIterator( mainSO );
        }
        if ( !it->_is_nil() ) {
          for ( it->InitEx(true); it->More(); it->Next() ) {
            SALOMEDS::SObject_wrap      so = it->Value();
            CORBA::Object_var         obj = SObjectToObject( so );
            GEOM::GEOM_Object_var subGeom = GEOM::GEOM_Object::_narrow( obj );
            if ( !subGeom->_is_nil() ) {
              GEOM::ListOfLong_var subList = subGeom->GetSubShapeIndices();
              if ( subList->length() == 1 && list[0] == subList[0] ) {
                SObj = so;
                geom = subGeom;
                break;
              }
            }
          }
        }
      }
    }
    if ( SObj->_is_nil() && !geomGen->_is_nil() ) // publish a new subshape
      SObj = geomGen->AddInStudy( geom, theGeomName, mainShape );

    // return only published geometry
    if ( !SObj->_is_nil() ) {
      //return geom._retn(); -- servant of geom must be UnRegister()ed;
      CORBA::Object_var    obj = SObjectToObject( SObj );
      GEOM::GEOM_Object_var go = GEOM::GEOM_Object::_narrow( obj );
      return go._retn();
    }
  }
  return GEOM::GEOM_Object::_nil();
}

//================================================================================
/*!
 * \brief Return geometrical object the given element is built on.
 *  \param theMesh - the mesh the element is in
 *  \param theElementID - the element ID
 *  \retval GEOM::GEOM_Object_ptr - the found or created (UnRegister()!) geom object
 */
//================================================================================

GEOM::GEOM_Object_ptr
SMESH_Gen_i::FindGeometryByMeshElement( SMESH::SMESH_Mesh_ptr  theMesh,
                                        SMESH::smIdType            theElementID)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( CORBA::is_nil( theMesh ) )
    THROW_SALOME_CORBA_EXCEPTION( "bad Mesh reference", SALOME::BAD_PARAM );

  GEOM::GEOM_Object_var mainShape = theMesh->GetShapeToMesh();
  GEOM::GEOM_Gen_var    geomGen   = GetGeomEngine( mainShape );

  // get a core mesh DS
  SMESH_Mesh_i* meshServant = SMESH::DownCast<SMESH_Mesh_i*>( theMesh );
  if ( meshServant && !geomGen->_is_nil() && !mainShape->_is_nil() )
  {
    ::SMESH_Mesh & mesh = meshServant->GetImpl();
    SMESHDS_Mesh* meshDS = mesh.GetMeshDS();
    // find the element in mesh
    if ( const SMDS_MeshElement * elem = meshDS->FindElement( theElementID ) ) {
      // find a shape id by the element
      if ( int shapeID = ::SMESH_MeshEditor( &mesh ).FindShape( elem )) {
        // get a geom object by the shape id
        GEOM::GEOM_Object_var geom = ShapeToGeomObject( meshDS->IndexToShape( shapeID ));
        if ( geom->_is_nil() ) {
          // try to find a published sub-shape
          SALOMEDS::SObject_wrap mainSO = ObjectToSObject( mainShape );
          SALOMEDS::ChildIterator_wrap it;
          if ( !mainSO->_is_nil() ) {
            it = getStudyServant()->NewChildIterator( mainSO );
          }
          if ( !it->_is_nil() ) {
            for ( it->InitEx(true); it->More(); it->Next() ) {
              SALOMEDS::SObject_wrap     so = it->Value();
              CORBA::Object_var         obj = SObjectToObject( so );
              GEOM::GEOM_Object_var subGeom = GEOM::GEOM_Object::_narrow( obj );
              if ( !subGeom->_is_nil() ) {
                GEOM::ListOfLong_var subList = subGeom->GetSubShapeIndices();
                if ( subList->length() == 1 && shapeID == subList[0] ) {
                  geom = subGeom;
                  break;
                }
              }
            }
          }
        }
        if ( geom->_is_nil() ) {
          // explode
          GEOM::GEOM_IShapesOperations_wrap op = geomGen->GetIShapesOperations();
          if ( !op->_is_nil() )
            geom = op->GetSubShape( mainShape, shapeID );
        }
        else {
          geom->Register();
        }
        if ( !geom->_is_nil() ) {
          GeomObjectToShape( geom ); // let geom client remember the found shape
          return geom._retn();
        }
      }
    }
  }
  return GEOM::GEOM_Object::_nil();
}

//================================================================================
/*!
 *  SMESH_Gen_i::Concatenate
 *
 *  Concatenate the given meshes into one mesh
 */
//================================================================================

SMESH::SMESH_Mesh_ptr
SMESH_Gen_i::Concatenate(const SMESH::ListOfIDSources& theMeshesArray,
                         CORBA::Boolean                theUniteIdenticalGroups,
                         CORBA::Boolean                theMergeNodesAndElements,
                         CORBA::Double                 theMergeTolerance,
                         SMESH::SMESH_Mesh_ptr         theMeshToAppendTo)
{
  return ConcatenateCommon(theMeshesArray,
                           theUniteIdenticalGroups,
                           theMergeNodesAndElements,
                           theMergeTolerance,
                           false,
                           theMeshToAppendTo);
}

//================================================================================
/*!
 *  SMESH_Gen_i::ConcatenateWithGroups
 *
 *  Concatenate the given meshes into one mesh
 *  Create the groups of all elements from initial meshes
 */
//================================================================================

SMESH::SMESH_Mesh_ptr
SMESH_Gen_i::ConcatenateWithGroups(const SMESH::ListOfIDSources& theMeshesArray,
                                   CORBA::Boolean                theUniteIdenticalGroups,
                                   CORBA::Boolean                theMergeNodesAndElements,
                                   CORBA::Double                 theMergeTolerance,
                                   SMESH::SMESH_Mesh_ptr         theMeshToAppendTo)
{
  return ConcatenateCommon(theMeshesArray,
                           theUniteIdenticalGroups,
                           theMergeNodesAndElements,
                           theMergeTolerance,
                           true,
                           theMeshToAppendTo);
}

//================================================================================
/*!
 *  SMESH_Gen_i::ConcatenateCommon
 *
 *  Concatenate the given meshes into one mesh
 */
//================================================================================

SMESH::SMESH_Mesh_ptr
SMESH_Gen_i::ConcatenateCommon(const SMESH::ListOfIDSources& theMeshesArray,
                               CORBA::Boolean                theUniteIdenticalGroups,
                               CORBA::Boolean                theMergeNodesAndElements,
                               CORBA::Double                 theMergeTolerance,
                               CORBA::Boolean                theCommonGroups,
                               SMESH::SMESH_Mesh_ptr         theMeshToAppendTo)
{
  std::unique_ptr< TPythonDump > pPythonDump( new TPythonDump(this) );
  TPythonDump& pythonDump = *pPythonDump; // prevent dump of called methods

  // create mesh if theMeshToAppendTo not provided
  SMESH::SMESH_Mesh_var newMesh;
  if ( CORBA::is_nil( theMeshToAppendTo ))
    newMesh = CreateEmptyMesh();
  else
    newMesh = SMESH::SMESH_Mesh::_duplicate( theMeshToAppendTo );
  SMESH_Mesh_i* newImpl = SMESH::DownCast<SMESH_Mesh_i*>( newMesh );
  if ( !newImpl ) return newMesh._retn();
  newImpl->Load();

  ::SMESH_Mesh&   locMesh = newImpl->GetImpl();
  SMESHDS_Mesh* newMeshDS = locMesh.GetMeshDS();

  typedef std::list<SMESH::SMESH_Group_var>          TListOfNewGroups;
  typedef std::pair<string, SMESH::ElementType >     TNameAndType;
  typedef std::map< TNameAndType, TListOfNewGroups > TGroupsMap;
  TGroupsMap       groupsMap;
  TListOfNewGroups listOfNewGroups;

  if ( !CORBA::is_nil( theMeshToAppendTo ))
  {
    // fill groupsMap with existing groups
    SMESH::ListOfGroups_var groups = theMeshToAppendTo->GetGroups();
    for ( CORBA::ULong i = 0; i < groups->length(); ++i )
    {
      SMESH::SMESH_Group_var group = SMESH::SMESH_Group::_narrow( groups[ i ]);
      if ( !group->_is_nil() )
      {
        CORBA::String_var  name = group->GetName();
        SMESH::ElementType type = group->GetType();
        groupsMap[ TNameAndType( name.in(), type ) ].push_back( group );
      }
    }
  }

  ::SMESH_MeshEditor               newEditor( &locMesh );
  ::SMESH_MeshEditor::ElemFeatures elemType;

  // loop on sub-meshes
  for ( CORBA::ULong i = 0; i < theMeshesArray.length(); i++ )
  {
    if ( CORBA::is_nil( theMeshesArray[i] )) continue;
    SMESH::SMESH_Mesh_var initMesh = theMeshesArray[i]->GetMesh();
    SMESH_Mesh_i*         initImpl = SMESH::DownCast<SMESH_Mesh_i*>( initMesh );
    if ( !initImpl ) continue;
    if ( initMesh->_is_equivalent( theMeshToAppendTo ))
      continue;
    initImpl->Load();

    // assure that IDs increment by one during iteration
    ::SMESH_Mesh& initLocMesh = initImpl->GetImpl();
    SMESHDS_Mesh*  initMeshDS = initLocMesh.GetMeshDS();
    if ( initMeshDS->MaxNodeID()    > initMeshDS->NbNodes() ||
         initMeshDS->MaxElementID() > initMeshDS->NbElements() )
    {
      initMeshDS->Modified();
      initMeshDS->CompactMesh();
    }

    // remember nb of elements before filling in
    SMESH::smIdType_array_var prevState =  newMesh->GetNbElementsByType();

    // copy nodes

    std::vector< const SMDS_MeshElement* > newNodes( initMeshDS->NbNodes() + 1, 0 );
    SMDS_ElemIteratorPtr elemIt = initImpl->GetElements( theMeshesArray[i], SMESH::NODE );
    while ( elemIt->more() )
    {
      SMESH_NodeXYZ node = elemIt->next();
      newNodes[ node->GetID() ] = newMeshDS->AddNode( node.X(), node.Y(), node.Z() );
    }

    // copy elements

    SMESH::array_of_ElementType_var srcElemTypes = theMeshesArray[i]->GetTypes();
    if ( srcElemTypes->length() == 1 && srcElemTypes[0] == SMESH::NODE ) // group of nodes
      continue;
    std::vector< const SMDS_MeshElement* > newElems( initMeshDS->NbElements() + 1, 0 );
    elemIt = initImpl->GetElements( theMeshesArray[i], SMESH::ALL );
    while ( elemIt->more() )
    {
      const SMDS_MeshElement* elem = elemIt->next();
      elemType.myNodes.resize( elem->NbNodes() );

      SMDS_NodeIteratorPtr itNodes = elem->nodeIterator();
      for ( int k = 0; itNodes->more(); k++)
      {
        const SMDS_MeshNode* node = itNodes->next();
        elemType.myNodes[ k ] = static_cast< const SMDS_MeshNode*> ( newNodes[ node->GetID() ]);
      }

      // creates a corresponding element on existent nodes in new mesh
      newElems[ elem->GetID() ] =
        newEditor.AddElement( elemType.myNodes, elemType.Init( elem, /*basicOnly=*/false ));
    }
    newEditor.ClearLastCreated(); // forget the history


    // create groups of just added elements
    SMESH::SMESH_Group_var newGroup;
    SMESH::ElementType     groupType;
    if ( theCommonGroups )
    {
      // type names
      const char* typeNames[] = { "All","Nodes","Edges","Faces","Volumes","0DElems","Balls" };
      { // check of typeNames: compilation failure mains that NB_ELEMENT_TYPES changed:
        const int nbNames = sizeof(typeNames) / sizeof(const char*);
        int _assert[( nbNames == SMESH::NB_ELEMENT_TYPES ) ? 2 : -1 ]; _assert[0]=_assert[1]=0;
      }

      SMESH::smIdType_array_var curState = newMesh->GetNbElementsByType();

      for( groupType = SMESH::NODE;
           groupType < SMESH::NB_ELEMENT_TYPES;
           groupType = (SMESH::ElementType)( groupType + 1 ))
      {
        if ( curState[ groupType ] <= prevState[ groupType ])
          continue; // no elements of groupType added from the i-th mesh

        // make a group name
        std::string groupName = "Gr";
        SALOMEDS::SObject_wrap meshSO = ObjectToSObject( theMeshesArray[i] );
        if ( meshSO ) {
          CORBA::String_var name = meshSO->GetName();
          groupName += name;
        }
        groupName += "_";
        groupName += typeNames[ groupType ];

        // make and fill a group
        newGroup = newImpl->CreateGroup( groupType, groupName.c_str() );
        std::vector< const SMDS_MeshElement* > & elemVec =
          ( groupType == SMESH::NODE ) ? newNodes : newElems;
        if ( SMESH_Group_i* grp_i = SMESH::DownCast<SMESH_Group_i*>( newGroup ))
        {
          if ( SMESHDS_Group* grpDS = dynamic_cast<SMESHDS_Group*>( grp_i->GetGroupDS() ))
          {
            for ( size_t j = 0; j < elemVec.size(); ++j )
            {
              if ( elemVec[j] && elemVec[j]->GetType() == grpDS->GetType() )
                grpDS->Add( elemVec[j] );
            }
          }
        }
        listOfNewGroups.clear();
        listOfNewGroups.push_back( newGroup );
        groupsMap.insert( std::make_pair( TNameAndType( groupName, groupType ),
                                          listOfNewGroups ));
      }
    }

    if ( SMESH_Mesh_i* initImpl = SMESH::DownCast<SMESH_Mesh_i*>( theMeshesArray[i] ))
    {
      // copy groups

      SMESH::SMESH_GroupBase_ptr group;
      CORBA::String_var          groupName;
      SMESH::smIdType_array_var newIDs = new SMESH::smIdType_array();

      // loop on groups of a source mesh
      SMESH::ListOfGroups_var listOfGroups = initImpl->GetGroups();
      for ( CORBA::ULong iG = 0; iG < listOfGroups->length(); iG++ )
      {
        group     = listOfGroups[iG];
        groupType = group->GetType();
        groupName = group->GetName();
        std::string name = groupName.in();

        // convert a list of IDs
        newIDs->length( group->Size() );
        std::vector< const SMDS_MeshElement* > & elemVec =
          ( groupType == SMESH::NODE ) ? newNodes : newElems;
        SMDS_ElemIteratorPtr itGrElems = initImpl->GetElements( group, SMESH::ALL );
        int nbElems = 0;
        while ( itGrElems->more() )
        {
          const SMDS_MeshElement*    elem = itGrElems->next();
          const SMDS_MeshElement* newElem = elemVec[ elem->GetID() ];
          if ( newElem )
            newIDs[ nbElems++ ] = newElem->GetID();
        }
        newIDs->length( nbElems );

        // check that a current group name and type don't have identical ones in final mesh
        listOfNewGroups.clear();
        TNameAndType nameAndType( name, groupType );
        TGroupsMap::iterator anIter = groupsMap.find( nameAndType );
        if ( anIter == groupsMap.end() )
        {
          // add a new group in the mesh
          newGroup = newImpl->CreateGroup( groupType, groupName.in() );
          newGroup->Add( newIDs );

          listOfNewGroups.push_back( newGroup );
          groupsMap.insert( std::make_pair( nameAndType, listOfNewGroups ));
        }
        else if ( theUniteIdenticalGroups )
        {
          // unite identical groups
          TListOfNewGroups& aNewGroups = anIter->second;
          aNewGroups.front()->Add( newIDs );
        }
        else
        {
          // rename identical groups
          newGroup = newImpl->CreateGroup( groupType, groupName );
          newGroup->Add( newIDs );

          TListOfNewGroups& newGroups = anIter->second;
          std::string newGroupName;
          if ( newGroups.size() == 1 )
          {
            newGroupName = name + "_1";
            newGroups.front()->SetName( newGroupName.c_str() );
          }
          newGroupName = name + "_" + SMESH_Comment( newGroups.size() + 1 );
          newGroup->SetName( newGroupName.c_str() );
          newGroups.push_back( newGroup );
        }
      } // loop on groups
    } // if an IDSource is a mesh
  } //meshes loop

  if ( theMergeNodesAndElements ) // merge nodes
  {
    TIDSortedNodeSet meshNodes; // no input nodes == treat all
    SMESH_MeshEditor::TListOfListOfNodes groupsOfNodes;
    newEditor.FindCoincidentNodes( meshNodes, theMergeTolerance, groupsOfNodes,
                                   /*SeparateCornersAndMedium=*/ false );
    newEditor.MergeNodes( groupsOfNodes );
    // merge elements
    newEditor.MergeEqualElements();
  }

  // Update Python script
  pythonDump << newMesh << " = " << this
             << "." << ( theCommonGroups ? "ConcatenateWithGroups" : "Concatenate" ) << "( "
             << theMeshesArray << ", "
             << theUniteIdenticalGroups << ", "
             << theMergeNodesAndElements << ", "
             << TVar( theMergeTolerance ) << ", "
             << theMeshToAppendTo << " )";

  pPythonDump.reset(); // enable python dump from GetGroups()

  // 0020577: EDF 1164 SMESH: Bad dump of concatenate with create common groups
  if ( !newMesh->_is_nil() )
  {
    SMESH::ListOfGroups_var groups = newMesh->GetGroups();
  }

  // IPAL21468 Change icon of compound because it need not be computed.
  SALOMEDS::SObject_wrap meshSO = ObjectToSObject( newMesh );
  SetPixMap( meshSO, "ICON_SMESH_TREE_MESH" );

  newMeshDS->Modified();

  return newMesh._retn();
}

//================================================================================
/*!
 * \brief Create a mesh by copying a part of another mesh
 *  \param meshPart - a part of mesh to copy
 *  \param toCopyGroups - to create in the new mesh groups
 *                        the copied elements belongs to
 *  \param toKeepIDs - to preserve IDs of the copied elements or not
 *  \retval SMESH::SMESH_Mesh_ptr - the new mesh
 */
//================================================================================

SMESH::SMESH_Mesh_ptr SMESH_Gen_i::CopyMesh(SMESH::SMESH_IDSource_ptr meshPart,
                                            const char*               meshName,
                                            CORBA::Boolean            toCopyGroups,
                                            CORBA::Boolean            toKeepIDs)
{
  Unexpect aCatch(SALOME_SalomeException);

  TPythonDump* pyDump = new TPythonDump(this); // prevent dump from CreateMesh()
  std::unique_ptr<TPythonDump> pyDumpDeleter( pyDump );

  // 1. Get source mesh

  if ( CORBA::is_nil( meshPart ))
    THROW_SALOME_CORBA_EXCEPTION( "bad IDSource", SALOME::BAD_PARAM );

  SMESH::SMESH_Mesh_var srcMesh = meshPart->GetMesh();
  SMESH_Mesh_i*       srcMesh_i = SMESH::DownCast<SMESH_Mesh_i*>( srcMesh );
  if ( !srcMesh_i )
    THROW_SALOME_CORBA_EXCEPTION( "bad mesh of IDSource", SALOME::BAD_PARAM );

  SMESHDS_Mesh* srcMeshDS = srcMesh_i->GetImpl().GetMeshDS();

  // 2. Make a new mesh

  SMESH::SMESH_Mesh_var newMesh = CreateMesh(GEOM::GEOM_Object::_nil());
  SMESH_Mesh_i*       newMesh_i = SMESH::DownCast<SMESH_Mesh_i*>( newMesh );
  if ( !newMesh_i )
    THROW_SALOME_CORBA_EXCEPTION( "can't create a mesh", SALOME::INTERNAL_ERROR );
  SALOMEDS::SObject_wrap meshSO = ObjectToSObject( newMesh );
  if ( !meshSO->_is_nil() )
  {
    SetName( meshSO, meshName, "Mesh" );
    SetPixMap( meshSO, "ICON_SMESH_TREE_MESH_IMPORTED");
  }
  SMESHDS_Mesh* newMeshDS = newMesh_i->GetImpl().GetMeshDS();
  ::SMESH_MeshEditor editor( &newMesh_i->GetImpl() );
  ::SMESH_MeshEditor::ElemFeatures elemType;

  // 3. Get elements to copy

  SMDS_ElemIteratorPtr srcElemIt; SMDS_NodeIteratorPtr srcNodeIt;
  TIDSortedElemSet srcElems;
  SMESH::array_of_ElementType_var srcElemTypes = meshPart->GetTypes();
  if ( SMESH::DownCast<SMESH_Mesh_i*>( meshPart ))
  {
    srcMesh_i->Load();
    srcElemIt = srcMeshDS->elementsIterator();
    srcNodeIt = srcMeshDS->nodesIterator();
  }
  else
  {
    SMESH::smIdType_array_var ids = meshPart->GetIDs();
    if ( srcElemTypes->length() == 1 && srcElemTypes[0] == SMESH::NODE ) // group of nodes
    {
      for ( CORBA::ULong i=0; i < ids->length(); i++ )
        if ( const SMDS_MeshElement * elem = srcMeshDS->FindNode( ids[i] ))
          srcElems.insert( elem );
    }
    else
    {
      for ( CORBA::ULong i = 0; i < ids->length(); i++ )
        if ( const SMDS_MeshElement * elem = srcMeshDS->FindElement( ids[i] ))
          srcElems.insert( elem );
    }
    if ( srcElems.empty() )
      return newMesh._retn();

    typedef SMDS_SetIterator< SMDS_pElement, TIDSortedElemSet::const_iterator > ElIter;
    srcElemIt = SMDS_ElemIteratorPtr( new ElIter( srcElems.begin(), srcElems.end() ));
  }

  // 4. Copy elements

  typedef map<SMDS_pElement, SMDS_pElement, TIDCompare> TE2EMap;
  TE2EMap e2eMapByType[ SMDSAbs_NbElementTypes ];
  TE2EMap& n2nMap = e2eMapByType[ SMDSAbs_Node ];
  int iN;
  const SMDS_MeshNode *nSrc, *nTgt;
  vector< const SMDS_MeshNode* > nodes;
  while ( srcElemIt->more() )
  {
    const SMDS_MeshElement * elem = srcElemIt->next();
    // find / add nodes
    nodes.resize( elem->NbNodes());
    SMDS_ElemIteratorPtr nIt = elem->nodesIterator();
    if ( toKeepIDs ) {
      for ( iN = 0; nIt->more(); ++iN )
      {
        nSrc = static_cast<const SMDS_MeshNode*>( nIt->next() );
        nTgt = newMeshDS->FindNode( nSrc->GetID());
        if ( !nTgt )
          nTgt = newMeshDS->AddNodeWithID( nSrc->X(), nSrc->Y(), nSrc->Z(), nSrc->GetID());
        nodes[ iN ] = nTgt;
      }
    }
    else {
      for ( iN = 0; nIt->more(); ++iN )
      {
        nSrc = static_cast<const SMDS_MeshNode*>( nIt->next() );
        TE2EMap::iterator n2n = n2nMap.insert( make_pair( nSrc, SMDS_pNode(0) )).first;
        if ( !n2n->second )
          n2n->second = newMeshDS->AddNode( nSrc->X(), nSrc->Y(), nSrc->Z() );
        nodes[ iN ] = (const SMDS_MeshNode*) n2n->second;
      }
    }
    // add elements
    if ( elem->GetType() != SMDSAbs_Node )
    {
      elemType.Init( elem, /*basicOnly=*/false );
      if ( toKeepIDs ) elemType.SetID( elem->GetID() );

      const SMDS_MeshElement * newElem = editor.AddElement( nodes, elemType );
      if ( toCopyGroups && !toKeepIDs )
        e2eMapByType[ elem->GetType() ].insert( make_pair( elem, newElem ));
    }
  } // while ( srcElemIt->more() )

  // 4(b). Copy free nodes

  if ( srcNodeIt && srcMeshDS->NbNodes() != newMeshDS->NbNodes() )
  {
    while ( srcNodeIt->more() )
    {
      nSrc = srcNodeIt->next();
      if ( nSrc->NbInverseElements() == 0 )
      {
        if ( toKeepIDs )
          nTgt = newMeshDS->AddNodeWithID( nSrc->X(), nSrc->Y(), nSrc->Z(), nSrc->GetID());
        else
          n2nMap[ nSrc ] = newMeshDS->AddNode( nSrc->X(), nSrc->Y(), nSrc->Z() );
      }
    }
  }

  // 5. Copy groups

  int nbNewGroups = 0;
  if ( toCopyGroups )
  {
    SMESH_Mesh::GroupIteratorPtr gIt = srcMesh_i->GetImpl().GetGroups();
    while ( gIt->more() )
    {
      SMESH_Group* group = gIt->next();
      const SMESHDS_GroupBase* groupDS = group->GetGroupDS();

      // Check group type. We copy nodal groups containing nodes of copied element
      SMDSAbs_ElementType groupType = groupDS->GetType();
      if ( groupType != SMDSAbs_Node &&
           newMeshDS->GetMeshInfo().NbElements( groupType ) == 0 )
        continue; // group type differs from types of meshPart

      // Find copied elements in the group
      vector< const SMDS_MeshElement* > groupElems;
      SMDS_ElemIteratorPtr eIt = groupDS->GetElements();
      if ( toKeepIDs )
      {
        const SMDS_MeshElement* foundElem;
        if ( groupType == SMDSAbs_Node )
        {
          while ( eIt->more() )
            if (( foundElem = newMeshDS->FindNode( eIt->next()->GetID() )))
              groupElems.push_back( foundElem );
        }
        else
        {
          while ( eIt->more() )
            if (( foundElem = newMeshDS->FindElement( eIt->next()->GetID() )))
              groupElems.push_back( foundElem );
        }
      }
      else
      {
        TE2EMap & e2eMap = e2eMapByType[ groupDS->GetType() ];
        if ( e2eMap.empty() ) continue;
        smIdType minID = e2eMap.begin()->first->GetID();
        smIdType maxID = e2eMap.rbegin()->first->GetID();
        TE2EMap::iterator e2e;
        while ( eIt->more() && groupElems.size() < e2eMap.size())
        {
          const SMDS_MeshElement* e = eIt->next();
          if ( e->GetID() < minID || e->GetID() > maxID ) continue;
          if ((e2e = e2eMap.find( e )) != e2eMap.end())
            groupElems.push_back( e2e->second );
        }
      }
      // Make a new group
      if ( !groupElems.empty() )
      {
        SMESH::SMESH_Group_var newGroupObj =
          newMesh->CreateGroup( SMESH::ElementType(groupType), group->GetName() );
        if ( SMESH_GroupBase_i* newGroup_i = SMESH::DownCast<SMESH_GroupBase_i*>( newGroupObj))
        {
          SMESHDS_GroupBase * newGroupDS = newGroup_i->GetGroupDS();
          SMDS_MeshGroup& smdsGroup = ((SMESHDS_Group*)newGroupDS)->SMDSGroup();
          for ( unsigned i = 0; i < groupElems.size(); ++i )
            smdsGroup.Add( groupElems[i] );

          nbNewGroups++;
        }
      }
    }
  }

  newMeshDS->Modified();

  *pyDump << newMesh << " = " << this
          << ".CopyMesh( " << meshPart << ", "
          << "'" << meshName << "', "
          << toCopyGroups << ", "
          << toKeepIDs << ")";

  pyDumpDeleter.reset(); // allow dump in GetGroups()

  if ( nbNewGroups > 0 ) // dump created groups
    SMESH::ListOfGroups_var groups = newMesh->GetGroups();

  return newMesh._retn();
}


namespace // utils for CopyMeshWithGeom()
{
  typedef std::map< std::string, std::string >             TStr2StrMap;
  typedef std::map< std::string, std::set< std::string > > TStr2StrSetMap;
  typedef std::map< std::set<int>, int >                   TIdSet2IndexMap;
  typedef std::map< std::string, int >                     TName2IndexMap;

  //================================================================================
  /*!
   * \brief Return a new sub-shape corresponding to an old one
   */
  //================================================================================

  struct ShapeMapper
  {
    SMESH_Mesh_i* mySrcMesh_i;
    SMESH_Mesh_i* myNewMesh_i;
    SMESH_Gen_i*  myGen_i;
    bool          myToPublish;
    bool          myIsSameGeom;

    TStr2StrMap   myOld2NewEntryMap; // map of study entries

    GEOM::ListOfGO_var         mySubshapes; // sub-shapes existing in the new geometry
    TIdSet2IndexMap            myIds2SubshapeIndex; // to find an existing sub-shape
    TName2IndexMap             myName2SubshapeIndex; // to find an existing sub-shape by name

    bool                       myGIPMapDone;
    GEOM::ListOfListOfLong_var myGIPMap; // filled by GetInPlaceMap()

    // not directly relating to shape search
    TStr2StrSetMap myInvalidMap; // blame shape -> invalid objects

    //================================================================================
    /*!
     * \brief Constructor
     */
    ShapeMapper( SMESH_Mesh_i* srcMesh_i,
                 SMESH_Mesh_i* newMesh_i,
                 SMESH_Gen_i*  smeshGen_i )
      : mySrcMesh_i( srcMesh_i ),
        myNewMesh_i( newMesh_i ),
        myGen_i    ( smeshGen_i ),
        myToPublish( smeshGen_i->IsEnablePublish() ),
        myGIPMapDone( false )
    {
      // retrieve from the study shape mapping made thanks to
      // "Set presentation parameters and sub-shapes from arguments" option

      GEOM::GEOM_Object_var mainShapeNew = myNewMesh_i->GetShapeToMesh();
      GEOM::GEOM_Object_var mainShapeOld = mySrcMesh_i->GetShapeToMesh();
      SALOMEDS::SObject_wrap oldSO = myGen_i->ObjectToSObject( mainShapeOld );
      SALOMEDS::SObject_wrap newSO = myGen_i->ObjectToSObject( mainShapeNew );
      if ( newSO->_is_nil() )
      {
        myToPublish = false;
        return;
      }
      if (( myIsSameGeom = mainShapeNew->_is_equivalent( mainShapeOld )))
        return;
      CORBA::String_var oldEntry = oldSO->GetID();
      CORBA::String_var newEntry = newSO->GetID();
      myOld2NewEntryMap.insert( std::make_pair( std::string( oldEntry.in() ),
                                                std::string( newEntry.in() )));
      std::string  newMainEntry = newEntry.in();

      SALOMEDS::Study_var            study = myGen_i->getStudyServant();
      GEOM::GEOM_Gen_var           geomGen = myGen_i->GetGeomEngine( mainShapeNew );
      GEOM::GEOM_IShapesOperations_wrap op = geomGen->GetIShapesOperations();
      mySubshapes                          = op->GetExistingSubObjects( mainShapeNew,
                                                                        /*groupsOnly=*/false );
      for ( CORBA::ULong i = 0; i < mySubshapes->length(); ++i )
      {
        newSO = myGen_i->ObjectToSObject( mySubshapes[ i ]);
        SALOMEDS::ChildIterator_wrap anIter = study->NewChildIterator( newSO );
        bool refFound = false;
        for ( ; anIter->More(); anIter->Next() )
        {
          SALOMEDS::SObject_wrap so = anIter->Value();
          if ( so->ReferencedObject( oldSO.inout() ))
          {
            oldEntry = oldSO->GetID();
            newEntry = newSO->GetID();
            if (( refFound = ( newMainEntry != oldEntry.in() )))
              myOld2NewEntryMap.insert( std::make_pair( std::string( oldEntry.in() ),
                                                        std::string( newEntry.in() )));
          }
        }
        if ( !refFound )
        {
          GEOM::GEOM_Object_var father = mySubshapes[ i ]->GetMainShape();
          if ( father->_is_equivalent( mainShapeNew ))
          {
            GEOM::ListOfLong_var ids = mySubshapes[ i ]->GetSubShapeIndices();
            std::set< int > idSet( &ids[0] , &ids[0] + ids->length() );
            myIds2SubshapeIndex.insert( std::make_pair( idSet, i ));
            CORBA::String_var name = newSO->GetName();
            if ( name.in()[0] )
              myName2SubshapeIndex.insert( std::make_pair( name.in(), i ));
          }
        }
      }
    }

    //================================================================================
    /*!
     * \brief Find a new sub-shape corresponding to an old one
     */
    GEOM::GEOM_Object_ptr FindNew( GEOM::GEOM_Object_ptr oldShape )
    {
      if ( myIsSameGeom )
        return GEOM::GEOM_Object::_duplicate( oldShape );

      GEOM::GEOM_Object_var newShape;

      if ( CORBA::is_nil( oldShape ))
        return newShape._retn();

      if ( !isChildOfOld( oldShape ))
        return GEOM::GEOM_Object::_duplicate( oldShape ); // shape independent of the old shape

      GEOM::GEOM_Object_var mainShapeNew = myNewMesh_i->GetShapeToMesh();
      GEOM::GEOM_Gen_var         geomGen = myGen_i->GetGeomEngine( mainShapeNew );

      // try to find by entry or name
      if ( myToPublish )
      {
        CORBA::String_var  oldEntry = oldShape->GetStudyEntry();
        TStr2StrMap::iterator o2nID = myOld2NewEntryMap.find( oldEntry.in() );
        if ( o2nID != myOld2NewEntryMap.end() )
        {
          newShape = getShapeByEntry( o2nID->second );
        }
        if ( newShape->_is_nil() )
        {
          CORBA::String_var name = oldShape->GetName();
          TName2IndexMap::iterator n2ind = myName2SubshapeIndex.find( name.in() );
          if ( n2ind != myName2SubshapeIndex.end() )
          {
            newShape = GEOM::GEOM_Object::_duplicate( mySubshapes[ n2ind->second ]);
            GEOM::ListOfLong_var oldIndices = oldShape->GetSubShapeIndices();
            GEOM::ListOfLong_var newIndices = newShape->GetSubShapeIndices();
            if ( oldIndices->length() == 0 ||
                 newIndices->length() == 0 ||
                 getShapeType( myNewMesh_i, newIndices[0] ) !=
                 getShapeType( mySrcMesh_i, oldIndices[0] ))
              newShape = GEOM::GEOM_Object::_nil();
          }
        }
      }

      if ( newShape->_is_nil() )
      {
        // try to construct a new sub-shape using myGIPMap
        buildGIPMap();
        std::vector< int >   newIndices;
        GEOM::ListOfLong_var oldIndices = oldShape->GetSubShapeIndices();
        for ( CORBA::ULong i = 0; i < oldIndices->length(); ++i )
        {
          findNewIDs( oldIndices[i], newIndices );
        }
        if ( newIndices.size() < oldIndices->length() ) // issue #17096
        {
          newIndices.clear();
          newShape = getInPlace( oldShape );
        }
        if ( !newIndices.empty() && newShape->_is_nil() )
        {
          // search for a sub-shape with same ids
          std::set< int > idSet( newIndices.begin(), newIndices.end() );
          TIdSet2IndexMap::iterator ids2ind = myIds2SubshapeIndex.find( idSet );
          if ( ids2ind != myIds2SubshapeIndex.end() ) {
            newShape = GEOM::GEOM_Object::_duplicate( mySubshapes[ ids2ind->second ]);
          }
          if ( newShape->_is_nil() )
            try
            {
              // create a new shape
              if ( newIndices.size() > 1 || oldShape->GetType() == GEOM_GROUP )
              {
                int groupType = getShapeType( myNewMesh_i, newIndices[0] );

                GEOM::GEOM_IGroupOperations_wrap grOp = geomGen->GetIGroupOperations();
                newShape = grOp->CreateGroup( mainShapeNew, groupType );

                GEOM::ListOfLong_var  newIndicesList = new GEOM::ListOfLong();
                newIndicesList->length( newIndices.size() );
                for ( size_t i = 0; i < newIndices.size(); ++i )
                  newIndicesList[ i ] = newIndices[ i ];
                grOp->UnionIDs( newShape, newIndicesList );
              }
              else
              {
                GEOM::GEOM_IShapesOperations_wrap shOp = geomGen->GetIShapesOperations();
                newShape = shOp->GetSubShape( mainShapeNew, newIndices[0] );
              }
            }
            catch (...)
            {
            }
        }
      }

      if ( !newShape->_is_nil() && myToPublish )
      {
        CORBA::String_var oldEntry, newEntry = newShape->GetStudyEntry();
        if ( !newEntry.in() || !newEntry.in()[0] )
        {
          CORBA::String_var    name = oldShape->GetName();
          SALOMEDS::SObject_wrap so = geomGen->AddInStudy( newShape, name, mainShapeNew );
          newEntry = newShape->GetStudyEntry();
          oldEntry = oldShape->GetStudyEntry();
          myOld2NewEntryMap.insert( std::make_pair( std::string( oldEntry.in() ),
                                                    std::string( newEntry.in() )));
        }
      }

      return newShape._retn();
    }

    //================================================================================
    /*!
     * \brief Return a study entry of a new shape by study entry of the old one
     */
    std::string FindNew( const std::string & oldEntry )
    {
      if ( myIsSameGeom )
        return oldEntry;

      TStr2StrMap::iterator o2nID = myOld2NewEntryMap.find( oldEntry );
      if ( o2nID != myOld2NewEntryMap.end() )
        return o2nID->second;

      GEOM::GEOM_Object_var oldShape = getShapeByEntry( oldEntry );
      if ( oldShape->_is_nil() || !isChildOfOld( oldShape ))
        return oldEntry;

      GEOM::GEOM_Object_ptr newShape = FindNew( oldShape );
      if ( newShape->_is_nil() )
        return std::string();

      CORBA::String_var newEntry = newShape->GetStudyEntry();
      return newEntry.in();
    }

    //================================================================================
    /*!
     * \brief Return a sub-shape ID of a new shape by a sub-shape ID of the old one.
     *        Return zero if not found or there are more than one new ID
     */
    int FindNew( int oldID )
    {
      if ( myIsSameGeom )
        return oldID;

      buildGIPMap();

      int newID = 0;

      if ( 0 < oldID && oldID < (int)myGIPMap->length() )
      {
        if ( myGIPMap[ oldID ].length() == 1 )
        {
          newID = myGIPMap[ oldID ][ 0 ];
        }
        else if ( myGIPMap[ oldID ].length() > 1 &&
                  getShapeType( mySrcMesh_i, oldID ) == TopAbs_VERTEX )
        {
          // select a meshed VERTEX
          SMESH_subMesh* newSM;
          for ( CORBA::ULong i = 0; i < myGIPMap[ oldID ].length() && !newID; ++i )
            if (( newSM = myNewMesh_i->GetImpl().GetSubMeshContaining( myGIPMap[ oldID ][ i ] )) &&
                ( !newSM->IsEmpty() ))
              newID = myGIPMap[ oldID ][ i ];
        }
      }
      return newID;
    }

    //================================================================================
    /*!
     * \brief Return a sub-shape ID of a new shape by an old sub-mesh.
     *        Return zero if the old shape is not kept as is in the new shape.
     */
    int FindNewNotChanged( SMESH_subMesh* oldSM )
    {
      if ( myIsSameGeom )
        return oldSM->GetId();

      int newID = FindNew( oldSM->GetId() );
      if ( !newID )
        return 0;

      SMESH_subMesh* newSM = myNewMesh_i->GetImpl().GetSubMeshContaining( newID );
      if ( !newSM )
        return 0;

      // consider a sub-shape as not changed if all its sub-shapes are mapped into
      // one new sub-shape of the same type.

      if ( oldSM->DependsOn().size() !=
           newSM->DependsOn().size() )
        return 0;

      SMESH_subMeshIteratorPtr srcSMIt = oldSM->getDependsOnIterator( /*includeSelf=*/true );
      while ( srcSMIt->more() )
      {
        oldSM = srcSMIt->next();
        int newSubID = FindNew( oldSM->GetId() );
        if ( getShapeType( myNewMesh_i, newSubID ) !=
             getShapeType( mySrcMesh_i, oldSM->GetId() ))
          return 0;
      }
      return newID;
    }

    //================================================================================
    /*!
     * \brief Return shape by study entry
     */
    GEOM::GEOM_Object_ptr getShapeByEntry( const std::string & entry )
    {
      GEOM::GEOM_Object_var shape;
      SALOMEDS::SObject_wrap so = myGen_i->getStudyServant()->FindObjectID( entry.c_str() );
      if ( !so->_is_nil() )
      {
        CORBA::Object_var obj = so->GetObject();
        shape = GEOM::GEOM_Object::_narrow( obj );
      }
      return shape._retn();
    }

    //================================================================================
    /*!
     * \brief Fill myGIPMap by calling GetInPlaceMap()
     */
    void buildGIPMap()
    {
      if ( !myGIPMapDone )
      {
        myGIPMapDone = true;

        GEOM::GEOM_Object_var   mainShapeNew = myNewMesh_i->GetShapeToMesh();
        GEOM::GEOM_Object_var   mainShapeOld = mySrcMesh_i->GetShapeToMesh();
        GEOM::GEOM_Gen_var           geomGen = myGen_i->GetGeomEngine( mainShapeNew );
        GEOM::GEOM_IShapesOperations_wrap op = geomGen->GetIShapesOperations();
        try
        {
          myGIPMap = op->GetInPlaceMap( mainShapeNew, mainShapeOld );
        }
        catch( ... )
        {
          myGIPMap = new GEOM::ListOfListOfLong();
        }
      }
    }

    //================================================================================
    /*!
     * \brief Get new sub-shape by calling GetInPlace()
     */
    GEOM::GEOM_Object_ptr getInPlace( GEOM::GEOM_Object_ptr oldShape )
    {
      GEOM::GEOM_Object_var newShape;

      GEOM::GEOM_Object_var   mainShapeNew = myNewMesh_i->GetShapeToMesh();
      GEOM::GEOM_Gen_var           geomGen = myGen_i->GetGeomEngine( mainShapeNew );
      GEOM::GEOM_IShapesOperations_wrap op = geomGen->GetIShapesOperations();
      try
      {
        newShape = op->GetInPlace( mainShapeNew, oldShape );
      }
      catch( ... )
      {
      }
      return newShape._retn();
    }

    //================================================================================
    /*!
     * \brief Find a new sub-shape indices by an old one in myGIPMap. Return
     *        number of found IDs
     */
    int findNewIDs( int oldID, std::vector< int >& newIDs  )
    {
      size_t prevNbIDs = newIDs.size();

      if ( 0 < oldID && oldID < (int) myGIPMap->length() )
      {
        for ( CORBA::ULong i = 0; i < myGIPMap[ oldID ].length(); ++i )
          newIDs.push_back( myGIPMap[ oldID ][ i ]);
      }
      return newIDs.size() - prevNbIDs;
    }

    //================================================================================
    /*!
     * \brief Check if an object relates to the old shape
     */
    bool isChildOfOld( GEOM::GEOM_Object_ptr oldShape )
    {
      if ( CORBA::is_nil( oldShape ))
        return false;
      GEOM::GEOM_Object_var mainShapeOld1 = mySrcMesh_i->GetShapeToMesh();
      GEOM::GEOM_Object_var mainShapeOld2 = oldShape->GetMainShape();
      return ( mainShapeOld1->_is_equivalent( mainShapeOld2 ) ||
               mainShapeOld1->_is_equivalent( oldShape ));
    }

    //================================================================================
    /*!
     * \brief Return shape type by shape ID
     */
    TopAbs_ShapeEnum getShapeType( SMESH_Mesh_i* mesh_i, int shapeID )
    {
      SMESHDS_Mesh* meshDS = mesh_i->GetImpl().GetMeshDS();
      const TopoDS_Shape& shape = meshDS->IndexToShape( shapeID );
      return shape.IsNull() ? TopAbs_SHAPE : shape.ShapeType();
    }

    //================================================================================
    /*!
     * \brief Store a source sub-shape for which a counterpart not found and
     *        a smesh object invalid due to that
     */
    void AddInvalid( GEOM::GEOM_Object_var  srcShape,
                     SALOMEDS::SObject_wrap smeshSO )
    {
      CORBA::String_var geomEntry = srcShape->GetStudyEntry();
      if ( geomEntry.in()[0] && !smeshSO->_is_nil() )
      {
        CORBA::String_var smeshEntry = smeshSO->GetID();
        myInvalidMap[ geomEntry.in() ].insert( smeshEntry.in() );
      }
    }

    //================================================================================
    /*!
     * \brief Store a source sub-shape for which a counterpart not found and
     *        a smesh object invalid due to that
     */
    void AddInvalid( std::string            geomEntry,
                     SALOMEDS::SObject_wrap smeshSO )
    {
      if ( !geomEntry.empty() )
      {
        CORBA::String_var smeshEntry = smeshSO->GetID();
        myInvalidMap[ geomEntry ].insert( smeshEntry.in() );
      }
    }

    //================================================================================
    /*!
     * \brief Store a source sub-shape for which a counterpart not found and
     *        a smesh object invalid due to that
     */
    void AddInvalid( int                    oldGeomID,
                     SALOMEDS::SObject_wrap smeshSO )
    {
      int shapeType = getShapeType( mySrcMesh_i, oldGeomID );
      if ( shapeType < 0 || shapeType > TopAbs_SHAPE )
        return;

      const char* typeName[] = { "COMPOUND","COMPSOLID","SOLID","SHELL",
                                 "FACE","WIRE","EDGE","VERTEX","SHAPE" };

      SMESH_Comment geomName( typeName[ shapeType ]);
      geomName << " #" << oldGeomID;

      CORBA::String_var smeshEntry = smeshSO->GetID();
      myInvalidMap[ geomName ].insert( smeshEntry.in() );
    }

    //================================================================================
    /*!
     * \brief Return entries of a source sub-shape for which a counterpart not found and
     *        of smesh objects invalid due to that
     */
    void GetInvalid( SMESH::string_array_out &               theInvalidEntries,
                     std::vector< SALOMEDS::SObject_wrap > & theInvalidMeshSObjects)
    {
      int nbSO = 0;
      TStr2StrSetMap::iterator entry2entrySet = myInvalidMap.begin();
      for ( ; entry2entrySet != myInvalidMap.end(); ++entry2entrySet )
      {
        nbSO += 1 + entry2entrySet->second.size();
      }
      int iSO = theInvalidMeshSObjects.size(), iEntry = 0;
      theInvalidEntries->length  ( nbSO );
      theInvalidMeshSObjects.resize( theInvalidMeshSObjects.size() + nbSO - myInvalidMap.size() );

      entry2entrySet = myInvalidMap.begin();
      for ( ; entry2entrySet != myInvalidMap.end(); ++entry2entrySet )
      {
        theInvalidEntries[ iEntry++ ] = entry2entrySet->first.c_str();

        std::set< std::string > & entrySet = entry2entrySet->second;
        std::set< std::string >::iterator entry = entrySet.begin();
        for ( ; entry != entrySet.end(); ++entry )
        {
          theInvalidEntries[ iEntry++ ] = entry->c_str();

          SALOMEDS::SObject_wrap so = myGen_i->getStudyServant()->FindObjectID( entry->c_str() );
          if ( !so->_is_nil() )
            theInvalidMeshSObjects[ iSO++ ] = so;
        }
      }
    }

  }; // struct ShapeMapper

  //================================================================================
  /*!
   * \brief Append an item to a CORBA sequence
   */
  template < class CORBA_seq, class ITEM >
  void append( CORBA_seq& seq, ITEM item )
  {
    if ( !CORBA::is_nil( item ))
    {
      seq->length( 1 + seq->length() );
      seq[ seq->length() - 1 ] = item;
    }
  }
} // namespace // utils for CopyMeshWithGeom()

//================================================================================
/*!
 * \brief Create a mesh by copying definitions of another mesh to a given geometry
 *  \param [in] sourceMesh - a mesh to copy
 *  \param [in] newGeometry - a new geometry
 *  \param [in] toCopyGroups - to create groups in the new mesh
 *  \param [in] toReuseHypotheses - if True, existing hypothesis will be used by the new mesh,
 *         otherwise new hypotheses with the same parameters will be created for the new mesh.
 *  \param [in] toCopyElements - to copy mesh elements of same sub-shapes of the two geometries
 *  \param [out] newMesh - return a new mesh
 *  \param [out] newGroups - return new groups
 *  \param [out] newSubmeshes - return new sub-meshes
 *  \param [out] newHypotheses - return new algorithms and hypotheses
 *  \param [out] invalidEntries - return study entries of objects whose
 *         counterparts are not found in the newGeometry, followed by entries
 *         of mesh sub-objects that are invalid because they depend on a not found
 *         preceding sub-shape
 *  \return CORBA::Boolean - is a success
 */
//================================================================================

CORBA::Boolean SMESH_Gen_i::CopyMeshWithGeom( SMESH::SMESH_Mesh_ptr       theSourceMesh,
                                              GEOM::GEOM_Object_ptr       theNewGeometry,
                                              const char*                 theMeshName,
                                              CORBA::Boolean              theToCopyGroups,
                                              CORBA::Boolean              theToReuseHypotheses,
                                              CORBA::Boolean              theToCopyElements,
                                              SMESH::SMESH_Mesh_out       theNewMesh,
                                              SMESH::ListOfGroups_out     theNewGroups,
                                              SMESH::submesh_array_out    theNewSubmeshes,
                                              SMESH::ListOfHypothesis_out theNewHypotheses,
                                              SMESH::string_array_out     theInvalidEntries)
{
  if ( CORBA::is_nil( theSourceMesh ) ||
       CORBA::is_nil( theNewGeometry ))
    THROW_SALOME_CORBA_EXCEPTION( "NULL arguments", SALOME::BAD_PARAM );

  if ( !theSourceMesh->HasShapeToMesh() )
    THROW_SALOME_CORBA_EXCEPTION( "Source mesh not on geometry", SALOME::BAD_PARAM );

  bool ok = true;
  SMESH_TRY;

  TPythonDump pyDump(this); // prevent dump from CreateMesh()

  theNewMesh        = CreateMesh( theNewGeometry );
  theNewGroups      = new SMESH::ListOfGroups();
  theNewSubmeshes   = new SMESH::submesh_array();
  theNewHypotheses  = new SMESH::ListOfHypothesis();
  theInvalidEntries = new SMESH::string_array();

  std::vector< SALOMEDS::SObject_wrap > invalidSObjects;

  GEOM::GEOM_Object_var srcGeom = theSourceMesh->GetShapeToMesh();
  GEOM::GEOM_Object_var geom, newGeom;
  SALOMEDS::SObject_wrap so;

  SMESH_Mesh_i* srcMesh_i = SMESH::DownCast<SMESH_Mesh_i*>( theSourceMesh );
  SMESH_Mesh_i* newMesh_i = SMESH::DownCast<SMESH_Mesh_i*>( theNewMesh );
  srcMesh_i->Load();

  ShapeMapper shapeMapper( srcMesh_i, newMesh_i, this );

  // treat hypotheses of mesh and sub-meshes
  SMESH::submesh_array_var smList = theSourceMesh->GetSubMeshes();
  for ( CORBA::ULong iSM = 0; iSM <= smList->length(); ++iSM )
  {
    bool isSubMesh = ( iSM < smList->length() );
    if ( isSubMesh )
    {
      // create a new sub-mesh
      SMESH::SMESH_subMesh_var newSM;
      geom = smList[iSM]->GetSubShape();
      so   = ObjectToSObject( smList[iSM] );
      CORBA::String_var name;
      if ( !so->_is_nil() )
        name = so->GetName();
      newGeom = shapeMapper.FindNew( geom );
      if ( newGeom->_is_nil() )
      {
        newSM = createInvalidSubMesh( theNewMesh, geom, name.in() );
        shapeMapper.AddInvalid( geom, ObjectToSObject( newSM ));
        ok = false;
      }
      else
      {
        newSM = theNewMesh->GetSubMesh( newGeom, name.in() );
      }
      append( theNewSubmeshes, newSM );

      if ( newGeom->_is_nil() )
        continue; // don't assign hypotheses
    }
    else
    {
      newGeom = GEOM::GEOM_Object::_duplicate( theNewGeometry );
      geom    = srcGeom;
      so      = ObjectToSObject( theNewMesh );
      SetName( so, theMeshName, "Mesh" );
    }

    // assign hypotheses
    SMESH::ListOfHypothesis_var hypList = theSourceMesh->GetHypothesisList( geom );
    for ( CORBA::ULong iHyp = 0; iHyp < hypList->length(); ++iHyp )
    {
      SMESH::SMESH_Hypothesis_var hyp = hypList[ iHyp ];
      SMESH_Hypothesis_i*       hyp_i = SMESH::DownCast< SMESH_Hypothesis_i* >( hyp );

      // get geometry hyp depends on
      std::vector< std::string > entryArray;
      std::vector< int >         subIDArray;
      bool dependsOnGeom = hyp_i->getObjectsDependOn( entryArray, subIDArray );

      if ( !theToReuseHypotheses || dependsOnGeom )
      {
        // create a new hypothesis
        CORBA::String_var type = hyp->GetName();
        CORBA::String_var lib  = hyp->GetLibName();
        CORBA::String_var data = hyp_i->SaveTo();
        if ( data.in()[0] )
        {
          hyp   = CreateHypothesis( type, lib );
          hyp_i = SMESH::DownCast< SMESH_Hypothesis_i* >( hyp );
          hyp_i->LoadFrom( data.in() );
          append( theNewHypotheses, hyp );
        }
      }

      // update geometry hyp depends on
      if ( dependsOnGeom )
      {
        for ( size_t iGeo = 0; iGeo < entryArray.size(); ++iGeo )
        {
          if ( !entryArray[ iGeo ].empty() )
          {
            std::string newEntry = shapeMapper.FindNew( entryArray[ iGeo ]);
            if ( newEntry.empty() )
            {
              ok = false;
              shapeMapper.AddInvalid( entryArray[ iGeo ], ObjectToSObject( hyp ));
              shapeMapper.AddInvalid( entryArray[ iGeo ], so ); // sub-mesh
            }
            entryArray[ iGeo ] = newEntry;
          }
        }
        for ( size_t iGeo = 0; iGeo < subIDArray.size(); ++iGeo )
        {
          if ( subIDArray[ iGeo ] > 0 )
          {
            int newID = shapeMapper.FindNew( subIDArray[ iGeo ]);
            if ( newID < 1 )
            {
              ok = false;
              shapeMapper.AddInvalid( subIDArray[ iGeo ], ObjectToSObject( hyp ));
              shapeMapper.AddInvalid( subIDArray[ iGeo ], so ); // sub-mesh
            }
            subIDArray[ iGeo ] = newID;
          }
        }
        if ( !hyp_i->setObjectsDependOn( entryArray, subIDArray ))
          ok = false;
      }

      CORBA::String_var errorText;
      theNewMesh->AddHypothesis( newGeom, hyp, errorText.out() );
      if ( errorText.in()[0] )
        ok = false;

    } // loop on hypotheses
  } // loop on sub-meshes and mesh


  // copy mesh elements, keeping IDs
  SMESHDS_Mesh* newMeshDS = newMesh_i->GetImpl().GetMeshDS();
  if ( theToCopyElements && theSourceMesh->NbNodes() > 0 )
  {
    ::SMESH_MeshEditor editor( &newMesh_i->GetImpl() );
    ::SMESH_MeshEditor::ElemFeatures elemData;

    SMESH_subMesh*         srcMainSM = srcMesh_i->GetImpl().GetSubMeshContaining( 1 );
    SMESH_subMeshIteratorPtr srcSMIt = srcMainSM->getDependsOnIterator( /*includeSelf=*/true,
                                                                        /*vertexLast=*/false);
    while ( srcSMIt->more() )
    {
      SMESH_subMesh* srcSM = srcSMIt->next();
      if ( srcSM->IsEmpty() )
        continue; // not yet computed
      int newID = shapeMapper.FindNewNotChanged( srcSM );
      if ( newID < 1 )
        continue;

      SMESHDS_SubMesh* srcSMDS = srcSM->GetSubMeshDS();
      SMDS_NodeIteratorPtr nIt = srcSMDS->GetNodes();
      while ( nIt->more() )
      {
        SMESH_NodeXYZ node( nIt->next() );
        const SMDS_MeshNode* newNode = newMeshDS->AddNodeWithID( node.X(), node.Y(), node.Z(),
                                                                 node->GetID() );
        const SMDS_PositionPtr pos = node->GetPosition();
        const double*           uv = pos->GetParameters();
        switch ( pos->GetTypeOfPosition() )
        {
        case SMDS_TOP_3DSPACE: newMeshDS->SetNodeInVolume( newNode, newID );               break;
        case SMDS_TOP_FACE:    newMeshDS->SetNodeOnFace  ( newNode, newID, uv[0], uv[1] ); break;
        case SMDS_TOP_EDGE:    newMeshDS->SetNodeOnEdge  ( newNode, newID, uv[0] );        break;
        case SMDS_TOP_VERTEX:  newMeshDS->SetNodeOnVertex( newNode, newID );               break;
        default: ;
        }
      }
      SMDS_ElemIteratorPtr eIt = srcSMDS->GetElements();
      while( eIt->more() )
      {
        const SMDS_MeshElement* e = eIt->next();
        elemData.Init( e, /*basicOnly=*/false );
        elemData.SetID( e->GetID() );
        elemData.myNodes.resize( e->NbNodes() );
        SMDS_NodeIteratorPtr nnIt = e->nodeIterator();
        size_t iN;
        for ( iN = 0; nnIt->more(); ++iN )
        {
          const SMDS_MeshNode* srcNode = nnIt->next();
          elemData.myNodes[ iN ] = newMeshDS->FindNode( srcNode->GetID() );
          if ( !elemData.myNodes[ iN ])
            break;
        }
        if ( iN == elemData.myNodes.size() )
          if ( const SMDS_MeshElement * newElem = editor.AddElement( elemData.myNodes, elemData ))
            newMeshDS->SetMeshElementOnShape( newElem, newID );
      }
      if ( SMESH_subMesh* newSM = newMesh_i->GetImpl().GetSubMeshContaining( newID ))
        newSM->ComputeStateEngine( SMESH_subMesh::CHECK_COMPUTE_STATE );
    }

    newMeshDS->Modified();
  }


  // treat groups

  TStr2StrMap old2newGroupMap;

  SALOME::GenericObj_wrap< SMESH::FilterManager > filterMgr = CreateFilterManager();

  SMESH::ListOfGroups_var groups = theSourceMesh->GetGroups();
  CORBA::ULong nbGroups = theToCopyGroups ? groups->length() : 0, nbAddedGroups = 0;
  for ( CORBA::ULong i = 0; i < nbGroups + nbAddedGroups; ++i )
  {
    SMESH::SMESH_Group_var         stdlGroup = SMESH::SMESH_Group::_narrow        ( groups[ i ]);
    SMESH::SMESH_GroupOnGeom_var   geomGroup = SMESH::SMESH_GroupOnGeom::_narrow  ( groups[ i ]);
    SMESH::SMESH_GroupOnFilter_var fltrGroup = SMESH::SMESH_GroupOnFilter::_narrow( groups[ i ]);

    CORBA::String_var      name = groups[ i ]->GetName();
    SMESH::ElementType elemType = groups[ i ]->GetType();

    SMESH::SMESH_GroupBase_var newGroup;

    if ( !stdlGroup->_is_nil() )
    {
      if ( newMeshDS->GetMeshInfo().NbElements( SMDSAbs_ElementType( elemType )) > 0 )
      {
        SMESH::smIdType_array_var elemIDs = stdlGroup->GetIDs();
        const bool isElem = ( elemType != SMESH::NODE );
        CORBA::ULong iE = 0;
        for ( ; iE < elemIDs->length(); ++iE ) // check if any element has been copied
          if ( newMeshDS->GetElementType( elemIDs[ iE ], isElem ) != SMDSAbs_All )
            break;
        if ( iE < elemIDs->length() )
        {
          stdlGroup = theNewMesh->CreateGroup( elemType, name );
          stdlGroup->Add( elemIDs );
          newGroup = SMESH::SMESH_GroupBase::_narrow( stdlGroup );
        }
      }
    }
    else if ( !geomGroup->_is_nil() )
    {
      GEOM::GEOM_Object_var    geom = geomGroup->GetShape();
      GEOM::GEOM_Object_var newGeom = shapeMapper.FindNew( geom );
      if ( newGeom->_is_nil() )
      {
        newGroup = theNewMesh->CreateGroup( elemType, name ); // just to notify the user
        shapeMapper.AddInvalid( geom, ObjectToSObject( newGroup ));
        ok = false;
      }
      else
      {
        newGroup = theNewMesh->CreateGroupFromGEOM( elemType, name, newGeom );
      }
    }
    else if ( !fltrGroup->_is_nil() )
    {
      // replace geometry in a filter
      SMESH::Filter_var filter = fltrGroup->GetFilter();
      SMESH::Filter::Criteria_var criteria;
      filter->GetCriteria( criteria.out() );

      bool isMissingGroup = false;
      std::vector< std::string > badEntries;

      for ( CORBA::ULong iCr = 0; iCr < criteria->length(); ++iCr )
      {
        const char* thresholdID = criteria[ iCr ].ThresholdID.in();
        switch ( criteria[ iCr ].Type )
        {
        case SMESH::FT_BelongToMeshGroup:
        {
          SALOME::GenericObj_wrap< SMESH::BelongToMeshGroup > btgg = filterMgr->CreateBelongToMeshGroup();
          btgg->SetGroupID( thresholdID );
          SMESH::SMESH_GroupBase_ptr refGroup = btgg->GetGroup();
          SALOMEDS::SObject_wrap   refGroupSO = ObjectToSObject( refGroup );
          if ( refGroupSO->_is_nil() )
            break;
          CORBA::String_var     refID = refGroupSO->GetID();
          TStr2StrMap::iterator o2nID = old2newGroupMap.find( refID.in() );
          if ( o2nID == old2newGroupMap.end() )
          {
            isMissingGroup = true; // corresponding new group not yet created
            break;
          }
          criteria[ iCr ].ThresholdID = o2nID->second.c_str();

          if ( o2nID->second.empty() ) // new referred group is invalid
            badEntries.push_back( refID.in() );
          break;
        }
        case SMESH::FT_BelongToGeom:
        case SMESH::FT_BelongToPlane:
        case SMESH::FT_BelongToCylinder:
        case SMESH::FT_BelongToGenSurface:
        case SMESH::FT_LyingOnGeom:
        {
          std::string newID = shapeMapper.FindNew( thresholdID );
          criteria[ iCr ].ThresholdID = newID.c_str();
          if ( newID.empty() )
            badEntries.push_back( thresholdID );
          break;
        }
        case SMESH::FT_ConnectedElements:
        {
          if ( thresholdID && thresholdID[0] )
          {
            std::string newID = shapeMapper.FindNew( thresholdID );
            criteria[ iCr ].ThresholdID = newID.c_str();
            if ( newID.empty() )
              badEntries.push_back( thresholdID );
          }
          break;
        }
        default:;
        }
      } // loop on criteria

      if ( isMissingGroup && i < nbGroups )
      {
        // to treat the group again
        append( groups, SMESH::SMESH_GroupBase::_duplicate( groups[ i ]));
        ++nbAddedGroups;
        continue;
      }
      SMESH::Filter_var newFilter = filterMgr->CreateFilter();
      newFilter->SetCriteria( criteria );

      newGroup = theNewMesh->CreateGroupFromFilter( elemType, name, newFilter );
      newFilter->UnRegister();

      SALOMEDS::SObject_wrap newSO = ObjectToSObject( newGroup );
      for ( size_t iEnt = 0; iEnt < badEntries.size(); ++iEnt )
        shapeMapper.AddInvalid( badEntries[ iEnt ], newSO );

      if ( isMissingGroup ) // all groups treated but a referred groups still not found
      {
        invalidSObjects.push_back( ObjectToSObject( newGroup ));
        ok = false;
      }
      if ( !badEntries.empty() )
        ok = false;

    } // treat a group on filter

    append( theNewGroups, newGroup );

    // fill old2newGroupMap
    SALOMEDS::SObject_wrap srcSO = ObjectToSObject( groups[i] );
    SALOMEDS::SObject_wrap newSO = ObjectToSObject( newGroup );
    if ( !srcSO->_is_nil() )
    {
      CORBA::String_var srcID, newID("");
      srcID = srcSO->GetID();
      if ( !newSO->_is_nil() )
        newID = newSO->GetID();
      old2newGroupMap.insert( std::make_pair( std::string( srcID.in() ),
                                              std::string( newID.in() )));
    }

    if ( newGroup->_is_nil() )
      ok = false;

  } // loop on groups

  newMeshDS->CompactMesh();

  // set mesh name
  if ( !theMeshName || !theMeshName[0] )
  {
    SALOMEDS::SObject_wrap soNew = ObjectToSObject( theNewMesh );
    SALOMEDS::SObject_wrap soOld = ObjectToSObject( theSourceMesh );
    CORBA::String_var oldName = soOld->GetName();
    SetName( soNew, oldName.in(), "Mesh" );
  }
  // mark invalid objects
  shapeMapper.GetInvalid( theInvalidEntries, invalidSObjects );

  for ( size_t i = 0; i < invalidSObjects.size(); ++i )
    highLightInvalid( invalidSObjects[i].in(), true );

  pyDump << "ok, "
         << theNewMesh << ", "
         << theNewGroups << ", "
         << *theNewSubmeshes.ptr() << ", "
         << *theNewHypotheses.ptr() << ", "
         << "invalidEntries = " << this << ".CopyMeshWithGeom( "
         << theSourceMesh << ", "
         << theNewGeometry << ", "
         << "'" << theMeshName << "', "
         << theToCopyGroups << ", "
         << theToReuseHypotheses << ", "
         << theToCopyElements << " )";

  SMESH_CATCH( SMESH::throwCorbaException );

  return ok;
}

//================================================================================
/*!
 * \brief Get version of MED format being used.
 */
//================================================================================

char* SMESH_Gen_i::GetMEDFileVersion()
{
  MED::TInt majeur, mineur, release;
  majeur =  mineur = release = 0;
  MED::GetVersionRelease(majeur, mineur, release);
  std::ostringstream version;
  version << majeur << "." << mineur << "." << release;
  return CORBA::string_dup( version.str().c_str() );
}

//================================================================================
/*!
 *  SMESH_Gen_i::GetMEDVersion
 *
 *  Get MED version of the file by its name
 */
//================================================================================

char* SMESH_Gen_i::GetMEDVersion(const char* theFileName)
{
  std::string version = MED::GetMEDVersion( theFileName );
  return CORBA::string_dup( version.c_str() );
}

//================================================================================
/*!
 *  SMESH_Gen_i::CheckCompatibility
 *
 *  Check compatibility of file with MED format being used, read only.
 */
//================================================================================

CORBA::Boolean SMESH_Gen_i::CheckCompatibility(const char* theFileName)
{
  return MED::CheckCompatibility( theFileName );
}

//================================================================================
/*!
 *  SMESH_Gen_i::CheckWriteCompatibility
 *
 *  Check compatibility of file with MED format being used, for append on write.
 */
//================================================================================

CORBA::Boolean SMESH_Gen_i::CheckWriteCompatibility(const char* theFileName)
{
  return MED::CheckCompatibility( theFileName, true );
}

//================================================================================
/*!
 *  SMESH_Gen_i::GetMeshNames
 *
 *  Get names of meshes defined in file with the specified name
 */
//================================================================================
SMESH::string_array* SMESH_Gen_i::GetMeshNames(const char* theFileName)

{
  SMESH::string_array_var aResult = new SMESH::string_array();
  MED::PWrapper aMed = MED::CrWrapperR( theFileName );
  MED::TErr anErr;
  MED::TInt aNbMeshes = aMed->GetNbMeshes( &anErr );
  if( anErr >= 0 ) {
    aResult->length( aNbMeshes );
    for( MED::TInt i = 0; i < aNbMeshes; i++ ) {
      MED::PMeshInfo aMeshInfo = aMed->GetPMeshInfo( i+1 );
      aResult[i] = CORBA::string_dup( aMeshInfo->GetName().c_str() );
    }
  }
  return aResult._retn();
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
  // localizing
  Kernel_Utils::Localizer loc;

  if (!myStudyContext)
    UpdateStudy();

  // Store study contents as a set of python commands
  SavePython();

  SALOMEDS::Study_var aStudy = getStudyServant();

  // Declare a byte stream
  SALOMEDS::TMPFile_var aStreamFile;

  // Obtain a temporary dir
  TCollection_AsciiString tmpDir =
    ( isMultiFile ) ? TCollection_AsciiString( ( char* )theURL ) : ( char* )SALOMEDS_Tool::GetTmpDir().c_str();

  // Create a sequence of files processed
  SALOMEDS_Tool::ListOfFiles aFileSeq;
  aFileSeq.reserve( NUM_TMP_FILES );

  TCollection_AsciiString aStudyName( "" );
  if ( isMultiFile )
    aStudyName = ( (char*)SALOMEDS_Tool::GetNameFromPath( Kernel_Utils::encode(aStudy->URL()) ).c_str() );

  // Set names of temporary files
  TCollection_AsciiString filename =
    aStudyName + TCollection_AsciiString( "_SMESH.hdf" );        // for SMESH data itself
  TCollection_AsciiString meshfile =
    aStudyName + TCollection_AsciiString( "_SMESH_Mesh.med" );   // for mesh data to be stored in MED file
  aFileSeq.push_back(CORBA::string_dup( filename.ToCString() ));
  aFileSeq.push_back(CORBA::string_dup( meshfile.ToCString() ));
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
#ifndef WIN32 /* unix functionality */
  TCollection_AsciiString cmd("rm -f \"");
#else /* windows */
  TCollection_AsciiString cmd("del /F \"");
#endif

  cmd+=filename;
  cmd+="\" \"";
  cmd+=meshfile;
  cmd+="\"";
#ifdef WIN32
  cmd+=" 2>NUL";
#endif
  system(cmd.ToCString());

  // MED writer to be used by storage process
  DriverMED_W_SMESHDS_Mesh myWriter;
  myWriter.SetFile( meshfile.ToCString() );

  // IMP issue 20918
  // SetStoreName() to groups before storing hypotheses to let them refer to
  // groups using "store name", which is "Group <group_persistent_id>"
  {
    SALOMEDS::ChildIterator_wrap itBig = aStudy->NewChildIterator( theComponent );
    for ( ; itBig->More(); itBig->Next() ) {
      SALOMEDS::SObject_wrap gotBranch = itBig->Value();
      if ( gotBranch->Tag() > GetAlgorithmsRootTag() ) {
        CORBA::Object_var anObject = SObjectToObject( gotBranch );
        if ( !CORBA::is_nil( anObject ) ) {
          SMESH::SMESH_Mesh_var myMesh = SMESH::SMESH_Mesh::_narrow( anObject ) ;
          if ( !myMesh->_is_nil() ) {
            myMesh->Load(); // load from study file if not yet done
            TPythonDump pd(this); // not to dump GetGroups()
            SMESH::ListOfGroups_var groups = myMesh->GetGroups();
            for ( CORBA::ULong i = 0; i < groups->length(); ++i )
            {
              SMESH_GroupBase_i* grImpl = SMESH::DownCast<SMESH_GroupBase_i*>( groups[i]);
              if ( grImpl )
              {
                CORBA::String_var objStr = GetORB()->object_to_string( grImpl->_this() );
                int anId = myStudyContext->findId( string( objStr.in() ) );
                char grpName[ 30 ];
                sprintf( grpName, "Group %d %d", anId, grImpl->GetLocalID() );
                SMESHDS_GroupBase* aGrpBaseDS = grImpl->GetGroupDS();
                aGrpBaseDS->SetStoreName( grpName );
              }
            }
          }
        }
      }
    }
  }

  // Write data
  // ---> create HDF file
  aFile = new HDFfile( (char*) filename.ToCString() );
  aFile->CreateOnDisk();

  // --> iterator for top-level objects
  SALOMEDS::ChildIterator_wrap itBig = aStudy->NewChildIterator( theComponent );
  for ( ; itBig->More(); itBig->Next() ) {
    SALOMEDS::SObject_wrap gotBranch = itBig->Value();

    // --> hypotheses root branch (only one for the study)
    if ( gotBranch->Tag() == GetHypothesisRootTag() ) {
      // create hypotheses root HDF group
      aTopGroup = new HDFgroup( "Hypotheses", aFile );
      aTopGroup->CreateOnDisk();

      // iterator for all hypotheses
      SALOMEDS::ChildIterator_wrap it = aStudy->NewChildIterator( gotBranch );
      for ( ; it->More(); it->Next() ) {
        SALOMEDS::SObject_wrap mySObject = it->Value();
        CORBA::Object_var anObject = SObjectToObject( mySObject );
        if ( !CORBA::is_nil( anObject ) ) {
          SMESH::SMESH_Hypothesis_var myHyp = SMESH::SMESH_Hypothesis::_narrow( anObject );
          if ( !myHyp->_is_nil() ) {
            SMESH_Hypothesis_i* myImpl = dynamic_cast<SMESH_Hypothesis_i*>( GetServant( myHyp ).in() );
            if ( myImpl ) {
              CORBA::String_var hn = myHyp->GetName(), ln = myHyp->GetLibName();
              std::string hypname = hn.in();
              std::string libname = ln.in();
              // BUG SWP13062
              // Needs for save crossplatform libname, i.e. parth of name ( ".dll" for
              // WIN32 and ".so" for X-system) must be deleted
              int libname_len = libname.length();
#ifdef WIN32
              if( libname_len > 4 )
                libname.resize( libname_len - 4 );
#else
              // PAL17753 (Regression: missing hypothesis in restored study)
              // "lib" also should be removed from the beginning
              if( libname_len > 6 )
                libname = libname.substr( 3, libname_len - 3 - 3 );
#endif
              CORBA::String_var  objStr = GetORB()->object_to_string( anObject );
              CORBA::String_var hypdata = myImpl->SaveTo();
              int                    id = myStudyContext->findId( string( objStr.in() ));

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
              aSize[ 0 ] = strlen( hypdata.in() ) + 1;
              aDataset = new HDFdataset( "Data", aGroup, HDF_STRING, aSize, 1 );
              aDataset->CreateOnDisk();
              aDataset->WriteOnDisk( ( char* )( hypdata.in() ) );
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
      SALOMEDS::ChildIterator_wrap it = aStudy->NewChildIterator( gotBranch );
      for ( ; it->More(); it->Next() ) {
        SALOMEDS::SObject_wrap mySObject = it->Value();
        CORBA::Object_var anObject = SObjectToObject( mySObject );
        if ( !CORBA::is_nil( anObject ) ) {
          SMESH::SMESH_Hypothesis_var myHyp = SMESH::SMESH_Hypothesis::_narrow( anObject );
          if ( !myHyp->_is_nil() ) {
            SMESH_Hypothesis_i* myImpl = dynamic_cast<SMESH_Hypothesis_i*>( GetServant( myHyp ).in() );
            if ( myImpl ) {
              CORBA::String_var hn = myHyp->GetName(), ln = myHyp->GetLibName();
              std::string hypname = hn.in();
              std::string libname = ln.in();
              // BUG SWP13062
              // Needs for save crossplatform libname, i.e. parth of name ( ".dll" for
              // WIN32 and ".so" for X-system) must be deleted
              int libname_len = libname.length();
#ifdef WIN32
              if( libname_len > 4 )
                libname.resize( libname_len - 4 );
#else
              // PAL17753 (Regression: missing hypothesis in restored study)
              // "lib" also should be removed from the beginning
              if( libname_len > 6 )
                libname = libname.substr( 3, libname_len - 3 - 3 );
#endif
              CORBA::String_var  objStr = GetORB()->object_to_string( anObject );
              CORBA::String_var hypdata = myImpl->SaveTo();
              int                    id = myStudyContext->findId( string( objStr.in() ) );

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
              aSize[0] = strlen( hypdata.in() ) + 1;
              aDataset = new HDFdataset( "Data", aGroup, HDF_STRING, aSize, 1 );
              aDataset->CreateOnDisk();
              aDataset->WriteOnDisk( ( char* )( hypdata.in() ));
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
            CORBA::String_var objStr = GetORB()->object_to_string( anObject );
            int id = myStudyContext->findId( string( objStr.in() ) );
            ::SMESH_Mesh& myLocMesh = myImpl->GetImpl();
            SMESHDS_Mesh* mySMESHDSMesh = myLocMesh.GetMeshDS();
            bool hasShape = myLocMesh.HasShapeToMesh();

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

            // ouv : NPAL12872
            // for each mesh open the HDF group basing on its auto color parameter
            char meshAutoColorName[ 30 ];
            sprintf( meshAutoColorName, "AutoColorMesh %d", id );
            int anAutoColor[1];
            anAutoColor[0] = myImpl->GetAutoColor();
            aSize[ 0 ] = 1;
            aDataset = new HDFdataset( meshAutoColorName, aTopGroup, HDF_INT32, aSize, 1 );
            aDataset->CreateOnDisk();
            aDataset->WriteOnDisk( anAutoColor );
            aDataset->CloseOnDisk();

            // issue 0020693. Store _isModified flag
            int isModified = myLocMesh.GetIsModified();
            aSize[ 0 ] = 1;
            aDataset = new HDFdataset( "_isModified", aTopGroup, HDF_INT32, aSize, 1 );
            aDataset->CreateOnDisk();
            aDataset->WriteOnDisk( &isModified );
            aDataset->CloseOnDisk();

            // issue 20918. Store Persistent Id of SMESHDS_Mesh
            int meshPersistentId = mySMESHDSMesh->GetPersistentId();
            aSize[ 0 ] = 1;
            aDataset = new HDFdataset( "meshPersistentId", aTopGroup, HDF_INT32, aSize, 1 );
            aDataset->CreateOnDisk();
            aDataset->WriteOnDisk( &meshPersistentId );
            aDataset->CloseOnDisk();

            // Store SMESH_Mesh_i::_mainShapeTick
            int shapeTick = myImpl->MainShapeTick();
            aSize[ 0 ] = 1;
            aDataset = new HDFdataset( "shapeTick", aTopGroup, HDF_INT32, aSize, 1 );
            aDataset->CreateOnDisk();
            aDataset->WriteOnDisk( &shapeTick );
            aDataset->CloseOnDisk();

            // write reference on a shape if exists
            SALOMEDS::SObject_wrap myRef;
            bool shapeRefFound = false;
            bool found = gotBranch->FindSubObject( (CORBA::Long)GetRefOnShapeTag(), myRef.inout() );
            if ( found ) {
              SALOMEDS::SObject_wrap myShape;
              bool ok = myRef->ReferencedObject( myShape.inout() );
              if ( ok ) {
                CORBA::Object_var shapeObj = myShape->GetObject();
                shapeRefFound = (! CORBA::is_nil( shapeObj ));
                CORBA::String_var myRefOnObject = myShape->GetID();
                if ( shapeRefFound && myRefOnObject.in()[0] ) {
                  aSize[ 0 ] = strlen( myRefOnObject.in() ) + 1;
                  aDataset = new HDFdataset( "Ref on shape", aTopGroup, HDF_STRING, aSize, 1 );
                  aDataset->CreateOnDisk();
                  aDataset->WriteOnDisk( ( char* )( myRefOnObject.in() ) );
                  aDataset->CloseOnDisk();
                }
              }
            }

            // Store file info
            std::string info = myImpl->FileInfoToString();
            if ( !info.empty() )
            {
              aSize[ 0 ] = info.size();
              aDataset = new HDFdataset( "file info", aTopGroup, HDF_STRING, aSize, 1 );
              aDataset->CreateOnDisk();
              aDataset->WriteOnDisk( (char*) info.data() );
              aDataset->CloseOnDisk();
            }

            // write applied hypotheses if exist
            SALOMEDS::SObject_wrap myHypBranch;
            found = gotBranch->FindSubObject( (CORBA::Long)GetRefOnAppliedHypothesisTag(), myHypBranch.inout() );
            if ( found && !shapeRefFound && hasShape ) { // remove applied hyps
              aStudy->NewBuilder()->RemoveObjectWithChildren( myHypBranch );
            }
            if ( found && (shapeRefFound || !hasShape) ) {
              aGroup = new HDFgroup( "Applied Hypotheses", aTopGroup );
              aGroup->CreateOnDisk();

              SALOMEDS::ChildIterator_wrap it = aStudy->NewChildIterator( myHypBranch );
              int hypNb = 0;
              for ( ; it->More(); it->Next() ) {
                SALOMEDS::SObject_wrap mySObject = it->Value();
                SALOMEDS::SObject_wrap myRefOnHyp;
                bool ok = mySObject->ReferencedObject( myRefOnHyp.inout() );
                if ( ok ) {
                  // san - it is impossible to recover applied hypotheses
                  //       using their entries within Load() method,
                  // for there are no AttributeIORs in the study when Load() is working.
                  // Hence, it is better to store persistent IDs of hypotheses as references to them

                  //string myRefOnObject = myRefOnHyp->GetID();
                  CORBA::Object_var anObject = SObjectToObject( myRefOnHyp );
                  CORBA::String_var objStr = GetORB()->object_to_string( anObject );
                  int id = myStudyContext->findId( string( objStr.in() ) );
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
            SALOMEDS::SObject_wrap myAlgoBranch;
            found = gotBranch->FindSubObject( GetRefOnAppliedAlgorithmsTag(),
                                              myAlgoBranch.inout() );
            if ( found && !shapeRefFound && hasShape) { // remove applied algos
              aStudy->NewBuilder()->RemoveObjectWithChildren( myAlgoBranch );
            }
            if ( found && (shapeRefFound || !hasShape)) {
              aGroup = new HDFgroup( "Applied Algorithms", aTopGroup );
              aGroup->CreateOnDisk();

              SALOMEDS::ChildIterator_wrap it = aStudy->NewChildIterator( myAlgoBranch );
              int algoNb = 0;
              for ( ; it->More(); it->Next() ) {
                SALOMEDS::SObject_wrap mySObject = it->Value();
                SALOMEDS::SObject_wrap myRefOnAlgo;
                bool ok = mySObject->ReferencedObject( myRefOnAlgo.inout() );
                if ( ok ) {
                  // san - it is impossible to recover applied algorithms
                  //       using their entries within Load() method,
                  // for there are no AttributeIORs in the study when Load() is working.
                  // Hence, it is better to store persistent IDs of algorithms as references to them

                  //string myRefOnObject = myRefOnAlgo->GetID();
                  CORBA::Object_var anObject = SObjectToObject( myRefOnAlgo );
                  CORBA::String_var objStr = GetORB()->object_to_string( anObject );
                  int id = myStudyContext->findId( string( objStr.in() ) );
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
              SALOMEDS::SObject_wrap mySubmeshBranch;
              found = gotBranch->FindSubObject( i, mySubmeshBranch.inout() );

              if ( found ) // check if there is shape reference in submeshes
              {
                bool hasShapeRef = false;
                SALOMEDS::ChildIterator_wrap itSM =
                  aStudy->NewChildIterator( mySubmeshBranch );
                for ( ; itSM->More(); itSM->Next() ) {
                  SALOMEDS::SObject_wrap mySubRef, myShape, mySObject = itSM->Value();
                  if ( mySObject->FindSubObject( GetRefOnShapeTag(), mySubRef.inout() ))
                    mySubRef->ReferencedObject( myShape.inout() );
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
                    aStudy->NewBuilder()->RemoveObjectWithChildren( mySObject );
                  }
                } // loop on submeshes of a type
                if ( !shapeRefFound || !hasShapeRef ) { // remove the whole submeshes branch
                  aStudy->NewBuilder()->RemoveObjectWithChildren( mySubmeshBranch );
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
                SALOMEDS::ChildIterator_wrap itSM = aStudy->NewChildIterator( mySubmeshBranch );
                for ( ; itSM->More(); itSM->Next() ) {
                  SALOMEDS::SObject_wrap mySObject = itSM->Value();
                  CORBA::Object_var anSubObject = SObjectToObject( mySObject );
                  if ( !CORBA::is_nil( anSubObject ))
                  {
                    SMESH::SMESH_subMesh_var mySubMesh = SMESH::SMESH_subMesh::_narrow( anSubObject ) ;
                    CORBA::String_var objStr = GetORB()->object_to_string( anSubObject );
                    int subid = myStudyContext->findId( string( objStr.in() ) );

                    // for each mesh open the HDF group basing on its id
                    char submeshGrpName[ 30 ];
                    sprintf( submeshGrpName, "SubMesh %d", subid );
                    aSubGroup = new HDFgroup( submeshGrpName, aGroup );
                    aSubGroup->CreateOnDisk();

                    // write reference on a shape, already checked if it exists
                    SALOMEDS::SObject_wrap mySubRef, myShape;
                    if ( mySObject->FindSubObject( GetRefOnShapeTag(), mySubRef.inout() ))
                      mySubRef->ReferencedObject( myShape.inout() );
                    string myRefOnObject = myShape->GetID();
                    if ( myRefOnObject.length() > 0 ) {
                      aSize[ 0 ] = myRefOnObject.length() + 1;
                      aDataset = new HDFdataset( "Ref on shape", aSubGroup, HDF_STRING, aSize, 1 );
                      aDataset->CreateOnDisk();
                      aDataset->WriteOnDisk( ( char* )( myRefOnObject.c_str() ) );
                      aDataset->CloseOnDisk();
                    }

                    // write applied hypotheses if exist
                    SALOMEDS::SObject_wrap mySubHypBranch;
                    found = mySObject->FindSubObject( GetRefOnAppliedHypothesisTag(),
                                                      mySubHypBranch.inout() );
                    if ( found ) {
                      aSubSubGroup = new HDFgroup( "Applied Hypotheses", aSubGroup );
                      aSubSubGroup->CreateOnDisk();

                      SALOMEDS::ChildIterator_wrap it = aStudy->NewChildIterator( mySubHypBranch );
                      int hypNb = 0;
                      for ( ; it->More(); it->Next() ) {
                        SALOMEDS::SObject_wrap mySubSObject = it->Value();
                        SALOMEDS::SObject_wrap myRefOnHyp;
                        bool ok = mySubSObject->ReferencedObject( myRefOnHyp.inout() );
                        if ( ok ) {
                          //string myRefOnObject = myRefOnHyp->GetID();
                          CORBA::Object_var anObject = SObjectToObject( myRefOnHyp );
                          CORBA::String_var objStr = GetORB()->object_to_string( anObject );
                          int id = myStudyContext->findId( string( objStr.in() ) );
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
                    SALOMEDS::SObject_wrap mySubAlgoBranch;
                    found = mySObject->FindSubObject( GetRefOnAppliedAlgorithmsTag(),
                                                      mySubAlgoBranch.inout() );
                    if ( found ) {
                      aSubSubGroup = new HDFgroup( "Applied Algorithms", aSubGroup );
                      aSubSubGroup->CreateOnDisk();

                      SALOMEDS::ChildIterator_wrap it =
                        aStudy->NewChildIterator( mySubAlgoBranch );
                      int algoNb = 0;
                      for ( ; it->More(); it->Next() ) {
                        SALOMEDS::SObject_wrap mySubSObject = it->Value();
                        SALOMEDS::SObject_wrap myRefOnAlgo;
                        bool ok = mySubSObject->ReferencedObject( myRefOnAlgo.inout() );
                        if ( ok ) {
                          //string myRefOnObject = myRefOnAlgo->GetID();
                          CORBA::Object_var anObject = SObjectToObject( myRefOnAlgo );
                          CORBA::String_var objStr = GetORB()->object_to_string( anObject );
                          int id = myStudyContext->findId( string( objStr.in() ) );
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

            // store submesh order if any
            const TListOfListOfInt& theOrderIds = myLocMesh.GetMeshOrder();
            const bool isNewOrederVersion = true; // old version saves ids, new one, entries
            if ( !theOrderIds.empty() && !isNewOrederVersion ) { // keep old version for reference
              char order_list[ 30 ];
              strcpy( order_list, "Mesh Order" );
              // count number of submesh ids
              int nbIDs = 0;
              TListOfListOfInt::const_iterator idIt = theOrderIds.begin();
              for ( ; idIt != theOrderIds.end(); idIt++ )
                nbIDs += (*idIt).size();
              // number of values = number of IDs +
              //                    number of lists (for separators) - 1
              int* smIDs = new int [ nbIDs + theOrderIds.size() - 1 ];
              idIt = theOrderIds.begin();
              for ( int i = 0; idIt != theOrderIds.end(); idIt++ ) {
                const TListOfInt& idList = *idIt;
                if (idIt != theOrderIds.begin()) // not first list
                  smIDs[ i++ ] = -1/* *idList.size()*/; // separator between lists
                // dump submesh ids from current list
                TListOfInt::const_iterator id_smId = idList.begin();
                for( ; id_smId != idList.end(); id_smId++ )
                  smIDs[ i++ ] = *id_smId;
              }
              // write HDF group
              aSize[ 0 ] = nbIDs + theOrderIds.size() - 1;

              aDataset = new HDFdataset( order_list, aTopGroup, HDF_INT32, aSize, 1 );
              aDataset->CreateOnDisk();
              aDataset->WriteOnDisk( smIDs );
              aDataset->CloseOnDisk();
              //
              delete[] smIDs;
            }
            if ( !theOrderIds.empty() && isNewOrederVersion )
            {
              // convert ids to entries
              std::list< std::list< std::string > > orderEntryLists;
              for ( const TListOfInt& idList : theOrderIds )
              {
                orderEntryLists.emplace_back();
                std::list< std::string > & entryList = orderEntryLists.back();
                for ( const int& id : idList )
                {
                  const TopoDS_Shape& shape = mySMESHDSMesh->IndexToShape( id );
                  GEOM::GEOM_Object_var  go = ShapeToGeomObject( shape );
                  SALOMEDS::SObject_var  so = ObjectToSObject( go );
                  if ( !so->_is_nil() )
                  {
                    CORBA::String_var entry = so->GetID();
                    entryList.emplace_back( entry.in() );
                  }
                }
              }
              // convert orderEntryLists to string
              std::ostringstream ostream;
              boost::archive::text_oarchive( ostream ) << orderEntryLists;
              std::string orderEntryString = ostream.str();

              // write HDF group
              aSize[ 0 ] = orderEntryString.size() + 1;
              aDataset = new HDFdataset( "MeshOrder_new", aTopGroup, HDF_STRING, aSize, 1 );
              aDataset->CreateOnDisk();
              aDataset->WriteOnDisk((char*) orderEntryString.data() );
              aDataset->CloseOnDisk();
            }

            // groups root sub-branch
            SALOMEDS::SObject_wrap myGroupsBranch;
            for ( int i = GetNodeGroupsTag(); i <= GetBallElementsGroupsTag(); i++ ) {
              found = gotBranch->FindSubObject( i, myGroupsBranch.inout() );
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
                else if ( i == Get0DElementsGroupsTag() )
                  strcpy( name_group, "Groups of 0D Elements" );
                else if ( i == GetBallElementsGroupsTag() )
                  strcpy( name_group, "Groups of Balls" );

                aGroup = new HDFgroup( name_group, aTopGroup );
                aGroup->CreateOnDisk();

                SALOMEDS::ChildIterator_wrap it = aStudy->NewChildIterator( myGroupsBranch );
                for ( ; it->More(); it->Next() ) {
                  SALOMEDS::SObject_wrap mySObject = it->Value();
                  CORBA::Object_var aSubObject = SObjectToObject( mySObject );
                  if ( !CORBA::is_nil( aSubObject ) ) {
                    SMESH_GroupBase_i* myGroupImpl =
                      dynamic_cast<SMESH_GroupBase_i*>( GetServant( aSubObject ).in() );
                    if ( !myGroupImpl )
                      continue;
                    SMESHDS_GroupBase* aGrpBaseDS = myGroupImpl->GetGroupDS();
                    if ( !aGrpBaseDS )
                      continue;

                    CORBA::String_var objStr = GetORB()->object_to_string( aSubObject );
                    int anId = myStudyContext->findId( string( objStr.in() ) );

                    // For each group, create a dataset named "Group <group_persistent_id>"
                    // and store the group's user name into it
                    const char*         grpName = aGrpBaseDS->GetStoreName();
                    CORBA::String_var aUserName = myGroupImpl->GetName();
                    aSize[ 0 ] = strlen( aUserName ) + 1;

                    aDataset = new HDFdataset( grpName, aGroup, HDF_STRING, aSize, 1 );
                    aDataset->CreateOnDisk();
                    aDataset->WriteOnDisk( aUserName );
                    aDataset->CloseOnDisk();

                    // ouv : NPAL12872
                    // For each group, create a dataset named "Group <group_persistent_id> Color"
                    // and store the group's color into it
                    char grpColorName[ 30 ];
                    sprintf( grpColorName, "ColorGroup %d", anId );
                    SALOMEDS::Color aColor = myGroupImpl->GetColor();
                    double anRGB[3];
                    anRGB[ 0 ] = aColor.R;
                    anRGB[ 1 ] = aColor.G;
                    anRGB[ 2 ] = aColor.B;
                    aSize[ 0 ] = 3;
                    aDataset = new HDFdataset( grpColorName, aGroup, HDF_FLOAT64, aSize, 1 );
                    aDataset->CreateOnDisk();
                    aDataset->WriteOnDisk( anRGB );
                    aDataset->CloseOnDisk();

                    // Pass SMESHDS_Group to MED writer
                    SMESHDS_Group* aGrpDS = dynamic_cast<SMESHDS_Group*>( aGrpBaseDS );
                    if ( aGrpDS )
                      myWriter.AddGroup( aGrpDS );

                    // write reference on a shape if exists
                    SMESHDS_GroupOnGeom* aGeomGrp =
                      dynamic_cast<SMESHDS_GroupOnGeom*>( aGrpBaseDS );
                    if ( aGeomGrp ) {
                      SALOMEDS::SObject_wrap mySubRef, myShape;
                      if (mySObject->FindSubObject( GetRefOnShapeTag(), mySubRef.inout() ) &&
                          mySubRef->ReferencedObject( myShape.inout() ) &&
                          !CORBA::is_nil( myShape->GetObject() ))
                      {
                        CORBA::String_var myRefOnObject = myShape->GetID();
                        if ( myRefOnObject.in()[0] ) {
                          char aRefName[ 30 ];
                          sprintf( aRefName, "Ref on shape %d", anId);
                          aSize[ 0 ] = strlen( myRefOnObject.in() ) + 1;
                          aDataset = new HDFdataset(aRefName, aGroup, HDF_STRING, aSize, 1);
                          aDataset->CreateOnDisk();
                          aDataset->WriteOnDisk( ( char* )( myRefOnObject.in() ));
                          aDataset->CloseOnDisk();
                        }
                      }
                      else // shape ref is invalid:
                      {
                        // save a group on geometry as ordinary group
                        myWriter.AddGroup( aGeomGrp );
                      }
                    }
                    else if ( SMESH_GroupOnFilter_i* aFilterGrp_i =
                              dynamic_cast<SMESH_GroupOnFilter_i*>( myGroupImpl ))
                    {
                      std::string str = aFilterGrp_i->FilterToString();
                      std::string hdfGrpName = "Filter " + SMESH_Comment(anId);
                      aSize[ 0 ] = str.length() + 1;
                      aDataset = new HDFdataset( hdfGrpName.c_str(), aGroup, HDF_STRING, aSize, 1);
                      aDataset->CreateOnDisk();
                      aDataset->WriteOnDisk( ( char* )( str.c_str() ) );
                      aDataset->CloseOnDisk();
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

              // save info on nb of elements
              SMESH_PreMeshInfo::SaveToFile( myImpl, id, aFile );

              // maybe a shape was deleted in the study
              if ( !shapeRefFound && !mySMESHDSMesh->ShapeToMesh().IsNull() && hasShape) {
                TopoDS_Shape nullShape;
                myLocMesh.ShapeToMesh( nullShape ); // remove shape referring data
              }

              SMESHDS_SubMeshIteratorPtr smIt = mySMESHDSMesh->SubMeshes();
              if ( smIt->more() )
              {
                // Store submeshes
                // ----------------
                aGroup = new HDFgroup( "Submeshes", aTopGroup );
                aGroup->CreateOnDisk();

                // each element belongs to one or none submesh,
                // so for each node/element, we store a submesh ID

                // Store submesh IDs
                for ( int isNode = 0; isNode < 2; ++isNode )
                {
                  SMDS_ElemIteratorPtr eIt =
                    mySMESHDSMesh->elementsIterator( isNode ? SMDSAbs_Node : SMDSAbs_All );
                  smIdType nbElems = isNode ? mySMESHDSMesh->NbNodes() : mySMESHDSMesh->GetMeshInfo().NbElements();
                  if ( nbElems < 1 )
                    continue;
                  std::vector<int> smIDs; smIDs.reserve( nbElems );
                  while ( eIt->more() )
                    if ( const SMDS_MeshElement* e = eIt->next())
                      smIDs.push_back( e->getshapeId() );
                  // write HDF group
                  aSize[ 0 ] = nbElems;
                  string aDSName( isNode ? "Node Submeshes" : "Element Submeshes");
                  aDataset = new HDFdataset( (char*)aDSName.c_str(), aGroup, HDF_INT32, aSize, 1 );
                  aDataset->CreateOnDisk();
                  aDataset->WriteOnDisk( & smIDs[0] );
                  aDataset->CloseOnDisk();
                }

                aGroup->CloseOnDisk();

                // Store node positions on sub-shapes (SMDS_Position):
                // ----------------------------------------------------

                aGroup = new HDFgroup( "Node Positions", aTopGroup );
                aGroup->CreateOnDisk();

                // in aGroup, create 5 datasets to contain:
                // "Nodes on Edges" - ID of node on edge
                // "Edge positions" - U parameter on node on edge
                // "Nodes on Faces" - ID of node on face
                // "Face U positions" - U parameter of node on face
                // "Face V positions" - V parameter of node on face

                // Find out nb of nodes on edges and faces
                // Collect corresponding sub-meshes
                int nbEdgeNodes = 0, nbFaceNodes = 0;
                list<SMESHDS_SubMesh*> aEdgeSM, aFaceSM;
                // loop on SMESHDS_SubMesh'es
                while ( smIt->more() )
                {
                  SMESHDS_SubMesh* aSubMesh = const_cast< SMESHDS_SubMesh* >( smIt->next() );
                  if ( aSubMesh->IsComplexSubmesh() )
                    continue; // submesh containing other submeshs
                  smIdType nbNodes = aSubMesh->NbNodes();
                  if ( nbNodes == 0 ) continue;

                  int aShapeID = aSubMesh->GetID();
                  if ( aShapeID < 1 || aShapeID > mySMESHDSMesh->MaxShapeIndex() )
                    continue;
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
                        SMDS_FacePositionPtr fPos = pos;
                        if ( fPos ) {
                          aUPos[ iNode ] = fPos->GetUParameter();
                          aVPos[ iNode ] = fPos->GetVParameter();
                          iNode++;
                        }
                        else
                          nbNodes--;
                      }
                      else { // on EDGE
                        SMDS_EdgePositionPtr ePos = pos;
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
  aStreamFile = SALOMEDS_Tool::PutFilesToStream( tmpDir.ToCString(), aFileSeq, isMultiFile );

  // Remove temporary files and directory
  if ( !isMultiFile )
    SALOMEDS_Tool::RemoveTemporaryFiles( tmpDir.ToCString(), aFileSeq, true );

  return aStreamFile._retn();
}

//=============================================================================
/*!
 *  SMESH_Gen_i::SaveASCII
 *
 *  Save SMESH module's data in ASCII format
 */
//=============================================================================

SALOMEDS::TMPFile* SMESH_Gen_i::SaveASCII( SALOMEDS::SComponent_ptr theComponent,
                                           const char*              theURL,
                                           bool                     isMultiFile ) {
  if(MYDEBUG) MESSAGE( "SMESH_Gen_i::SaveASCII" );
  SALOMEDS::TMPFile_var aStreamFile = Save( theComponent, theURL, isMultiFile );
  return aStreamFile._retn();

  //after usual saving needs to encipher binary to text string
  //Any binary symbol will be represent as "|xx" () hexadecimal format number
  int size = aStreamFile.in().length();
  _CORBA_Octet* buffer = new _CORBA_Octet[size*3+1];
  for ( int i = 0; i < size; i++ )
    sprintf( (char*)&(buffer[i*3]), "|%02x", aStreamFile[i] );

  buffer[size * 3] = '\0';

  SALOMEDS::TMPFile_var anAsciiStreamFile = new SALOMEDS::TMPFile(size*3, size*3, buffer, 1);

  return anAsciiStreamFile._retn();
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
  UpdateStudy(); // load geom data
  Kernel_Utils::Localizer loc;

  SALOMEDS::Study_var aStudy = getStudyServant();

  // Get temporary files location
  TCollection_AsciiString tmpDir =
    ( char* )( isMultiFile ? theURL : SALOMEDS_Tool::GetTmpDir().c_str() );

  // Convert the stream into sequence of files to process
  SALOMEDS_Tool::ListOfFiles aFileSeq = SALOMEDS_Tool::PutStreamToFiles( theStream,
                                                                         tmpDir.ToCString(),
                                                                         isMultiFile );
  TCollection_AsciiString aStudyName( "" );
  if ( isMultiFile ) {
    CORBA::WString_var url = aStudy->URL();
    aStudyName = (char*)SALOMEDS_Tool::GetNameFromPath( Kernel_Utils::encode(url.in()) ).c_str();
  }
  // Set names of temporary files
  TCollection_AsciiString filename = tmpDir + aStudyName + "_SMESH.hdf";
  TCollection_AsciiString meshfile = tmpDir + aStudyName + "_SMESH_Mesh.med";

  int size;
  HDFfile*    aFile;
  HDFdataset* aDataset;
  HDFgroup*   aTopGroup;
  HDFgroup*   aGroup;
  HDFgroup*   aSubGroup;
  HDFgroup*   aSubSubGroup;

  // Read data
  // ---> open HDF file
  aFile = new HDFfile( (char*) filename.ToCString() );
  try {
    aFile->OpenOnDisk( HDF_RDONLY );
  }
  catch ( HDFexception ) {
    INFOS( "Load(): " << filename << " not found!" );
    return false;
  }

  TPythonDump pd(this); // prevent dump during loading

  // For PAL13473 ("Repetitive mesh") implementation.
  // New dependencies between SMESH objects are established:
  // now hypotheses can refer to meshes, shapes and other hypotheses.
  // To keep data consistent, the following order of data restoration
  // imposed:
  // 1. Create hypotheses
  // 2. Create all meshes
  // 3. Load hypotheses' data
  // 4. All the rest

  list< pair< SMESH_Hypothesis_i*, string > >    hypDataList;
  list< pair< SMESH_Mesh_i*,       HDFgroup* > > meshGroupList;
  list< SMESH::Filter_var >                      filters;

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
              // myImpl->LoadFrom( hypdata.c_str() );
              hypDataList.push_back( make_pair( myImpl, hypdata ));
              CORBA::String_var iorString = GetORB()->object_to_string( myHyp );
              int newId = myStudyContext->findId( iorString.in() );
              myStudyContext->mapOldToNew( id, newId );
            }
            else
              if(MYDEBUG) MESSAGE( "VSR - SMESH_Gen::Load - can't get servant" );
          }
        }
      }
      // close hypotheses root HDF group
      aTopGroup->CloseOnDisk();
      aTopGroup = 0;
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
            catch( SALOME::SALOME_Exception& ex )
            {
              INFOS( "Exception during hypothesis creation: " << ex.details.text );
            }
            catch (...) {
              INFOS( "Exception during hypothesis creation" );
            }

            SMESH_Hypothesis_i* myImpl = dynamic_cast<SMESH_Hypothesis_i*>( GetServant( myHyp ).in() );
            if ( myImpl ) {
              //myImpl->LoadFrom( hypdata.c_str() );
              hypDataList.push_back( make_pair( myImpl, hypdata ));
              CORBA::String_var iorString = GetORB()->object_to_string( myHyp );
              int newId = myStudyContext->findId( iorString.in() );
              myStudyContext->mapOldToNew( id, newId );
            }
            else
              if(MYDEBUG) MESSAGE( "VSR - SMESH_Gen::Load - can't get servant" );
          }
        }
      }
      // close algorithms root HDF group
      aTopGroup->CloseOnDisk();
      aTopGroup = 0;
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
          meshGroupList.push_back( make_pair( myNewMeshImpl, aTopGroup ));

          CORBA::String_var iorString = GetORB()->object_to_string( myNewMesh );
          int newId = myStudyContext->findId( iorString.in() );
          myStudyContext->mapOldToNew( id, newId );

          // ouv : NPAL12872
          // try to read and set auto color flag
          char aMeshAutoColorName[ 30 ];
          sprintf( aMeshAutoColorName, "AutoColorMesh %d", id);
          if( aTopGroup->ExistInternalObject( aMeshAutoColorName ) )
          {
            aDataset = new HDFdataset( aMeshAutoColorName, aTopGroup );
            aDataset->OpenOnDisk();
            size = aDataset->GetSize();
            int* anAutoColor = new int[ size ];
            aDataset->ReadFromDisk( anAutoColor );
            aDataset->CloseOnDisk();
            myNewMeshImpl->GetImpl().SetAutoColor( (bool)anAutoColor[0] );
            delete [] anAutoColor;
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
              SALOMEDS::SObject_wrap shapeSO = aStudy->FindObjectID( refFromFile );
              CORBA::Object_var  shapeObject = SObjectToObject( shapeSO );
              if ( !CORBA::is_nil( shapeObject ) ) {
                aShapeObject = GEOM::GEOM_Object::_narrow( shapeObject );
                if ( !aShapeObject->_is_nil() )
                  myNewMeshImpl->SetShape( aShapeObject );
              }
            }
            delete [] refFromFile;
          }

          // issue 20918. Restore Persistent Id of SMESHDS_Mesh
          if ( aTopGroup->ExistInternalObject( "meshPersistentId" ) )
          {
            aDataset = new HDFdataset( "meshPersistentId", aTopGroup );
            aDataset->OpenOnDisk();
            size = aDataset->GetSize();
            int* meshPersistentId = new int[ size ];
            aDataset->ReadFromDisk( meshPersistentId );
            aDataset->CloseOnDisk();
            myNewMeshImpl->GetImpl().GetMeshDS()->SetPersistentId( *meshPersistentId );
            delete [] meshPersistentId;
          }

          // Restore SMESH_Mesh_i::_mainShapeTick
          if ( aTopGroup->ExistInternalObject( "shapeTick" ))
          {
            aDataset = new HDFdataset( "shapeTick", aTopGroup );
            aDataset->OpenOnDisk();
            int* shapeTick = & myNewMeshImpl->MainShapeTick();
            aDataset->ReadFromDisk( shapeTick );
            aDataset->CloseOnDisk();
          }

          // Restore file info
          if ( aTopGroup->ExistInternalObject( "file info" ))
          {
            aDataset = new HDFdataset( "file info", aTopGroup );
            aDataset->OpenOnDisk();
            size = aDataset->GetSize();
            std::string info( size, ' ');
            aDataset->ReadFromDisk( (char*) info.data() );
            aDataset->CloseOnDisk();
            myNewMeshImpl->FileInfoFromString( info );
          }
        }
      }
    } // reading MESHes

    // As all object that can be referred by hypothesis are created,
    // we can restore hypothesis data

    list< pair< SMESH_Hypothesis_i*, string > >::iterator hyp_data;
    for ( hyp_data = hypDataList.begin(); hyp_data != hypDataList.end(); ++hyp_data )
    {
      SMESH_Hypothesis_i* hyp  = hyp_data->first;
      string &            data = hyp_data->second;
      hyp->LoadFrom( data.c_str() );
    }

    // Restore the rest mesh data

    list< pair< SMESH_Mesh_i*, HDFgroup* > >::iterator meshi_group;
    for ( meshi_group = meshGroupList.begin(); meshi_group != meshGroupList.end(); ++meshi_group )
    {
      aTopGroup                   = meshi_group->second;
      SMESH_Mesh_i* myNewMeshImpl = meshi_group->first;

      GEOM::GEOM_Object_var aShapeObject = myNewMeshImpl->GetShapeToMesh();
      bool hasData = false;

      // get mesh old id
      CORBA::String_var iorString = GetORB()->object_to_string( myNewMeshImpl->_this() );
      int newId = myStudyContext->findId( iorString.in() );
      int meshOldId = myStudyContext->getOldId( newId );

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
          hasData = true;
        }
        delete [] strHasData;
      }

      // Try to get applied ALGORITHMS (mesh is not cleared by algo addition because
      // nodes and elements are not yet put into sub-meshes)
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
            int id = atoi( refFromFile );
            delete [] refFromFile;
            string anIOR = myStudyContext->getIORbyOldId( id );
            if ( !anIOR.empty() ) {
              CORBA::Object_var hypObject = GetORB()->string_to_object( anIOR.c_str() );
              if ( !CORBA::is_nil( hypObject ) ) {
                SMESH::SMESH_Hypothesis_var anHyp = SMESH::SMESH_Hypothesis::_narrow( hypObject );
                if ( !anHyp->_is_nil() && (!aShapeObject->_is_nil()
                                           || !myNewMeshImpl->HasShapeToMesh()) )
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
            int id = atoi( refFromFile );
            delete [] refFromFile;
            string anIOR = myStudyContext->getIORbyOldId( id );
            if ( !anIOR.empty() ) {
              CORBA::Object_var hypObject = GetORB()->string_to_object( anIOR.c_str() );
              if ( !CORBA::is_nil( hypObject ) ) {
                SMESH::SMESH_Hypothesis_var anHyp = SMESH::SMESH_Hypothesis::_narrow( hypObject );
                if ( !anHyp->_is_nil() && (!aShapeObject->_is_nil()
                                           || !myNewMeshImpl->HasShapeToMesh()) )
                  myNewMeshImpl->addHypothesis( aShapeObject, anHyp );
              }
            }
          }
        }
        aGroup->CloseOnDisk();
      }

      // --> try to find SUB-MESHES containers for each type of submesh
      for ( int j = GetSubMeshOnVertexTag(); j <= GetSubMeshOnCompoundTag(); j++ ) {
        const char* name_meshgroup = 0;
        if ( j == GetSubMeshOnVertexTag() )
          name_meshgroup = "SubMeshes On Vertex";
        else if ( j == GetSubMeshOnEdgeTag() )
          name_meshgroup = "SubMeshes On Edge";
        else if ( j == GetSubMeshOnWireTag() )
          name_meshgroup = "SubMeshes On Wire";
        else if ( j == GetSubMeshOnFaceTag() )
          name_meshgroup = "SubMeshes On Face";
        else if ( j == GetSubMeshOnShellTag() )
          name_meshgroup = "SubMeshes On Shell";
        else if ( j == GetSubMeshOnSolidTag() )
          name_meshgroup = "SubMeshes On Solid";
        else if ( j == GetSubMeshOnCompoundTag() )
          name_meshgroup = "SubMeshes On Compound";

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
            if ( strncmp( name_submeshgroup, "SubMesh", 7 ) == 0 ) {
              // --> get submesh id
              int subid = atoi( name_submeshgroup + 7 );
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
                  SALOMEDS::SObject_wrap subShapeSO = aStudy->FindObjectID( refFromFile );
                  CORBA::Object_var subShapeObject = SObjectToObject( subShapeSO );
                  if ( !CORBA::is_nil( subShapeObject ) ) {
                    aSubShapeObject = GEOM::GEOM_Object::_narrow( subShapeObject );
                    if ( !aSubShapeObject->_is_nil() )
                      aSubMesh = SMESH::SMESH_subMesh::_duplicate
                        ( myNewMeshImpl->createSubMesh( aSubShapeObject ) );
                    if ( aSubMesh->_is_nil() )
                      continue;
                    string iorSubString = GetORB()->object_to_string( aSubMesh );
                    int        newSubId = myStudyContext->findId( iorSubString );
                    myStudyContext->mapOldToNew( subid, newSubId );
                  }
                }
                delete [] refFromFile;
              }

              if ( aSubMesh->_is_nil() )
                continue;

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
                  if ( strncmp( name_dataset, "Algo", 4 ) == 0 ) {
                    aDataset = new HDFdataset( name_dataset, aSubSubGroup );
                    aDataset->OpenOnDisk();
                    size = aDataset->GetSize();
                    char* refFromFile = new char[ size ];
                    aDataset->ReadFromDisk( refFromFile );
                    aDataset->CloseOnDisk();

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
                    delete [] refFromFile;
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
                    delete [] refFromFile;
                  }
                }
                // close "APPLIED HYPOTHESES" hdf group
                aSubSubGroup->CloseOnDisk();
              }

              // close SUB-MESH hdf group
              aSubGroup->CloseOnDisk();
            }
          }
          // close SUB-MESHES containers hdf group
          aGroup->CloseOnDisk();
        }
      }

      // try to get GROUPS
      for ( int ii = GetNodeGroupsTag(); ii <= GetBallElementsGroupsTag(); ii++ ) {
        char name_group[ 30 ];
        if ( ii == GetNodeGroupsTag() )
          strcpy( name_group, "Groups of Nodes" );
        else if ( ii == GetEdgeGroupsTag() )
          strcpy( name_group, "Groups of Edges" );
        else if ( ii == GetFaceGroupsTag() )
          strcpy( name_group, "Groups of Faces" );
        else if ( ii == GetVolumeGroupsTag() )
          strcpy( name_group, "Groups of Volumes" );
        else if ( ii == Get0DElementsGroupsTag() )
          strcpy( name_group, "Groups of 0D Elements" );
        else if ( ii == GetBallElementsGroupsTag() )
          strcpy( name_group, "Groups of Balls" );

        if ( aTopGroup->ExistInternalObject( name_group ) ) {
          aGroup = new HDFgroup( name_group, aTopGroup );
          aGroup->OpenOnDisk();
          // PAL23514: get all names from the HDFgroup to avoid iteration on its contents
          // within aGroup->ExistInternalObject( name )
          std::vector< std::string > subNames;
          TColStd_MapOfAsciiString mapOfNames;
          aGroup->GetAllObjects( subNames );
          for ( size_t iN = 0; iN < subNames.size(); ++iN )
            mapOfNames.Add( subNames[ iN ].c_str() );
          // loop on groups
          for ( size_t j = 0; j < subNames.size(); j++ ) {
            const std::string& name_dataset = subNames[ j ];
            // check if it is a group
            if ( name_dataset.substr( 0, 5 ) == "Group" ) {
              // --> get group id
              char * endptr;
              int subid = strtol( name_dataset.data() + 5, &endptr, 10 );
              if ( subid <= 0 )
                continue;
              int groupID = -1; // group local ID (also persistent)
              if ( *endptr )
                groupID = atoi( endptr + 1 );
              aDataset = new HDFdataset( name_dataset.c_str(), aGroup );
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
              if ( mapOfNames.Contains( aRefName ))
              {
                // load mesh "Ref on shape" - it's an entry to SObject
                aDataset = new HDFdataset( aRefName, aGroup );
                aDataset->OpenOnDisk();
                size = aDataset->GetSize();
                char* refFromFile = new char[ size ];
                aDataset->ReadFromDisk( refFromFile );
                aDataset->CloseOnDisk();
                if ( strlen( refFromFile ) > 0 ) {
                  SALOMEDS::SObject_wrap shapeSO = aStudy->FindObjectID( refFromFile );
                  CORBA::Object_var shapeObject = SObjectToObject( shapeSO );
                  if ( !CORBA::is_nil( shapeObject ) ) {
                    aShapeObject = GEOM::GEOM_Object::_narrow( shapeObject );
                    if ( !aShapeObject->_is_nil() )
                      aShape = GeomObjectToShape( aShapeObject );
                  }
                }
                delete [] refFromFile;
              }
              // Try to read a filter of SMESH_GroupOnFilter
              SMESH::Filter_var filter;
              SMESH_PredicatePtr predicate;
              std::string hdfGrpName = ( SMESH_Comment( "Filter ") << subid );
              if ( mapOfNames.Contains( hdfGrpName.c_str() ))
              {
                aDataset = new HDFdataset( hdfGrpName.c_str(), aGroup );
                aDataset->OpenOnDisk();
                size = aDataset->GetSize();
                char* persistStr = new char[ size ];
                aDataset->ReadFromDisk( persistStr );
                aDataset->CloseOnDisk();
                if ( strlen( persistStr ) > 0 ) {
                  filter = SMESH_GroupOnFilter_i::StringToFilter( persistStr );
                  predicate = SMESH_GroupOnFilter_i::GetPredicate( filter );
                  filters.push_back( filter );
                }
                delete [] persistStr;
              }

              // Create group servant
              SMESH::ElementType type = (SMESH::ElementType)(ii - GetNodeGroupsTag() + 1);
              SMESH::SMESH_GroupBase_var aNewGroup = SMESH::SMESH_GroupBase::_duplicate
                ( myNewMeshImpl->createGroup( type, nameFromFile, groupID, aShape, predicate ) );
              delete [] nameFromFile;
              // Obtain a SMESHDS_Group object
              if ( aNewGroup->_is_nil() )
                continue;

              string iorSubString = GetORB()->object_to_string( aNewGroup );
              int        newSubId = myStudyContext->findId( iorSubString );
              myStudyContext->mapOldToNew( subid, newSubId );

              SMESH_GroupBase_i* aGroupImpl = SMESH::DownCast< SMESH_GroupBase_i*>( aNewGroup );
              if ( !aGroupImpl )
                continue;

              if ( SMESH_GroupOnFilter_i* aFilterGroup =
                   dynamic_cast< SMESH_GroupOnFilter_i*>( aGroupImpl ))
              {
                aFilterGroup->SetFilter( filter );
                filter->UnRegister();
              }
              SMESHDS_GroupBase* aGroupBaseDS = aGroupImpl->GetGroupDS();
              if ( !aGroupBaseDS )
                continue;

              aGroupBaseDS->SetStoreName( name_dataset.c_str() );

              // ouv : NPAL12872
              // Read color of the group
              char aGroupColorName[ 30 ];
              sprintf( aGroupColorName, "ColorGroup %d", subid);
              if ( mapOfNames.Contains( aGroupColorName ))
              {
                aDataset = new HDFdataset( aGroupColorName, aGroup );
                aDataset->OpenOnDisk();
                size = aDataset->GetSize();
                double* anRGB = new double[ size ];
                aDataset->ReadFromDisk( anRGB );
                aDataset->CloseOnDisk();
                Quantity_Color aColor( anRGB[0], anRGB[1], anRGB[2], Quantity_TOC_RGB );
                aGroupBaseDS->SetColor( aColor );
                delete [] anRGB;
              }
            }
          }
          aGroup->CloseOnDisk();
        }
      } // reading GROUPs

      // instead of reading mesh data, we read only brief information of all
      // objects: mesh, groups, sub-meshes (issue 0021208)
      if ( hasData )
      {
        SMESH_PreMeshInfo::LoadFromFile( myNewMeshImpl, meshOldId,
                                         meshfile.ToCString(), filename.ToCString(),
                                         !isMultiFile );
      }

      // read Sub-Mesh ORDER if any
      if ( aTopGroup->ExistInternalObject( "Mesh Order" )) { // old version keeps ids
        aDataset = new HDFdataset( "Mesh Order", aTopGroup );
        aDataset->OpenOnDisk();
        size = aDataset->GetSize();
        int* smIDs = new int[ size ];
        aDataset->ReadFromDisk( smIDs );
        aDataset->CloseOnDisk();
        TListOfListOfInt anOrderIds;
        anOrderIds.push_back( TListOfInt() );
        for ( int i = 0; i < size; i++ )
          if ( smIDs[ i ] < 0 ) // is separator
            anOrderIds.push_back( TListOfInt() );
          else
            anOrderIds.back().push_back(smIDs[ i ]);

        myNewMeshImpl->GetImpl().SetMeshOrder( anOrderIds );
        delete [] smIDs;
      }
      if ( aTopGroup->ExistInternalObject( "MeshOrder_new" )) // new version keeps entries
      {
        aDataset = new HDFdataset( "MeshOrder_new", aTopGroup );
        aDataset->OpenOnDisk();
        size = aDataset->GetSize();
        std::string dataString; dataString.resize( size );
        aDataset->ReadFromDisk((char*) dataString.data() );
        aDataset->CloseOnDisk();

        std::istringstream istream( dataString.data() );
        boost::archive::text_iarchive archive( istream );
        std::list< std::list< std::string > > orderEntryLists;
        try {
          archive >> orderEntryLists;
        }
        catch (...) {}

        TListOfListOfInt anOrderIds;
        for ( const std::list< std::string >& entryList : orderEntryLists )
        {
          anOrderIds.emplace_back();
          for ( const std::string & entry : entryList )
          {
            GEOM::GEOM_Object_var go = GetGeomObjectByEntry( entry );
            TopoDS_Shape       shape = GeomObjectToShape( go );
            if ( SMESH_subMesh*   sm = myNewMeshImpl->GetImpl().GetSubMesh( shape ))
              anOrderIds.back().emplace_back( sm->GetId() );
          }
        }
        myNewMeshImpl->GetImpl().SetMeshOrder( anOrderIds );
      }
    } // loop on meshes

    // update hyps needing full mesh data restored (issue 20918)
    for ( hyp_data = hypDataList.begin(); hyp_data != hypDataList.end(); ++hyp_data )
    {
      SMESH_Hypothesis_i* hyp  = hyp_data->first;
      hyp->UpdateAsMeshesRestored();
    }

    // notify algos on completed restoration to set sub-mesh event listeners
    for ( meshi_group = meshGroupList.begin(); meshi_group != meshGroupList.end(); ++meshi_group )
    {
      SMESH_Mesh_i* myNewMeshImpl = meshi_group->first;
      ::SMESH_Mesh& myLocMesh     = myNewMeshImpl->GetImpl();

      TopoDS_Shape myLocShape;
      if(myLocMesh.HasShapeToMesh())
        myLocShape = myLocMesh.GetShapeToMesh();
      else
        myLocShape = SMESH_Mesh::PseudoShape();

      myLocMesh.GetSubMesh(myLocShape)->
        ComputeStateEngine (SMESH_subMesh::SUBMESH_RESTORED);
    }

    // let filters detect dependency on mesh groups via FT_BelongToMeshGroup predicate (22877)
    list< SMESH::Filter_var >::iterator f = filters.begin();
    for ( ; f != filters.end(); ++f )
      if ( SMESH::Filter_i * fi = SMESH::DownCast< SMESH::Filter_i*>( *f ))
        fi->FindBaseObjects();


    // close mesh group
    if(aTopGroup)
      aTopGroup->CloseOnDisk();
  }
  // close HDF file
  aFile->CloseOnDisk();
  delete aFile;

  // Remove temporary files created from the stream
  if ( !isMultiFile )
  {
    SMESH_File meshFile( meshfile.ToCString() );
    if ( !meshFile ) // no meshfile exists
    {
      SALOMEDS_Tool::RemoveTemporaryFiles( tmpDir.ToCString(), aFileSeq, true );
    }
    else
    {
      Engines::Container_var container = GetContainerRef();
      if ( Engines_Container_i* container_i = SMESH::DownCast<Engines_Container_i*>( container ))
      {
        container_i->registerTemporaryFile( filename.ToCString() );
        container_i->registerTemporaryFile( meshfile.ToCString() );
        container_i->registerTemporaryFile( tmpDir.ToCString() );
      }
    }
  }

  // creation of tree nodes for all data objects in the study
  // to support tree representation customization and drag-n-drop:
  SALOMEDS::UseCaseBuilder_wrap useCaseBuilder = aStudy->GetUseCaseBuilder();
  if ( !useCaseBuilder->IsUseCaseNode( theComponent ) ) {
    useCaseBuilder->SetRootCurrent();
    useCaseBuilder->Append( theComponent ); // component object is added as the top level item
    SALOMEDS::ChildIterator_wrap it = aStudy->NewChildIterator( theComponent );
    for (it->InitEx(true); it->More(); it->Next()) {
      useCaseBuilder->AppendTo( it->Value()->GetFather(), it->Value() );
    }
  }

  return true;
}

//=============================================================================
/*!
 *  SMESH_Gen_i::LoadASCII
 *
 *  Load SMESH module's data in ASCII format
 */
//=============================================================================

bool SMESH_Gen_i::LoadASCII( SALOMEDS::SComponent_ptr theComponent,
                             const SALOMEDS::TMPFile& theStream,
                             const char*              theURL,
                             bool                     isMultiFile ) {
  if(MYDEBUG) MESSAGE( "SMESH_Gen_i::LoadASCII" );
  return Load( theComponent, theStream, theURL, isMultiFile );

  //before call main ::Load method it's need for decipher text format to
  //binary ( "|xx" => x' )
  int size = theStream.length();
  if ( int((size / 3 )*3) != size ) //error size of buffer
    return false;

  int real_size = int(size / 3);

  _CORBA_Octet* buffer = new _CORBA_Octet[real_size];
  char tmp[3];
  tmp[2]='\0';
  unsigned int c = -1;
  for ( int i = 0; i < real_size; i++ )
  {
    memcpy( &(tmp[0]), &(theStream[i*3+1]), 2 );
    sscanf( tmp, "%x", &c );
    sprintf( (char*)&(buffer[i]), "%c", (char)c );
  }

  SALOMEDS::TMPFile_var aRealStreamFile = new SALOMEDS::TMPFile(real_size, real_size, buffer, 1);

  return Load( theComponent, *(aRealStreamFile._retn()), theURL, isMultiFile );
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
  myStudyContext->Clear();

  // remove the tmp files meshes are loaded from
  SMESH_PreMeshInfo::RemoveStudyFiles_TMP_METHOD( theComponent );

  // Clean trace of API methods calls
  CleanPythonTrace();

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
  if ( myStudyContext && !CORBA::is_nil( theObject )) {
    CORBA::String_var iorString = GetORB()->object_to_string( theObject );
    return myStudyContext->addObject( string( iorString.in() ) );
  }
  return 0;
}

//================================================================================
/*!
 * \brief Return id of registered object
  * \param theObject - the Object
  * \retval int - Object id
 */
//================================================================================

CORBA::Long  SMESH_Gen_i::GetObjectId(CORBA::Object_ptr theObject)
{
  if ( myStudyContext && !CORBA::is_nil( theObject )) {
    string iorString = GetORB()->object_to_string( theObject );
    return myStudyContext->findId( iorString );
  }
  return 0;
}

//=============================================================================
/*!
 *  SMESH_Gen_i::SetName
 *
 *  Set a new object name
 */
//=============================================================================

void SMESH_Gen_i::SetName(const char* theIOR,
                          const char* theName)
{
  if ( theIOR && strcmp( theIOR, "" ) ) {
    CORBA::Object_var anObject = GetORB()->string_to_object( theIOR );
    SALOMEDS::SObject_wrap aSO = ObjectToSObject( anObject );
    if ( !aSO->_is_nil() ) {
      SetName( aSO, theName );
    }
  }
}

// Version information
char* SMESH_Gen_i::getVersion()
{
#if SMESH_DEVELOPMENT
  return CORBA::string_dup(SMESH_VERSION_STR"dev");
#else
  return CORBA::string_dup(SMESH_VERSION_STR);
#endif
}

//=================================================================================
// function : Move()
// purpose  : Moves objects to the specified position.
//            Is used in the drag-n-drop functionality.
//=================================================================================

void SMESH_Gen_i::Move( const SMESH::sobject_list& what,
                        SALOMEDS::SObject_ptr      where,
                        CORBA::Long                row )
{
  if ( CORBA::is_nil( where ) ) return;

  SALOMEDS::StudyBuilder_var studyBuilder = getStudyServant()->NewBuilder();
  SALOMEDS::UseCaseBuilder_var useCaseBuilder = getStudyServant()->GetUseCaseBuilder();
  SALOMEDS::SComponent_var father = where->GetFatherComponent();
  std::string dataType = father->ComponentDataType();
  if ( dataType != "SMESH" ) return; // not a SMESH component

  SALOMEDS::SObject_var objAfter;
  if ( row >= 0 && useCaseBuilder->HasChildren( where ) ) {
    // insert at given row -> find insertion position
    SALOMEDS::UseCaseIterator_var useCaseIt = useCaseBuilder->GetUseCaseIterator( where );
    int i;
    for ( i = 0; i < row && useCaseIt->More(); i++, useCaseIt->Next() );
    if ( i == row && useCaseIt->More() ) {
      objAfter = useCaseIt->Value();
    }
  }

  for ( CORBA::ULong i = 0; i < what.length(); i++ ) {
    SALOMEDS::SObject_var sobj = what[i];
    if ( CORBA::is_nil( sobj ) ) continue; // skip bad object
    // insert the object to the use case tree
    if ( !CORBA::is_nil( objAfter ) )
      useCaseBuilder->InsertBefore( sobj, objAfter ); // insert at given row
    else
      useCaseBuilder->AppendTo( where, sobj );        // append to the end of list
  }
}
//================================================================================
/*!
 * \brief Returns true if algorithm can be used to mesh a given geometry
 *  \param [in] theAlgoType - the algorithm type
 *  \param [in] theLibName - a name of the Plug-in library implementing the algorithm
 *  \param [in] theGeomObject - the geometry to mesh
 *  \param [in] toCheckAll - if \c True, returns \c True if all shapes are meshable,
 *         else, returns \c True if at least one shape is meshable
 *  \return CORBA::Boolean - can or can't
 */
//================================================================================

CORBA::Boolean SMESH_Gen_i::IsApplicable ( const char*           theAlgoType,
                                           const char*           theLibName,
                                           GEOM::GEOM_Object_ptr theGeomObject,
                                           CORBA::Boolean        toCheckAll)
{
  SMESH_TRY;

  std::string aPlatformLibName;
  GenericHypothesisCreator_i* aCreator =
    getHypothesisCreator(theAlgoType, theLibName, aPlatformLibName);
  if (aCreator)
  {
    TopoDS_Shape shape = GeomObjectToShape( theGeomObject );
    const SMESH_Algo::Features& feat = SMESH_Algo::GetFeatures( theAlgoType );
    return shape.IsNull() || aCreator->IsApplicable( shape, toCheckAll, feat._dim );
  }
  else
  {
    return false;
  }

  SMESH_CATCH( SMESH::doNothing );

#ifdef _DEBUG_
  cout << "SMESH_Gen_i::IsApplicable(): exception in " << ( theAlgoType ? theAlgoType : "") << endl;
#endif
  return true;
}

//================================================================================
/*!
 * \brief Collect indices of elements, which are located inside the sphere
 */
//================================================================================

SMESH::long_array* SMESH_Gen_i::GetInsideSphere( SMESH::SMESH_IDSource_ptr meshPart,
                                                 SMESH::ElementType     theElemType,
                                                 CORBA::Double         theX,
                                                 CORBA::Double         theY,
                                                 CORBA::Double         theZ,
                                                 CORBA::Double         theR)
{
  SMESH::long_array_var aResult = new SMESH::long_array();
  if ( meshPart->_is_nil() )
    return aResult._retn();

  // 1. Create geometrical object
  gp_Pnt aP( theX, theY, theZ );
  TopoDS_Shape aShape = BRepPrimAPI_MakeSphere( aP, theR ).Shape();

  std::vector<long> lst =_GetInside(meshPart, theElemType, aShape);

  if ( lst.size() > 0 ) {
    aResult->length( lst.size() );
    for ( size_t i = 0; i < lst.size(); i++ ) {
      aResult[i] = lst[i];
    }
  }
  return aResult._retn();
}

//================================================================================
/*!
 * \brief Collect indices of elements, which are located inside the box
 */
//================================================================================

SMESH::long_array* SMESH_Gen_i::GetInsideBox( SMESH::SMESH_IDSource_ptr meshPart,
                                              SMESH::ElementType        theElemType,
                                              CORBA::Double             theX1,
                                              CORBA::Double             theY1,
                                              CORBA::Double             theZ1,
                                              CORBA::Double             theX2,
                                              CORBA::Double             theY2,
                                              CORBA::Double             theZ2)
{
  SMESH::long_array_var aResult = new SMESH::long_array();
  if( meshPart->_is_nil() )
    return aResult._retn();

  TopoDS_Shape aShape = BRepPrimAPI_MakeBox( gp_Pnt( theX1, theY1, theZ1 ),
                                             gp_Pnt( theX2, theY2, theZ2 ) ).Shape();

  std::vector<long> lst =_GetInside(meshPart, theElemType, aShape);

  if( lst.size() > 0 ) {
    aResult->length( lst.size() );
    for ( size_t i = 0; i < lst.size(); i++ ) {
      aResult[i] = lst[i];
    }
  }
  return aResult._retn();
}

//================================================================================
/*!
 * \brief Collect indices of elements, which are located inside the cylinder
 */
//================================================================================

SMESH::long_array* SMESH_Gen_i::GetInsideCylinder( SMESH::SMESH_IDSource_ptr meshPart,
                                                   SMESH::ElementType        theElemType,
                                                   CORBA::Double             theX,
                                                   CORBA::Double             theY,
                                                   CORBA::Double             theZ,
                                                   CORBA::Double             theDX,
                                                   CORBA::Double             theDY,
                                                   CORBA::Double             theDZ,
                                                   CORBA::Double             theH,
                                                   CORBA::Double             theR )
{
  SMESH::long_array_var aResult = new SMESH::long_array();
  if( meshPart->_is_nil() )
    return aResult._retn();

  gp_Pnt aP( theX, theY, theZ );
  gp_Vec aV( theDX, theDY, theDZ );
  gp_Ax2 anAxes (aP, aV);

  TopoDS_Shape aShape = BRepPrimAPI_MakeCylinder(anAxes, theR, Abs(theH)).Shape();

  std::vector<long> lst =_GetInside(meshPart, theElemType, aShape);

  if( lst.size() > 0 ) {
    aResult->length( lst.size() );
    for ( size_t i = 0; i < lst.size(); i++ ) {
      aResult[i] = lst[i];
    }
  }
  return aResult._retn();
}

//================================================================================
/*!
 * \brief Collect indices of elements, which are located inside the geom object
 */
//================================================================================

SMESH::long_array* SMESH_Gen_i::GetInside( SMESH::SMESH_IDSource_ptr meshPart,
                                           SMESH::ElementType        theElemType,
                                           GEOM::GEOM_Object_ptr     theGeom,
                                           CORBA::Double             theTolerance )
{
  SMESH::long_array_var aResult = new SMESH::long_array();
  if( meshPart->_is_nil() || theGeom->_is_nil() )
    return aResult._retn();

  TopoDS_Shape aShape = GeomObjectToShape( theGeom );

  std::vector<long> lst =_GetInside(meshPart, theElemType, aShape, &theTolerance);

  if( lst.size() > 0 ) {
    aResult->length( lst.size() );
    for ( size_t i = 0; i < lst.size(); i++ ) {
      aResult[i] = lst[i];
    }
  }
  return aResult._retn();
}

//================================================================================
/*!
 * \brief Collect indices of elements, which are located inside the TopoDS_Shape
 */
//================================================================================

std::vector<long> SMESH_Gen_i::_GetInside( SMESH::SMESH_IDSource_ptr meshPart,
                                           SMESH::ElementType        theElemType,
                                           const TopoDS_Shape&       theShape,
                                           double*                   theTolerance) {

  std::vector<long> res;
  SMESH::SMESH_Mesh_var mesh = meshPart->GetMesh();

  if ( mesh->_is_nil() )
    return res;

  SMESH_Mesh_i* anImpl = dynamic_cast<SMESH_Mesh_i*>( GetServant( mesh ).in() );
  if ( !anImpl )
    return res;

  const SMDS_Mesh* meshDS = anImpl->GetImpl().GetMeshDS();

  if ( !meshDS )
    return res;

  SMDSAbs_ElementType aType = SMDSAbs_ElementType(theElemType);
  SMESH::Controls::ElementsOnShape* anElementsOnShape = new SMESH::Controls::ElementsOnShape();
  anElementsOnShape->SetAllNodes( true );
  anElementsOnShape->SetMesh( meshDS );
  anElementsOnShape->SetShape( theShape, aType );

  if(theTolerance)
    anElementsOnShape->SetTolerance(*theTolerance);

  SMESH::SMESH_Mesh_var msource = SMESH::SMESH_Mesh::_narrow(meshPart);
  if ( !msource->_is_nil() ) { // Mesh case
    SMDS_ElemIteratorPtr elemIt = meshDS->elementsIterator( aType );
    if ( elemIt ) {
      while ( elemIt->more() ) {
        const SMDS_MeshElement* anElem = elemIt->next();
        long anId = anElem->GetID();
        if ( anElementsOnShape->IsSatisfy( anId ) )
          res.push_back( anId );
      }
    }
  }
  SMESH::SMESH_Group_var gsource = SMESH::SMESH_Group::_narrow(meshPart);
  if ( !gsource->_is_nil() ) {
    if(theElemType == SMESH::NODE) {
      SMESH::smIdType_array_var nodes = gsource->GetNodeIDs();
      for ( CORBA::ULong i = 0; i < nodes->length(); ++i ) {
        if ( const SMDS_MeshNode* node = meshDS->FindNode( nodes[i] )) {
          long anId = node->GetID();
          if ( anElementsOnShape->IsSatisfy( anId ) )
            res.push_back( anId );
        }
      }
    } else if (gsource->GetType() == theElemType || theElemType == SMESH::ALL ) {
      SMESH::smIdType_array_var elems = gsource->GetListOfID();
      for ( CORBA::ULong i = 0; i < elems->length(); ++i ) {
        if ( const SMDS_MeshElement* elem = meshDS->FindElement( elems[i] )) {
          long anId = elem->GetID();
          if ( anElementsOnShape->IsSatisfy( anId ) )
            res.push_back( anId );
        }
      }
    }
  }
  SMESH::SMESH_subMesh_var smsource = SMESH::SMESH_subMesh::_narrow(meshPart);
  if ( !smsource->_is_nil() ) {
    SMESH::smIdType_array_var elems = smsource->GetElementsByType( theElemType );
    for ( CORBA::ULong i = 0; i < elems->length(); ++i ) {
      const SMDS_MeshElement* elem = ( theElemType == SMESH::NODE ) ? meshDS->FindNode( elems[i] ) : meshDS->FindElement( elems[i] );
      if (elem) {
        long anId = elem->GetID();
        if ( anElementsOnShape->IsSatisfy( anId ) )
          res.push_back( anId );
      }
    }
  }
  return res;
}
