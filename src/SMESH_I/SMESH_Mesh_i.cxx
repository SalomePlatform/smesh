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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//
//
//  File   : SMESH_Mesh_i.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

#include "SMESH_Mesh_i.hxx"

#include "SMESH_Filter_i.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Group_i.hxx"
#include "SMESH_MEDMesh_i.hxx"
#include "SMESH_MeshEditor_i.hxx"
#include "SMESH_PythonDump.hxx"
#include "SMESH_subMesh_i.hxx"

#include "DriverMED_R_SMESHDS_Mesh.h"
#include "DriverMED_W_SMESHDS_Mesh.h"
#include "SMDS_VolumeTool.hxx"
#include "SMESHDS_Command.hxx"
#include "SMESHDS_CommandType.hxx"
#include "SMESHDS_GroupOnGeom.hxx"
#include "SMESH_Group.hxx"
#include "SMESH_MeshEditor.hxx"
#include "SMESH_MesherHelper.hxx"
#include "SMDS_EdgePosition.hxx"
#include "SMDS_FacePosition.hxx"

#include "OpUtil.hxx"
#include "SALOME_NamingService.hxx"
#include "Utils_CorbaException.hxx"
#include "Utils_ExceptHandlers.hxx"
#include "Utils_SINGLETON.hxx"
#include "utilities.h"

// OCCT Includes
#include <BRep_Builder.hxx>
#include <OSD_Directory.hxx>
#include <OSD_File.hxx>
#include <OSD_Path.hxx>
#include <OSD_Protection.hxx>
#include <TColStd_MapIteratorOfMapOfInteger.hxx>
#include <TColStd_MapOfInteger.hxx>
#include <TColStd_SequenceOfInteger.hxx>
#include <TCollection_AsciiString.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Compound.hxx>

// STL Includes
#include <string>
#include <iostream>
#include <sstream>

#ifdef _DEBUG_
static int MYDEBUG = 0;
#else
static int MYDEBUG = 0;
#endif

using namespace std;
using SMESH::TPythonDump;

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
  MESSAGE("SMESH_Mesh_i");
  _impl = NULL;
  _gen_i = gen_i;
  _id = myIdGenerator++;
  _studyId = studyId;
}

//=============================================================================
/*!
 *  Destructor
 */
//=============================================================================

SMESH_Mesh_i::~SMESH_Mesh_i()
{
  INFOS("~SMESH_Mesh_i");
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

//================================================================================
/*!
 * \brief return true if mesh has a shape to build a shape on
 */
//================================================================================

CORBA::Boolean SMESH_Mesh_i::HasShapeToMesh()
  throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  bool res = false;
  try {
    res = _impl->HasShapeToMesh();
  }
  catch(SALOME_Exception & S_ex) {
    THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
  }
  return res;
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
    status = _impl->MEDToMesh( theFileName, theMeshName );
  }
  catch( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
  }
  catch ( ... ) {
    THROW_SALOME_CORBA_EXCEPTION("ImportMEDFile(): unknown exception", SALOME::BAD_PARAM);
  }

  CreateGroupServants();

  return ConvertDriverMEDReadStatus(status);
}

//================================================================================
/*!
 * \brief Return string representation of a MED file version comprising nbDigits
 */
//================================================================================

