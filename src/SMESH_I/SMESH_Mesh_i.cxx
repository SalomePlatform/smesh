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
//  File   : SMESH_Mesh_i.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

#include "SMESH_Mesh_i.hxx"
#include "SMESH_subMesh_i.hxx"
#include "SMESH_MEDMesh_i.hxx"
#include "SMESH_Group_i.hxx"
#include "SMESH_Filter_i.hxx"

#include "Utils_CorbaException.hxx"
#include "Utils_ExceptHandlers.hxx"
#include "utilities.h"

#include "SALOME_NamingService.hxx"
#include "Utils_SINGLETON.hxx"
#include "OpUtil.hxx"

#include "TCollection_AsciiString.hxx"
#include "SMESHDS_Command.hxx"
#include "SMESHDS_CommandType.hxx"
#include "SMESH_MeshEditor_i.hxx"
#include "SMESH_Gen_i.hxx"
#include "DriverMED_R_SMESHDS_Mesh.h"

#include <TColStd_MapOfInteger.hxx>
#include <TColStd_MapIteratorOfMapOfInteger.hxx>
#include <TColStd_SequenceOfInteger.hxx>

#include <string>
#include <iostream>
#include <sstream>

#ifdef _DEBUG_
static int MYDEBUG = 0;
#else
static int MYDEBUG = 0;
#endif

using namespace std;

int SMESH_Mesh_i::myIdGenerator = 0;

//=============================================================================
/*!
 *  Constructor
 */
//=============================================================================

SMESH_Mesh_i::SMESH_Mesh_i( PortableServer::POA_ptr thePOA,
			    SMESH_Gen_i*            gen_i,
			    CORBA::Long studyId )
: SALOME::GenericObj_i( thePOA )
{
  INFOS("SMESH_Mesh_i; this = "<<this);
  _impl = NULL;
  _gen_i = gen_i;
  _id = myIdGenerator++;
  _studyId = studyId;
  thePOA->activate_object( this );
}

//=============================================================================
/*!
 *  Destructor
 */
//=============================================================================

SMESH_Mesh_i::~SMESH_Mesh_i()
{
  INFOS("~SMESH_Mesh_i; this = "<<this);
  map<int, SMESH::SMESH_GroupBase_ptr>::iterator it;
  for ( it = _mapGroups.begin(); it != _mapGroups.end(); it++ ) {
    SMESH_GroupBase_i* aGroup = dynamic_cast<SMESH_GroupBase_i*>( SMESH_Gen_i::GetServant( it->second ).in() );
    if ( aGroup ) {

      // this method is colled from destructor of group (PAL6331)
      //_impl->RemoveGroup( aGroup->GetLocalID() );
      
      aGroup->Destroy();
    }
  }
  _mapGroups.clear();
}

//=============================================================================
/*!
 *  SetShape
 *
 *  Associates <this> mesh with <theShape> and puts a reference  
 *  to <theShape> into the current study; 
 *  the previous shape is substituted by the new one.
 */
//=============================================================================

void SMESH_Mesh_i::SetShape( GEOM::GEOM_Object_ptr theShapeObject )
    throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  try {
    _impl->ShapeToMesh( _gen_i->GeomObjectToShape( theShapeObject ));
  }
  catch(SALOME_Exception & S_ex) {
    THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
  }
}

//=======================================================================
//function : GetShapeToMesh
//purpose  : 
//=======================================================================

GEOM::GEOM_Object_ptr SMESH_Mesh_i::GetShapeToMesh()
    throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  GEOM::GEOM_Object_var aShapeObj;
  try {
    TopoDS_Shape S = _impl->GetMeshDS()->ShapeToMesh();
    if ( !S.IsNull() )
      aShapeObj = _gen_i->ShapeToGeomObject( S );
  }
  catch(SALOME_Exception & S_ex) {
    THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
  }
  return aShapeObj._retn();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

static SMESH::DriverMED_ReadStatus ConvertDriverMEDReadStatus (int theStatus)
{
  SMESH::DriverMED_ReadStatus res;
  switch (theStatus)
  {
  case DriverMED_R_SMESHDS_Mesh::DRS_OK:
    res = SMESH::DRS_OK; break;
  case DriverMED_R_SMESHDS_Mesh::DRS_EMPTY:
    res = SMESH::DRS_EMPTY; break;
  case DriverMED_R_SMESHDS_Mesh::DRS_WARN_RENUMBER:
    res = SMESH::DRS_WARN_RENUMBER; break;
  case DriverMED_R_SMESHDS_Mesh::DRS_WARN_SKIP_ELEM:
    res = SMESH::DRS_WARN_SKIP_ELEM; break;
  case DriverMED_R_SMESHDS_Mesh::DRS_FAIL:
  default:
    res = SMESH::DRS_FAIL; break;
  }
  return res;
}

//=============================================================================
/*!
 *  ImportMEDFile
 *
 *  Imports mesh data from MED file
 */
//=============================================================================

