//  Copyright (C) 2007-2010  CEA/DEN, EDF R&D, OPEN CASCADE
//
//  Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
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
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
//  File   : SMESH_Mesh_i.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH

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
#include "SMDS_ElemIterator.hxx"
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
#include "GEOMImpl_Types.hxx"

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
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Compound.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopTools_MapIteratorOfMapOfShape.hxx>

// STL Includes
#include <algorithm>
#include <string>
#include <iostream>
#include <sstream>
#include <sys/stat.h>

#ifdef _DEBUG_
static int MYDEBUG = 0;
#else
static int MYDEBUG = 0;
#endif

using namespace std;
using SMESH::TPythonDump;

int SMESH_Mesh_i::myIdGenerator = 0;

//To disable automatic genericobj management, the following line should be commented.
//Otherwise, it should be uncommented. Refer to KERNEL_SRC/src/SALOMEDSImpl/SALOMEDSImpl_AttributeIOR.cxx
#define WITHGENERICOBJ

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

  // destroy groups
  map<int, SMESH::SMESH_GroupBase_ptr>::iterator itGr;
  for (itGr = _mapGroups.begin(); itGr != _mapGroups.end(); itGr++) {
    SMESH_GroupBase_i* aGroup = dynamic_cast<SMESH_GroupBase_i*>(SMESH_Gen_i::GetServant(itGr->second).in());
    if (aGroup) {
      // this method is called from destructor of group (PAL6331)
      //_impl->RemoveGroup( aGroup->GetLocalID() );
#ifdef WITHGENERICOBJ
      aGroup->Destroy();
#endif
    }
  }
  _mapGroups.clear();

#ifdef WITHGENERICOBJ
  // destroy submeshes
  map<int, SMESH::SMESH_subMesh_ptr>::iterator itSM;
  for ( itSM = _mapSubMeshIor.begin(); itSM != _mapSubMeshIor.end(); itSM++ ) {
    SMESH_subMesh_i* aSubMesh = dynamic_cast<SMESH_subMesh_i*>(SMESH_Gen_i::GetServant(itSM->second).in());
    if (aSubMesh) {
      aSubMesh->Destroy();
    }
  }
  _mapSubMeshIor.clear();

  // destroy hypotheses
  map<int, SMESH::SMESH_Hypothesis_ptr>::iterator itH;
  for ( itH = _mapHypo.begin(); itH != _mapHypo.end(); itH++ ) {
    SMESH_Hypothesis_i* aHypo = dynamic_cast<SMESH_Hypothesis_i*>(SMESH_Gen_i::GetServant(itH->second).in());
    if (aHypo) {
      aHypo->Destroy();
    }
  }
  _mapHypo.clear();
#endif

  delete _impl;
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
  // to track changes of GEOM groups
  addGeomGroupData( theShapeObject, _this() );
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

//================================================================================
/*!
 * \brief Remove all nodes and elements
 */
//================================================================================

void SMESH_Mesh_i::Clear() throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  try {
    _impl->Clear();
    CheckGeomGroupModif(); // issue 20145
  }
  catch(SALOME_Exception & S_ex) {
    THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
  }
  TPythonDump() <<  _this() << ".Clear()";
}

//================================================================================
/*!
 * \brief Remove all nodes and elements for indicated shape
 */
//================================================================================

void SMESH_Mesh_i::ClearSubMesh(CORBA::Long ShapeID)
  throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  try {
    _impl->ClearSubMesh( ShapeID );
  }
  catch(SALOME_Exception & S_ex) {
    THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
  }
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

  int major, minor, release;
  if( !MED::getMEDVersion( theFileName, major, minor, release ) )
    major = minor = release = -1;
  myFileInfo           = new SALOME_MED::MedFileInfo();
  myFileInfo->fileName = theFileName;
  myFileInfo->fileSize = 0;
#ifdef WIN32
  struct _stati64 d;
  if ( ::_stati64( theFileName, &d ) != -1 )
#else
  struct stat64 d;
  if ( ::stat64( theFileName, &d ) != -1 )
#endif
    myFileInfo->fileSize = d.st_size;
  myFileInfo->major    = major;
  myFileInfo->minor    = minor;
  myFileInfo->release  = release;

  return ConvertDriverMEDReadStatus(status);
}

//================================================================================
/*!
 * \brief Return string representation of a MED file version comprising nbDigits
 */
//================================================================================