char* SMESH_Mesh_i::GetVersionString(SMESH::MED_VERSION version, CORBA::Short nbDigits)
{
  std::string ver = DriverMED_W_SMESHDS_Mesh::GetVersionString(MED::EVersion(version),
                                                               nbDigits);
  return CORBA::string_dup( ver.c_str() );
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

  CreateGroupServants();

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

// int SMESH_Mesh_i::importMEDFile( const char* theFileName, const char* theMeshName )
// {
//   // Read mesh with name = <theMeshName> and all its groups into SMESH_Mesh
//   int status = _impl->MEDToMesh( theFileName, theMeshName );
//   CreateGroupServants();

//   return status;
// }

//=============================================================================
/*!
 *
 */
//=============================================================================

#define RETURNCASE(hyp_stat) case SMESH_Hypothesis::hyp_stat: return SMESH::hyp_stat;

SMESH::Hypothesis_Status SMESH_Mesh_i::ConvertHypothesisStatus
                         (SMESH_Hypothesis::Hypothesis_Status theStatus)
{
  switch (theStatus) {
  RETURNCASE( HYP_OK            );
  RETURNCASE( HYP_MISSING       );
  RETURNCASE( HYP_CONCURENT     );
  RETURNCASE( HYP_BAD_PARAMETER );
  RETURNCASE( HYP_HIDDEN_ALGO   );
  RETURNCASE( HYP_HIDING_ALGO   );
  RETURNCASE( HYP_UNKNOWN_FATAL );
  RETURNCASE( HYP_INCOMPATIBLE  );
  RETURNCASE( HYP_NOTCONFORM    );
  RETURNCASE( HYP_ALREADY_EXIST );
  RETURNCASE( HYP_BAD_DIM       );
  RETURNCASE( HYP_BAD_SUBSHAPE  );
  RETURNCASE( HYP_BAD_GEOMETRY  );
  default:;
  }
  return SMESH::HYP_UNKNOWN_FATAL;
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

  // Update Python script
  if(_impl->HasShapeToMesh()) {
    TPythonDump() << "status = " << _this() << ".AddHypothesis( "
                  << aSubShapeObject << ", " << anHyp << " )";
  }
  else {
    TPythonDump() << "status = " << _this() << ".AddHypothesis( "<< anHyp << " )";
  }
  
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

  if (CORBA::is_nil(aSubShapeObject) && HasShapeToMesh())
    THROW_SALOME_CORBA_EXCEPTION("bad subShape reference",
                                 SALOME::BAD_PARAM);

  SMESH::SMESH_Hypothesis_var myHyp = SMESH::SMESH_Hypothesis::_narrow(anHyp);
  if (CORBA::is_nil(myHyp))
    THROW_SALOME_CORBA_EXCEPTION("bad hypothesis reference",
                                 SALOME::BAD_PARAM);

  SMESH_Hypothesis::Hypothesis_Status status = SMESH_Hypothesis::HYP_OK;
  try
  {
    TopoDS_Shape myLocSubShape;
    //use PseudoShape in case if mesh has no shape
    if(HasShapeToMesh())
      myLocSubShape = _gen_i->GeomObjectToShape( aSubShapeObject);
    else              
      myLocSubShape = _impl->GetShapeToMesh();
    
    int hypId = myHyp->GetId();
    status = _impl->AddHypothesis(myLocSubShape, hypId);
    if ( !SMESH_Hypothesis::IsStatusFatal(status) ) {
      _mapHypo[hypId] = SMESH::SMESH_Hypothesis::_duplicate( myHyp );
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

  // Update Python script
    // Update Python script
  if(_impl->HasShapeToMesh()) {
  TPythonDump() << "status = " << _this() << ".RemoveHypothesis( "
                << aSubShapeObject << ", " << anHyp << " )";
  }
  else {
    TPythonDump() << "status = " << _this() << ".RemoveHypothesis( "
                  << anHyp << " )";
  }

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

	if (CORBA::is_nil(aSubShapeObject) && HasShapeToMesh())
		THROW_SALOME_CORBA_EXCEPTION("bad subShape reference",
			SALOME::BAD_PARAM);

	SMESH::SMESH_Hypothesis_var myHyp = SMESH::SMESH_Hypothesis::_narrow(anHyp);
	if (CORBA::is_nil(myHyp))
	  THROW_SALOME_CORBA_EXCEPTION("bad hypothesis reference",
			SALOME::BAD_PARAM);

	SMESH_Hypothesis::Hypothesis_Status status = SMESH_Hypothesis::HYP_OK;
	try
	{
                TopoDS_Shape myLocSubShape;
                //use PseudoShape in case if mesh has no shape
                if(HasShapeToMesh())
                  myLocSubShape = _gen_i->GeomObjectToShape( aSubShapeObject);
                else
                  myLocSubShape = _impl->GetShapeToMesh();
                
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

    if ( _gen_i->CanPublishInStudy( subMesh )) {
      SALOMEDS::SObject_var aSO =
        _gen_i->PublishSubMesh(_gen_i->GetCurrentStudy(), aMesh,
                               subMesh, aSubShapeObject, theName );
      if ( !aSO->_is_nil()) {
        // Update Python script
        TPythonDump() << aSO << " = " << _this() << ".GetSubMesh( "
                      << aSubShapeObject << ", '" << theName << "' )";
      }
    }
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

      // Update Python script
      TPythonDump() << _this() << ".RemoveSubMesh( " << anSO << " )";
    }
  }

  removeSubMesh( theSubMesh, aSubShapeObject.in() );
}

//=============================================================================
/*!
 *  ElementTypeString
 */
//=============================================================================
#define CASE2STRING(enum) case SMESH::enum: return "SMESH."#enum;
inline TCollection_AsciiString ElementTypeString (SMESH::ElementType theElemType)
{
  switch (theElemType) {
    CASE2STRING( ALL );
    CASE2STRING( NODE );
    CASE2STRING( EDGE );
    CASE2STRING( FACE );
    CASE2STRING( VOLUME );
  default:;
  }
  return "";
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

  if ( _gen_i->CanPublishInStudy( aNewGroup ) ) {
    SALOMEDS::SObject_var aSO =
      _gen_i->PublishGroup(_gen_i->GetCurrentStudy(), _this(),
                           aNewGroup, GEOM::GEOM_Object::_nil(), theName);
    if ( !aSO->_is_nil()) {
      // Update Python script
      TPythonDump() << aSO << " = " << _this() << ".CreateGroup( "
                    << ElementTypeString(theElemType) << ", '" << theName << "' )";
    }
  }
  return aNewGroup._retn();
}


//=============================================================================
/*!
 *
 */
//=============================================================================
SMESH::SMESH_GroupOnGeom_ptr SMESH_Mesh_i::CreateGroupFromGEOM (SMESH::ElementType    theElemType,
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
    if ( _gen_i->CanPublishInStudy( aNewGroup ) ) {
      SALOMEDS::SObject_var aSO =
        _gen_i->PublishGroup(_gen_i->GetCurrentStudy(), _this(),
                             aNewGroup, theGeomObj, theName);
      if ( !aSO->_is_nil()) {
        // Update Python script
        TPythonDump() << aSO << " = " << _this() << ".CreateGroupFromGEOM("
                      << ElementTypeString(theElemType) << ", '" << theName << "', "
                      << theGeomObj << " )";
      }
    }
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
    SALOMEDS::SObject_var aGroupSO = _gen_i->ObjectToSObject( aStudy, theGroup );

    if ( !aGroupSO->_is_nil() ) {
      // Update Python script
      TPythonDump() << _this() << ".RemoveGroup( " << aGroupSO << " )";

      // Remove group's SObject
      aStudy->NewBuilder()->RemoveObject( aGroupSO );
    }
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

  // Update Python script
  TPythonDump() << _this() << ".RemoveGroupWithContents( " << theGroup << " )";

  // Remove contents
  if ( aGroup->GetType() == SMESH::NODE )
    aMeshEditor->RemoveNodes( anIds );
  else
    aMeshEditor->RemoveElements( anIds );

  // Remove group
  RemoveGroup( theGroup );

  // Clear python lines, created by RemoveNodes/Elements() and RemoveGroup()
  _gen_i->RemoveLastFromPythonScript(_gen_i->GetCurrentStudy()->StudyId());
  _gen_i->RemoveLastFromPythonScript(_gen_i->GetCurrentStudy()->StudyId());
}


//================================================================================
/*!
 * \brief Get the list of groups existing in the mesh
  * \retval SMESH::ListOfGroups * - list of groups
 */
//================================================================================

SMESH::ListOfGroups * SMESH_Mesh_i::GetGroups() throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if (MYDEBUG) MESSAGE("GetGroups");

  SMESH::ListOfGroups_var aList = new SMESH::ListOfGroups();

  // Python Dump
  TPythonDump aPythonDump;
  if ( !_mapGroups.empty() ) // (IMP13463) avoid "SyntaxError: can't assign to []"
    aPythonDump << "[ ";

  try {
    aList->length( _mapGroups.size() );
    int i = 0;
    map<int, SMESH::SMESH_GroupBase_ptr>::iterator it = _mapGroups.begin();
    for ( ; it != _mapGroups.end(); it++ ) {
      if ( CORBA::is_nil( it->second )) continue;
      aList[i++] = SMESH::SMESH_GroupBase::_duplicate( it->second );
      // Python Dump
      if (i > 1) aPythonDump << ", ";
      aPythonDump << it->second;
    }
    aList->length( i );
  }
  catch(SALOME_Exception & S_ex) {
    THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
  }

  // Update Python script
  if ( !_mapGroups.empty() ) // (IMP13463) avoid "SyntaxError: can't assign to []"
    aPythonDump << " ] = " << _this() << ".GetGroups()";

  return aList._retn();
}
//=============================================================================
/*!
 *  Get number of groups existing in the mesh
 */
//=============================================================================

CORBA::Long SMESH_Mesh_i::NbGroups() throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  return _mapGroups.size();
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
    if ( theGroup1->_is_nil() || theGroup2->_is_nil() ||
         theGroup1->GetType() != theGroup2->GetType() )
      return SMESH::SMESH_Group::_nil();

    // Create Union
    SMESH::SMESH_Group_var aResGrp = CreateGroup( theGroup1->GetType(), theName );
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

    // Clear python lines, created by CreateGroup() and Add()
    SALOMEDS::Study_ptr aStudy = _gen_i->GetCurrentStudy();
    _gen_i->RemoveLastFromPythonScript(aStudy->StudyId());
    _gen_i->RemoveLastFromPythonScript(aStudy->StudyId());

    // Update Python script
    TPythonDump() << aResGrp << " = " << _this() << ".UnionGroups( "
                  << theGroup1 << ", " << theGroup2 << ", '"
                  << theName << "' )";

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
  if ( theGroup1->_is_nil() || theGroup2->_is_nil() ||
       theGroup1->GetType() != theGroup2->GetType() )
    return SMESH::SMESH_Group::_nil();

  // Create Intersection
  SMESH::SMESH_Group_var aResGrp = CreateGroup( theGroup1->GetType(), theName );
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

  // Clear python lines, created by CreateGroup() and Add()
  SALOMEDS::Study_ptr aStudy = _gen_i->GetCurrentStudy();
  _gen_i->RemoveLastFromPythonScript(aStudy->StudyId());
  _gen_i->RemoveLastFromPythonScript(aStudy->StudyId());

  // Update Python script
  TPythonDump() << aResGrp << " = " << _this() << ".IntersectGroups( "
                << theGroup1 << ", " << theGroup2 << ", '" << theName << "')";

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
  if ( theGroup1->_is_nil() || theGroup2->_is_nil() ||
       theGroup1->GetType() != theGroup2->GetType() )
    return SMESH::SMESH_Group::_nil();

  // Perform Cutting
  SMESH::SMESH_Group_var aResGrp = CreateGroup( theGroup1->GetType(), theName );
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

  // Clear python lines, created by CreateGroup() and Add()
  SALOMEDS::Study_ptr aStudy = _gen_i->GetCurrentStudy();
  _gen_i->RemoveLastFromPythonScript(aStudy->StudyId());
  _gen_i->RemoveLastFromPythonScript(aStudy->StudyId());

  // Update Python script
  TPythonDump() << aResGrp << " = " << _this() << ".CutGroups( "
                << theGroup1 << ", " << theGroup2 << ", '"
                << theName << "' )";

  return aResGrp._retn();
}