SMESH::DriverMED_ReadStatus
SMESH_Mesh_i::ImportMEDFile( const char* theFileName, const char* theMeshName )
  throw ( SALOME::SALOME_Exception )
{
  Unexpect aCatch(SALOME_SalomeException);
  int status;
  try {
    status = importMEDFile( theFileName, theMeshName );
  }
  catch( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
  }  
  catch ( ... ) {
    THROW_SALOME_CORBA_EXCEPTION("ImportMEDFile(): unknown exception", SALOME::BAD_PARAM);
  }

  SALOMEDS::Study_ptr aStudy = _gen_i->GetCurrentStudy();
  if ( !aStudy->_is_nil() ) {
    // publishing of the groups in the study (sub-meshes are out of scope of MED import)
    map<int, SMESH::SMESH_GroupBase_ptr>::iterator it = _mapGroups.begin();
    for (; it != _mapGroups.end(); it++ ) {
      SMESH::SMESH_GroupBase_var aGroup = SMESH::SMESH_GroupBase::_duplicate( it->second );
      _gen_i->PublishGroup( aStudy, _this(), aGroup,
                           GEOM::GEOM_Object::_nil(), aGroup->GetName());
    }
  }
  return ConvertDriverMEDReadStatus(status);
}

//=============================================================================
/*!
 *  ImportUNVFile
 *
 *  Imports mesh data from MED file
 */
//=============================================================================

int SMESH_Mesh_i::ImportUNVFile( const char* theFileName )
  throw ( SALOME::SALOME_Exception )
{
  // Read mesh with name = <theMeshName> into SMESH_Mesh
  _impl->UNVToMesh( theFileName );

  return 1;
}

//=============================================================================
/*!
 *  ImportSTLFile
 *
 *  Imports mesh data from STL file
 */
//=============================================================================
int SMESH_Mesh_i::ImportSTLFile( const char* theFileName )
  throw ( SALOME::SALOME_Exception )
{
  // Read mesh with name = <theMeshName> into SMESH_Mesh
  _impl->STLToMesh( theFileName );

  return 1;
}

//=============================================================================
/*!
 *  importMEDFile
 *
 *  Imports mesh data from MED file
 */
//=============================================================================