char* SMESH_Mesh_i::GetVersionString(SMESH::MED_VERSION version, CORBA::Short nbDigits)
{
  string ver = DriverMED_W_SMESHDS_Mesh::GetVersionString(MED::EVersion(version),
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
  RETURNCASE( HYP_NEED_SHAPE    );
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
#ifdef WITHGENERICOBJ
      _mapHypo[hypId]->Register();
#endif
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

SMESH_Hypothesis::Hypothesis_Status
SMESH_Mesh_i::removeHypothesis(GEOM::GEOM_Object_ptr       aSubShapeObject,
                               SMESH::SMESH_Hypothesis_ptr anHyp)
{
  if(MYDEBUG) MESSAGE("removeHypothesis()");
  // **** proposer liste de subShape (selection multiple)

  if (CORBA::is_nil(aSubShapeObject) && HasShapeToMesh())
    THROW_SALOME_CORBA_EXCEPTION("bad subShape reference", SALOME::BAD_PARAM);

  SMESH::SMESH_Hypothesis_var myHyp = SMESH::SMESH_Hypothesis::_narrow(anHyp);
  if (CORBA::is_nil(myHyp))
    THROW_SALOME_CORBA_EXCEPTION("bad hypothesis reference", SALOME::BAD_PARAM);

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
//     if ( !SMESH_Hypothesis::IsStatusFatal(status) ) EAP: hyp can be used on many subshapes
//       _mapHypo.erase( hypId );
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
  if (_impl->HasShapeToMesh() && CORBA::is_nil(aSubShapeObject))
    THROW_SALOME_CORBA_EXCEPTION("bad subShape reference", SALOME::BAD_PARAM);

  SMESH::ListOfHypothesis_var aList = new SMESH::ListOfHypothesis();

  try {
    TopoDS_Shape myLocSubShape = _gen_i->GeomObjectToShape(aSubShapeObject);
    if ( myLocSubShape.IsNull() && !_impl->HasShapeToMesh() )
      myLocSubShape = _impl->GetShapeToMesh();
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

//       if ( aSubShapeObject->_is_nil() ) // not published shape (IPAL13617)
//         aSubShapeObject = theSubMesh->GetSubShape();

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
  if ( !aShape.IsNull() )
  {
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
      aStudy->NewBuilder()->RemoveObjectWithChildren( aGroupSO );
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
/*!
  \brief Union list of groups. New group is created. All mesh elements that are
   present in initial groups are added to the new one.
  \param theGroups list of groups
  \param theName name of group to be created
  \return pointer on the group
*/
//=============================================================================
SMESH::SMESH_Group_ptr SMESH_Mesh_i::UnionListOfGroups(const SMESH::ListOfGroups& theGroups,
                                                       const char*                theName )
throw (SALOME::SALOME_Exception)
{
  if ( !theName )
    return SMESH::SMESH_Group::_nil();

  try
  {
    NCollection_Map< int > anIds;
    SMESH::ElementType aType = SMESH::ALL;
    for ( int g = 0, n = theGroups.length(); g < n; g++ )
    {
      SMESH::SMESH_GroupBase_var aGrp = theGroups[ g ];
      if ( CORBA::is_nil( aGrp ) )
        continue;

      // check type
      SMESH::ElementType aCurrType = aGrp->GetType();
      if ( aType == SMESH::ALL )
        aType = aCurrType;
      else 
      {
        if ( aType != aCurrType )
          return SMESH::SMESH_Group::_nil();
      }

      // unite ids
      SMESH::long_array_var aCurrIds = aGrp->GetListOfID();
      for ( int i = 0, n = aCurrIds->length(); i < n; i++ )
      {
        int aCurrId = aCurrIds[ i ];
        anIds.Add( aCurrId );
      }
    }

    // Create group
    SMESH::SMESH_Group_var aResGrp = CreateGroup( aType, theName );
    if ( aResGrp->_is_nil() )
      return SMESH::SMESH_Group::_nil();
    
    // Create array of identifiers
    SMESH::long_array_var aResIds = new SMESH::long_array;
    aResIds->length( anIds.Extent() );
    
    NCollection_Map< int >::Iterator anIter( anIds );
    for ( int i = 0; anIter.More(); anIter.Next(), i++ )
    {
      aResIds[ i ] = anIter.Value();
    }
    aResGrp->Add( aResIds );

    // Clear python lines, created by CreateGroup() and Add()
    SALOMEDS::Study_ptr aStudy = _gen_i->GetCurrentStudy();
    _gen_i->RemoveLastFromPythonScript( aStudy->StudyId() );
    _gen_i->RemoveLastFromPythonScript( aStudy->StudyId() );

    // Update Python script
    
    TPythonDump() << aResGrp << " = " << _this() << ".UnionListOfGroups( "
                  << &theGroups << ", '" << theName << "' )";

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
/*!
  \brief Intersect list of groups. New group is created. All mesh elements that 
  are present in all initial groups simultaneously are added to the new one.
  \param theGroups list of groups
  \param theName name of group to be created
  \return pointer on the group
*/
//=============================================================================
SMESH::SMESH_Group_ptr SMESH_Mesh_i::IntersectListOfGroups( 
  const SMESH::ListOfGroups& theGroups, const char* theName )
throw (SALOME::SALOME_Exception)
{
  if ( !theName )
    return SMESH::SMESH_Group::_nil();

  try
  {
    NCollection_DataMap< int, int > anIdToCount;
    SMESH::ElementType aType = SMESH::ALL;
    for ( int g = 0, n = theGroups.length(); g < n; g++ )
    {
      SMESH::SMESH_GroupBase_var aGrp = theGroups[ g ];
      if ( CORBA::is_nil( aGrp ) )
        continue;

      // check type
      SMESH::ElementType aCurrType = aGrp->GetType();
      if ( aType == SMESH::ALL )
        aType = aCurrType;
      else 
      {
        if ( aType != aCurrType )
          return SMESH::SMESH_Group::_nil();
      }

      // calculates number of occurance ids in groups
      SMESH::long_array_var aCurrIds = aGrp->GetListOfID();
      for ( int i = 0, n = aCurrIds->length(); i < n; i++ )
      {
        int aCurrId = aCurrIds[ i ];
        if ( !anIdToCount.IsBound( aCurrId ) )
          anIdToCount.Bind( aCurrId, 1 );
        else 
          anIdToCount( aCurrId ) = anIdToCount( aCurrId ) + 1;
      }
    }
    
    // create map of ids
    int nbGrp = theGroups.length();
    NCollection_Map< int > anIds;
    NCollection_DataMap< int, int >::Iterator anIter( anIdToCount );
    for ( ; anIter.More(); anIter.Next() )
    {
      int aCurrId = anIter.Key();
      int aCurrNb = anIter.Value();
      if ( aCurrNb == nbGrp )
        anIds.Add( aCurrId );
    }

    // Create group
    SMESH::SMESH_Group_var aResGrp = CreateGroup( aType, theName );
    if ( aResGrp->_is_nil() )
      return SMESH::SMESH_Group::_nil();
    
    // Create array of identifiers
    SMESH::long_array_var aResIds = new SMESH::long_array;
    aResIds->length( anIds.Extent() );
    
    NCollection_Map< int >::Iterator aListIter( anIds );
    for ( int i = 0; aListIter.More(); aListIter.Next(), i++ )
    {
      aResIds[ i ] = aListIter.Value();
    }
    aResGrp->Add( aResIds );

    // Clear python lines, created by CreateGroup() and Add()
    SALOMEDS::Study_ptr aStudy = _gen_i->GetCurrentStudy();
    _gen_i->RemoveLastFromPythonScript( aStudy->StudyId() );
    _gen_i->RemoveLastFromPythonScript( aStudy->StudyId() );

    // Update Python script
    
    TPythonDump() << aResGrp << " = " << _this() << ".IntersectListOfGroups( "
                  << &theGroups << ", '" << theName << "' )";

    return aResGrp._retn();
  }
  catch( ... )
  {
    return SMESH::SMESH_Group::_nil();
  }
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

//=============================================================================
/*!
  \brief Cut lists of groups. New group is created. All mesh elements that are 
  present in main groups but do not present in tool groups are added to the new one
  \param theMainGroups list of main groups
  \param theToolGroups list of tool groups
  \param theName name of group to be created
  \return pointer on the group
*/
//=============================================================================
SMESH::SMESH_Group_ptr SMESH_Mesh_i::CutListOfGroups( 
  const SMESH::ListOfGroups& theMainGroups, 
  const SMESH::ListOfGroups& theToolGroups, 
  const char* theName )
  throw (SALOME::SALOME_Exception)
{
  if ( !theName )
    return SMESH::SMESH_Group::_nil();

  try
  {
    NCollection_Map< int > aToolIds;
    SMESH::ElementType aType = SMESH::ALL;
    int g, n;
    // iterate through tool groups
    for ( g = 0, n = theToolGroups.length(); g < n; g++ )
    {
      SMESH::SMESH_GroupBase_var aGrp = theToolGroups[ g ];
      if ( CORBA::is_nil( aGrp ) )
        continue;

      // check type
      SMESH::ElementType aCurrType = aGrp->GetType();
      if ( aType == SMESH::ALL )
        aType = aCurrType;
      else 
      {
        if ( aType != aCurrType )
          return SMESH::SMESH_Group::_nil();
      }

      // unite tool ids
      SMESH::long_array_var aCurrIds = aGrp->GetListOfID();
      for ( int i = 0, n = aCurrIds->length(); i < n; i++ )
      {
        int aCurrId = aCurrIds[ i ];
        aToolIds.Add( aCurrId );
      }
    }

    NCollection_Map< int > anIds; // result

    // Iterate through main group 
    for ( g = 0, n = theMainGroups.length(); g < n; g++ )
    {
      SMESH::SMESH_GroupBase_var aGrp = theMainGroups[ g ];
      if ( CORBA::is_nil( aGrp ) )
        continue;

      // check type
      SMESH::ElementType aCurrType = aGrp->GetType();
      if ( aType == SMESH::ALL )
        aType = aCurrType;
      else 
      {
        if ( aType != aCurrType )
          return SMESH::SMESH_Group::_nil();
      }

      // unite tool ids
      SMESH::long_array_var aCurrIds = aGrp->GetListOfID();
      for ( int i = 0, n = aCurrIds->length(); i < n; i++ )
      {
        int aCurrId = aCurrIds[ i ];
        if ( !aToolIds.Contains( aCurrId ) )
          anIds.Add( aCurrId );
      }
    }

    // Create group
    SMESH::SMESH_Group_var aResGrp = CreateGroup( aType, theName );
    if ( aResGrp->_is_nil() )
      return SMESH::SMESH_Group::_nil();
    
    // Create array of identifiers
    SMESH::long_array_var aResIds = new SMESH::long_array;
    aResIds->length( anIds.Extent() );
    
    NCollection_Map< int >::Iterator anIter( anIds );
    for ( int i = 0; anIter.More(); anIter.Next(), i++ )
    {
      aResIds[ i ] = anIter.Value();
    }
    aResGrp->Add( aResIds );

    // Clear python lines, created by CreateGroup() and Add()
    SALOMEDS::Study_ptr aStudy = _gen_i->GetCurrentStudy();
    _gen_i->RemoveLastFromPythonScript( aStudy->StudyId() );
    _gen_i->RemoveLastFromPythonScript( aStudy->StudyId() );

    // Update Python script

    TPythonDump() << aResGrp << " = " << _this() << ".CutListOfGroups( "
                  << &theMainGroups << ", " << &theToolGroups << ", '"
                  << theName << "' )";
    
    return aResGrp._retn();
  }
  catch( ... )
  {
    return SMESH::SMESH_Group::_nil();
  }
}

//=============================================================================
/*!
  \brief Create groups of entities from existing groups of superior dimensions 
  System 
  1) extract all nodes from each group,
  2) combine all elements of specified dimension laying on these nodes.
  \param theGroups list of source groups 
  \param theElemType dimension of elements 
  \param theName name of new group
  \return pointer on new group
*/
//=============================================================================
SMESH::SMESH_Group_ptr SMESH_Mesh_i::CreateDimGroup( 
  const SMESH::ListOfGroups& theGroups, 
  SMESH::ElementType         theElemType, 
  const char*                theName )
  throw (SALOME::SALOME_Exception)
{
  SMESHDS_Mesh* aMeshDS = _impl->GetMeshDS();

  if ( !theName || !aMeshDS )
    return SMESH::SMESH_Group::_nil();

  SMDSAbs_ElementType anElemType = (SMDSAbs_ElementType)theElemType;

  try
  {
    // Create map of nodes from all groups 

    NCollection_Map< int > aNodeMap;
    
    for ( int g = 0, n = theGroups.length(); g < n; g++ )
    {
      SMESH::SMESH_GroupBase_var aGrp = theGroups[ g ];
      if ( CORBA::is_nil( aGrp ) )
        continue;

      SMESH::ElementType aType = aGrp->GetType();
      if ( aType == SMESH::ALL )
        continue;
      else if ( aType == SMESH::NODE )
      {
        SMESH::long_array_var aCurrIds = aGrp->GetListOfID();
        for ( int i = 0, n = aCurrIds->length(); i < n; i++ )
        {
          int aCurrId = aCurrIds[ i ];
          const SMDS_MeshNode* aNode = aMeshDS->FindNode( aCurrId );
          if ( aNode )
            aNodeMap.Add( aNode->GetID() );
        }
      }
      else 
      {
        SMESH::long_array_var aCurrIds = aGrp->GetListOfID();
        for ( int i = 0, n = aCurrIds->length(); i < n; i++ )
        {
          int aCurrId = aCurrIds[ i ];
          const SMDS_MeshElement* anElem = aMeshDS->FindElement( aCurrId );
          if ( !anElem )
            continue;
          SMDS_ElemIteratorPtr aNodeIter = anElem->nodesIterator();
          while( aNodeIter->more() )
          {
            const SMDS_MeshNode* aNode = 
              dynamic_cast<const SMDS_MeshNode*>( aNodeIter->next() );
            if ( aNode )
              aNodeMap.Add( aNode->GetID() );
          }
        }
      }
    }

    // Get result identifiers 

    NCollection_Map< int > aResultIds;
    if ( theElemType == SMESH::NODE )
    {
      NCollection_Map< int >::Iterator aNodeIter( aNodeMap );
      for ( ; aNodeIter.More(); aNodeIter.Next() )
        aResultIds.Add( aNodeIter.Value() );
    }
    else
    {
      // Create list of elements of given dimension constructed on the nodes
      NCollection_Map< int > anElemList;
      NCollection_Map< int >::Iterator aNodeIter( aNodeMap );
      for ( ; aNodeIter.More(); aNodeIter.Next() )
      {
        const SMDS_MeshElement* aNode = 
          dynamic_cast<const SMDS_MeshElement*>( aMeshDS->FindNode( aNodeIter.Value() ) );
        if ( !aNode )
          continue;

         SMDS_ElemIteratorPtr anElemIter = aNode->elementsIterator( anElemType );
        while( anElemIter->more() )
        {
          const SMDS_MeshElement* anElem = 
            dynamic_cast<const SMDS_MeshElement*>( anElemIter->next() );
          if ( anElem && anElem->GetType() == anElemType )
            anElemList.Add( anElem->GetID() );
        }
      }

      // check whether all nodes of elements are present in nodes map
      NCollection_Map< int >::Iterator anIter( anElemList );
      for ( ; anIter.More(); anIter.Next() )
      {
        const SMDS_MeshElement* anElem = aMeshDS->FindElement( anIter.Value() );
        if ( !anElem )
          continue;

        bool isOk = true;
        SMDS_ElemIteratorPtr aNodeIter = anElem->nodesIterator();
        while( aNodeIter->more() )
        {
          const SMDS_MeshNode* aNode = 
            dynamic_cast<const SMDS_MeshNode*>( aNodeIter->next() );
          if ( !aNode || !aNodeMap.Contains( aNode->GetID() ) )
          {
            isOk = false;
            break;
          }
        } 
        if ( isOk )
          aResultIds.Add( anElem->GetID() );
      }
    }

    // Create group

    SMESH::SMESH_Group_var aResGrp = CreateGroup( theElemType, theName );
    if ( aResGrp->_is_nil() )
      return SMESH::SMESH_Group::_nil();
    
    // Create array of identifiers
    SMESH::long_array_var aResIds = new SMESH::long_array;
    aResIds->length( aResultIds.Extent() );
    
    NCollection_Map< int >::Iterator aResIter( aResultIds );
    for ( int i = 0; aResIter.More(); aResIter.Next(), i++ )
      aResIds[ i ] = aResIter.Value();
    aResGrp->Add( aResIds );

    // Remove strings corresponding to group creation
    SALOMEDS::Study_ptr aStudy = _gen_i->GetCurrentStudy();
    _gen_i->RemoveLastFromPythonScript( aStudy->StudyId() );
    _gen_i->RemoveLastFromPythonScript( aStudy->StudyId() );

    // Update Python script
    
    TPythonDump() << aResGrp << " = " << _this() << ".CreateDimGroup( "
                  << &theGroups << ", " << theElemType << ", '" << theName << "' )";

    return aResGrp._retn();
  }
  catch( ... )
  {
    return SMESH::SMESH_Group::_nil();
  }
}

//================================================================================
/*!
 * \brief Remember GEOM group data
 */
//================================================================================

void SMESH_Mesh_i::addGeomGroupData(GEOM::GEOM_Object_ptr theGeomObj,
                                    CORBA::Object_ptr     theSmeshObj)
{
  if ( CORBA::is_nil( theGeomObj ) || theGeomObj->GetType() != GEOM_GROUP )
    return;
  // group SO
  SALOMEDS::Study_var   study  = _gen_i->GetCurrentStudy();
  SALOMEDS::SObject_var groupSO = _gen_i->ObjectToSObject( study, theGeomObj );
  if ( groupSO->_is_nil() )
    return;
  // group indices
  GEOM::GEOM_Gen_var geomGen = _gen_i->GetGeomEngine();
  GEOM::GEOM_IGroupOperations_var groupOp =
    geomGen->GetIGroupOperations( _gen_i->GetCurrentStudyID() );
  GEOM::ListOfLong_var ids = groupOp->GetObjects( theGeomObj );

  // store data
  _geomGroupData.push_back( TGeomGroupData() );
  TGeomGroupData & groupData = _geomGroupData.back();
  // entry
  CORBA::String_var entry = groupSO->GetID();
  groupData._groupEntry = entry.in();
  // indices
  for ( int i = 0; i < ids->length(); ++i )
    groupData._indices.insert( ids[i] );
  // SMESH object
  groupData._smeshObject = theSmeshObj;
}

//================================================================================
/*!
 * Remove GEOM group data relating to removed smesh object
 */
//================================================================================

void SMESH_Mesh_i::removeGeomGroupData(CORBA::Object_ptr theSmeshObj)
{
  list<TGeomGroupData>::iterator
    data = _geomGroupData.begin(), dataEnd = _geomGroupData.end();
  for ( ; data != dataEnd; ++data ) {
    if ( theSmeshObj->_is_equivalent( data->_smeshObject )) {
      _geomGroupData.erase( data );
      return;
    }
  }
}

//================================================================================
/*!
 * \brief Return new group contents if it has been changed and update group data
 */
//================================================================================

TopoDS_Shape SMESH_Mesh_i::newGroupShape( TGeomGroupData & groupData)
{
  TopoDS_Shape newShape;

  // get geom group
  SALOMEDS::Study_var study = _gen_i->GetCurrentStudy();
  if ( study->_is_nil() ) return newShape; // means "not changed"
  SALOMEDS::SObject_var groupSO = study->FindObjectID( groupData._groupEntry.c_str() );
  if ( !groupSO->_is_nil() )
  {
    CORBA::Object_var groupObj = _gen_i->SObjectToObject( groupSO );
    if ( CORBA::is_nil( groupObj )) return newShape;
    GEOM::GEOM_Object_var geomGroup = GEOM::GEOM_Object::_narrow( groupObj );

    // get indices of group items
    set<int> curIndices;
    GEOM::GEOM_Gen_var geomGen = _gen_i->GetGeomEngine();
    GEOM::GEOM_IGroupOperations_var groupOp =
      geomGen->GetIGroupOperations( _gen_i->GetCurrentStudyID() );
    GEOM::ListOfLong_var ids = groupOp->GetObjects( geomGroup );
    for ( int i = 0; i < ids->length(); ++i )
      curIndices.insert( ids[i] );

    if ( groupData._indices == curIndices )
      return newShape; // group not changed

    // update data
    groupData._indices = curIndices;

    GEOM_Client* geomClient = _gen_i->GetShapeReader();
    if ( !geomClient ) return newShape;
    TCollection_AsciiString groupIOR = geomGen->GetStringFromIOR( geomGroup );
    geomClient->RemoveShapeFromBuffer( groupIOR );
    newShape = _gen_i->GeomObjectToShape( geomGroup );
  }    

  if ( newShape.IsNull() ) {
    // geom group becomes empty - return empty compound
    TopoDS_Compound compound;
    BRep_Builder().MakeCompound(compound);
    newShape = compound;
  }
  return newShape;
}

namespace {
  //=============================================================================
  /*!
   * \brief Storage of shape and index used in CheckGeomGroupModif()
   */
  //=============================================================================
  struct TIndexedShape {
    int          _index;
    TopoDS_Shape _shape;
    TIndexedShape( int i, const TopoDS_Shape& s ):_index(i), _shape(s) {}
  };
}
//=============================================================================
/*!
 * \brief Update objects depending on changed geom groups
 * 
 * NPAL16168: geometrical group edition from a submesh don't modifiy mesh computation
 * issue 0020210: Update of a smesh group after modification of the associated geom group
 */
//=============================================================================

void SMESH_Mesh_i::CheckGeomGroupModif()
{
  if ( !_impl->HasShapeToMesh() ) return;

  SALOMEDS::Study_var study = _gen_i->GetCurrentStudy();
  if ( study->_is_nil() ) return;

  CORBA::Long nbEntities = NbNodes() + NbElements();

  // Check if group contents changed

  typedef map< string, TopoDS_Shape > TEntry2Geom;
  TEntry2Geom newGroupContents;

  list<TGeomGroupData>::iterator
    data = _geomGroupData.begin(), dataEnd = _geomGroupData.end();
  for ( ; data != dataEnd; ++data )
  {
    pair< TEntry2Geom::iterator, bool > it_new =
      newGroupContents.insert( make_pair( data->_groupEntry, TopoDS_Shape() ));
    bool processedGroup    = !it_new.second;
    TopoDS_Shape& newShape = it_new.first->second;
    if ( !processedGroup )
      newShape = newGroupShape( *data );
    if ( newShape.IsNull() )
      continue; // no changes

    if ( processedGroup ) { // update group indices
      list<TGeomGroupData>::iterator data2 = data;
      for ( --data2; data2->_groupEntry != data->_groupEntry; --data2) {}
      data->_indices = data2->_indices;
    }

    // Update SMESH objects according to new GEOM group contents

    SMESH::SMESH_subMesh_var submesh = SMESH::SMESH_subMesh::_narrow( data->_smeshObject );
    if ( !submesh->_is_nil() ) // -------------- Sub mesh ---------------------
    {
      int oldID = submesh->GetId();
      if ( _mapSubMeshIor.find( oldID ) == _mapSubMeshIor.end() )
        continue;
      TopoDS_Shape oldShape = _mapSubMesh[oldID]->GetSubShape();

      // update hypotheses
      list <const SMESHDS_Hypothesis * > hyps = _impl->GetHypothesisList(oldShape);
      list <const SMESHDS_Hypothesis * >::iterator hypIt;
      for ( hypIt = hyps.begin(); hypIt != hyps.end(); ++hypIt )
      {
        _impl->RemoveHypothesis( oldShape, (*hypIt)->GetID());
        _impl->AddHypothesis   ( newShape, (*hypIt)->GetID());
      }
      // care of submeshes
      SMESH_subMesh* newSubmesh = _impl->GetSubMesh( newShape );
      int newID = newSubmesh->GetId();
      if ( newID != oldID ) {
        _mapSubMesh   [ newID ] = newSubmesh;
        _mapSubMesh_i [ newID ] = _mapSubMesh_i [ oldID ];
        _mapSubMeshIor[ newID ] = _mapSubMeshIor[ oldID ];
        _mapSubMesh.   erase(oldID);
        _mapSubMesh_i. erase(oldID);
        _mapSubMeshIor.erase(oldID);
        _mapSubMesh_i [ newID ]->changeLocalId( newID );
      }
      continue;
    }

    SMESH::SMESH_GroupOnGeom_var smeshGroup =
      SMESH::SMESH_GroupOnGeom::_narrow( data->_smeshObject );
    if ( !smeshGroup->_is_nil() ) // ------------ GROUP -----------------------
    {
      SMESH_GroupOnGeom_i* group_i = SMESH::DownCast<SMESH_GroupOnGeom_i*>( smeshGroup );
      if ( group_i ) {
        ::SMESH_Group* group = _impl->GetGroup( group_i->GetLocalID() );
        SMESHDS_GroupOnGeom* ds = static_cast<SMESHDS_GroupOnGeom*>( group->GetGroupDS() );
        ds->SetShape( newShape );
      }
      continue;
    }

    SMESH::SMESH_Mesh_var mesh = SMESH::SMESH_Mesh::_narrow( data->_smeshObject );
    if ( !mesh->_is_nil() ) // -------------- MESH ----------------------------
    {
      // Remove groups and submeshes basing on removed sub-shapes

      TopTools_MapOfShape newShapeMap;
      TopoDS_Iterator shapeIt( newShape );
      for ( ; shapeIt.More(); shapeIt.Next() )
        newShapeMap.Add( shapeIt.Value() );

      SMESHDS_Mesh* meshDS = _impl->GetMeshDS();
      for ( shapeIt.Initialize( meshDS->ShapeToMesh() ); shapeIt.More(); shapeIt.Next() )
      {
        if ( newShapeMap.Contains( shapeIt.Value() ))
          continue;
        TopTools_IndexedMapOfShape oldShapeMap;
        TopExp::MapShapes( shapeIt.Value(), oldShapeMap );
        for ( int i = 1; i <= oldShapeMap.Extent(); ++i )
        {
          const TopoDS_Shape& oldShape = oldShapeMap(i);
          int oldInd = meshDS->ShapeToIndex( oldShape );
          // -- submeshes --
          map<int, SMESH::SMESH_subMesh_ptr>::iterator i_smIor = _mapSubMeshIor.find( oldInd );
          if ( i_smIor != _mapSubMeshIor.end() ) {
            RemoveSubMesh( i_smIor->second ); // one submesh per shape index
          }
          // --- groups ---
          map<int, SMESH::SMESH_GroupBase_ptr>::iterator i_grp = _mapGroups.begin();
          for ( ; i_grp != _mapGroups.end(); ++i_grp )
          {
            // check if a group bases on oldInd shape
            SMESHDS_GroupOnGeom* grpOnGeom = 0;
            if ( ::SMESH_Group* g = _impl->GetGroup( i_grp->first ))
              grpOnGeom = dynamic_cast<SMESHDS_GroupOnGeom*>( g->GetGroupDS() );
            if ( grpOnGeom && oldShape.IsSame( grpOnGeom->GetShape() ))
            { // remove
              RemoveGroup( i_grp->second ); // several groups can base on same shape
              i_grp = _mapGroups.begin(); // _mapGroups changed - restart iteration
            }
          }
        }
      }
      // Reassign hypotheses and update groups after setting the new shape to mesh

      // collect anassigned hypotheses
      typedef list< pair< TIndexedShape, list<const SMESHDS_Hypothesis*> > > TShapeHypList;
      list <const SMESHDS_Hypothesis * >::const_iterator hypIt;
      TShapeHypList assignedHyps;
      for ( int i = 1; i <= meshDS->MaxShapeIndex(); ++i )
      {
        const TopoDS_Shape& oldShape = meshDS->IndexToShape(i);
        list<const SMESHDS_Hypothesis*> hyps = meshDS->GetHypothesis( oldShape );// copy
        if ( !hyps.empty() ) {
          assignedHyps.push_back( make_pair( TIndexedShape(i,oldShape), hyps ));
          for ( hypIt = hyps.begin(); hypIt != hyps.end(); ++hypIt )
            _impl->RemoveHypothesis( oldShape, (*hypIt)->GetID());
        }
      }
      // collect shapes supporting groups
      typedef list < pair< TIndexedShape, SMDSAbs_ElementType > > TShapeTypeList;
      TShapeTypeList groupData;
      const set<SMESHDS_GroupBase*>& groups = meshDS->GetGroups();
      set<SMESHDS_GroupBase*>::const_iterator grIt = groups.begin();
      for ( ; grIt != groups.end(); ++grIt )
      {
        if ( SMESHDS_GroupOnGeom* gog = dynamic_cast<SMESHDS_GroupOnGeom*>( *grIt ))
          groupData.push_back
            ( make_pair( TIndexedShape( gog->GetID(),gog->GetShape()), gog->GetType()));
      }
      // set new shape to mesh -> DS of submeshes and geom groups is deleted
      _impl->ShapeToMesh( newShape );
      
      // reassign hypotheses
      TShapeHypList::iterator indS_hyps = assignedHyps.begin();
      for ( ; indS_hyps != assignedHyps.end(); ++indS_hyps )
      {
        TIndexedShape&                   geom = indS_hyps->first;
        list<const SMESHDS_Hypothesis*>& hyps = indS_hyps->second;
        int oldID = geom._index;
        int newID = meshDS->ShapeToIndex( geom._shape );
        if ( !newID )
          continue;
        if ( oldID == 1 ) { // main shape
          newID = 1;
          geom._shape = newShape;
        }
        for ( hypIt = hyps.begin(); hypIt != hyps.end(); ++hypIt )
          _impl->AddHypothesis( geom._shape, (*hypIt)->GetID());
        // care of submeshes
        SMESH_subMesh* newSubmesh = _impl->GetSubMesh( geom._shape );
        if ( newID != oldID ) {
          _mapSubMesh   [ newID ] = newSubmesh;
          _mapSubMesh_i [ newID ] = _mapSubMesh_i [ oldID ];
          _mapSubMeshIor[ newID ] = _mapSubMeshIor[ oldID ];
          _mapSubMesh.   erase(oldID);
          _mapSubMesh_i. erase(oldID);
          _mapSubMeshIor.erase(oldID);
          _mapSubMesh_i [ newID ]->changeLocalId( newID );
        }
      }
      // recreate groups
      TShapeTypeList::iterator geomType = groupData.begin();
      for ( ; geomType != groupData.end(); ++geomType )
      {
        const TIndexedShape& geom = geomType->first;
        int oldID = geom._index;
        if ( _mapGroups.find( oldID ) == _mapGroups.end() )
          continue;
        // get group name
        SALOMEDS::SObject_var groupSO = _gen_i->ObjectToSObject( study,_mapGroups[oldID] );
        CORBA::String_var     name    = groupSO->GetName();
        // update
        SMESH_GroupBase_i* group_i    = SMESH::DownCast<SMESH_GroupBase_i*>(_mapGroups[oldID] );
        int newID;
        if ( group_i && _impl->AddGroup( geomType->second, name.in(), newID, geom._shape ))
          group_i->changeLocalId( newID );
      }

      break; // everything has been updated

    } // update mesh
  } // loop on group data

  // Update icons

  CORBA::Long newNbEntities = NbNodes() + NbElements();
  list< SALOMEDS::SObject_var > soToUpdateIcons;
  if ( newNbEntities != nbEntities )
  {
    // Add all SObjects with icons
    soToUpdateIcons.push_back( _gen_i->ObjectToSObject( study, _this() )); // mesh

    for (map<int, SMESH::SMESH_subMesh_ptr>::iterator i_sm = _mapSubMeshIor.begin();
         i_sm != _mapSubMeshIor.end(); ++i_sm ) // submeshes
      soToUpdateIcons.push_back( _gen_i->ObjectToSObject( study, i_sm->second ));

    for ( map<int, SMESH::SMESH_GroupBase_ptr>::iterator i_gr = _mapGroups.begin();
          i_gr != _mapGroups.end(); ++i_gr ) // groups
      soToUpdateIcons.push_back( _gen_i->ObjectToSObject( study, i_gr->second ));
  }

  list< SALOMEDS::SObject_var >::iterator so = soToUpdateIcons.begin();
  for ( ; so != soToUpdateIcons.end(); ++so )
    _gen_i->SetPixMap( *so, "ICON_SMESH_TREE_MESH_WARN" );
}

//=============================================================================
/*!
 * \brief Create standalone group instead if group on geometry
 */
//=============================================================================

SMESH::SMESH_Group_ptr SMESH_Mesh_i::ConvertToStandalone( SMESH::SMESH_GroupOnGeom_ptr theGroup )
{
  SMESH::SMESH_Group_var aGroup;
  if ( theGroup->_is_nil() )
    return aGroup._retn();

  Unexpect aCatch(SALOME_SalomeException);

  SMESH_GroupBase_i* aGroupToRem =
    dynamic_cast<SMESH_GroupBase_i*>( SMESH_Gen_i::GetServant( theGroup ).in() );
  if ( !aGroupToRem )
    return aGroup._retn();

  int anId = aGroupToRem->GetLocalID();
  if ( !_impl->ConvertToStandalone( anId ) )
    return aGroup._retn();
  removeGeomGroupData( theGroup );

  SMESH_GroupBase_i* aGroupImpl = new SMESH_Group_i( SMESH_Gen_i::GetPOA(), this, anId );

  // remove old instance of group from own map
  _mapGroups.erase( anId );

  SALOMEDS::StudyBuilder_var builder;
  SALOMEDS::SObject_var aGroupSO;
  SALOMEDS::Study_ptr aStudy = _gen_i->GetCurrentStudy();
  if ( !aStudy->_is_nil() )  {
    builder = aStudy->NewBuilder();
    aGroupSO = _gen_i->ObjectToSObject( aStudy, theGroup );
    if ( !aGroupSO->_is_nil() ) {

    // remove reference to geometry
    SALOMEDS::ChildIterator_var chItr = aStudy->NewChildIterator(aGroupSO);
    for ( ; chItr->More(); chItr->Next() )
      // Remove group's child SObject
      builder->RemoveObject( chItr->Value() );

      // Update Python script
      TPythonDump() << aGroupSO << " = " << _this() << ".ConvertToStandalone( "
                    << aGroupSO << " )";
    }
  }

  // PAL7962: san -- To ensure correct mapping of servant and correct reference counting in GenericObj_i
  SMESH_Gen_i::GetPOA()->activate_object( aGroupImpl );
  aGroupImpl->Register();
  // PAL7962: san -- To ensure correct mapping of servant and correct reference counting in GenericObj_i

  // remember new group in own map
  aGroup = SMESH::SMESH_Group::_narrow( aGroupImpl->_this() );
  _mapGroups[anId] = SMESH::SMESH_GroupBase::_duplicate( aGroup );

  // register CORBA object for persistence
  /*int nextId =*/ _gen_i->RegisterObject( aGroup );

  builder->SetIOR( aGroupSO, _gen_i->GetORB()->object_to_string( aGroup ) );

  return aGroup._retn();
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

  // to track changes of GEOM groups
  addGeomGroupData( theSubShapeObject, subMesh );

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
  if ( theSubMesh->_is_nil() /*|| theSubShapeObject->_is_nil()*/ )
    return;

  if ( theSubShapeObject->_is_nil() )  // not published shape (IPAL13617)
  {
    CORBA::Long shapeId = theSubMesh->GetId();
    if ( _mapSubMesh.find( shapeId ) != _mapSubMesh.end())
    {
      TopoDS_Shape S = _mapSubMesh[ shapeId ]->GetSubShape();
      if ( !S.IsNull() )
      {
        list<const SMESHDS_Hypothesis*> hyps = _impl->GetHypothesisList( S );
        list<const SMESHDS_Hypothesis*>::const_iterator hyp = hyps.begin();
        for ( ; hyp != hyps.end(); ++hyp )
          _impl->RemoveHypothesis(S, (*hyp)->GetID());
      }
    }
  }
  else
  {
    try {
      SMESH::ListOfHypothesis_var aHypList = GetHypothesisList( theSubShapeObject );
      for ( int i = 0, n = aHypList->length(); i < n; i++ ) {
        removeHypothesis( theSubShapeObject, aHypList[i] );
      }
    }
    catch( const SALOME::SALOME_Exception& ) {
      INFOS("SMESH_Mesh_i::removeSubMesh(): exception caught!");
    }
    removeGeomGroupData( theSubShapeObject );
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

    // to track changes of GEOM groups
    if ( !theShape.IsNull() ) {
      GEOM::GEOM_Object_var geom = _gen_i->ShapeToGeomObject( theShape );
      addGeomGroupData( geom, aGroup );
    }
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
    removeGeomGroupData( _mapGroups[theId] );
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
  _impl->ClearLog();
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

//================================================================================
/*!
 * \brief Return true if the mesh has been edited since a last total re-compute
 *        and those modifications may prevent successful partial re-compute
 */
//================================================================================

CORBA::Boolean SMESH_Mesh_i::HasModificationsToDiscard() throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  return _impl->HasModificationsToDiscard();
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

void SMESH_Mesh_i::PrepareForWriting (const char* file, bool overwrite)
{
  TCollection_AsciiString aFullName ((char*)file);
  OSD_Path aPath (aFullName);
  OSD_File aFile (aPath);
  if (aFile.Exists()) {
    // existing filesystem node
    if (aFile.KindOfFile() == OSD_FILE) {
      if (aFile.IsWriteable()) {
        if (overwrite) {
          aFile.Reset();
          aFile.Remove();
        }
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

void SMESH_Mesh_i::ExportToMEDX (const char* file,
                                 CORBA::Boolean auto_groups,
                                 SMESH::MED_VERSION theVersion,
                                 CORBA::Boolean overwrite)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);

  // Perform Export
  PrepareForWriting(file, overwrite);
  const char* aMeshName = "Mesh";
  SALOMEDS::Study_ptr aStudy = _gen_i->GetCurrentStudy();
  if ( !aStudy->_is_nil() ) {
    SALOMEDS::SObject_var aMeshSO = _gen_i->ObjectToSObject( aStudy, _this() );
    if ( !aMeshSO->_is_nil() ) {
      aMeshName = aMeshSO->GetName();
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
  // Update Python script
  // set name of mesh before export
  TPythonDump() << _gen_i << ".SetName(" << _this() << ", '" << aMeshName << "')";
  
  // check names of groups
  checkGroupNames();

  TPythonDump() << _this() << ".ExportToMEDX( '"
                << file << "', " << auto_groups << ", " << theVersion << ", " << overwrite << " )";

  _impl->ExportMED( file, aMeshName, auto_groups, theVersion );
}

void SMESH_Mesh_i::ExportToMED (const char* file,
                                CORBA::Boolean auto_groups,
                                SMESH::MED_VERSION theVersion)
  throw(SALOME::SALOME_Exception)
{
  ExportToMEDX(file,auto_groups,theVersion,true);
}

void SMESH_Mesh_i::ExportMED (const char* file,
                              CORBA::Boolean auto_groups)
  throw(SALOME::SALOME_Exception)
{
  ExportToMEDX(file,auto_groups,SMESH::MED_V2_1,true);
}

void SMESH_Mesh_i::ExportDAT (const char *file)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);

  // Update Python script
  // check names of groups
  checkGroupNames();
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
  // check names of groups
  checkGroupNames();
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
  // check names of groups
  checkGroupNames();
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
  return Nb0DElements() + NbEdges() + NbFaces() + NbVolumes();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
CORBA::Long SMESH_Mesh_i::Nb0DElements()throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  return _impl->Nb0DElements();
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
  return _mapSubMesh_i.size();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
char* SMESH_Mesh_i::Dump()
{
  ostringstream os;
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
 *
 */
//=============================================================================

SMESH::EntityType SMESH_Mesh_i::GetElementGeomType( const CORBA::Long id )
  throw (SALOME::SALOME_Exception)
{
  const SMDS_MeshElement* e = _impl->GetMeshDS()->FindElement(id);
  if ( !e )
    THROW_SALOME_CORBA_EXCEPTION( "invalid element id", SALOME::BAD_PARAM );

  return ( SMESH::EntityType ) e->GetEntityType();
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
  if ( MYDEBUG )
    MESSAGE("CORBA::LongLong SMESH_Mesh_i::GetMeshPtr() "<<pointeur);
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

//=======================================================================
//function : GetElemFaceNodes
//purpose  : Returns nodes of given face (counted from zero) for given element.
//=======================================================================

SMESH::long_array* SMESH_Mesh_i::GetElemFaceNodes(CORBA::Long  elemId,
                                                  CORBA::Short faceIndex)
{
  SMESH::long_array_var aResult = new SMESH::long_array();
  if ( SMESHDS_Mesh* aSMESHDS_Mesh = _impl->GetMeshDS() )
  {
    if ( const SMDS_MeshElement* elem = aSMESHDS_Mesh->FindElement(elemId) )
    {
      SMDS_VolumeTool vtool( elem );
      if ( faceIndex < vtool.NbFaces() )
      {
        aResult->length( vtool.NbFaceNodes( faceIndex ));
        const SMDS_MeshNode** nn = vtool.GetFaceNodes( faceIndex );
        for ( int i = 0; i < aResult->length(); ++i )
          aResult[ i ] = nn[ i ]->GetID();
      }
    }
  }
  return aResult._retn();
}

//=======================================================================
//function : FindElementByNodes
//purpose  : Returns an element based on all given nodes.
//=======================================================================

CORBA::Long SMESH_Mesh_i::FindElementByNodes(const SMESH::long_array& nodes)
{
  CORBA::Long elemID(0);
  if ( SMESHDS_Mesh* mesh = _impl->GetMeshDS() )
  {
    vector< const SMDS_MeshNode * > nn( nodes.length() );
    for ( int i = 0; i < nodes.length(); ++i )
      if ( !( nn[i] = mesh->FindNode( nodes[i] )))
        return elemID;

    const SMDS_MeshElement* elem = mesh->FindElement( nn );
    if ( !elem && ( _impl->NbEdges( ORDER_QUADRATIC ) ||
                    _impl->NbFaces( ORDER_QUADRATIC ) ||
                    _impl->NbVolumes( ORDER_QUADRATIC )))
      elem = mesh->FindElement( nn, SMDSAbs_All, /*noMedium=*/true );

    if ( elem ) elemID = CORBA::Long( elem->GetID() );
  }
  return elemID;
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

//=============================================================================
/*!
 * \brief Return information about imported file
 */
//=============================================================================

SALOME_MED::MedFileInfo* SMESH_Mesh_i::GetMEDFileInfo()
{
  SALOME_MED::MedFileInfo_var res( myFileInfo );
  if ( !res.operator->() ) {
    res = new SALOME_MED::MedFileInfo;
    res->fileName = "";
    res->fileSize = res->major = res->minor = res->release = -1;
  }
  return res._retn();
}

//=============================================================================
/*!
 * \brief Check and correct names of mesh groups
 */
//=============================================================================

void SMESH_Mesh_i::checkGroupNames()
{
  int nbGrp = NbGroups();
  if ( !nbGrp )
    return;

  SALOMEDS::Study_ptr aStudy = _gen_i->GetCurrentStudy();
  if ( aStudy->_is_nil() )
    return; // nothing to do
  
  SMESH::ListOfGroups* grpList = 0;
  // avoid dump of "GetGroups"
  {
    // store python dump into a local variable inside local scope
    SMESH::TPythonDump pDump; // do not delete this line of code
    grpList = GetGroups();
  }

  for ( int gIndx = 0; gIndx < nbGrp; gIndx++ ) {
    SMESH::SMESH_GroupBase_ptr aGrp = (*grpList)[ gIndx ];
    if ( !aGrp )
      continue;
    SALOMEDS::SObject_var aGrpSO = _gen_i->ObjectToSObject( aStudy, aGrp );
    if ( aGrpSO->_is_nil() )
      continue;
    // correct name of the mesh group if necessary
    const char* guiName = aGrpSO->GetName();
    if ( strcmp(guiName, aGrp->GetName()) )
      aGrp->SetName( guiName );
  }
}

//=============================================================================
/*!
 * \brief Sets list of notebook variables used for Mesh operations separated by ":" symbol
 */
//=============================================================================
void SMESH_Mesh_i::SetParameters(const char* theParameters)
{
  SMESH_Gen_i::GetSMESHGen()->UpdateParameters(SMESH::SMESH_Mesh::_narrow(_this()),
                                               CORBA::string_dup(theParameters));
}

//=============================================================================
/*!
 * \brief Returns list of notebook variables used for Mesh operations separated by ":" symbol
 */
//=============================================================================
char* SMESH_Mesh_i::GetParameters()
{
  SMESH_Gen_i *gen = SMESH_Gen_i::GetSMESHGen();
  return CORBA::string_dup(gen->GetParameters(SMESH::SMESH_Mesh::_narrow(_this())));
}

//=============================================================================
/*!
 * \brief Returns list of notebook variables used for last Mesh operation
 */
//=============================================================================
SMESH::string_array* SMESH_Mesh_i::GetLastParameters()
{
  SMESH::string_array_var aResult = new SMESH::string_array();
  SMESH_Gen_i *gen = SMESH_Gen_i::GetSMESHGen();
  if(gen) {
    char *aParameters = GetParameters();
    SALOMEDS::Study_ptr aStudy = gen->GetCurrentStudy();
    if(!aStudy->_is_nil()) {
      SALOMEDS::ListOfListOfStrings_var aSections = aStudy->ParseVariables(aParameters); 
      if(aSections->length() > 0) {
        SALOMEDS::ListOfStrings aVars = aSections[aSections->length()-1];
        aResult->length(aVars.length());
        for(int i = 0;i < aVars.length();i++)
          aResult[i] = CORBA::string_dup( aVars[i]);
      }
    }
  }
  return aResult._retn();
}

//=============================================================================
/*!
 * \brief Returns statistic of mesh elements
 */
//=============================================================================
SMESH::long_array* SMESH_Mesh_i::GetMeshInfo()
{
  SMESH::long_array_var aRes = new SMESH::long_array();
  aRes->length(SMESH::Entity_Last);
  for (int i = SMESH::Entity_Node; i < SMESH::Entity_Last; i++)
    aRes[i] = 0;
  SMESHDS_Mesh* aMeshDS = _impl->GetMeshDS();
  if (!aMeshDS)
    return aRes._retn();
  const SMDS_MeshInfo& aMeshInfo = aMeshDS->GetMeshInfo();
  for (int i = SMESH::Entity_Node; i < SMESH::Entity_Last; i++)
    aRes[i] = aMeshInfo.NbEntities((SMDSAbs_EntityType)i);
  return aRes._retn();
}

//=============================================================================
/*!
 * \brief Collect statistic of mesh elements given by iterator
 */
//=============================================================================
void SMESH_Mesh_i::CollectMeshInfo(const SMDS_ElemIteratorPtr theItr,
                                   SMESH::long_array&         theInfo)
{
  if (!theItr) return;
  while (theItr->more())
    theInfo[ theItr->next()->GetEntityType() ]++;
}

//=============================================================================
/*!
 * \brief mapping of mesh dimension into shape type
 */
//=============================================================================
static TopAbs_ShapeEnum shapeTypeByDim(const int theDim)
{
  TopAbs_ShapeEnum aType = TopAbs_SOLID;
  switch ( theDim ) {
  case 0: aType = TopAbs_VERTEX; break;
  case 1: aType = TopAbs_EDGE; break;
  case 2: aType = TopAbs_FACE; break;
  case 3:
  default:aType = TopAbs_SOLID; break;
  }
  return aType;
}

//=============================================================================
/*!
 * \brief Internal structure used to find concurent submeshes
 *
 * It represents a pair < submesh, concurent dimension >, where
 * 'concurrent dimension' is dimension of shape where the submesh can concurent
 *  with another submesh. In other words, it is dimension of a hypothesis assigned
 *  to submesh.
 */
//=============================================================================

class SMESH_DimHyp
{
 public:
  //! fileds
  int _dim;    //!< a dimension the algo can build (concurrent dimension)
  int _ownDim; //!< dimension of shape of _subMesh (>=_dim)
  TopTools_MapOfShape _shapeMap;
  SMESH_subMesh*      _subMesh;
  list<const SMESHDS_Hypothesis*> _hypothesises; //!< algo is first, then its parameters

  //! Constructors
  SMESH_DimHyp(const SMESH_subMesh*  theSubMesh,
               const int             theDim,
               const TopoDS_Shape&   theShape)
  {
    _subMesh = (SMESH_subMesh*)theSubMesh;
    SetShape( theDim, theShape );
  }

  //! set shape
  void SetShape(const int theDim,
                const TopoDS_Shape& theShape)
  {
    _dim = theDim;
    _ownDim = (int)SMESH_Gen::GetShapeDim(theShape);
    if (_dim >= _ownDim)
      _shapeMap.Add( theShape );
    else {
      TopExp_Explorer anExp( theShape, shapeTypeByDim(theDim) );
      for( ; anExp.More(); anExp.Next() )
        _shapeMap.Add( anExp.Current() );
    }
  }

  //! Check sharing of sub shapes
  static bool isShareSubShapes(const TopTools_MapOfShape& theToCheck,
                               const TopTools_MapOfShape& theToFind,
                               const TopAbs_ShapeEnum     theType)
  {
    bool isShared = false;
    TopTools_MapIteratorOfMapOfShape anItr( theToCheck );
    for (; !isShared && anItr.More(); anItr.Next() ) {
      const TopoDS_Shape aSubSh = anItr.Key();
      // check for case when concurrent dimensions are same
      isShared = theToFind.Contains( aSubSh );
      // check for subshape with concurrent dimension
      TopExp_Explorer anExp( aSubSh, theType );
      for ( ; !isShared && anExp.More(); anExp.Next() )
        isShared = theToFind.Contains( anExp.Current() );
    }
    return isShared;
  }
  
  //! check algorithms
  static bool checkAlgo(const SMESHDS_Hypothesis* theA1,
                        const SMESHDS_Hypothesis* theA2)
  {
    if ( theA1->GetType() == SMESHDS_Hypothesis::PARAM_ALGO ||
         theA2->GetType() == SMESHDS_Hypothesis::PARAM_ALGO )
      return false; // one of the hypothesis is not algorithm
    // check algorithm names (should be equal)
    return strcmp( theA1->GetName(), theA2->GetName() ) == 0;
  }

  
  //! Check if subshape hypotheses are concurrent
  bool IsConcurrent(const SMESH_DimHyp* theOther) const
  {
    if ( _subMesh == theOther->_subMesh )
      return false; // same subshape - should not be

    // if ( <own dim of either of submeshes> == <concurrent dim> &&
    //      any of the two submeshes is not on COMPOUND shape )
    //  -> no concurrency
    bool meIsCompound = (_subMesh->GetSubMeshDS() && _subMesh->GetSubMeshDS()->IsComplexSubmesh());
    bool otherIsCompound = (theOther->_subMesh->GetSubMeshDS() && theOther->_subMesh->GetSubMeshDS()->IsComplexSubmesh());
    if ( (_ownDim == _dim  || theOther->_ownDim == _dim ) && (!meIsCompound || !otherIsCompound))
      return false;

//     bool checkSubShape = ( _dim >= theOther->_dim )
//       ? isShareSubShapes( _shapeMap, theOther->_shapeMap, shapeTypeByDim(theOther->_dim) )
//       : isShareSubShapes( theOther->_shapeMap, _shapeMap, shapeTypeByDim(_dim) ) ;
    bool checkSubShape = isShareSubShapes( _shapeMap, theOther->_shapeMap, shapeTypeByDim(_dim));
    if ( !checkSubShape )
        return false;

    // check algorithms to be same
    if (!checkAlgo( _hypothesises.front(), theOther->_hypothesises.front() ))
      return true; // different algorithms
    
    // check hypothesises for concurrence (skip first as algorithm)
    int nbSame = 0;
    // pointers should be same, becase it is referenes from mesh hypothesis partition
    list <const SMESHDS_Hypothesis*>::const_iterator hypIt = _hypothesises.begin();
    list <const SMESHDS_Hypothesis*>::const_iterator otheEndIt = theOther->_hypothesises.end();
    for ( hypIt++ /*skip first as algo*/; hypIt != _hypothesises.end(); hypIt++ )
      if ( find( theOther->_hypothesises.begin(), otheEndIt, *hypIt ) != otheEndIt )
        nbSame++;
    // the submeshes are concurrent if their algorithms has different parameters
    return nbSame != theOther->_hypothesises.size() - 1;
  }
  
}; // end of SMESH_DimHyp

typedef list<SMESH_DimHyp*> TDimHypList;

static void addDimHypInstance(const int               theDim, 
                              const TopoDS_Shape&     theShape,
                              const SMESH_Algo*       theAlgo,
                              const SMESH_subMesh*    theSubMesh,
                              const list <const SMESHDS_Hypothesis*>& theHypList,
                              TDimHypList*            theDimHypListArr )
{
  TDimHypList& listOfdimHyp = theDimHypListArr[theDim];
  if ( listOfdimHyp.empty() || listOfdimHyp.back()->_subMesh != theSubMesh ) {
    SMESH_DimHyp* dimHyp = new SMESH_DimHyp( theSubMesh, theDim, theShape );
    listOfdimHyp.push_back( dimHyp );
  }
  
  SMESH_DimHyp* dimHyp = listOfdimHyp.back();
  dimHyp->_hypothesises.push_front(theAlgo);
  list <const SMESHDS_Hypothesis*>::const_iterator hypIt = theHypList.begin();
  for( ; hypIt != theHypList.end(); hypIt++ )
    dimHyp->_hypothesises.push_back( *hypIt );
}

static void findConcurrents(const SMESH_DimHyp* theDimHyp,
                            const TDimHypList&  theListOfDimHyp,
                            TListOfInt&         theListOfConcurr )
{
  TDimHypList::const_reverse_iterator rIt = theListOfDimHyp.rbegin();
  for ( ; rIt != theListOfDimHyp.rend(); rIt++ ) {
    const SMESH_DimHyp* curDimHyp = *rIt;
    if ( curDimHyp == theDimHyp )
      break; // meet own dimHyp pointer in same dimension
    else if ( theDimHyp->IsConcurrent( curDimHyp ) )
      if ( find( theListOfConcurr.begin(),
                 theListOfConcurr.end(),
                 curDimHyp->_subMesh->GetId() ) == theListOfConcurr.end() )
        theListOfConcurr.push_back( curDimHyp->_subMesh->GetId() );
  }
}

static void unionLists(TListOfInt&       theListOfId,
                       TListOfListOfInt& theListOfListOfId,
                       const int         theIndx )
{
  TListOfListOfInt::iterator it = theListOfListOfId.begin();
  for ( int i = 0; it != theListOfListOfId.end(); it++, i++ ) {
    if ( i < theIndx )
      continue; //skip already treated lists
    // check if other list has any same submesh object
    TListOfInt& otherListOfId = *it;
    if ( find_first_of( theListOfId.begin(), theListOfId.end(),
                        otherListOfId.begin(), otherListOfId.end() ) == theListOfId.end() )
      continue;
         
    // union two lists (from source into target)
    TListOfInt::iterator it2 = otherListOfId.begin();
    for ( ; it2 != otherListOfId.end(); it2++ ) {
      if ( find( theListOfId.begin(), theListOfId.end(), (*it2) ) == theListOfId.end() )
        theListOfId.push_back(*it2);
    }
    // clear source list
    otherListOfId.clear();
  }
}

//! free memory allocated for dimension-hypothesis objects
static void removeDimHyps( TDimHypList* theArrOfList )
{
  for (int i = 0; i < 4; i++ ) {
    TDimHypList& listOfdimHyp = theArrOfList[i];
    TDimHypList::const_iterator it = listOfdimHyp.begin();
    for ( ; it != listOfdimHyp.end(); it++ )
      delete (*it);
  }
}

//=============================================================================
/*!
 * \brief Return submesh objects list in meshing order
 */
//=============================================================================

SMESH::submesh_array_array* SMESH_Mesh_i::GetMeshOrder()
{
  SMESH::submesh_array_array_var aResult = new SMESH::submesh_array_array();

  SMESHDS_Mesh* aMeshDS = _impl->GetMeshDS();
  if ( !aMeshDS )
    return aResult._retn();
  
  ::SMESH_Mesh& mesh = GetImpl();
  TListOfListOfInt anOrder = mesh.GetMeshOrder(); // is there already defined order?
  if ( !anOrder.size() ) {

    // collect submeshes detecting concurrent algorithms and hypothesises
    TDimHypList dimHypListArr[4]; // dimHyp list for each shape dimension
    
    map<int, ::SMESH_subMesh*>::iterator i_sm = _mapSubMesh.begin();
    for ( ; i_sm != _mapSubMesh.end(); i_sm++ ) {
      ::SMESH_subMesh* sm = (*i_sm).second;
      // shape of submesh
      const TopoDS_Shape& aSubMeshShape = sm->GetSubShape();
      
      // list of assigned hypothesises
      const list <const SMESHDS_Hypothesis*>& hypList = mesh.GetHypothesisList(aSubMeshShape);
      // Find out dimensions where the submesh can be concurrent.
      // We define the dimensions by algo of each of hypotheses in hypList
      list <const SMESHDS_Hypothesis*>::const_iterator hypIt = hypList.begin();
      for( ; hypIt != hypList.end(); hypIt++ ) {
        SMESH_Algo* anAlgo = 0;
        const SMESH_Hypothesis* hyp = dynamic_cast<const SMESH_Hypothesis*>(*hypIt);
        if ( hyp->GetType() != SMESHDS_Hypothesis::PARAM_ALGO )
          // hyp it-self is algo
          anAlgo = (SMESH_Algo*)dynamic_cast<const SMESH_Algo*>(hyp);
        else {
          // try to find algorithm with help of subshapes
          TopExp_Explorer anExp( aSubMeshShape, shapeTypeByDim(hyp->GetDim()) );
          for ( ; !anAlgo && anExp.More(); anExp.Next() )
            anAlgo = mesh.GetGen()->GetAlgo( mesh, anExp.Current() );
        }
        if (!anAlgo)
          continue; // no assigned algorithm to current submesh

        int dim = anAlgo->GetDim(); // top concurrent dimension (see comment to SMESH_DimHyp)
        // the submesh can concurrent at <dim> (or lower dims if !anAlgo->NeedDescretBoundary())

        // create instance of dimension-hypothesis for found concurrent dimension(s) and algorithm
        for ( int j = anAlgo->NeedDescretBoundary() ? dim : 1, jn = dim; j <= jn; j++ )
          addDimHypInstance( j, aSubMeshShape, anAlgo, sm, hypList, dimHypListArr );
      }
    } // end iterations on submesh
    
    // iterate on created dimension-hypotheses and check for concurrents
    for ( int i = 0; i < 4; i++ ) {
      const list<SMESH_DimHyp*>& listOfDimHyp = dimHypListArr[i];
      // check for concurrents in own and other dimensions (step-by-step)
      TDimHypList::const_iterator dhIt = listOfDimHyp.begin();
      for ( ; dhIt != listOfDimHyp.end(); dhIt++ ) {
        const SMESH_DimHyp* dimHyp = *dhIt;
        TListOfInt listOfConcurr;
        // looking for concurrents and collect into own list
        for ( int j = i; j < 4; j++ )
          findConcurrents( dimHyp, dimHypListArr[j], listOfConcurr );
        // check if any concurrents found
        if ( listOfConcurr.size() > 0 ) {
          // add own submesh to list of concurrent
          listOfConcurr.push_front( dimHyp->_subMesh->GetId() );
          anOrder.push_back( listOfConcurr );
        }
      }
    }
    
    removeDimHyps(dimHypListArr);
    
    // now, minimise the number of concurrent groups
    // Here we assume that lists of submhes can has same submesh
    // in case of multi-dimension algorithms, as result
    //  list with common submesh have to be union into one list
    int listIndx = 0;
    TListOfListOfInt::iterator listIt = anOrder.begin();
    for(; listIt != anOrder.end(); listIt++, listIndx++ )
      unionLists( *listIt,  anOrder, listIndx + 1 );
  }
  // convert submesh ids into interface instances
  //  and dump command into python
  convertMeshOrder( anOrder, aResult, true );

  return aResult._retn();
}

//=============================================================================
/*!
 * \brief find common submeshes with given submesh
 * \param theSubMeshList list of already collected submesh to check
 * \param theSubMesh given submesh to intersect with other
 * \param theCommonSubMeshes collected common submeshes
 */
//=============================================================================

static void findCommonSubMesh
 (list<const SMESH_subMesh*>& theSubMeshList,
  const SMESH_subMesh*             theSubMesh,
  set<const SMESH_subMesh*>&  theCommon )
{
  if ( !theSubMesh )
    return;
  list<const SMESH_subMesh*>::const_iterator it = theSubMeshList.begin();
  for ( ; it != theSubMeshList.end(); it++ )
    theSubMesh->FindIntersection( *it, theCommon );
  theSubMeshList.push_back( theSubMesh );
  //theCommon.insert( theSubMesh );
}

//=============================================================================
/*!
 * \brief Set submesh object order
 * \param theSubMeshArray submesh array order
 */
//=============================================================================

::CORBA::Boolean SMESH_Mesh_i::SetMeshOrder(const SMESH::submesh_array_array& theSubMeshArray)
{
  bool res = false;
  ::SMESH_Mesh& mesh = GetImpl();

  TPythonDump aPythonDump; // prevent dump of called methods
  aPythonDump << "isDone = " << _this() << ".SetMeshOrder( [ ";

  TListOfListOfInt subMeshOrder;
  for ( int i = 0, n = theSubMeshArray.length(); i < n; i++ )
  {
    const SMESH::submesh_array& aSMArray = theSubMeshArray[i];
    TListOfInt subMeshIds;
    aPythonDump << "[ ";
    // Collect subMeshes which should be clear
    //  do it list-by-list, because modification of submesh order
    //  take effect between concurrent submeshes only
    set<const SMESH_subMesh*> subMeshToClear;
    list<const SMESH_subMesh*> subMeshList;
    for ( int j = 0, jn = aSMArray.length(); j < jn; j++ )
    {
      const SMESH::SMESH_subMesh_var subMesh = SMESH::SMESH_subMesh::_duplicate(aSMArray[j]);
      if ( j > 0 )
        aPythonDump << ", ";
      aPythonDump << subMesh;
      subMeshIds.push_back( subMesh->GetId() );
      // detect common parts of submeshes
      if ( _mapSubMesh.find(subMesh->GetId()) != _mapSubMesh.end() )
        findCommonSubMesh( subMeshList, _mapSubMesh[ subMesh->GetId() ], subMeshToClear );
    }
    aPythonDump << " ]";
    subMeshOrder.push_back( subMeshIds );

    // clear collected submeshes
    set<const SMESH_subMesh*>::iterator clrIt = subMeshToClear.begin();
    for ( ; clrIt != subMeshToClear.end(); clrIt++ ) {
      SMESH_subMesh* sm = (SMESH_subMesh*)*clrIt;
        if ( sm )
          sm->ComputeStateEngine( SMESH_subMesh::CLEAN );
        // ClearSubMesh( *clrIt );
      }
  }
  aPythonDump << " ])";

  mesh.SetMeshOrder( subMeshOrder );
  res = true;
  
  return res;
}

//=============================================================================
/*!
 * \brief Convert submesh ids into submesh interfaces
 */
//=============================================================================

void SMESH_Mesh_i::convertMeshOrder
(const TListOfListOfInt& theIdsOrder,
 SMESH::submesh_array_array& theResOrder,
 const bool theIsDump)
{
  int nbSet = theIdsOrder.size();
  TPythonDump aPythonDump; // prevent dump of called methods
  if ( theIsDump )
    aPythonDump << "[ ";
  theResOrder.length(nbSet);
  TListOfListOfInt::const_iterator it = theIdsOrder.begin();
  int listIndx = 0;
  for( ; it != theIdsOrder.end(); it++ ) {
    // translate submesh identificators into submesh objects
    //  takeing into account real number of concurrent lists
    const TListOfInt& aSubOrder = (*it);
    if (!aSubOrder.size())
      continue;
    if ( theIsDump )
      aPythonDump << "[ ";
    // convert shape indeces into interfaces
    SMESH::submesh_array_var aResSubSet = new SMESH::submesh_array();
    aResSubSet->length(aSubOrder.size());
    TListOfInt::const_iterator subIt = aSubOrder.begin();
    for( int j = 0; subIt != aSubOrder.end(); subIt++ ) {
      if ( _mapSubMeshIor.find(*subIt) == _mapSubMeshIor.end() )
        continue;
      SMESH::SMESH_subMesh_var subMesh =
        SMESH::SMESH_subMesh::_duplicate( _mapSubMeshIor[*subIt] );
      if ( theIsDump ) {
        if ( j > 0 )
          aPythonDump << ", ";
        aPythonDump << subMesh;
      }
      aResSubSet[ j++ ] = subMesh;
    }
    if ( theIsDump )
      aPythonDump << " ]";
    theResOrder[ listIndx++ ] = aResSubSet;
  }
  // correct number of lists
  theResOrder.length( listIndx );

  if ( theIsDump ) {
    // finilise python dump
    aPythonDump << " ]";
    aPythonDump << " = " << _this() << ".GetMeshOrder()";
  }
}