//================================================================================
/*!
 * \brief Return group items of a group present in a study
 */
//================================================================================

static GEOM::GEOM_Object_ptr getGroupItemsFromStudy(CORBA::Object_ptr    theMesh,
                                                    SMESH_Gen_i*         theGen,
                                                    list<TopoDS_Shape> & theItems)
{
  GEOM::GEOM_Object_var groupObj;
  SALOMEDS::Study_var  study = theGen->GetCurrentStudy();
  GEOM::GEOM_Gen_var geomGen = theGen->GetGeomEngine();
  if ( study->_is_nil() || geomGen->_is_nil() )
    return groupObj._retn();
  
  GEOM::GEOM_IGroupOperations_var groupOp =
    geomGen->GetIGroupOperations( theGen->GetCurrentStudyID() );
  GEOM::GEOM_IShapesOperations_var shapeOp =
    geomGen->GetIShapesOperations( theGen->GetCurrentStudyID() );

  SALOMEDS::SObject_var meshOS = theGen->ObjectToSObject(study, theMesh);
  if ( meshOS->_is_nil() || groupOp->_is_nil() || shapeOp->_is_nil() )
    return groupObj._retn();
  SALOMEDS::SObject_var fatherSO = meshOS->GetFather();
  if ( fatherSO->_is_nil() || fatherSO->Tag() != theGen->GetSubMeshOnCompoundTag() )
    return groupObj._retn(); // keep only submeshes on groups

  SALOMEDS::ChildIterator_var anIter = study->NewChildIterator(meshOS);
  if ( anIter->_is_nil() ) return groupObj._retn();
  for ( ; anIter->More(); anIter->Next())
  {
    SALOMEDS::SObject_var aSObject = anIter->Value();
    SALOMEDS::SObject_var aRefSO;
    if ( !aSObject->_is_nil() && aSObject->ReferencedObject(aRefSO) )
    {
      groupObj = GEOM::GEOM_Object::_narrow(aRefSO->GetObject());
      if ( groupObj->_is_nil() ) break;
      GEOM::ListOfLong_var  ids = groupOp->GetObjects( groupObj );
      GEOM::GEOM_Object_var mainShape = groupObj->GetMainShape();
      for ( int i = 0; i < ids->length(); ++i ) {
        GEOM::GEOM_Object_var subShape = shapeOp->GetSubShape( mainShape, ids[i] );
        TopoDS_Shape S = theGen->GeomObjectToShape( subShape );
        if ( !S.IsNull() )
          theItems.push_back( S );
      }
      break;
    }
  }
  return groupObj._retn();
}

//=============================================================================
/*!
 * \brief Update hypotheses assigned to geom groups if the latter change
 * 
 * NPAL16168: "geometrical group edition from a submesh don't modifiy mesh computation"
 */
//=============================================================================