int SMESH_Mesh_i::importMEDFile( const char* theFileName, const char* theMeshName )
{
  // Read mesh with name = <theMeshName> and all its groups into SMESH_Mesh
  int status = _impl->MEDToMesh( theFileName, theMeshName );

  // Create group servants, if any groups were imported
  list<int> aGroupIds = _impl->GetGroupIds();
  for ( list<int>::iterator it = aGroupIds.begin(); it != aGroupIds.end(); it++ ) {
    SMESH_Group_i* aGroupImpl     = new SMESH_Group_i( SMESH_Gen_i::GetPOA(), this, *it );

    // PAL7962: san -- To ensure correct mapping of servant and correct reference counting in GenericObj_i
    SMESH_Gen_i::GetPOA()->activate_object( aGroupImpl );
    aGroupImpl->Register();
    // PAL7962: san -- To ensure correct mapping of servant and correct reference counting in GenericObj_i

    SMESH::SMESH_Group_var aGroup = SMESH::SMESH_Group::_narrow( aGroupImpl->_this() );
    _mapGroups[*it]               = SMESH::SMESH_Group::_duplicate( aGroup );

    // register CORBA object for persistence
    int nextId = _gen_i->RegisterObject( aGroup );
    if(MYDEBUG) MESSAGE( "Add group to map with id = "<< nextId);
  }

  return status;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

static SMESH::Hypothesis_Status ConvertHypothesisStatus
                         (SMESH_Hypothesis::Hypothesis_Status theStatus)
{
  SMESH::Hypothesis_Status res;
  switch (theStatus)
  {
  case SMESH_Hypothesis::HYP_OK:
    res = SMESH::HYP_OK; break;
  case SMESH_Hypothesis::HYP_MISSING:
    res = SMESH::HYP_MISSING; break;
  case SMESH_Hypothesis::HYP_CONCURENT:
    res = SMESH::HYP_CONCURENT; break;
  case SMESH_Hypothesis::HYP_BAD_PARAMETER:
    res = SMESH::HYP_BAD_PARAMETER; break;
  case SMESH_Hypothesis::HYP_INCOMPATIBLE:
    res = SMESH::HYP_INCOMPATIBLE; break;
  case SMESH_Hypothesis::HYP_NOTCONFORM:
    res = SMESH::HYP_NOTCONFORM; break;
  case SMESH_Hypothesis::HYP_ALREADY_EXIST:
    res = SMESH::HYP_ALREADY_EXIST; break;
  case SMESH_Hypothesis::HYP_BAD_DIM:
    res = SMESH::HYP_BAD_DIM; break;
  default:
    res = SMESH::HYP_UNKNOWN_FATAL;
  }
  return res;
}

//=============================================================================
/*!
 *  AddHypothesis
 *
 *  calls internal addHypothesis() and then adds a reference to <anHyp> under 
 *  the SObject actually having a reference to <aSubShape>.
 *  NB: For this method to work, it is necessary to add a reference to sub-shape first.
 */
//=============================================================================

SMESH::Hypothesis_Status SMESH_Mesh_i::AddHypothesis(GEOM::GEOM_Object_ptr aSubShapeObject,
                                                     SMESH::SMESH_Hypothesis_ptr anHyp)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  SMESH_Hypothesis::Hypothesis_Status status = addHypothesis( aSubShapeObject, anHyp );

  if ( !SMESH_Hypothesis::IsStatusFatal(status) )
    _gen_i->AddHypothesisToShape(_gen_i->GetCurrentStudy(), _this(),
                                 aSubShapeObject, anHyp );

  if(MYDEBUG) MESSAGE( " AddHypothesis(): status = " << status );

  return ConvertHypothesisStatus(status);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_Hypothesis::Hypothesis_Status
  SMESH_Mesh_i::addHypothesis(GEOM::GEOM_Object_ptr       aSubShapeObject,
                              SMESH::SMESH_Hypothesis_ptr anHyp)
{
  if(MYDEBUG) MESSAGE("addHypothesis");

  if (CORBA::is_nil(aSubShapeObject))
    THROW_SALOME_CORBA_EXCEPTION("bad subShape reference",
                                 SALOME::BAD_PARAM);

  SMESH::SMESH_Hypothesis_var myHyp = SMESH::SMESH_Hypothesis::_narrow(anHyp);
  if (CORBA::is_nil(myHyp))
    THROW_SALOME_CORBA_EXCEPTION("bad hypothesis reference",
                                 SALOME::BAD_PARAM);

  SMESH_Hypothesis::Hypothesis_Status status = SMESH_Hypothesis::HYP_OK;
  try
  {
    TopoDS_Shape myLocSubShape = _gen_i->GeomObjectToShape( aSubShapeObject);
    int hypId = myHyp->GetId();
    status = _impl->AddHypothesis(myLocSubShape, hypId);
    if ( !SMESH_Hypothesis::IsStatusFatal(status) ) {
      _mapHypo[hypId] = myHyp;
      // assure there is a corresponding submesh
      if ( !_impl->IsMainShape( myLocSubShape )) {
        int shapeId = _impl->GetMeshDS()->ShapeToIndex( myLocSubShape );
        if ( _mapSubMesh_i.find( shapeId ) == _mapSubMesh_i.end() )
          createSubMesh( aSubShapeObject );
      }
    }
  }
  catch(SALOME_Exception & S_ex)
  {
    THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
  }
  return status;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH::Hypothesis_Status SMESH_Mesh_i::RemoveHypothesis(GEOM::GEOM_Object_ptr aSubShapeObject,
                                                        SMESH::SMESH_Hypothesis_ptr anHyp)
     throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  SMESH_Hypothesis::Hypothesis_Status status = removeHypothesis( aSubShapeObject, anHyp );

  if ( !SMESH_Hypothesis::IsStatusFatal(status) )
    _gen_i->RemoveHypothesisFromShape(_gen_i->GetCurrentStudy(), _this(),
                                      aSubShapeObject, anHyp );

  return ConvertHypothesisStatus(status);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_Hypothesis::Hypothesis_Status SMESH_Mesh_i::removeHypothesis(GEOM::GEOM_Object_ptr aSubShapeObject,
                                 SMESH::SMESH_Hypothesis_ptr anHyp)
{
	if(MYDEBUG) MESSAGE("removeHypothesis()");
	// **** proposer liste de subShape (selection multiple)

	if (CORBA::is_nil(aSubShapeObject))
		THROW_SALOME_CORBA_EXCEPTION("bad subShape reference",
			SALOME::BAD_PARAM);

	SMESH::SMESH_Hypothesis_var myHyp = SMESH::SMESH_Hypothesis::_narrow(anHyp);
	if (CORBA::is_nil(myHyp))
	  THROW_SALOME_CORBA_EXCEPTION("bad hypothesis reference",
			SALOME::BAD_PARAM);

	SMESH_Hypothesis::Hypothesis_Status status = SMESH_Hypothesis::HYP_OK;
	try
	{
		TopoDS_Shape myLocSubShape = _gen_i->GeomObjectToShape(aSubShapeObject);
		int hypId = myHyp->GetId();
		status = _impl->RemoveHypothesis(myLocSubShape, hypId);
                if ( !SMESH_Hypothesis::IsStatusFatal(status) )
                  _mapHypo.erase( hypId );
	}
	catch(SALOME_Exception & S_ex)
	{
		THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
	}
	return status;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH::ListOfHypothesis *
	SMESH_Mesh_i::GetHypothesisList(GEOM::GEOM_Object_ptr aSubShapeObject)
throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if (MYDEBUG) MESSAGE("GetHypothesisList");
  if (CORBA::is_nil(aSubShapeObject))
    THROW_SALOME_CORBA_EXCEPTION("bad subShape reference",
				 SALOME::BAD_PARAM);
  
  SMESH::ListOfHypothesis_var aList = new SMESH::ListOfHypothesis();

  try {
    TopoDS_Shape myLocSubShape = _gen_i->GeomObjectToShape(aSubShapeObject);
    const list<const SMESHDS_Hypothesis*>& aLocalList = _impl->GetHypothesisList( myLocSubShape );
    int i = 0, n = aLocalList.size();
    aList->length( n );

    for ( list<const SMESHDS_Hypothesis*>::const_iterator anIt = aLocalList.begin(); i < n && anIt != aLocalList.end(); anIt++ ) {
      SMESHDS_Hypothesis* aHyp = (SMESHDS_Hypothesis*)(*anIt);
      if ( _mapHypo.find( aHyp->GetID() ) != _mapHypo.end() )
	aList[i++] = SMESH::SMESH_Hypothesis::_narrow( _mapHypo[aHyp->GetID()] );
    }

    aList->length( i );
  }
  catch(SALOME_Exception & S_ex) {
    THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
  }
  
  return aList._retn();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
SMESH::SMESH_subMesh_ptr SMESH_Mesh_i::GetSubMesh(GEOM::GEOM_Object_ptr aSubShapeObject,
						  const char*           theName ) 
     throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  MESSAGE("SMESH_Mesh_i::GetSubMesh");
  if (CORBA::is_nil(aSubShapeObject))
    THROW_SALOME_CORBA_EXCEPTION("bad subShape reference",
				 SALOME::BAD_PARAM);
  
  SMESH::SMESH_subMesh_var subMesh;
  SMESH::SMESH_Mesh_var    aMesh = SMESH::SMESH_Mesh::_narrow(_this());
  try {
    TopoDS_Shape myLocSubShape = _gen_i->GeomObjectToShape(aSubShapeObject);
    
    //Get or Create the SMESH_subMesh object implementation
    
    int subMeshId = _impl->GetMeshDS()->ShapeToIndex( myLocSubShape );
    subMesh = getSubMesh( subMeshId );

    // create a new subMesh object servant if there is none for the shape
    if ( subMesh->_is_nil() )
      subMesh = createSubMesh( aSubShapeObject );

    if ( _gen_i->CanPublishInStudy( subMesh ))
      _gen_i->PublishSubMesh (_gen_i->GetCurrentStudy(), aMesh,
                              subMesh, aSubShapeObject, theName );
  }
  catch(SALOME_Exception & S_ex) {
    THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
  }
  return subMesh._retn();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_Mesh_i::RemoveSubMesh( SMESH::SMESH_subMesh_ptr theSubMesh )
     throw (SALOME::SALOME_Exception)
{
  if(MYDEBUG) MESSAGE("SMESH_Mesh_i::RemoveSubMesh");
  if ( theSubMesh->_is_nil() )
    return;

  GEOM::GEOM_Object_var aSubShapeObject;
  SALOMEDS::Study_ptr aStudy = _gen_i->GetCurrentStudy();
  if ( !aStudy->_is_nil() )  {
    // Remove submesh's SObject
    SALOMEDS::SObject_var anSO = _gen_i->ObjectToSObject( aStudy, theSubMesh );
    if ( !anSO->_is_nil() ) {
      long aTag = SMESH_Gen_i::GetRefOnShapeTag(); 
      SALOMEDS::SObject_var anObj, aRef;
      if ( anSO->FindSubObject( aTag, anObj ) && anObj->ReferencedObject( aRef ) )
	aSubShapeObject = GEOM::GEOM_Object::_narrow( aRef->GetObject() );

      aStudy->NewBuilder()->RemoveObjectWithChildren( anSO );
    }
  }

  removeSubMesh( theSubMesh, aSubShapeObject.in() );
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH::SMESH_Group_ptr SMESH_Mesh_i::CreateGroup( SMESH::ElementType theElemType,
                                                 const char*         theName )
     throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  SMESH::SMESH_Group_var aNewGroup =
    SMESH::SMESH_Group::_narrow( createGroup( theElemType, theName ));

  _gen_i->PublishGroup( _gen_i->GetCurrentStudy(), _this(),
                       aNewGroup, GEOM::GEOM_Object::_nil(), theName);

  return aNewGroup._retn();
}


//=============================================================================
/*!
 *  
 */
//=============================================================================
SMESH::SMESH_GroupOnGeom_ptr SMESH_Mesh_i::CreateGroupFromGEOM( SMESH::ElementType    theElemType,
                                                               const char*           theName,
                                                               GEOM::GEOM_Object_ptr theGeomObj)
     throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  SMESH::SMESH_GroupOnGeom_var aNewGroup;

  TopoDS_Shape aShape = _gen_i->GeomObjectToShape( theGeomObj );
  if ( !aShape.IsNull() ) {
    aNewGroup = SMESH::SMESH_GroupOnGeom::_narrow
      ( createGroup( theElemType, theName, aShape ));
    if ( _gen_i->CanPublishInStudy( aNewGroup ) )
      _gen_i->PublishGroup( _gen_i->GetCurrentStudy(), _this(), 
                           aNewGroup, theGeomObj, theName );
  }

  return aNewGroup._retn();
}
//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_Mesh_i::RemoveGroup( SMESH::SMESH_GroupBase_ptr theGroup )
     throw (SALOME::SALOME_Exception)
{
  if ( theGroup->_is_nil() )
    return;

  SMESH_GroupBase_i* aGroup =
    dynamic_cast<SMESH_GroupBase_i*>( SMESH_Gen_i::GetServant( theGroup ).in() );
  if ( !aGroup )
    return;

  SALOMEDS::Study_ptr aStudy = _gen_i->GetCurrentStudy();
  if ( !aStudy->_is_nil() )  {
    // Remove group's SObject
    SALOMEDS::SObject_var aGroupSO = _gen_i->ObjectToSObject( aStudy, theGroup );
    if ( !aGroupSO->_is_nil() )
      aStudy->NewBuilder()->RemoveObject( aGroupSO );
  }

  // Remove the group from SMESH data structures
  removeGroup( aGroup->GetLocalID() );
}

//=============================================================================
/*! RemoveGroupWithContents
 *  Remove group with its contents
 */ 
//=============================================================================
void SMESH_Mesh_i::RemoveGroupWithContents( SMESH::SMESH_GroupBase_ptr theGroup )
  throw (SALOME::SALOME_Exception)
{
  if ( theGroup->_is_nil() )
    return;

  SMESH_GroupBase_i* aGroup =
    dynamic_cast<SMESH_GroupBase_i*>( SMESH_Gen_i::GetServant( theGroup ).in() );
  if ( !aGroup )
    return;
  
  SMESH::long_array_var anIds = aGroup->GetListOfID();
  SMESH::SMESH_MeshEditor_var aMeshEditor = SMESH_Mesh_i::GetMeshEditor();
    
  if ( aGroup->GetType() == SMESH::NODE )
    aMeshEditor->RemoveNodes( anIds );
  else
    aMeshEditor->RemoveElements( anIds );
  
  RemoveGroup( theGroup );
}

//=============================================================================
/*! UnionGroups
 *  New group is created. All mesh elements that are 
 *  present in initial groups are added to the new one
 */
//=============================================================================
SMESH::SMESH_Group_ptr SMESH_Mesh_i::UnionGroups( SMESH::SMESH_GroupBase_ptr theGroup1, 
                                                  SMESH::SMESH_GroupBase_ptr theGroup2, 
                                                  const char* theName )
  throw (SALOME::SALOME_Exception)
{
  try
  {
    SMESH::SMESH_Group_var aResGrp;

    if ( theGroup1->_is_nil() || theGroup2->_is_nil() ||
         theGroup1->GetType() != theGroup2->GetType() )
      return SMESH::SMESH_Group::_nil();

    aResGrp = CreateGroup( theGroup1->GetType(), theName );
    if ( aResGrp->_is_nil() )
      return SMESH::SMESH_Group::_nil();

    SMESH::long_array_var anIds1 = theGroup1->GetListOfID();
    SMESH::long_array_var anIds2 = theGroup2->GetListOfID();

    TColStd_MapOfInteger aResMap;

    for ( int i1 = 0, n1 = anIds1->length(); i1 < n1; i1++ )
      aResMap.Add( anIds1[ i1 ] );

    for ( int i2 = 0, n2 = anIds2->length(); i2 < n2; i2++ )
      aResMap.Add( anIds2[ i2 ] );

    SMESH::long_array_var aResIds = new SMESH::long_array;
    aResIds->length( aResMap.Extent() );

    int resI = 0;
    TColStd_MapIteratorOfMapOfInteger anIter( aResMap );
    for( ; anIter.More(); anIter.Next() )
      aResIds[ resI++ ] = anIter.Key();

    aResGrp->Add( aResIds );

    return aResGrp._retn();
  }
  catch( ... )
  {
    return SMESH::SMESH_Group::_nil();
  }
}
  
//=============================================================================
/*! IntersectGroups
 *  New group is created. All mesh elements that are 
 *  present in both initial groups are added to the new one.
 */
//=============================================================================
SMESH::SMESH_Group_ptr SMESH_Mesh_i::IntersectGroups( SMESH::SMESH_GroupBase_ptr theGroup1, 
                                                      SMESH::SMESH_GroupBase_ptr theGroup2, 
                                                      const char* theName )
  throw (SALOME::SALOME_Exception)
{
  SMESH::SMESH_Group_var aResGrp;
  
  if ( theGroup1->_is_nil() || theGroup2->_is_nil() || 
       theGroup1->GetType() != theGroup2->GetType() )
    return aResGrp;
  
  aResGrp = CreateGroup( theGroup1->GetType(), theName );
  if ( aResGrp->_is_nil() )
    return aResGrp;
  
  SMESH::long_array_var anIds1 = theGroup1->GetListOfID();
  SMESH::long_array_var anIds2 = theGroup2->GetListOfID();
  
  TColStd_MapOfInteger aMap1;
  
  for ( int i1 = 0, n1 = anIds1->length(); i1 < n1; i1++ )
    aMap1.Add( anIds1[ i1 ] );

  TColStd_SequenceOfInteger aSeq;

  for ( int i2 = 0, n2 = anIds2->length(); i2 < n2; i2++ )
    if ( aMap1.Contains( anIds2[ i2 ] ) )
      aSeq.Append( anIds2[ i2 ] );
  
  SMESH::long_array_var aResIds = new SMESH::long_array;
  aResIds->length( aSeq.Length() );
  
  for ( int resI = 0, resN = aSeq.Length(); resI < resN; resI++ )
    aResIds[ resI ] = aSeq( resI + 1 );
  
  aResGrp->Add( aResIds );
  
  return aResGrp._retn();
}

//=============================================================================
/*! CutGroups
 *  New group is created. All mesh elements that are present in 
 *  main group but do not present in tool group are added to the new one 
 */
//=============================================================================
SMESH::SMESH_Group_ptr SMESH_Mesh_i::CutGroups( SMESH::SMESH_GroupBase_ptr theGroup1, 
                                                SMESH::SMESH_GroupBase_ptr theGroup2, 
                                                const char* theName )
  throw (SALOME::SALOME_Exception)
{
  SMESH::SMESH_Group_var aResGrp;
  
  if ( theGroup1->_is_nil() || theGroup2->_is_nil() || 
       theGroup1->GetType() != theGroup2->GetType() )
    return aResGrp;
  
  aResGrp = CreateGroup( theGroup1->GetType(), theName );
  if ( aResGrp->_is_nil() )
    return aResGrp;
  
  SMESH::long_array_var anIds1 = theGroup1->GetListOfID();
  SMESH::long_array_var anIds2 = theGroup2->GetListOfID();
  
  TColStd_MapOfInteger aMap2;
  
  for ( int i2 = 0, n2 = anIds2->length(); i2 < n2; i2++ )
    aMap2.Add( anIds2[ i2 ] );


  TColStd_SequenceOfInteger aSeq;
  for ( int i1 = 0, n1 = anIds1->length(); i1 < n1; i1++ )
    if ( !aMap2.Contains( anIds1[ i1 ] ) )
      aSeq.Append( anIds1[ i1 ] );

  SMESH::long_array_var aResIds = new SMESH::long_array;
  aResIds->length( aSeq.Length() );

  for ( int resI = 0, resN = aSeq.Length(); resI < resN; resI++ )
    aResIds[ resI ] = aSeq( resI + 1 );  
  
  aResGrp->Add( aResIds );
  
  return aResGrp._retn();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH::SMESH_subMesh_ptr SMESH_Mesh_i::createSubMesh( GEOM::GEOM_Object_ptr theSubShapeObject )
{
  if(MYDEBUG) MESSAGE( "createSubMesh" );
  TopoDS_Shape myLocSubShape = _gen_i->GeomObjectToShape(theSubShapeObject);

  ::SMESH_subMesh * mySubMesh = _impl->GetSubMesh(myLocSubShape);
  int subMeshId = _impl->GetMeshDS()->ShapeToIndex( myLocSubShape );
  SMESH_subMesh_i *subMeshServant = new SMESH_subMesh_i(myPOA, _gen_i, this, subMeshId);
  SMESH::SMESH_subMesh_var subMesh
    = SMESH::SMESH_subMesh::_narrow(subMeshServant->_this());

  _mapSubMesh[subMeshId] = mySubMesh;
  _mapSubMesh_i[subMeshId] = subMeshServant;
  _mapSubMeshIor[subMeshId] = SMESH::SMESH_subMesh::_duplicate(subMesh);

  // register CORBA object for persistence
  int nextId = _gen_i->RegisterObject( subMesh );
  if(MYDEBUG) MESSAGE( "Add submesh to map with id = "<< nextId);

  return subMesh._retn(); 
}

//=======================================================================
//function : getSubMesh
//purpose  : 
//=======================================================================

SMESH::SMESH_subMesh_ptr SMESH_Mesh_i::getSubMesh(int shapeID)
{
  map<int, SMESH::SMESH_subMesh_ptr>::iterator it = _mapSubMeshIor.find( shapeID );
  if ( it == _mapSubMeshIor.end() )
    return SMESH::SMESH_subMesh::_nil();

  return SMESH::SMESH_subMesh::_duplicate( (*it).second );
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_Mesh_i::removeSubMesh (SMESH::SMESH_subMesh_ptr theSubMesh,
                                  GEOM::GEOM_Object_ptr    theSubShapeObject )
{
  MESSAGE("SMESH_Mesh_i::removeSubMesh()");
  if ( theSubMesh->_is_nil() || theSubShapeObject->_is_nil() )
    return;

  try {
    SMESH::ListOfHypothesis_var aHypList = GetHypothesisList( theSubShapeObject );
    for ( int i = 0, n = aHypList->length(); i < n; i++ ) {
      removeHypothesis( theSubShapeObject, aHypList[i] );
    }
  }
  catch( const SALOME::SALOME_Exception& ) {
    INFOS("SMESH_Mesh_i::removeSubMesh(): exception caught!");
  }

  int subMeshId = theSubMesh->GetId();

  _mapSubMesh.erase(subMeshId);
  _mapSubMesh_i.erase(subMeshId);
  _mapSubMeshIor.erase(subMeshId);
  if(MYDEBUG) MESSAGE("SMESH_Mesh_i::removeSubMesh() completed");
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH::SMESH_GroupBase_ptr SMESH_Mesh_i::createGroup (SMESH::ElementType theElemType,
                                                      const char*         theName,
                                                      const TopoDS_Shape& theShape )
{
  int anId;
  SMESH::SMESH_GroupBase_var aGroup;
  if ( _impl->AddGroup( (SMDSAbs_ElementType)theElemType, theName, anId, theShape )) {
    SMESH_GroupBase_i* aGroupImpl;
    if ( !theShape.IsNull() )
      aGroupImpl = new SMESH_GroupOnGeom_i( SMESH_Gen_i::GetPOA(), this, anId );
    else
      aGroupImpl = new SMESH_Group_i( SMESH_Gen_i::GetPOA(), this, anId );

    // PAL7962: san -- To ensure correct mapping of servant and correct reference counting in GenericObj_i
    SMESH_Gen_i::GetPOA()->activate_object( aGroupImpl );
    aGroupImpl->Register();
    // PAL7962: san -- To ensure correct mapping of servant and correct reference counting in GenericObj_i

    aGroup = SMESH::SMESH_GroupBase::_narrow( aGroupImpl->_this() );
    _mapGroups[anId] = SMESH::SMESH_GroupBase::_duplicate( aGroup );

    // register CORBA object for persistence
    int nextId = _gen_i->RegisterObject( aGroup );
    if(MYDEBUG) MESSAGE( "Add group to map with id = "<< nextId);
  }
  return aGroup._retn();
}

//=============================================================================
/*!
 * SMESH_Mesh_i::removeGroup
 *
 * Should be called by ~SMESH_Group_i() 
 */
//=============================================================================

void SMESH_Mesh_i::removeGroup( const int theId )
{
  if(MYDEBUG) MESSAGE("SMESH_Mesh_i::removeGroup()" );  
  if ( _mapGroups.find( theId ) != _mapGroups.end() ) {
    _mapGroups.erase( theId );
    _impl->RemoveGroup( theId );
  }
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH::log_array * SMESH_Mesh_i::GetLog(CORBA::Boolean clearAfterGet)
throw(SALOME::SALOME_Exception)
{
  if(MYDEBUG) MESSAGE("SMESH_Mesh_i::GetLog");
  
  SMESH::log_array_var aLog;
  try{
    list < SMESHDS_Command * >logDS = _impl->GetLog();
    aLog = new SMESH::log_array;
    int indexLog = 0;
    int lg = logDS.size();
    SCRUTE(lg);
    aLog->length(lg);
    list < SMESHDS_Command * >::iterator its = logDS.begin();
    while(its != logDS.end()){
      SMESHDS_Command *com = *its;
      int comType = com->GetType();
      //SCRUTE(comType);
      int lgcom = com->GetNumber();
      //SCRUTE(lgcom);
      const list < int >&intList = com->GetIndexes();
      int inum = intList.size();
      //SCRUTE(inum);
      list < int >::const_iterator ii = intList.begin();
      const list < double >&coordList = com->GetCoords();
      int rnum = coordList.size();
      //SCRUTE(rnum);
      list < double >::const_iterator ir = coordList.begin();
      aLog[indexLog].commandType = comType;
      aLog[indexLog].number = lgcom;
      aLog[indexLog].coords.length(rnum);
      aLog[indexLog].indexes.length(inum);
      for(int i = 0; i < rnum; i++){
	aLog[indexLog].coords[i] = *ir;
	//MESSAGE(" "<<i<<" "<<ir.Value());
	ir++;
      }
      for(int i = 0; i < inum; i++){
	aLog[indexLog].indexes[i] = *ii;
	//MESSAGE(" "<<i<<" "<<ii.Value());
	ii++;
      }
      indexLog++;
      its++;
    }
    if(clearAfterGet)
      _impl->ClearLog();
  }
  catch(SALOME_Exception & S_ex){
    THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
  }
  return aLog._retn();
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_Mesh_i::ClearLog() throw(SALOME::SALOME_Exception)
{
  if(MYDEBUG) MESSAGE("SMESH_Mesh_i::ClearLog");
  // ****
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Long SMESH_Mesh_i::GetId()throw(SALOME::SALOME_Exception)
{
  if(MYDEBUG) MESSAGE("SMESH_Mesh_i::GetId");
  return _id;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Long SMESH_Mesh_i::GetStudyId()throw(SALOME::SALOME_Exception)
{
  return _studyId;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_Mesh_i::SetImpl(::SMESH_Mesh * impl)
{
  if(MYDEBUG) MESSAGE("SMESH_Mesh_i::SetImpl");
  _impl = impl;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

::SMESH_Mesh & SMESH_Mesh_i::GetImpl()
{
  if(MYDEBUG) MESSAGE("SMESH_Mesh_i::GetImpl()");
  return *_impl;
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH::SMESH_MeshEditor_ptr SMESH_Mesh_i::GetMeshEditor()
{
	SMESH_MeshEditor_i *aMeshEditor = new SMESH_MeshEditor_i( _impl );
	SMESH::SMESH_MeshEditor_var aMesh = aMeshEditor->_this();
	return aMesh._retn();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_Mesh_i::ExportToMED( const char* file, 
				CORBA::Boolean auto_groups, 
				SMESH::MED_VERSION theVersion )
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  
  char* aMeshName = "Mesh";
  SALOMEDS::Study_ptr aStudy = _gen_i->GetCurrentStudy();
  if ( !aStudy->_is_nil() ) {
    SALOMEDS::SObject_var aMeshSO = _gen_i->ObjectToSObject( aStudy, _this() );
    if ( !aMeshSO->_is_nil() ) {
      aMeshName = aMeshSO->GetName();
      //SCRUTE(file);
      //SCRUTE(aMeshName);
      //SCRUTE(aMeshSO->GetID());
      
      // asv : 27.10.04 : fix of 6903: check for StudyLocked before adding attributes 
      if ( !aStudy->GetProperties()->IsLocked() ) 
	{
	SALOMEDS::GenericAttribute_var anAttr;
	SALOMEDS::StudyBuilder_var aStudyBuilder = aStudy->NewBuilder();
	SALOMEDS::AttributeExternalFileDef_var aFileName;
	anAttr=aStudyBuilder->FindOrCreateAttribute(aMeshSO, "AttributeExternalFileDef");
	aFileName = SALOMEDS::AttributeExternalFileDef::_narrow(anAttr);
	ASSERT(!aFileName->_is_nil());
        aFileName->SetValue(file);
        SALOMEDS::AttributeFileType_var aFileType;
        anAttr=aStudyBuilder->FindOrCreateAttribute(aMeshSO, "AttributeFileType");
        aFileType = SALOMEDS::AttributeFileType::_narrow(anAttr);
        ASSERT(!aFileType->_is_nil());
        aFileType->SetValue("FICHIERMED");
	}
    }
  }
  _impl->ExportMED( file, aMeshName, auto_groups, theVersion );
}

void SMESH_Mesh_i::ExportMED( const char* file, 
			      CORBA::Boolean auto_groups)
  throw(SALOME::SALOME_Exception)
{
  ExportToMED(file,auto_groups,SMESH::MED_V2_1);
}

void SMESH_Mesh_i::ExportDAT(const char *file) throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  _impl->ExportDAT(file);
}
void SMESH_Mesh_i::ExportUNV(const char *file) throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  _impl->ExportUNV(file);
}

void SMESH_Mesh_i::ExportSTL(const char *file, const bool isascii) throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  _impl->ExportSTL(file, isascii);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SALOME_MED::MESH_ptr SMESH_Mesh_i::GetMEDMesh()throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  SMESH_MEDMesh_i *aMedMesh = new SMESH_MEDMesh_i(this);
  SALOME_MED::MESH_var aMesh = aMedMesh->_this();
  return aMesh._retn();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
CORBA::Long SMESH_Mesh_i::NbNodes()throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  return _impl->NbNodes();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
CORBA::Long SMESH_Mesh_i::NbElements()throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  return NbEdges() + NbFaces() + NbVolumes();
}
  
//=============================================================================
/*!
 *  
 */
//=============================================================================
CORBA::Long SMESH_Mesh_i::NbEdges()throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  return _impl->NbEdges();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
CORBA::Long SMESH_Mesh_i::NbFaces()throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  return _impl->NbFaces();
}

CORBA::Long SMESH_Mesh_i::NbTriangles()throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  return _impl->NbTriangles();
}

CORBA::Long SMESH_Mesh_i::NbQuadrangles()throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  return _impl->NbQuadrangles();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
CORBA::Long SMESH_Mesh_i::NbVolumes()throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  return _impl->NbVolumes();
}

CORBA::Long SMESH_Mesh_i::NbTetras()throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  return _impl->NbTetras();
}

CORBA::Long SMESH_Mesh_i::NbHexas()throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  return _impl->NbHexas();
}

CORBA::Long SMESH_Mesh_i::NbPyramids()throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  return _impl->NbPyramids();
}

CORBA::Long SMESH_Mesh_i::NbPrisms()throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  return _impl->NbPrisms();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
CORBA::Long SMESH_Mesh_i::NbSubMesh()throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  return _impl->NbSubMesh();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
char* SMESH_Mesh_i::Dump()
{
  std::ostringstream os;
  _impl->Dump( os );
  return CORBA::string_dup( os.str().c_str() );
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
SMESH::long_array* SMESH_Mesh_i::GetIDs()
{
  SMESH::long_array_var aResult = new SMESH::long_array();
  SMESHDS_Mesh* aSMESHDS_Mesh = _impl->GetMeshDS();
  int aMinId = aSMESHDS_Mesh->MinElementID();
  int aMaxId =  aSMESHDS_Mesh->MaxElementID();

  aResult->length(aMaxId - aMinId + 1);
  
  for (int i = 0, id = aMinId; id <= aMaxId; id++  )
    aResult[i++] = id;
  
  return aResult._retn();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
  
SMESH::long_array* SMESH_Mesh_i::GetElementsId()
     throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  MESSAGE("SMESH_Mesh_i::GetElementsId");
  SMESH::long_array_var aResult = new SMESH::long_array();
  SMESHDS_Mesh* aSMESHDS_Mesh = _impl->GetMeshDS();

  if ( aSMESHDS_Mesh == NULL )
    return aResult._retn();

  long nbElements = NbElements();
  aResult->length( nbElements );
  SMDS_ElemIteratorPtr anIt = aSMESHDS_Mesh->elementsIterator();
  for ( int i = 0, n = nbElements; i < n && anIt->more(); i++ )
    aResult[i] = anIt->next()->GetID();

  return aResult._retn();
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH::long_array* SMESH_Mesh_i::GetElementsByType( SMESH::ElementType theElemType )
    throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  MESSAGE("SMESH_subMesh_i::GetElementsByType");
  SMESH::long_array_var aResult = new SMESH::long_array();
  SMESHDS_Mesh* aSMESHDS_Mesh = _impl->GetMeshDS();

  if ( aSMESHDS_Mesh == NULL )
    return aResult._retn();

  long nbElements = NbElements();

  // No sense in returning ids of elements along with ids of nodes:
  // when theElemType == SMESH::ALL, return node ids only if
  // there are no elements
  if ( theElemType == SMESH::NODE || theElemType == SMESH::ALL && nbElements == 0 )
    return GetNodesId();

  aResult->length( nbElements );

  int i = 0;

  SMDS_ElemIteratorPtr anIt = aSMESHDS_Mesh->elementsIterator();
  while ( i < nbElements && anIt->more() ) {
    const SMDS_MeshElement* anElem = anIt->next();
    if ( theElemType == SMESH::ALL || anElem->GetType() == (SMDSAbs_ElementType)theElemType )
      aResult[i++] = anElem->GetID();
  }

  aResult->length( i );

  return aResult._retn();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
  
SMESH::long_array* SMESH_Mesh_i::GetNodesId()
  throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  MESSAGE("SMESH_subMesh_i::GetNodesId");
  SMESH::long_array_var aResult = new SMESH::long_array();
  SMESHDS_Mesh* aSMESHDS_Mesh = _impl->GetMeshDS();

  if ( aSMESHDS_Mesh == NULL )
    return aResult._retn();

  long nbNodes = NbNodes();
  aResult->length( nbNodes );
  SMDS_NodeIteratorPtr anIt = aSMESHDS_Mesh->nodesIterator();
  for ( int i = 0, n = nbNodes; i < n && anIt->more(); i++ )
    aResult[i] = anIt->next()->GetID();

  return aResult._retn();
}