void SMESH_Mesh_i::CheckGeomGroupModif()
{
  if ( !_impl->HasShapeToMesh() ) return;

  SALOMEDS::Study_var study = _gen_i->GetCurrentStudy();
  if ( study->_is_nil() ) return;

  // check if items of groups changed
  map<int, ::SMESH_subMesh*>::iterator i_sm = _mapSubMesh.begin();
  for ( ; i_sm != _mapSubMesh.end(); ++i_sm )
  {
    const TopoDS_Shape & oldGroupShape = i_sm->second->GetSubShape();
    SMESHDS_SubMesh * oldDS = i_sm->second->GetSubMeshDS();
    if ( !oldDS /*|| !oldDS->IsComplexSubmesh()*/ )
      continue;
    int oldID = i_sm->first;
    map<int, SMESH::SMESH_subMesh_ptr>::iterator i_smIor = _mapSubMeshIor.find( oldID );
    if ( i_smIor == _mapSubMeshIor.end() )
      continue;
    list< TopoDS_Shape> newItems;
    GEOM::GEOM_Object_var groupObj = getGroupItemsFromStudy ( i_smIor->second, _gen_i, newItems );
    if ( groupObj->_is_nil() )
      continue;

    int nbOldItems = oldDS->IsComplexSubmesh() ? oldDS->NbSubMeshes() : 1;
    int nbNewItems = newItems.size();
    bool groupChanged = ( nbOldItems != nbNewItems);
    if ( !groupChanged ) {
      if ( !oldDS->IsComplexSubmesh() ) { // old group has one item
        groupChanged = ( oldGroupShape != newItems.front() );
      }
      else {
        list<TopoDS_Shape>::iterator item = newItems.begin();
        for ( ; item != newItems.end() && !groupChanged; ++item )
        {
          SMESHDS_SubMesh * itemDS = _impl->GetMeshDS()->MeshElements( *item );
          groupChanged = ( !itemDS || !oldDS->ContainsSubMesh( itemDS ));
        }
      }
    }
    // update hypotheses and submeshes if necessary
    if ( groupChanged )
    {
      // get a new group shape
      GEOM_Client* geomClient = _gen_i->GetShapeReader();
      if ( !geomClient ) continue;
      TCollection_AsciiString groupIOR = _gen_i->GetGeomEngine()->GetStringFromIOR( groupObj );
      geomClient->RemoveShapeFromBuffer( groupIOR );
      TopoDS_Shape newGroupShape = _gen_i->GeomObjectToShape( groupObj );
      // update hypotheses
      list <const SMESHDS_Hypothesis * > hyps = _impl->GetHypothesisList(oldGroupShape);
      list <const SMESHDS_Hypothesis * >::iterator hypIt;
      for ( hypIt = hyps.begin(); hypIt != hyps.end(); ++hypIt )
      {
        _impl->RemoveHypothesis( oldGroupShape, (*hypIt)->GetID());
        _impl->AddHypothesis   ( newGroupShape, (*hypIt)->GetID());
      }
      // care of submeshes
      SMESH_subMesh* newSubmesh = _impl->GetSubMesh( newGroupShape );
      int newID = newSubmesh->GetId();
      if ( newID != oldID ) {
        _mapSubMesh   [ newID ] = newSubmesh;
        _mapSubMesh_i [ newID ] = _mapSubMesh_i [ oldID ];
        _mapSubMeshIor[ newID ] = _mapSubMeshIor[ oldID ];
        _mapSubMesh.erase   (oldID);
        _mapSubMesh_i.erase (oldID);
        _mapSubMeshIor.erase(oldID);
        _mapSubMesh_i [ newID ]->changeLocalId( newID );
      }
    }
  }
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
 * Return mesh editor
 */
//=============================================================================

SMESH::SMESH_MeshEditor_ptr SMESH_Mesh_i::GetMeshEditor()
{
  // Create MeshEditor
  SMESH_MeshEditor_i *aMeshEditor = new SMESH_MeshEditor_i( this, false );
  SMESH::SMESH_MeshEditor_var aMesh = aMeshEditor->_this();

  // Update Python script
  TPythonDump() << aMeshEditor << " = " << _this() << ".GetMeshEditor()";

  return aMesh._retn();
}

//=============================================================================
/*!
 * Return mesh edition previewer
 */
//=============================================================================

SMESH::SMESH_MeshEditor_ptr SMESH_Mesh_i::GetMeshEditPreviewer()
{
  SMESH_MeshEditor_i *aMeshEditor = new SMESH_MeshEditor_i( this, true );
  SMESH::SMESH_MeshEditor_var aMesh = aMeshEditor->_this();
  return aMesh._retn();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
void SMESH_Mesh_i::SetAutoColor(CORBA::Boolean theAutoColor) throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  _impl->SetAutoColor(theAutoColor);
}

//=============================================================================
/*!
 *
 */
//=============================================================================
CORBA::Boolean SMESH_Mesh_i::GetAutoColor() throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  return _impl->GetAutoColor();
}


//=============================================================================
/*!
 *  Export in different formats
 */
//=============================================================================

CORBA::Boolean SMESH_Mesh_i::HasDuplicatedGroupNamesMED()
{
  return _impl->HasDuplicatedGroupNamesMED();
}

static void PrepareForWriting (const char* file)
{
  TCollection_AsciiString aFullName ((char*)file);
  OSD_Path aPath (aFullName);
  OSD_File aFile (aPath);
  if (aFile.Exists()) {
    // existing filesystem node
    if (aFile.KindOfFile() == OSD_FILE) {
      if (aFile.IsWriteable()) {
        aFile.Reset();
        aFile.Remove();
        if (aFile.Failed()) {
          TCollection_AsciiString msg ("File ");
          msg += aFullName + " cannot be replaced.";
          THROW_SALOME_CORBA_EXCEPTION(msg.ToCString(), SALOME::BAD_PARAM);
        }
      } else {
        TCollection_AsciiString msg ("File ");
        msg += aFullName + " cannot be overwritten.";
        THROW_SALOME_CORBA_EXCEPTION(msg.ToCString(), SALOME::BAD_PARAM);
      }
    } else {
      TCollection_AsciiString msg ("Location ");
      msg += aFullName + " is not a file.";
      THROW_SALOME_CORBA_EXCEPTION(msg.ToCString(), SALOME::BAD_PARAM);
    }
  } else {
    // nonexisting file; check if it can be created
    aFile.Reset();
    aFile.Build(OSD_WriteOnly, OSD_Protection());
    if (aFile.Failed()) {
      TCollection_AsciiString msg ("You cannot create the file ");
      msg += aFullName + ". Check the directory existance and access rights.";
      THROW_SALOME_CORBA_EXCEPTION(msg.ToCString(), SALOME::BAD_PARAM);
    } else {
      aFile.Close();
      aFile.Remove();
    }
  }
}

void SMESH_Mesh_i::ExportToMED (const char* file,
				CORBA::Boolean auto_groups,
				SMESH::MED_VERSION theVersion)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);

  // Update Python script
  TPythonDump() << _this() << ".ExportToMED( '"
                << file << "', " << auto_groups << ", " << theVersion << " )";

  // Perform Export
  PrepareForWriting(file);
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

void SMESH_Mesh_i::ExportMED (const char* file,
			      CORBA::Boolean auto_groups)
  throw(SALOME::SALOME_Exception)
{
  ExportToMED(file,auto_groups,SMESH::MED_V2_1);
}

void SMESH_Mesh_i::ExportDAT (const char *file)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);

  // Update Python script
  TPythonDump() << _this() << ".ExportDAT( '" << file << "' )";

  // Perform Export
  PrepareForWriting(file);
  _impl->ExportDAT(file);
}

void SMESH_Mesh_i::ExportUNV (const char *file)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);

  // Update Python script
  TPythonDump() << _this() << ".ExportUNV( '" << file << "' )";

  // Perform Export
  PrepareForWriting(file);
  _impl->ExportUNV(file);
}

void SMESH_Mesh_i::ExportSTL (const char *file, const bool isascii)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);

  // Update Python script
  TPythonDump() << _this() << ".ExportSTL( '" << file << "', " << isascii << " )";

  // Perform Export
  PrepareForWriting(file);
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

CORBA::Long SMESH_Mesh_i::NbEdgesOfOrder(SMESH::ElementOrder order)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  return _impl->NbEdges( (SMDSAbs_ElementOrder) order);
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

CORBA::Long SMESH_Mesh_i::NbPolygons()throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  return _impl->NbPolygons();
}

CORBA::Long SMESH_Mesh_i::NbFacesOfOrder(SMESH::ElementOrder order)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  return _impl->NbFaces( (SMDSAbs_ElementOrder) order);
}

CORBA::Long SMESH_Mesh_i::NbTrianglesOfOrder(SMESH::ElementOrder order)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  return _impl->NbTriangles( (SMDSAbs_ElementOrder) order);
}

CORBA::Long SMESH_Mesh_i::NbQuadranglesOfOrder(SMESH::ElementOrder order)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  return _impl->NbQuadrangles( (SMDSAbs_ElementOrder) order);
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

CORBA::Long SMESH_Mesh_i::NbPolyhedrons()throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  return _impl->NbPolyhedrons();
}

CORBA::Long SMESH_Mesh_i::NbVolumesOfOrder(SMESH::ElementOrder order)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  return _impl->NbVolumes( (SMDSAbs_ElementOrder) order);
}

CORBA::Long SMESH_Mesh_i::NbTetrasOfOrder(SMESH::ElementOrder order)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  return _impl->NbTetras( (SMDSAbs_ElementOrder) order);
}

CORBA::Long SMESH_Mesh_i::NbHexasOfOrder(SMESH::ElementOrder order)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  return _impl->NbHexas( (SMDSAbs_ElementOrder) order);
}

CORBA::Long SMESH_Mesh_i::NbPyramidsOfOrder(SMESH::ElementOrder order)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  return _impl->NbPyramids( (SMDSAbs_ElementOrder) order);
}

CORBA::Long SMESH_Mesh_i::NbPrismsOfOrder(SMESH::ElementOrder order)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  return _impl->NbPrisms( (SMDSAbs_ElementOrder) order);
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
//   SMESH::long_array_var aResult = new SMESH::long_array();
//   SMESHDS_Mesh* aSMESHDS_Mesh = _impl->GetMeshDS();
//   int aMinId = aSMESHDS_Mesh->MinElementID();
//   int aMaxId =  aSMESHDS_Mesh->MaxElementID();

//   aResult->length(aMaxId - aMinId + 1);

//   for (int i = 0, id = aMinId; id <= aMaxId; id++  )
//     aResult[i++] = id;

//   return aResult._retn();
  // PAL12398
  return GetElementsId();
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

//=============================================================================
/*!
 *
 */
//=============================================================================

SMESH::ElementType SMESH_Mesh_i::GetElementType( const CORBA::Long id, const bool iselem )
  throw (SALOME::SALOME_Exception)
{
  return ( SMESH::ElementType )_impl->GetElementType( id, iselem );
}


//=============================================================================
/*!
 * Returns ID of elements for given submesh
 */
//=============================================================================
SMESH::long_array* SMESH_Mesh_i::GetSubMeshElementsId(const CORBA::Long ShapeID)
     throw (SALOME::SALOME_Exception)
{
  SMESH::long_array_var aResult = new SMESH::long_array();

  SMESH_subMesh* SM = _impl->GetSubMeshContaining(ShapeID);
  if(!SM) return aResult._retn();

  SMESHDS_SubMesh* SDSM = SM->GetSubMeshDS();
  if(!SDSM) return aResult._retn();

  aResult->length(SDSM->NbElements());

  SMDS_ElemIteratorPtr eIt = SDSM->GetElements();
  int i = 0;
  while ( eIt->more() ) {
    aResult[i++] = eIt->next()->GetID();
  }

  return aResult._retn();
}


//=============================================================================
/*!
 * Returns ID of nodes for given submesh
 * If param all==true - returns all nodes, else -
 * returns only nodes on shapes.
 */
//=============================================================================
SMESH::long_array* SMESH_Mesh_i::GetSubMeshNodesId(const CORBA::Long ShapeID, CORBA::Boolean all)
     throw (SALOME::SALOME_Exception)
{
  SMESH::long_array_var aResult = new SMESH::long_array();

  SMESH_subMesh* SM = _impl->GetSubMeshContaining(ShapeID);
  if(!SM) return aResult._retn();

  SMESHDS_SubMesh* SDSM = SM->GetSubMeshDS();
  if(!SDSM) return aResult._retn();

  set<int> theElems;
  if( !all || (SDSM->NbElements()==0) ) { // internal nodes or vertex submesh
    SMDS_NodeIteratorPtr nIt = SDSM->GetNodes();
    while ( nIt->more() ) {
      const SMDS_MeshNode* elem = nIt->next();
      theElems.insert( elem->GetID() );
    }
  }
  else { // all nodes of submesh elements
    SMDS_ElemIteratorPtr eIt = SDSM->GetElements();
    while ( eIt->more() ) {
      const SMDS_MeshElement* anElem = eIt->next();
      SMDS_ElemIteratorPtr nIt = anElem->nodesIterator();
      while ( nIt->more() ) {
        const SMDS_MeshElement* elem = nIt->next();
        theElems.insert( elem->GetID() );
      }
    }
  }

  aResult->length(theElems.size());
  set<int>::iterator itElem;
  int i = 0;
  for ( itElem = theElems.begin(); itElem != theElems.end(); itElem++ )
    aResult[i++] = *itElem;

  return aResult._retn();
}
  

//=============================================================================
/*!
 * Returns type of elements for given submesh
 */
//=============================================================================
SMESH::ElementType SMESH_Mesh_i::GetSubMeshElementType(const CORBA::Long ShapeID)
     throw (SALOME::SALOME_Exception)
{
  SMESH_subMesh* SM = _impl->GetSubMeshContaining(ShapeID);
  if(!SM) return SMESH::ALL;

  SMESHDS_SubMesh* SDSM = SM->GetSubMeshDS();
  if(!SDSM) return SMESH::ALL;

  if(SDSM->NbElements()==0)
    return (SM->GetSubShape().ShapeType() == TopAbs_VERTEX) ? SMESH::NODE : SMESH::ALL;

  SMDS_ElemIteratorPtr eIt = SDSM->GetElements();
  const SMDS_MeshElement* anElem = eIt->next();
  return ( SMESH::ElementType ) anElem->GetType();
}
  

//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::LongLong SMESH_Mesh_i::GetMeshPtr()
{
  CORBA::LongLong pointeur = CORBA::LongLong(_impl);
  cerr << "CORBA::LongLong SMESH_Mesh_i::GetMeshPtr() " << pointeur << endl;
  return pointeur;
}


//=============================================================================
/*!
 * Get XYZ coordinates of node as list of double
 * If there is not node for given ID - returns empty list
 */
//=============================================================================

SMESH::double_array* SMESH_Mesh_i::GetNodeXYZ(const CORBA::Long id)
{
  SMESH::double_array_var aResult = new SMESH::double_array();
  SMESHDS_Mesh* aSMESHDS_Mesh = _impl->GetMeshDS();
  if ( aSMESHDS_Mesh == NULL )
    return aResult._retn();

  // find node
  const SMDS_MeshNode* aNode = aSMESHDS_Mesh->FindNode(id);
  if(!aNode)
    return aResult._retn();

  // add coordinates
  aResult->length(3);
  aResult[0] = aNode->X();
  aResult[1] = aNode->Y();
  aResult[2] = aNode->Z();
  return aResult._retn();
}


//=============================================================================
/*!
 * For given node returns list of IDs of inverse elements
 * If there is not node for given ID - returns empty list
 */
//=============================================================================

SMESH::long_array* SMESH_Mesh_i::GetNodeInverseElements(const CORBA::Long id)
{
  SMESH::long_array_var aResult = new SMESH::long_array();
  SMESHDS_Mesh* aSMESHDS_Mesh = _impl->GetMeshDS();
  if ( aSMESHDS_Mesh == NULL )
    return aResult._retn();

  // find node
  const SMDS_MeshNode* aNode = aSMESHDS_Mesh->FindNode(id);
  if(!aNode)
    return aResult._retn();

  // find inverse elements
  SMDS_ElemIteratorPtr eIt = aNode->GetInverseElementIterator();
  TColStd_SequenceOfInteger IDs;
  while(eIt->more()) {
    const SMDS_MeshElement* elem = eIt->next();
    IDs.Append(elem->GetID());
  }
  if(IDs.Length()>0) {
    aResult->length(IDs.Length());
    int i = 1;
    for(; i<=IDs.Length(); i++) {
      aResult[i-1] = IDs.Value(i);
    }
  }
  return aResult._retn();
}

//=============================================================================
/*!
 * \brief Return position of a node on shape
 */
//=============================================================================

SMESH::NodePosition* SMESH_Mesh_i::GetNodePosition(CORBA::Long NodeID)
{
  SMESH::NodePosition* aNodePosition = new SMESH::NodePosition();
  aNodePosition->shapeID = 0;
  aNodePosition->shapeType = GEOM::SHAPE;

  SMESHDS_Mesh* mesh = _impl->GetMeshDS();
  if ( !mesh ) return aNodePosition;

  if ( const SMDS_MeshNode* aNode = mesh->FindNode(NodeID) )
  {
    if ( SMDS_PositionPtr pos = aNode->GetPosition() )
    {
      aNodePosition->shapeID = pos->GetShapeId();
      switch ( pos->GetTypeOfPosition() ) {
      case SMDS_TOP_EDGE:
        aNodePosition->shapeType = GEOM::EDGE;
        aNodePosition->params.length(1);
        aNodePosition->params[0] =
          static_cast<SMDS_EdgePosition*>( pos.get() )->GetUParameter();
        break;
      case SMDS_TOP_FACE:
        aNodePosition->shapeType = GEOM::FACE;
        aNodePosition->params.length(2);
        aNodePosition->params[0] =
          static_cast<SMDS_FacePosition*>( pos.get() )->GetUParameter();
        aNodePosition->params[1] =
          static_cast<SMDS_FacePosition*>( pos.get() )->GetVParameter();
        break;
      case SMDS_TOP_VERTEX:
        aNodePosition->shapeType = GEOM::VERTEX;
        break;
      case SMDS_TOP_3DSPACE:
        if ( TopExp_Explorer(_impl->GetShapeToMesh(), TopAbs_SOLID).More() )
          aNodePosition->shapeType = GEOM::SOLID;
        else if ( TopExp_Explorer(_impl->GetShapeToMesh(), TopAbs_SHELL).More() )
          aNodePosition->shapeType = GEOM::SHELL;
        break;
      default:;
      }
    }
  }
  return aNodePosition;
}

//=============================================================================
/*!
 * If given element is node returns IDs of shape from position
 * If there is not node for given ID - returns -1
 */
//=============================================================================

CORBA::Long SMESH_Mesh_i::GetShapeID(const CORBA::Long id)
{
  SMESHDS_Mesh* aSMESHDS_Mesh = _impl->GetMeshDS();
  if ( aSMESHDS_Mesh == NULL )
    return -1;

  // try to find node
  const SMDS_MeshNode* aNode = aSMESHDS_Mesh->FindNode(id);
  if(aNode) {
    SMDS_PositionPtr pos = aNode->GetPosition();
    if(!pos)
      return -1;
    else
      return pos->GetShapeId();
  }

  return -1;
}


//=============================================================================
/*!
 * For given element returns ID of result shape after 
 * ::FindShape() from SMESH_MeshEditor
 * If there is not element for given ID - returns -1
 */
//=============================================================================

CORBA::Long SMESH_Mesh_i::GetShapeIDForElem(const CORBA::Long id)
{
  SMESHDS_Mesh* aSMESHDS_Mesh = _impl->GetMeshDS();
  if ( aSMESHDS_Mesh == NULL )
    return -1;

  // try to find element
  const SMDS_MeshElement* elem = aSMESHDS_Mesh->FindElement(id);
  if(!elem)
    return -1;

  //SMESH::SMESH_MeshEditor_var aMeshEditor = SMESH_Mesh_i::GetMeshEditor();
  ::SMESH_MeshEditor aMeshEditor(_impl);
  int index = aMeshEditor.FindShape( elem );
  if(index>0)
    return index;

  return -1;
}


//=============================================================================
/*!
 * Returns number of nodes for given element
 * If there is not element for given ID - returns -1
 */
//=============================================================================

CORBA::Long SMESH_Mesh_i::GetElemNbNodes(const CORBA::Long id)
{
  SMESHDS_Mesh* aSMESHDS_Mesh = _impl->GetMeshDS();
  if ( aSMESHDS_Mesh == NULL ) return -1;
  // try to find element
  const SMDS_MeshElement* elem = aSMESHDS_Mesh->FindElement(id);
  if(!elem) return -1;
  return elem->NbNodes();
}


//=============================================================================
/*!
 * Returns ID of node by given index for given element
 * If there is not element for given ID - returns -1
 * If there is not node for given index - returns -2
 */
//=============================================================================

CORBA::Long SMESH_Mesh_i::GetElemNode(const CORBA::Long id, const CORBA::Long index)
{
  SMESHDS_Mesh* aSMESHDS_Mesh = _impl->GetMeshDS();
  if ( aSMESHDS_Mesh == NULL ) return -1;
  const SMDS_MeshElement* elem = aSMESHDS_Mesh->FindElement(id);
  if(!elem) return -1;
  if( index>=elem->NbNodes() || index<0 ) return -1;
  return elem->GetNode(index)->GetID();
}

//=============================================================================
/*!
 * Returns IDs of nodes of given element
 */
//=============================================================================

SMESH::long_array* SMESH_Mesh_i::GetElemNodes(const CORBA::Long id)
{
  SMESH::long_array_var aResult = new SMESH::long_array();
  if ( SMESHDS_Mesh* aSMESHDS_Mesh = _impl->GetMeshDS() )
  {
    if ( const SMDS_MeshElement* elem = aSMESHDS_Mesh->FindElement(id) )
    {
      aResult->length( elem->NbNodes() );
      for ( int i = 0; i < elem->NbNodes(); ++i )
        aResult[ i ] = elem->GetNode( i )->GetID();
    }
  }
  return aResult._retn();
}

//=============================================================================
/*!
 * Returns true if given node is medium node
 * in given quadratic element
 */
//=============================================================================

CORBA::Boolean SMESH_Mesh_i::IsMediumNode(const CORBA::Long ide, const CORBA::Long idn)
{
  SMESHDS_Mesh* aSMESHDS_Mesh = _impl->GetMeshDS();
  if ( aSMESHDS_Mesh == NULL ) return false;
  // try to find node
  const SMDS_MeshNode* aNode = aSMESHDS_Mesh->FindNode(idn);
  if(!aNode) return false;
  // try to find element
  const SMDS_MeshElement* elem = aSMESHDS_Mesh->FindElement(ide);
  if(!elem) return false;

  return elem->IsMediumNode(aNode);
}


//=============================================================================
/*!
 * Returns true if given node is medium node
 * in one of quadratic elements
 */
//=============================================================================

CORBA::Boolean SMESH_Mesh_i::IsMediumNodeOfAnyElem(const CORBA::Long idn,
                                                   SMESH::ElementType theElemType)
{
  SMESHDS_Mesh* aSMESHDS_Mesh = _impl->GetMeshDS();
  if ( aSMESHDS_Mesh == NULL ) return false;

  // try to find node
  const SMDS_MeshNode* aNode = aSMESHDS_Mesh->FindNode(idn);
  if(!aNode) return false;

  SMESH_MesherHelper aHelper( *(_impl) );

  SMDSAbs_ElementType aType;
  if(theElemType==SMESH::EDGE) aType = SMDSAbs_Edge;
  else if(theElemType==SMESH::FACE) aType = SMDSAbs_Face;
  else if(theElemType==SMESH::VOLUME) aType = SMDSAbs_Volume;
  else aType = SMDSAbs_All;

  return aHelper.IsMedium(aNode,aType);
}


//=============================================================================
/*!
 * Returns number of edges for given element
 */
//=============================================================================

CORBA::Long SMESH_Mesh_i::ElemNbEdges(const CORBA::Long id)
{
  SMESHDS_Mesh* aSMESHDS_Mesh = _impl->GetMeshDS();
  if ( aSMESHDS_Mesh == NULL ) return -1;
  const SMDS_MeshElement* elem = aSMESHDS_Mesh->FindElement(id);
  if(!elem) return -1;
  return elem->NbEdges();
}


//=============================================================================
/*!
 * Returns number of faces for given element
 */
//=============================================================================

CORBA::Long SMESH_Mesh_i::ElemNbFaces(const CORBA::Long id)
{
  SMESHDS_Mesh* aSMESHDS_Mesh = _impl->GetMeshDS();
  if ( aSMESHDS_Mesh == NULL ) return -1;
  const SMDS_MeshElement* elem = aSMESHDS_Mesh->FindElement(id);
  if(!elem) return -1;
  return elem->NbFaces();
}


//=============================================================================
/*!
 * Returns true if given element is polygon
 */
//=============================================================================

CORBA::Boolean SMESH_Mesh_i::IsPoly(const CORBA::Long id)
{
  SMESHDS_Mesh* aSMESHDS_Mesh = _impl->GetMeshDS();
  if ( aSMESHDS_Mesh == NULL ) return false;
  const SMDS_MeshElement* elem = aSMESHDS_Mesh->FindElement(id);
  if(!elem) return false;
  return elem->IsPoly();
}


//=============================================================================
/*!
 * Returns true if given element is quadratic
 */
//=============================================================================

CORBA::Boolean SMESH_Mesh_i::IsQuadratic(const CORBA::Long id)
{
  SMESHDS_Mesh* aSMESHDS_Mesh = _impl->GetMeshDS();
  if ( aSMESHDS_Mesh == NULL ) return false;
  const SMDS_MeshElement* elem = aSMESHDS_Mesh->FindElement(id);
  if(!elem) return false;
  return elem->IsQuadratic();
}


//=============================================================================
/*!
 * Returns bary center for given element
 */
//=============================================================================

SMESH::double_array* SMESH_Mesh_i::BaryCenter(const CORBA::Long id)
{
  SMESH::double_array_var aResult = new SMESH::double_array();
  SMESHDS_Mesh* aSMESHDS_Mesh = _impl->GetMeshDS();
  if ( aSMESHDS_Mesh == NULL )
    return aResult._retn();

  const SMDS_MeshElement* elem = aSMESHDS_Mesh->FindElement(id);
  if(!elem)
    return aResult._retn();

  if(elem->GetType()==SMDSAbs_Volume) {
    SMDS_VolumeTool aTool;
    if(aTool.Set(elem)) {
      aResult->length(3);
      if (!aTool.GetBaryCenter( aResult[0], aResult[1], aResult[2]) )
        aResult->length(0);
    }
  }
  else {
    SMDS_ElemIteratorPtr anIt = elem->nodesIterator();
    int nbn = 0;
    double x=0., y=0., z=0.;
    for(; anIt->more(); ) {
      nbn++;
      const SMDS_MeshNode* aNode = static_cast<const SMDS_MeshNode*>(anIt->next());
      x += aNode->X();
      y += aNode->Y();
      z += aNode->Z();
    }
    if(nbn>0) {
      // add coordinates
      aResult->length(3);
      aResult[0] = x/nbn;
      aResult[1] = y/nbn;
      aResult[2] = z/nbn;
    }
  }

  return aResult._retn();
}


//=============================================================================
/*!
 * Create and publish group servants if any groups were imported or created anyhow
 */
//=============================================================================

void SMESH_Mesh_i::CreateGroupServants() 
{
  SALOMEDS::Study_ptr aStudy = _gen_i->GetCurrentStudy();

  ::SMESH_Mesh::GroupIteratorPtr groupIt = _impl->GetGroups();
  while ( groupIt->more() )
  {
    ::SMESH_Group* group = groupIt->next();
    int            anId = group->GetGroupDS()->GetID();

    map<int, SMESH::SMESH_GroupBase_ptr>::iterator it = _mapGroups.find(anId);
    if ( it != _mapGroups.end() && !CORBA::is_nil( it->second ))
      continue;

    SMESH_GroupBase_i* aGroupImpl;
    TopoDS_Shape       shape;
    if ( SMESHDS_GroupOnGeom* groupOnGeom =
         dynamic_cast<SMESHDS_GroupOnGeom*>( group->GetGroupDS() ))
    {
      aGroupImpl = new SMESH_GroupOnGeom_i( SMESH_Gen_i::GetPOA(), this, anId );
      shape      = groupOnGeom->GetShape();
    }
    else {
      aGroupImpl = new SMESH_Group_i( SMESH_Gen_i::GetPOA(), this, anId );
    }

    // To ensure correct mapping of servant and correct reference counting in GenericObj_i
    SMESH_Gen_i::GetPOA()->activate_object( aGroupImpl );
    aGroupImpl->Register();

    SMESH::SMESH_GroupBase_var groupVar =
      SMESH::SMESH_GroupBase::_narrow( aGroupImpl->_this() );
    _mapGroups[anId] = SMESH::SMESH_GroupBase::_duplicate( groupVar );

    // register CORBA object for persistence
    int nextId = _gen_i->RegisterObject( groupVar );
    if(MYDEBUG) MESSAGE( "Add group to map with id = "<< nextId);

    // publishing of the groups in the study
    if ( !aStudy->_is_nil() ) {
      GEOM::GEOM_Object_var shapeVar = _gen_i->ShapeToGeomObject( shape );
      _gen_i->PublishGroup( aStudy, _this(), groupVar, shapeVar, groupVar->GetName());
    }
  }
}

//=============================================================================
/*!
 * \brief Return groups cantained in _mapGroups by their IDs
 */
//=============================================================================

SMESH::ListOfGroups* SMESH_Mesh_i::GetGroups(const list<int>& groupIDs) const
{
  int nbGroups = groupIDs.size();
  SMESH::ListOfGroups_var aList = new SMESH::ListOfGroups();
  aList->length( nbGroups );

  list<int>::const_iterator ids = groupIDs.begin();
  for ( nbGroups = 0; ids != groupIDs.end(); ++ids )
  {
    map<int, SMESH::SMESH_GroupBase_ptr>::const_iterator it = _mapGroups.find( *ids );
    if ( it != _mapGroups.end() && !CORBA::is_nil( it->second ))
      aList[nbGroups++] = SMESH::SMESH_GroupBase::_duplicate( it->second );
  }
  aList->length( nbGroups );
  return aList._retn();
}
