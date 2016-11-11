// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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
//  File   : SMESH_Mesh_i.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH

#include "SMESH_Mesh_i.hxx"

#include "DriverMED_R_SMESHDS_Mesh.h"
#include "DriverMED_W_Field.h"
#include "DriverMED_W_SMESHDS_Mesh.h"
#include "MED_Factory.hxx"
#include "SMDS_EdgePosition.hxx"
#include "SMDS_ElemIterator.hxx"
#include "SMDS_FacePosition.hxx"
#include "SMDS_IteratorOnIterators.hxx"
#include "SMDS_MeshGroup.hxx"
#include "SMDS_SetIterator.hxx"
#include "SMDS_VolumeTool.hxx"
#include "SMESHDS_Command.hxx"
#include "SMESHDS_CommandType.hxx"
#include "SMESHDS_Group.hxx"
#include "SMESHDS_GroupOnGeom.hxx"
#include "SMESH_Controls.hxx"
#include "SMESH_File.hxx"
#include "SMESH_Filter_i.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Group.hxx"
#include "SMESH_Group_i.hxx"
#include "SMESH_Mesh.hxx"
#include "SMESH_MeshAlgos.hxx"
#include "SMESH_MeshEditor.hxx"
#include "SMESH_MeshEditor_i.hxx"
#include "SMESH_MeshPartDS.hxx"
#include "SMESH_MesherHelper.hxx"
#include "SMESH_PreMeshInfo.hxx"
#include "SMESH_PythonDump.hxx"
#include "SMESH_subMesh_i.hxx"

#include <SALOMEDS_Attributes_wrap.hxx>
#include <SALOMEDS_wrap.hxx>
#include <Utils_ExceptHandlers.hxx>
#include <utilities.h>

#include <GEOMImpl_Types.hxx>
#include <GEOM_wrap.hxx>

// OCCT Includes
#include <BRep_Builder.hxx>
#include <Standard_ErrorHandler.hxx>
#include <TColStd_MapOfInteger.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_MapIteratorOfMapOfShape.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopoDS_Compound.hxx>

// STL Includes
#include <algorithm>
#include <iostream>
#include <sstream>

#include <vtkUnstructuredGridWriter.h>

// to pass CORBA exception through SMESH_TRY
#define SMY_OWN_CATCH catch( SALOME::SALOME_Exception& se ) { throw se; }

#include "SMESH_TryCatch.hxx" // include after OCCT headers!

#ifdef _DEBUG_
static int MYDEBUG = 0;
#else
static int MYDEBUG = 0;
#endif

using namespace std;
using SMESH::TPythonDump;

int SMESH_Mesh_i::_idGenerator = 0;

//=============================================================================
/*!
 *  Constructor
 */
//=============================================================================

SMESH_Mesh_i::SMESH_Mesh_i( PortableServer::POA_ptr thePOA,
                            SMESH_Gen_i*            gen_i,
                            CORBA::Long             studyId )
: SALOME::GenericObj_i( thePOA )
{
  _impl          = NULL;
  _gen_i         = gen_i;
  _id            = _idGenerator++;
  _studyId       = studyId;
  _editor        = NULL;
  _previewEditor = NULL;
  _preMeshInfo   = NULL;
  _mainShapeTick = 0;
}

//=============================================================================
/*!
 *  Destructor
 */
//=============================================================================

SMESH_Mesh_i::~SMESH_Mesh_i()
{
  // destroy groups
  map<int, SMESH::SMESH_GroupBase_ptr>::iterator itGr;
  for (itGr = _mapGroups.begin(); itGr != _mapGroups.end(); itGr++)
    if (SMESH_GroupBase_i* aGroup = SMESH::DownCast<SMESH_GroupBase_i*>(itGr->second))
    {
      aGroup->UnRegister();
      SMESH::SMESH_GroupBase_var( itGr->second );
    }
  _mapGroups.clear();

  // destroy submeshes
  map<int, SMESH::SMESH_subMesh_ptr>::iterator itSM;
  for ( itSM = _mapSubMeshIor.begin(); itSM != _mapSubMeshIor.end(); itSM++ )
    if ( SMESH_subMesh_i* aSubMesh = SMESH::DownCast<SMESH_subMesh_i*>( itSM->second ))
    {
      aSubMesh->UnRegister();
      SMESH::SMESH_subMesh_var( itSM->second );
    }
  _mapSubMeshIor.clear();

  // destroy hypotheses. _mapHypo contains all hyps ever been assigned
  map<int, SMESH::SMESH_Hypothesis_ptr>::iterator itH;
  for ( itH = _mapHypo.begin(); itH != _mapHypo.end(); itH++ ) {
    if ( SMESH_Hypothesis_i* hyp_i = SMESH::DownCast<SMESH_Hypothesis_i*>( itH->second ))
      if ( SMESH_Hypothesis * smHyp = _impl->GetHypothesis( itH->first ))
        if ( _impl->GetMeshDS()->IsUsedHypothesis( smHyp ))
          hyp_i->UnRegister();

    SMESH::SMESH_Hypothesis_var( itH->second ); // decref CORBA object
  }
  _mapHypo.clear();

  // clear cashed shapes if no more meshes remain; (the cash is blame,
  // together with publishing, of spent time increasing in issue 22874)
  if ( _impl->NbMeshes() == 1 )
    _gen_i->GetShapeReader()->ClearClientBuffer();

  delete _editor; _editor = NULL;
  delete _previewEditor; _previewEditor = NULL;
  delete _impl; _impl = NULL;
  delete _preMeshInfo; _preMeshInfo = NULL;
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
  SMESH::SMESH_Mesh_var mesh = _this();
  addGeomGroupData( theShapeObject, mesh );
  if ( !CORBA::is_nil( theShapeObject ))
    _mainShapeTick = theShapeObject->GetTick();
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
    {
      aShapeObj = _gen_i->ShapeToGeomObject( S );
      if ( aShapeObj->_is_nil() )
      {
        // S was removed from GEOM_Client by newGroupShape() called by other mesh;
        // find GEOM_Object by entry (IPAL52735)
        list<TGeomGroupData>::iterator data = _geomGroupData.begin();
        for ( ; data != _geomGroupData.end(); ++data )
          if ( data->_smeshObject->_is_equivalent( _this() ))
          {
            SALOMEDS::Study_var study = _gen_i->GetCurrentStudy();
            if ( study->_is_nil() ) break;
            SALOMEDS::SObject_wrap so = study->FindObjectID( data->_groupEntry.c_str() );
            CORBA::Object_var     obj = _gen_i->SObjectToObject( so );
            aShapeObj = GEOM::GEOM_Object::_narrow( obj );
            break;
          }
      }
    }
  }
  catch(SALOME_Exception & S_ex) {
    THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
  }
  return aShapeObj._retn();
}

//================================================================================
/*!
 * \brief Return false if the mesh is not yet fully loaded from the study file
 */
//================================================================================

CORBA::Boolean SMESH_Mesh_i::IsLoaded() throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  return !_preMeshInfo;
}

//================================================================================
/*!
 * \brief Load full mesh data from the study file
 */
//================================================================================

void SMESH_Mesh_i::Load() throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();
}

//================================================================================
/*!
 * \brief Remove all nodes and elements
 */
//================================================================================

void SMESH_Mesh_i::Clear() throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    _preMeshInfo->ForgetOrLoad(); // load in case if !HasShapeToMesh()

  try {
    _impl->Clear();
    //CheckGeomGroupModif(); // issue 20145
  }
  catch(SALOME_Exception & S_ex) {
    THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
  }

  TPythonDump() <<  SMESH::SMESH_Mesh_var(_this()) << ".Clear()";
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
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  try {
    _impl->ClearSubMesh( ShapeID );
  }
  catch(SALOME_Exception & S_ex) {
    THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
  }
  _impl->GetMeshDS()->Modified();

  TPythonDump() <<  SMESH::SMESH_Mesh_var(_this()) << ".ClearSubMesh( " << ShapeID << " )";
}

//=============================================================================
/*!
 * Convert enum Driver_Mesh::Status to SMESH::DriverMED_ReadStatus
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
  case DriverMED_R_SMESHDS_Mesh::DRS_WARN_DESCENDING:
    res = SMESH::DRS_WARN_DESCENDING; break;
  case DriverMED_R_SMESHDS_Mesh::DRS_FAIL:
  default:
    res = SMESH::DRS_FAIL; break;
  }
  return res;
}

//=============================================================================
/*!
 * Convert ::SMESH_ComputeError to SMESH::ComputeError
 */
//=============================================================================

static SMESH::ComputeError* ConvertComputeError( SMESH_ComputeErrorPtr errorPtr )
{
  SMESH::ComputeError_var errVar = new SMESH::ComputeError();
  errVar->subShapeID = -1;
  errVar->hasBadMesh = false;

  if ( !errorPtr || errorPtr->IsOK() )
  {
    errVar->code = SMESH::COMPERR_OK;
  }
  else
  {
    errVar->code    = ConvertDriverMEDReadStatus( errorPtr->myName );
    errVar->comment = errorPtr->myComment.c_str();
  }
  return errVar._retn();
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
  _medFileInfo           = new SMESH::MedFileInfo();
  _medFileInfo->fileName = theFileName;
  _medFileInfo->fileSize = 0;
  _medFileInfo->major    = major;
  _medFileInfo->minor    = minor;
  _medFileInfo->release  = release;
  _medFileInfo->fileSize = SMESH_File( theFileName ).size();

  return ConvertDriverMEDReadStatus(status);
}

//================================================================================
/*!
 * \brief Imports mesh data from the CGNS file
 */
//================================================================================

SMESH::DriverMED_ReadStatus SMESH_Mesh_i::ImportCGNSFile( const char*  theFileName,
                                                          const int    theMeshIndex,
                                                          std::string& theMeshName )
  throw ( SALOME::SALOME_Exception )
{
  Unexpect aCatch(SALOME_SalomeException);
  int status;
  try {
    status = _impl->CGNSToMesh( theFileName, theMeshIndex, theMeshName );
  }
  catch( SALOME_Exception& S_ex ) {
    THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
  }
  catch ( ... ) {
    THROW_SALOME_CORBA_EXCEPTION("ImportCGNSFile(): unknown exception", SALOME::BAD_PARAM);
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
  SMESH_TRY;

  // Read mesh with name = <theMeshName> into SMESH_Mesh
  _impl->UNVToMesh( theFileName );

  CreateGroupServants();

  SMESH_CATCH( SMESH::throwCorbaException );

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
  SMESH_TRY;

  // Read mesh with name = <theMeshName> into SMESH_Mesh
  _impl->STLToMesh( theFileName );

  SMESH_CATCH( SMESH::throwCorbaException );

  return 1;
}

//================================================================================
/*!
 * \brief Function used in SMESH_CATCH by ImportGMFFile()
 */
//================================================================================

namespace
{
  SMESH_ComputeErrorPtr exceptionToComputeError(const char* excText)
  {
    return SMESH_ComputeError::New( Driver_Mesh::DRS_FAIL, excText );
  }
}

//================================================================================
/*!
 * \brief Imports data from a GMF file and returns an error description
 */
//================================================================================

SMESH::ComputeError* SMESH_Mesh_i::ImportGMFFile( const char* theFileName,
                                                  bool        theMakeRequiredGroups )
  throw (SALOME::SALOME_Exception)
{
  SMESH_ComputeErrorPtr error;

#undef SMESH_CAUGHT
#define SMESH_CAUGHT error =
  SMESH_TRY;

  error = _impl->GMFToMesh( theFileName, theMakeRequiredGroups );

  SMESH_CATCH( exceptionToComputeError );
#undef SMESH_CAUGHT
#define SMESH_CAUGHT

  CreateGroupServants();

  return ConvertComputeError( error );
}

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
  RETURNCASE( HYP_INCOMPAT_HYPS );
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

SMESH::Hypothesis_Status
SMESH_Mesh_i::AddHypothesis(GEOM::GEOM_Object_ptr       aSubShape,
                            SMESH::SMESH_Hypothesis_ptr anHyp,
                            CORBA::String_out           anErrorText)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    _preMeshInfo->ForgetOrLoad();

  std::string error;
  SMESH_Hypothesis::Hypothesis_Status status = addHypothesis( aSubShape, anHyp, &error );
  anErrorText = error.c_str();

  SMESH::SMESH_Mesh_var mesh( _this() );
  if ( !SMESH_Hypothesis::IsStatusFatal(status) )
  {
    SALOMEDS::Study_var study = _gen_i->GetCurrentStudy();
    _gen_i->AddHypothesisToShape( study, mesh, aSubShape, anHyp );
  }
  if(MYDEBUG) MESSAGE( " AddHypothesis(): status = " << status );

  // Update Python script
  TPythonDump() << "status = " << mesh << ".AddHypothesis( "
                << aSubShape << ", " << anHyp << " )";

  return ConvertHypothesisStatus(status);
}

//=============================================================================
/*!
 *
 */
//=============================================================================

SMESH_Hypothesis::Hypothesis_Status
SMESH_Mesh_i::addHypothesis(GEOM::GEOM_Object_ptr       aSubShape,
                            SMESH::SMESH_Hypothesis_ptr anHyp,
                            std::string*                anErrorText)
{
  if(MYDEBUG) MESSAGE("addHypothesis");

  if (CORBA::is_nil( aSubShape ) && HasShapeToMesh())
    THROW_SALOME_CORBA_EXCEPTION("bad Sub-shape reference",SALOME::BAD_PARAM);

  if (CORBA::is_nil( anHyp ))
    THROW_SALOME_CORBA_EXCEPTION("bad hypothesis reference",SALOME::BAD_PARAM);

  SMESH_Hypothesis::Hypothesis_Status status = SMESH_Hypothesis::HYP_OK;
  try
  {
    TopoDS_Shape myLocSubShape;
    //use PseudoShape in case if mesh has no shape
    if(HasShapeToMesh())
      myLocSubShape = _gen_i->GeomObjectToShape( aSubShape);
    else              
      myLocSubShape = _impl->GetShapeToMesh();
    
    const int hypId = anHyp->GetId();
    std::string error;
    status = _impl->AddHypothesis( myLocSubShape, hypId, &error );
    if ( !SMESH_Hypothesis::IsStatusFatal(status) )
    {
      _mapHypo[hypId] = SMESH::SMESH_Hypothesis::_duplicate( anHyp );
      anHyp->Register();
      // assure there is a corresponding submesh
      if ( !_impl->IsMainShape( myLocSubShape )) {
        int shapeId = _impl->GetMeshDS()->ShapeToIndex( myLocSubShape );
        if ( _mapSubMesh_i.find( shapeId ) == _mapSubMesh_i.end() )
          SMESH::SMESH_subMesh_var( createSubMesh( aSubShape ));
      }
    }
    else if ( anErrorText )
    {
      *anErrorText = error;
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

SMESH::Hypothesis_Status SMESH_Mesh_i::RemoveHypothesis(GEOM::GEOM_Object_ptr aSubShape,
                                                        SMESH::SMESH_Hypothesis_ptr anHyp)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    _preMeshInfo->ForgetOrLoad();

  SMESH_Hypothesis::Hypothesis_Status status = removeHypothesis( aSubShape, anHyp );
  SMESH::SMESH_Mesh_var mesh = _this();

  if ( !SMESH_Hypothesis::IsStatusFatal(status) )
  {
    SALOMEDS::Study_var study = _gen_i->GetCurrentStudy();
    _gen_i->RemoveHypothesisFromShape( study, mesh, aSubShape, anHyp );
  }
  // Update Python script
  if(_impl->HasShapeToMesh())
    TPythonDump() << "status = " << mesh << ".RemoveHypothesis( "
                  << aSubShape << ", " << anHyp << " )";
  else
    TPythonDump() << "status = " << mesh << ".RemoveHypothesis( "
                  << anHyp << " )";

  return ConvertHypothesisStatus(status);
}

//=============================================================================
/*!
 *
 */
//=============================================================================

SMESH_Hypothesis::Hypothesis_Status
SMESH_Mesh_i::removeHypothesis(GEOM::GEOM_Object_ptr       aSubShape,
                               SMESH::SMESH_Hypothesis_ptr anHyp)
{
  if(MYDEBUG) MESSAGE("removeHypothesis()");

  if (CORBA::is_nil( aSubShape ) && HasShapeToMesh())
    THROW_SALOME_CORBA_EXCEPTION("bad Sub-shape reference", SALOME::BAD_PARAM);

  if (CORBA::is_nil( anHyp ))
    THROW_SALOME_CORBA_EXCEPTION("bad hypothesis reference", SALOME::BAD_PARAM);

  SMESH_Hypothesis::Hypothesis_Status status = SMESH_Hypothesis::HYP_OK;
  try
  {
    TopoDS_Shape myLocSubShape;
    //use PseudoShape in case if mesh has no shape
    if( _impl->HasShapeToMesh() )
      myLocSubShape = _gen_i->GeomObjectToShape( aSubShape );
    else
      myLocSubShape = _impl->GetShapeToMesh();

    const int hypId = anHyp->GetId();
    status = _impl->RemoveHypothesis(myLocSubShape, hypId);
    if ( !SMESH_Hypothesis::IsStatusFatal(status) )
    {
      // _mapHypo.erase( hypId ); EAP: hyp can be used on many sub-shapes
      anHyp->UnRegister();
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

SMESH::ListOfHypothesis *
SMESH_Mesh_i::GetHypothesisList(GEOM::GEOM_Object_ptr aSubShape)
throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if (MYDEBUG) MESSAGE("GetHypothesisList");
  if (_impl->HasShapeToMesh() && CORBA::is_nil(aSubShape))
    THROW_SALOME_CORBA_EXCEPTION("bad Sub-shape reference", SALOME::BAD_PARAM);

  SMESH::ListOfHypothesis_var aList = new SMESH::ListOfHypothesis();

  try {
    TopoDS_Shape myLocSubShape = _gen_i->GeomObjectToShape(aSubShape);
    if ( myLocSubShape.IsNull() && !_impl->HasShapeToMesh() )
      myLocSubShape = _impl->GetShapeToMesh();
    const list<const SMESHDS_Hypothesis*>& aLocalList = _impl->GetHypothesisList( myLocSubShape );
    int i = 0, n = aLocalList.size();
    aList->length( n );

    list<const SMESHDS_Hypothesis*>::const_iterator aHyp = aLocalList.begin();
    std::map<int, SMESH::SMESH_Hypothesis_ptr>::iterator id_hypptr;
    for ( ; i < n && aHyp != aLocalList.end(); aHyp++ )
    {
      id_hypptr = _mapHypo.find( (*aHyp)->GetID() );
      if ( id_hypptr != _mapHypo.end() )
        aList[i++] = SMESH::SMESH_Hypothesis::_narrow( id_hypptr->second );
    }
    aList->length( i );
  }
  catch(SALOME_Exception & S_ex) {
    THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
  }

  return aList._retn();
}

SMESH::submesh_array* SMESH_Mesh_i::GetSubMeshes() throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if (MYDEBUG) MESSAGE("GetSubMeshes");

  SMESH::submesh_array_var aList = new SMESH::submesh_array();

  // Python Dump
  TPythonDump aPythonDump;
  if ( !_mapSubMeshIor.empty() )
    aPythonDump << "[ ";

  try {
    aList->length( _mapSubMeshIor.size() );
    int i = 0;
    map<int, SMESH::SMESH_subMesh_ptr>::iterator it = _mapSubMeshIor.begin();
    for ( ; it != _mapSubMeshIor.end(); it++ ) {
      if ( CORBA::is_nil( it->second )) continue;
      aList[i++] = SMESH::SMESH_subMesh::_duplicate( it->second );
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
  if ( !_mapSubMeshIor.empty() )
    aPythonDump << " ] = " << SMESH::SMESH_Mesh_var( _this() ) << ".GetSubMeshes()";

  return aList._retn();
}

//=============================================================================
/*!
 *
 */
//=============================================================================

SMESH::SMESH_subMesh_ptr SMESH_Mesh_i::GetSubMesh(GEOM::GEOM_Object_ptr aSubShape,
                                                  const char*           theName )
     throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if (CORBA::is_nil(aSubShape))
    THROW_SALOME_CORBA_EXCEPTION("bad Sub-shape reference", SALOME::BAD_PARAM);

  SMESH::SMESH_subMesh_var subMesh;
  SMESH::SMESH_Mesh_var    aMesh = _this();
  try {
    TopoDS_Shape myLocSubShape = _gen_i->GeomObjectToShape(aSubShape);

    //Get or Create the SMESH_subMesh object implementation

    int subMeshId = _impl->GetMeshDS()->ShapeToIndex( myLocSubShape );

    if ( !subMeshId && ! _impl->GetMeshDS()->IsGroupOfSubShapes( myLocSubShape ))
    {
      TopoDS_Iterator it( myLocSubShape );
      if ( it.More() )
        THROW_SALOME_CORBA_EXCEPTION("not sub-shape of the main shape", SALOME::BAD_PARAM);
    }
    subMesh = getSubMesh( subMeshId );

    // create a new subMesh object servant if there is none for the shape
    if ( subMesh->_is_nil() )
      subMesh = createSubMesh( aSubShape );
    if ( _gen_i->CanPublishInStudy( subMesh ))
    {
      SALOMEDS::Study_var study = _gen_i->GetCurrentStudy();
      SALOMEDS::SObject_wrap aSO =
        _gen_i->PublishSubMesh( study, aMesh, subMesh, aSubShape, theName );
      if ( !aSO->_is_nil()) {
        // Update Python script
        TPythonDump() << aSO << " = " << aMesh << ".GetSubMesh( "
                      << aSubShape << ", '" << theName << "' )";
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
  SMESH_TRY;

  if ( theSubMesh->_is_nil() )
    return;

  GEOM::GEOM_Object_var aSubShape;
  SALOMEDS::Study_var aStudy = _gen_i->GetCurrentStudy();
  if ( !aStudy->_is_nil() )  {
    // Remove submesh's SObject
    SALOMEDS::SObject_wrap anSO = _gen_i->ObjectToSObject( aStudy, theSubMesh );
    if ( !anSO->_is_nil() ) {
      long aTag = SMESH_Gen_i::GetRefOnShapeTag();
      SALOMEDS::SObject_wrap anObj, aRef;
      if ( anSO->FindSubObject( aTag, anObj.inout() ) &&
           anObj->ReferencedObject( aRef.inout() ))
      {
        CORBA::Object_var obj = aRef->GetObject();
        aSubShape = GEOM::GEOM_Object::_narrow( obj );
      }
      // if ( aSubShape->_is_nil() ) // not published shape (IPAL13617)
      //   aSubShape = theSubMesh->GetSubShape();

      SALOMEDS::StudyBuilder_var builder = aStudy->NewBuilder();
      builder->RemoveObjectWithChildren( anSO );

      // Update Python script
      TPythonDump() << SMESH::SMESH_Mesh_var( _this() ) << ".RemoveSubMesh( " << anSO << " )";
    }
  }

  if ( removeSubMesh( theSubMesh, aSubShape.in() ))
    if ( _preMeshInfo )
      _preMeshInfo->ForgetOrLoad();

  SMESH_CATCH( SMESH::throwCorbaException );
}

//=============================================================================
/*!
 *
 */
//=============================================================================

SMESH::SMESH_Group_ptr SMESH_Mesh_i::CreateGroup( SMESH::ElementType theElemType,
                                                  const char*        theName )
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESH::SMESH_Group_var aNewGroup =
    SMESH::SMESH_Group::_narrow( createGroup( theElemType, theName ));

  if ( _gen_i->CanPublishInStudy( aNewGroup ) )
  {
    SMESH::SMESH_Mesh_var mesh = _this();
    SALOMEDS::Study_var  study = _gen_i->GetCurrentStudy();
    SALOMEDS::SObject_wrap aSO =
      _gen_i->PublishGroup( study, mesh, aNewGroup, GEOM::GEOM_Object::_nil(), theName);
    if ( !aSO->_is_nil())
      // Update Python script
      TPythonDump() << aSO << " = " << mesh << ".CreateGroup( "
                    << theElemType << ", '" << theName << "' )";
  }
  return aNewGroup._retn();
}

//=============================================================================
/*!
 *
 */
//=============================================================================
SMESH::SMESH_GroupOnGeom_ptr
SMESH_Mesh_i::CreateGroupFromGEOM (SMESH::ElementType    theElemType,
                                   const char*           theName,
                                   GEOM::GEOM_Object_ptr theGeomObj)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESH::SMESH_GroupOnGeom_var aNewGroup;

  TopoDS_Shape aShape = _gen_i->GeomObjectToShape( theGeomObj );
  if ( !aShape.IsNull() )
  {
    aNewGroup = 
      SMESH::SMESH_GroupOnGeom::_narrow( createGroup( theElemType, theName, aShape ));

    if ( _gen_i->CanPublishInStudy( aNewGroup ) )
    {
      SMESH::SMESH_Mesh_var mesh = _this();
      SALOMEDS::Study_var  study = _gen_i->GetCurrentStudy();
      SALOMEDS::SObject_wrap aSO =
        _gen_i->PublishGroup( study, mesh, aNewGroup, theGeomObj, theName );
      if ( !aSO->_is_nil())
        TPythonDump() << aSO << " = " << mesh << ".CreateGroupFromGEOM( "
                      << theElemType << ", '" << theName << "', " << theGeomObj << " )";
    }
  }

  return aNewGroup._retn();
}

//================================================================================
/*!
 * \brief Creates a group whose contents is defined by filter
 *  \param theElemType - group type
 *  \param theName - group name
 *  \param theFilter - the filter
 *  \retval SMESH::SMESH_GroupOnFilter_ptr - group defined by filter
 */
//================================================================================

SMESH::SMESH_GroupOnFilter_ptr
SMESH_Mesh_i::CreateGroupFromFilter(SMESH::ElementType theElemType,
                                    const char*        theName,
                                    SMESH::Filter_ptr  theFilter )
  throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  if ( CORBA::is_nil( theFilter ))
    THROW_SALOME_CORBA_EXCEPTION("NULL filter", SALOME::BAD_PARAM);

  SMESH_PredicatePtr predicate = SMESH_GroupOnFilter_i::GetPredicate( theFilter );
  if ( !predicate )
    THROW_SALOME_CORBA_EXCEPTION("Invalid filter", SALOME::BAD_PARAM);

  SMESH::SMESH_GroupOnFilter_var aNewGroup = SMESH::SMESH_GroupOnFilter::_narrow
    ( createGroup( theElemType, theName, TopoDS_Shape(), predicate ));

  TPythonDump pd;
  if ( !aNewGroup->_is_nil() )
    aNewGroup->SetFilter( theFilter );

  if ( _gen_i->CanPublishInStudy( aNewGroup ) )
  {
    SMESH::SMESH_Mesh_var mesh = _this();
    SALOMEDS::Study_var  study = _gen_i->GetCurrentStudy();
    SALOMEDS::SObject_wrap aSO =
      _gen_i->PublishGroup( study, mesh, aNewGroup, GEOM::GEOM_Object::_nil(), theName );

    if ( !aSO->_is_nil())
      pd << aSO << " = " << mesh << ".CreateGroupFromFilter( "
         << theElemType << ", '" << theName << "', " << theFilter << " )";
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

  SMESH_TRY;

  SMESH_GroupBase_i* aGroup = SMESH::DownCast<SMESH_GroupBase_i*>( theGroup );
  if ( !aGroup )
    return;

  SALOMEDS::Study_var aStudy = _gen_i->GetCurrentStudy();
  if ( !aStudy->_is_nil() )
  {
    SALOMEDS::SObject_wrap aGroupSO = _gen_i->ObjectToSObject( aStudy, theGroup );
    if ( !aGroupSO->_is_nil() )
    {
      // Update Python script
      TPythonDump() << SMESH::SMESH_Mesh_var(_this()) << ".RemoveGroup( " << aGroupSO << " )";

      // Remove group's SObject
      SALOMEDS::StudyBuilder_var builder = aStudy->NewBuilder();
      builder->RemoveObjectWithChildren( aGroupSO );
    }
  }
  aGroup->Modified(/*removed=*/true); // notify dependent Filter with FT_BelongToMeshGroup criterion

  // Remove the group from SMESH data structures
  removeGroup( aGroup->GetLocalID() );

  SMESH_CATCH( SMESH::throwCorbaException );
}

//=============================================================================
/*!
 *  Remove group with its contents
 */
//=============================================================================

void SMESH_Mesh_i::RemoveGroupWithContents( SMESH::SMESH_GroupBase_ptr theGroup )
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  if ( theGroup->_is_nil() )
    return;

  vector<int> nodeIds; // to remove nodes becoming free
  if ( !theGroup->IsEmpty() )
  {
    CORBA::Long elemID = theGroup->GetID( 1 );
    int nbElemNodes = GetElemNbNodes( elemID );
    if ( nbElemNodes > 0 )
      nodeIds.reserve( theGroup->Size() * nbElemNodes );
  }

  // Remove contents
  SMESH::SMESH_IDSource_var idSrc = SMESH::SMESH_IDSource::_narrow( theGroup );
  SMDS_ElemIteratorPtr     elemIt = GetElements( idSrc, theGroup->GetType() );
  while ( elemIt->more() )
  {
    const SMDS_MeshElement* e = elemIt->next();

    SMDS_ElemIteratorPtr nIt = e->nodesIterator();
    while ( nIt->more() )
      nodeIds.push_back( nIt->next()->GetID() );

    _impl->GetMeshDS()->RemoveElement( e );
  }

  // Remove free nodes
  if ( theGroup->GetType() != SMESH::NODE )
    for ( size_t i = 0 ; i < nodeIds.size(); ++i )
      if ( const SMDS_MeshNode* n = _impl->GetMeshDS()->FindNode( nodeIds[i] ))
        if ( n->NbInverseElements() == 0 )
          _impl->GetMeshDS()->RemoveFreeNode( n, /*sm=*/0 );

  TPythonDump pyDump; // Supress dump from RemoveGroup()

  // Update Python script (theGroup must be alive for this)
  pyDump << SMESH::SMESH_Mesh_var(_this())
         << ".RemoveGroupWithContents( " << theGroup << " )";

  // Remove group
  RemoveGroup( theGroup );

  SMESH_CATCH( SMESH::throwCorbaException );
}

//================================================================================
/*!
 * \brief Get the list of groups existing in the mesh
 *  \retval SMESH::ListOfGroups * - list of groups
 */
//================================================================================

SMESH::ListOfGroups * SMESH_Mesh_i::GetGroups() throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if (MYDEBUG) MESSAGE("GetGroups");

  SMESH::ListOfGroups_var aList = new SMESH::ListOfGroups();

  // Python Dump
  TPythonDump aPythonDump;
  if ( !_mapGroups.empty() )
  {
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
    aPythonDump << " ] = " << SMESH::SMESH_Mesh_var(_this()) << ".GetGroups()";
  }
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
/*!
 * New group including all mesh elements present in initial groups is created.
 */
//=============================================================================

SMESH::SMESH_Group_ptr SMESH_Mesh_i::UnionGroups( SMESH::SMESH_GroupBase_ptr theGroup1,
                                                  SMESH::SMESH_GroupBase_ptr theGroup2,
                                                  const char*                theName )
  throw (SALOME::SALOME_Exception)
{
  SMESH::SMESH_Group_var aResGrp;

  SMESH_TRY;
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  if ( theGroup1->_is_nil() || theGroup2->_is_nil() )
    THROW_SALOME_CORBA_EXCEPTION("UnionGroups(): NULL Group",
                                 SALOME::BAD_PARAM);
  if ( theGroup1->GetType() != theGroup2->GetType() )
    THROW_SALOME_CORBA_EXCEPTION("UnionGroups(): different group types",
                                 SALOME::BAD_PARAM);
  TPythonDump pyDump;

  // Create Union
  aResGrp = CreateGroup( theGroup1->GetType(), theName );
  if ( aResGrp->_is_nil() )
    return SMESH::SMESH_Group::_nil();

  aResGrp->AddFrom( theGroup1 );
  aResGrp->AddFrom( theGroup2 );

  // Update Python script
  pyDump << aResGrp << " = " << SMESH::SMESH_Mesh_var(_this())
         << ".UnionGroups( " << theGroup1 << ", " << theGroup2 << ", '" << theName << "' )";

  SMESH_CATCH( SMESH::throwCorbaException );

  return aResGrp._retn();
}

//=============================================================================
/*!
 * \brief New group including all mesh elements present in initial groups is created.
 *  \param theGroups list of groups
 *  \param theName name of group to be created
 *  \return pointer to the new group
 */
//=============================================================================

SMESH::SMESH_Group_ptr SMESH_Mesh_i::UnionListOfGroups(const SMESH::ListOfGroups& theGroups,
                                                       const char*                theName )
  throw (SALOME::SALOME_Exception)
{
  SMESH::SMESH_Group_var aResGrp;

  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  if ( !theName )
    return SMESH::SMESH_Group::_nil();

  SMESH_TRY;

  // check types
  SMESH::ElementType aType = SMESH::ALL;
  for ( int g = 0, n = theGroups.length(); g < n; g++ )
  {
    SMESH::SMESH_GroupBase_var aGrp = theGroups[ g ];
    if ( CORBA::is_nil( aGrp ) )
      continue;
    if ( aType == SMESH::ALL )
      aType = aGrp->GetType();
    else if ( aType != aGrp->GetType() )
      THROW_SALOME_CORBA_EXCEPTION("UnionListOfGroups(): different group types",
                                   SALOME::BAD_PARAM);
  }
  if ( aType == SMESH::ALL )
    return SMESH::SMESH_Group::_nil();

  TPythonDump pyDump;

  // Create Union
  aResGrp = CreateGroup( aType, theName );
  if ( aResGrp->_is_nil() )
    return SMESH::SMESH_Group::_nil();

  pyDump << aResGrp << " = " << SMESH::SMESH_Mesh_var(_this()) << ".UnionListOfGroups([ ";
  for ( int g = 0, n = theGroups.length(); g < n; g++ )
  {
    SMESH::SMESH_GroupBase_var aGrp = theGroups[ g ];
    if ( !CORBA::is_nil( aGrp ) )
    {
      aResGrp->AddFrom( aGrp );
      if ( g > 0 ) pyDump << ", ";
      pyDump << aGrp;
    }
  }
  pyDump << " ], '" << theName << "' )";

  SMESH_CATCH( SMESH::throwCorbaException );

  return aResGrp._retn();
}

//=============================================================================
/*!
 *  New group is created. All mesh elements that are
 *  present in both initial groups are added to the new one.
 */
//=============================================================================

SMESH::SMESH_Group_ptr SMESH_Mesh_i::IntersectGroups( SMESH::SMESH_GroupBase_ptr theGroup1,
                                                      SMESH::SMESH_GroupBase_ptr theGroup2,
                                                      const char*                theName )
  throw (SALOME::SALOME_Exception)
{
  SMESH::SMESH_Group_var aResGrp;

  SMESH_TRY;

  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  if ( theGroup1->_is_nil() || theGroup2->_is_nil() )
    THROW_SALOME_CORBA_EXCEPTION("IntersectGroups(): NULL Group",
                                 SALOME::BAD_PARAM);
  if ( theGroup1->GetType() != theGroup2->GetType() )
    THROW_SALOME_CORBA_EXCEPTION("IntersectGroups(): different group types",
                                 SALOME::BAD_PARAM);
  TPythonDump pyDump;

  // Create Intersection
  aResGrp = CreateGroup( theGroup1->GetType(), theName );
  if ( aResGrp->_is_nil() )
    return aResGrp._retn();

  SMESHDS_GroupBase* groupDS1 = 0;
  if ( SMESH_GroupBase_i* grp_i = SMESH::DownCast< SMESH_GroupBase_i* >( theGroup1 ))
    groupDS1 = grp_i->GetGroupDS();

  SMESHDS_GroupBase* groupDS2 = 0;
  if ( SMESH_GroupBase_i* grp_i = SMESH::DownCast< SMESH_GroupBase_i* >( theGroup2 ))
    groupDS2 = grp_i->GetGroupDS();

  SMESHDS_Group* resGroupDS = 0;
  if ( SMESH_GroupBase_i* grp_i = SMESH::DownCast< SMESH_GroupBase_i* >( aResGrp ))
    resGroupDS = dynamic_cast<SMESHDS_Group*>( grp_i->GetGroupDS() );

  if ( groupDS1 && groupDS2 && resGroupDS && !groupDS2->IsEmpty() )
  {
    SMDS_ElemIteratorPtr elemIt1 = groupDS1->GetElements();
    while ( elemIt1->more() )
    {
      const SMDS_MeshElement* e = elemIt1->next();
      if ( groupDS2->Contains( e ))
        resGroupDS->SMDSGroup().Add( e );
    }
  }
  // Update Python script
  pyDump << aResGrp << " = " << SMESH::SMESH_Mesh_var(_this()) << ".IntersectGroups( "
         << theGroup1 << ", " << theGroup2 << ", '" << theName << "')";

  SMESH_CATCH( SMESH::throwCorbaException );

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
SMESH::SMESH_Group_ptr
SMESH_Mesh_i::IntersectListOfGroups(const SMESH::ListOfGroups& theGroups,
                                    const char*                theName )
  throw (SALOME::SALOME_Exception)
{
  SMESH::SMESH_Group_var aResGrp;

  SMESH_TRY;

  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  if ( !theName )
    return SMESH::SMESH_Group::_nil();

  // check types and get SMESHDS_GroupBase's
  SMESH::ElementType aType = SMESH::ALL;
  vector< SMESHDS_GroupBase* > groupVec;
  for ( int g = 0, n = theGroups.length(); g < n; g++ )
  {
    SMESH::SMESH_GroupBase_var aGrp = theGroups[ g ];
    if ( CORBA::is_nil( aGrp ) )
      continue;
    if ( aType == SMESH::ALL )
      aType = aGrp->GetType();
    else if ( aType != aGrp->GetType() )
      THROW_SALOME_CORBA_EXCEPTION("IntersectListOfGroups(): different group types",
                                   SALOME::BAD_PARAM);

    if ( SMESH_GroupBase_i* grp_i = SMESH::DownCast< SMESH_GroupBase_i* >( aGrp ))
      if ( SMESHDS_GroupBase* grpDS = grp_i->GetGroupDS() )
      {
        if ( grpDS->IsEmpty() )
        {
          groupVec.clear();
          break;
        }
        groupVec.push_back( grpDS );
      }
  }
  if ( aType == SMESH::ALL ) // all groups are nil
    return SMESH::SMESH_Group::_nil();

  TPythonDump pyDump;

  // Create a group
  aResGrp = CreateGroup( aType, theName );

  SMESHDS_Group* resGroupDS = 0;
  if ( SMESH_GroupBase_i* grp_i = SMESH::DownCast< SMESH_GroupBase_i* >( aResGrp ))
    resGroupDS = dynamic_cast<SMESHDS_Group*>( grp_i->GetGroupDS() );
  if ( !resGroupDS || groupVec.empty() )
    return aResGrp._retn();

  // Fill the group
  size_t i, nb = groupVec.size();
  SMDS_ElemIteratorPtr elemIt1 = groupVec[0]->GetElements();
  while ( elemIt1->more() )
  {
    const SMDS_MeshElement* e = elemIt1->next();
    bool inAll = true;
    for ( i = 1; ( i < nb && inAll ); ++i )
      inAll = groupVec[i]->Contains( e );

    if ( inAll )
      resGroupDS->SMDSGroup().Add( e );
  }

  // Update Python script
  pyDump << aResGrp << " = " << SMESH::SMESH_Mesh_var( _this() )
         << ".IntersectListOfGroups( " << theGroups << ", '" << theName << "' )";

  SMESH_CATCH( SMESH::throwCorbaException );

  return aResGrp._retn();
}

//=============================================================================
/*! 
 *  New group is created. All mesh elements that are present in
 *  a main group but is not present in a tool group are added to the new one
 */
//=============================================================================

SMESH::SMESH_Group_ptr SMESH_Mesh_i::CutGroups( SMESH::SMESH_GroupBase_ptr theGroup1,
                                                SMESH::SMESH_GroupBase_ptr theGroup2,
                                                const char*                theName )
  throw (SALOME::SALOME_Exception)
{
  SMESH::SMESH_Group_var aResGrp;

  SMESH_TRY;

  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  if ( theGroup1->_is_nil() || theGroup2->_is_nil() )
    THROW_SALOME_CORBA_EXCEPTION("CutGroups(): NULL Group",
                                 SALOME::BAD_PARAM);
  if ( theGroup1->GetType() != theGroup2->GetType() )
    THROW_SALOME_CORBA_EXCEPTION("CutGroups(): different group types",
                                 SALOME::BAD_PARAM);
  TPythonDump pyDump;

  aResGrp = CreateGroup( theGroup1->GetType(), theName );
  if ( aResGrp->_is_nil() )
    return aResGrp._retn();

  SMESHDS_GroupBase* groupDS1 = 0;
  if ( SMESH_GroupBase_i* grp_i = SMESH::DownCast< SMESH_GroupBase_i* >( theGroup1 ))
    groupDS1 = grp_i->GetGroupDS();

  SMESHDS_GroupBase* groupDS2 = 0;
  if ( SMESH_GroupBase_i* grp_i = SMESH::DownCast< SMESH_GroupBase_i* >( theGroup2 ))
    groupDS2 = grp_i->GetGroupDS();

  SMESHDS_Group* resGroupDS = 0;
  if ( SMESH_GroupBase_i* grp_i = SMESH::DownCast< SMESH_GroupBase_i* >( aResGrp ))
    resGroupDS = dynamic_cast<SMESHDS_Group*>( grp_i->GetGroupDS() );

  if ( groupDS1 && groupDS2 && resGroupDS )
  {
    SMDS_ElemIteratorPtr elemIt1 = groupDS1->GetElements();
    while ( elemIt1->more() )
    {
      const SMDS_MeshElement* e = elemIt1->next();
      if ( !groupDS2->Contains( e ))
        resGroupDS->SMDSGroup().Add( e );
    }
  }
  // Update Python script
  pyDump << aResGrp << " = " << SMESH::SMESH_Mesh_var(_this()) << ".CutGroups( "
         << theGroup1 << ", " << theGroup2 << ", '" << theName << "')";

  SMESH_CATCH( SMESH::throwCorbaException );

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
SMESH::SMESH_Group_ptr
SMESH_Mesh_i::CutListOfGroups(const SMESH::ListOfGroups& theMainGroups, 
                              const SMESH::ListOfGroups& theToolGroups, 
                              const char*                theName )
  throw (SALOME::SALOME_Exception)
{
  SMESH::SMESH_Group_var aResGrp;

  SMESH_TRY;

  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  if ( !theName )
    return SMESH::SMESH_Group::_nil();

  // check types and get SMESHDS_GroupBase's
  SMESH::ElementType aType = SMESH::ALL;
  vector< SMESHDS_GroupBase* >   toolGroupVec;
  vector< SMDS_ElemIteratorPtr > mainIterVec;

  for ( int g = 0, n = theMainGroups.length(); g < n; g++ )
  {
    SMESH::SMESH_GroupBase_var aGrp = theMainGroups[ g ];
    if ( CORBA::is_nil( aGrp ) )
      continue;
    if ( aType == SMESH::ALL )
      aType = aGrp->GetType();
    else if ( aType != aGrp->GetType() )
      THROW_SALOME_CORBA_EXCEPTION("CutListOfGroups(): different group types",
                                   SALOME::BAD_PARAM);
    if ( SMESH_GroupBase_i* grp_i = SMESH::DownCast< SMESH_GroupBase_i* >( aGrp ))
      if ( SMESHDS_GroupBase* grpDS = grp_i->GetGroupDS() )
        if ( !grpDS->IsEmpty() )
          mainIterVec.push_back( grpDS->GetElements() );
  }
  if ( aType == SMESH::ALL ) // all main groups are nil
    return SMESH::SMESH_Group::_nil();
  if ( mainIterVec.empty() ) // all main groups are empty
    return aResGrp._retn();

  for ( int g = 0, n = theToolGroups.length(); g < n; g++ )
  {
    SMESH::SMESH_GroupBase_var aGrp = theToolGroups[ g ];
    if ( CORBA::is_nil( aGrp ) )
      continue;
    if ( aType != aGrp->GetType() )
      THROW_SALOME_CORBA_EXCEPTION("CutListOfGroups(): different group types",
                                   SALOME::BAD_PARAM);
    if ( SMESH_GroupBase_i* grp_i = SMESH::DownCast< SMESH_GroupBase_i* >( aGrp ))
      if ( SMESHDS_GroupBase* grpDS = grp_i->GetGroupDS() )
        toolGroupVec.push_back( grpDS );
  }

  TPythonDump pyDump;

  // Create a group
  aResGrp = CreateGroup( aType, theName );

  SMESHDS_Group* resGroupDS = 0;
  if ( SMESH_GroupBase_i* grp_i = SMESH::DownCast< SMESH_GroupBase_i* >( aResGrp ))
    resGroupDS = dynamic_cast<SMESHDS_Group*>( grp_i->GetGroupDS() );
  if ( !resGroupDS )
    return aResGrp._retn();

  // Fill the group
  size_t i, nb = toolGroupVec.size();
  SMDS_ElemIteratorPtr mainElemIt
    ( new SMDS_IteratorOnIterators
      < const SMDS_MeshElement*, vector< SMDS_ElemIteratorPtr > >( mainIterVec ));
  while ( mainElemIt->more() )
  {
    const SMDS_MeshElement* e = mainElemIt->next();
    bool isIn = false;
    for ( i = 0; ( i < nb && !isIn ); ++i )
      isIn = toolGroupVec[i]->Contains( e );

    if ( !isIn )
      resGroupDS->SMDSGroup().Add( e );
  }

  // Update Python script
  pyDump << aResGrp << " = " << SMESH::SMESH_Mesh_var( _this() )
         << ".CutListOfGroups( " << theMainGroups << ", "
         << theToolGroups << ", '" << theName << "' )";

  SMESH_CATCH( SMESH::throwCorbaException );

  return aResGrp._retn();
}

namespace // functions making checks according to SMESH::NB_COMMON_NODES_ENUM
{
  bool isAllNodesCommon(int nbChecked, int nbCommon, int nbNodes, int nbCorners,
                        bool & toStopChecking )
  {
    toStopChecking = ( nbCommon < nbChecked );
    return nbCommon == nbNodes;
  }
  bool isMainNodesCommon(int nbChecked, int nbCommon, int nbNodes, int nbCorners,
                         bool & toStopChecking )
  {
    toStopChecking = ( nbCommon < nbChecked || nbChecked >= nbCorners );
    return nbCommon == nbCorners;
  }
  bool isAtLeastOneNodeCommon(int nbChecked, int nbCommon, int nbNodes, int nbCorners,
                              bool & toStopChecking )
  {
    return nbCommon > 0;
  }
  bool isMajorityOfNodesCommon(int nbChecked, int nbCommon, int nbNodes, int nbCorners,
                               bool & toStopChecking )
  {
    return nbCommon >= (nbNodes+1) / 2;
  }
}

//=============================================================================
/*!
 * Create a group of entities basing on nodes of other groups.
 *  \param [in] theGroups - list of either groups, sub-meshes or filters.
 *  \param [in] anElemType - a type of elements to include to the new group.
 *  \param [in] theName - a name of the new group.
 *  \param [in] theNbCommonNodes - criterion of inclusion of an element to the new group.
 *  \param [in] theUnderlyingOnly - if \c True, an element is included to the
 *         new group provided that it is based on nodes of an element of \a aListOfGroups
 *  \return SMESH_Group - the created group
*/
// IMP 19939, bug 22010, IMP 22635
//=============================================================================

SMESH::SMESH_Group_ptr
SMESH_Mesh_i::CreateDimGroup(const SMESH::ListOfIDSources& theGroups,
                             SMESH::ElementType            theElemType,
                             const char*                   theName,
                             SMESH::NB_COMMON_NODES_ENUM   theNbCommonNodes,
                             CORBA::Boolean                theUnderlyingOnly)
  throw (SALOME::SALOME_Exception)
{
  SMESH::SMESH_Group_var aResGrp;

  SMESH_TRY;
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESHDS_Mesh* aMeshDS = _impl->GetMeshDS();

  if ( !theName || !aMeshDS )
    return SMESH::SMESH_Group::_nil();

  SMDSAbs_ElementType anElemType = (SMDSAbs_ElementType)theElemType;

  bool (*isToInclude)(int nbChecked, int nbCommon, int nbNodes, int nbCorners, bool & toStop);
  SMESH_Comment nbCoNoStr( "SMESH.");
  switch ( theNbCommonNodes ) {
  case SMESH::ALL_NODES   : isToInclude = isAllNodesCommon;        nbCoNoStr<<"ALL_NODES"   ;break;
  case SMESH::MAIN        : isToInclude = isMainNodesCommon;       nbCoNoStr<<"MAIN"        ;break;
  case SMESH::AT_LEAST_ONE: isToInclude = isAtLeastOneNodeCommon;  nbCoNoStr<<"AT_LEAST_ONE";break;
  case SMESH::MAJORITY    : isToInclude = isMajorityOfNodesCommon; nbCoNoStr<<"MAJORITY"    ;break;
  default: return aResGrp._retn();
  }
  int nbChecked, nbCommon, nbNodes, nbCorners;

  // Create a group

  TPythonDump pyDump;

  aResGrp = CreateGroup( theElemType, theName );
  if ( aResGrp->_is_nil() )
    return SMESH::SMESH_Group::_nil();

  SMESHDS_GroupBase* groupBaseDS =
    SMESH::DownCast<SMESH_GroupBase_i*>( aResGrp )->GetGroupDS();
  SMDS_MeshGroup& resGroupCore = static_cast< SMESHDS_Group* >( groupBaseDS )->SMDSGroup();

  vector<bool> isNodeInGroups;

  for ( int g = 0, n = theGroups.length(); g < n; g++ ) // loop on theGroups
  {
    SMESH::SMESH_IDSource_var aGrp = theGroups[ g ];
    if ( CORBA::is_nil( aGrp ) )
      continue;
    SMESH::SMESH_Mesh_var mesh = aGrp->GetMesh();
    if ( mesh->_is_nil() || mesh->GetId() != this->GetId() )
      continue;

    SMDS_ElemIteratorPtr elIt = GetElements( aGrp, SMESH::ALL );
    if ( !elIt ) continue;

    if ( theElemType == SMESH::NODE ) // get all nodes of elements
    {
      while ( elIt->more() ) {
        const SMDS_MeshElement* el = elIt->next();
        SMDS_ElemIteratorPtr nIt = el->nodesIterator();
        while ( nIt->more() )
          resGroupCore.Add( nIt->next() );
      }
    }
    // get elements of theElemType based on nodes of every element of group
    else if ( theUnderlyingOnly )
    {
      while ( elIt->more() )
      {
        const SMDS_MeshElement* el = elIt->next(); // an element of ref group
        TIDSortedElemSet elNodes( el->begin_nodes(), el->end_nodes() );
        TIDSortedElemSet checkedElems;
        SMDS_NodeIteratorPtr nIt = el->nodeIterator();
        while ( nIt->more() )
        {
          const SMDS_MeshNode* n = nIt->next();
          SMDS_ElemIteratorPtr elOfTypeIt = n->GetInverseElementIterator( anElemType );
          // check nodes of elements of theElemType around el
          while ( elOfTypeIt->more() )
          {
            const SMDS_MeshElement* elOfType = elOfTypeIt->next();
            if ( !checkedElems.insert( elOfType ).second ) continue;
            nbNodes   = elOfType->NbNodes();
            nbCorners = elOfType->NbCornerNodes();
            nbCommon  = 0;
            bool toStopChecking = false;
            SMDS_ElemIteratorPtr nIt2 = elOfType->nodesIterator();
            for ( nbChecked = 1; nIt2->more() && !toStopChecking; ++nbChecked )
              if ( elNodes.count( nIt2->next() ) &&
                   isToInclude( nbChecked, ++nbCommon, nbNodes, nbCorners, toStopChecking ))
              {
                resGroupCore.Add( elOfType );
                break;
              }
          }
        }
      }
    }
    // get all nodes of elements of groups
    else
    {
      while ( elIt->more() )
      {
        const SMDS_MeshElement* el = elIt->next(); // an element of group
        SMDS_NodeIteratorPtr nIt = el->nodeIterator();
        while ( nIt->more() )
        {
          const SMDS_MeshNode* n = nIt->next();
          if ( n->GetID() >= (int) isNodeInGroups.size() )
            isNodeInGroups.resize( n->GetID() + 1, false );
          isNodeInGroups[ n->GetID() ] = true;
        }
      }
    }
  }

  // Get elements of theElemType based on a certain number of nodes of elements of groups
  if ( !theUnderlyingOnly && !isNodeInGroups.empty() )
  {
    const SMDS_MeshNode* n;
    vector<bool> isElemChecked( aMeshDS->MaxElementID() + 1 );
    const int isNodeInGroupsSize = isNodeInGroups.size();
    for ( int iN = 0; iN < isNodeInGroupsSize; ++iN )
    {
      if ( !isNodeInGroups[ iN ] ||
           !( n = aMeshDS->FindNode( iN )))
        continue;

      // check nodes of elements of theElemType around n
      SMDS_ElemIteratorPtr elOfTypeIt = n->GetInverseElementIterator( anElemType );
      while ( elOfTypeIt->more() )
      {
        const SMDS_MeshElement*  elOfType = elOfTypeIt->next();
        vector<bool>::reference isChecked = isElemChecked[ elOfType->GetID() ];
        if ( isChecked )
          continue;
        isChecked = true;

        nbNodes   = elOfType->NbNodes();
        nbCorners = elOfType->NbCornerNodes();
        nbCommon  = 0;
        bool toStopChecking = false;
        SMDS_ElemIteratorPtr nIt = elOfType->nodesIterator();
        for ( nbChecked = 1; nIt->more() && !toStopChecking; ++nbChecked )
        {
          const int nID = nIt->next()->GetID();
          if ( nID < isNodeInGroupsSize && isNodeInGroups[ nID ] &&
               isToInclude( nbChecked, ++nbCommon, nbNodes, nbCorners, toStopChecking ))
          {
            resGroupCore.Add( elOfType );
            break;
          }
        }
      }
    }
  }

  // Update Python script
  pyDump << aResGrp << " = " << SMESH::SMESH_Mesh_var( _this())
         << ".CreateDimGroup( "
         << theGroups << ", " << theElemType << ", '" << theName << "', "
         << nbCoNoStr << ", " << theUnderlyingOnly << ")";

  SMESH_CATCH( SMESH::throwCorbaException );

  return aResGrp._retn();
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
  SALOMEDS::Study_var    study   = _gen_i->GetCurrentStudy();
  SALOMEDS::SObject_wrap groupSO = _gen_i->ObjectToSObject( study, theGeomObj );
  if ( groupSO->_is_nil() )
    return;
  // group indices
  GEOM::GEOM_Gen_var geomGen = _gen_i->GetGeomEngine();
  GEOM::GEOM_IGroupOperations_wrap groupOp =
    geomGen->GetIGroupOperations( _gen_i->GetCurrentStudyID() );
  GEOM::ListOfLong_var ids = groupOp->GetObjects( theGeomObj );

  // store data
  _geomGroupData.push_back( TGeomGroupData() );
  TGeomGroupData & groupData = _geomGroupData.back();
  // entry
  CORBA::String_var entry = groupSO->GetID();
  groupData._groupEntry = entry.in();
  // indices
  for ( CORBA::ULong i = 0; i < ids->length(); ++i )
    groupData._indices.insert( ids[i] );
  // SMESH object
  groupData._smeshObject = CORBA::Object::_duplicate( theSmeshObj );
  // shape index in SMESHDS
  // TopoDS_Shape shape = _gen_i->GeomObjectToShape( theGeomObj );
  // groupData._dsID = shape.IsNull() ? 0 : _impl->GetSubMesh( shape )->GetId();
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
  SALOMEDS::SObject_wrap groupSO = study->FindObjectID( groupData._groupEntry.c_str() );
  if ( !groupSO->_is_nil() )
  {
    CORBA::Object_var groupObj = _gen_i->SObjectToObject( groupSO );
    if ( CORBA::is_nil( groupObj )) return newShape;
    GEOM::GEOM_Object_var geomGroup = GEOM::GEOM_Object::_narrow( groupObj );

    // get indices of group items
    set<int> curIndices;
    GEOM::GEOM_Gen_var geomGen = _gen_i->GetGeomEngine();
    GEOM::GEOM_IGroupOperations_wrap groupOp =
      geomGen->GetIGroupOperations( _gen_i->GetCurrentStudyID() );
    GEOM::ListOfLong_var   ids = groupOp->GetObjects( geomGroup );
    for ( CORBA::ULong i = 0; i < ids->length(); ++i )
      curIndices.insert( ids[i] );

    if ( groupData._indices == curIndices )
      return newShape; // group not changed

    // update data
    groupData._indices = curIndices;

    GEOM_Client* geomClient = _gen_i->GetShapeReader();
    if ( !geomClient ) return newShape;
    CORBA::String_var groupIOR = geomGen->GetStringFromIOR( geomGroup );
    geomClient->RemoveShapeFromBuffer( groupIOR.in() );
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

namespace
{
  //-----------------------------------------------------------------------------
  /*!
   * \brief Storage of shape and index used in CheckGeomGroupModif()
   */
  struct TIndexedShape
  {
    int          _index;
    TopoDS_Shape _shape;
    TIndexedShape( int i, const TopoDS_Shape& s ):_index(i), _shape(s) {}
  };
  //-----------------------------------------------------------------------------
  /*!
   * \brief Data to re-create a group on geometry
   */
  struct TGroupOnGeomData
  {
    int                 _oldID;
    int                 _shapeID;
    SMDSAbs_ElementType _type;
    std::string         _name;
    Quantity_Color      _color;
  };
}

//=============================================================================
/*!
 * \brief Update data if geometry changes
 *
 * Issue 0022501
 */
//=============================================================================

void SMESH_Mesh_i::CheckGeomModif()
{
  if ( !_impl->HasShapeToMesh() ) return;

  SALOMEDS::Study_var study = _gen_i->GetCurrentStudy();
  if ( study->_is_nil() ) return;

  GEOM::GEOM_Object_var mainGO = _gen_i->ShapeToGeomObject( _impl->GetShapeToMesh() );
  //if ( mainGO->_is_nil() ) return;

  // Update after group modification

  if ( mainGO->_is_nil() || /* shape was removed from GEOM_Client by newGroupShape()
                               called by other mesh (IPAL52735) */
       mainGO->GetType() == GEOM_GROUP ||
       mainGO->GetTick() == _mainShapeTick )
  {
    CheckGeomGroupModif();
    return;
  }

  // Update after shape transformation like Translate

  GEOM_Client* geomClient = _gen_i->GetShapeReader();
  if ( !geomClient ) return;
  GEOM::GEOM_Gen_var geomGen = _gen_i->GetGeomEngine();
  if ( geomGen->_is_nil() ) return;

  CORBA::String_var ior = geomGen->GetStringFromIOR( mainGO );
  geomClient->RemoveShapeFromBuffer( ior.in() );

  // Update data taking into account that
  // all sub-shapes change but IDs of sub-shapes remain (except for geom groups)

  _impl->Clear();
  TopoDS_Shape newShape = _gen_i->GeomObjectToShape( mainGO );
  if ( newShape.IsNull() )
    return;

  _mainShapeTick = mainGO->GetTick();

  SMESHDS_Mesh * meshDS = _impl->GetMeshDS();

  // store data of groups on geometry
  vector< TGroupOnGeomData > groupsData;
  const set<SMESHDS_GroupBase*>& groups = meshDS->GetGroups();
  groupsData.reserve( groups.size() );
  set<SMESHDS_GroupBase*>::const_iterator g = groups.begin();
  for ( ; g != groups.end(); ++g )
    if ( const SMESHDS_GroupOnGeom* group = dynamic_cast< SMESHDS_GroupOnGeom* >( *g ))
    {
      TGroupOnGeomData data;
      data._oldID   = group->GetID();
      data._shapeID = meshDS->ShapeToIndex( group->GetShape() );
      data._type    = group->GetType();
      data._name    = group->GetStoreName();
      data._color   = group->GetColor();
      groupsData.push_back( data );
    }
  // store assigned hypotheses
  vector< pair< int, THypList > > ids2Hyps;
  const ShapeToHypothesis & hyps = meshDS->GetHypotheses();
  for ( ShapeToHypothesis::Iterator s2hyps( hyps ); s2hyps.More(); s2hyps.Next() )
  {
    const TopoDS_Shape& s = s2hyps.Key();
    const THypList&  hyps = s2hyps.ChangeValue();
    ids2Hyps.push_back( make_pair( meshDS->ShapeToIndex( s ), hyps ));
  }

  // change shape to mesh
  int oldNbSubShapes = meshDS->MaxShapeIndex();
  _impl->ShapeToMesh( TopoDS_Shape() );
  _impl->ShapeToMesh( newShape );

  // re-add shapes of geom groups
  list<TGeomGroupData>::iterator data = _geomGroupData.begin();
  for ( ; data != _geomGroupData.end(); ++data )
  {
    TopoDS_Shape newShape = newGroupShape( *data );
    if ( !newShape.IsNull() )
    {
      if ( meshDS->ShapeToIndex( newShape ) > 0 ) // a group reduced to one sub-shape
      {
        TopoDS_Compound compound;
        BRep_Builder().MakeCompound( compound );
        BRep_Builder().Add( compound, newShape );
        newShape = compound;
      }
      _impl->GetSubMesh( newShape );
    }
  }
  if ( oldNbSubShapes != meshDS->MaxShapeIndex() )
    THROW_SALOME_CORBA_EXCEPTION( "SMESH_Mesh_i::CheckGeomModif() bug",
                                  SALOME::INTERNAL_ERROR );

  // re-assign hypotheses
  for ( size_t i = 0; i < ids2Hyps.size(); ++i )
  {
    const TopoDS_Shape& s = meshDS->IndexToShape( ids2Hyps[i].first );
    const THypList&  hyps = ids2Hyps[i].second;
    THypList::const_iterator h = hyps.begin();
    for ( ; h != hyps.end(); ++h )
      _impl->AddHypothesis( s, (*h)->GetID() );
  }

  // restore groups
  for ( size_t i = 0; i < groupsData.size(); ++i )
  {
    const TGroupOnGeomData& data = groupsData[i];

    map<int, SMESH::SMESH_GroupBase_ptr>::iterator i2g = _mapGroups.find( data._oldID );
    if ( i2g == _mapGroups.end() ) continue;

    SMESH_GroupBase_i* gr_i = SMESH::DownCast<SMESH_GroupBase_i*>( i2g->second );
    if ( !gr_i ) continue;

    int id;
    SMESH_Group* g = _impl->AddGroup( data._type, data._name.c_str(), id,
                                      meshDS->IndexToShape( data._shapeID ));
    if ( !g )
    {
      _mapGroups.erase( i2g );
    }
    else
    {
      g->GetGroupDS()->SetColor( data._color );
      gr_i->changeLocalId( id );
      _mapGroups[ id ] = i2g->second;
      if ( data._oldID != id )
        _mapGroups.erase( i2g );
    }
  }

  // update _mapSubMesh
  map<int, ::SMESH_subMesh*>::iterator i_sm = _mapSubMesh.begin();
  for ( ; i_sm != _mapSubMesh.end(); ++i_sm )
    i_sm->second = _impl->GetSubMesh( meshDS->IndexToShape( i_sm->first ));

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

    if ( _preMeshInfo )
      _preMeshInfo->ForgetOrLoad();

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
      if ( !_mapSubMeshIor.count( oldID ))
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
      // set new shape to mesh -> DS of sub-meshes and geom groups are deleted
      _impl->Clear();
      _impl->ShapeToMesh( TopoDS_Shape() ); // IPAL52730
      _impl->ShapeToMesh( newShape );

      // reassign hypotheses
      TShapeHypList::iterator indS_hyps = assignedHyps.begin();
      for ( ; indS_hyps != assignedHyps.end(); ++indS_hyps )
      {
        TIndexedShape&                   geom = indS_hyps->first;
        list<const SMESHDS_Hypothesis*>& hyps = indS_hyps->second;
        int oldID = geom._index;
        int newID = meshDS->ShapeToIndex( geom._shape );
        if ( oldID == 1 ) { // main shape
          newID = 1;
          geom._shape = newShape;
        }
        if ( !newID )
          continue;
        for ( hypIt = hyps.begin(); hypIt != hyps.end(); ++hypIt )
          _impl->AddHypothesis( geom._shape, (*hypIt)->GetID());
        // care of sub-meshes
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
        SALOMEDS::SObject_wrap groupSO = _gen_i->ObjectToSObject( study,_mapGroups[oldID] );
        CORBA::String_var      name    = groupSO->GetName();
        // update
        SMESH_GroupBase_i*  group_i    = SMESH::DownCast<SMESH_GroupBase_i*>(_mapGroups[oldID] );
        int newID;
        if ( group_i && _impl->AddGroup( geomType->second, name.in(), newID, geom._shape ))
          group_i->changeLocalId( newID );
      }

      break; // everything has been updated

    } // update mesh
  } // loop on group data

  // Update icons

  CORBA::Long newNbEntities = NbNodes() + NbElements();
  list< SALOMEDS::SObject_wrap > soToUpdateIcons;
  if ( newNbEntities != nbEntities )
  {
    // Add all SObjects with icons to soToUpdateIcons
    soToUpdateIcons.push_back( _gen_i->ObjectToSObject( study, _this() )); // mesh

    for (map<int, SMESH::SMESH_subMesh_ptr>::iterator i_sm = _mapSubMeshIor.begin();
         i_sm != _mapSubMeshIor.end(); ++i_sm ) // submeshes
      soToUpdateIcons.push_back( _gen_i->ObjectToSObject( study, i_sm->second ));

    for ( map<int, SMESH::SMESH_GroupBase_ptr>::iterator i_gr = _mapGroups.begin();
          i_gr != _mapGroups.end(); ++i_gr ) // groups
      soToUpdateIcons.push_back( _gen_i->ObjectToSObject( study, i_gr->second ));
  }

  list< SALOMEDS::SObject_wrap >::iterator so = soToUpdateIcons.begin();
  for ( ; so != soToUpdateIcons.end(); ++so )
    _gen_i->SetPixMap( *so, "ICON_SMESH_TREE_MESH_WARN" );
}

//=============================================================================
/*!
 * \brief Create standalone group from a group on geometry or filter
 */
//=============================================================================

SMESH::SMESH_Group_ptr SMESH_Mesh_i::ConvertToStandalone( SMESH::SMESH_GroupBase_ptr theGroup )
  throw (SALOME::SALOME_Exception)
{
  SMESH::SMESH_Group_var aGroup;

  SMESH_TRY;

  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  if ( theGroup->_is_nil() )
    return aGroup._retn();

  SMESH_GroupBase_i* aGroupToRem = SMESH::DownCast<SMESH_GroupBase_i*>( theGroup );
  if ( !aGroupToRem )
    return aGroup._retn();

  const bool isOnFilter = ( SMESH::DownCast< SMESH_GroupOnFilter_i* > ( theGroup ));

  const int anId = aGroupToRem->GetLocalID();
  if ( !_impl->ConvertToStandalone( anId ) )
    return aGroup._retn();
  removeGeomGroupData( theGroup );

  SMESH_GroupBase_i* aGroupImpl = new SMESH_Group_i( SMESH_Gen_i::GetPOA(), this, anId );

  // remove old instance of group from own map
  { SMESH::SMESH_GroupBase_var var( _mapGroups[anId] ); } // decref CORBA object
  _mapGroups.erase( anId );

  SALOMEDS::StudyBuilder_var builder;
  SALOMEDS::SObject_wrap     aGroupSO;
  SALOMEDS::Study_var        aStudy = _gen_i->GetCurrentStudy();
  if ( !aStudy->_is_nil() ) {
    builder  = aStudy->NewBuilder();
    aGroupSO = _gen_i->ObjectToSObject( aStudy, theGroup );
    if ( !aGroupSO->_is_nil() )
    {
      // remove reference to geometry
      SALOMEDS::ChildIterator_wrap chItr = aStudy->NewChildIterator(aGroupSO);
      for ( ; chItr->More(); chItr->Next() )
        // Remove group's child SObject
        builder->RemoveObject( chItr->Value() );

      // Update Python script
      TPythonDump() << aGroupSO << " = " << SMESH::SMESH_Mesh_var(_this())
                    << ".ConvertToStandalone( " << aGroupSO << " )";

      // change icon of Group on Filter
      if ( isOnFilter )
      {
        SMESH::array_of_ElementType_var elemTypes = aGroupImpl->GetTypes();
        const int isEmpty = ( elemTypes->length() == 0 );
        if ( !isEmpty )
        {
          SALOMEDS::GenericAttribute_wrap anAttr =
            builder->FindOrCreateAttribute( aGroupSO, "AttributePixMap" );
          SALOMEDS::AttributePixMap_wrap pm = anAttr;
          pm->SetPixMap( "ICON_SMESH_TREE_GROUP" );
        }
      }
    }
  }

  // remember new group in own map
  aGroup = SMESH::SMESH_Group::_narrow( aGroupImpl->_this() );
  _mapGroups[anId] = SMESH::SMESH_GroupBase::_duplicate( aGroup );

  // register CORBA object for persistence
  _gen_i->RegisterObject( aGroup );

  CORBA::String_var ior = _gen_i->GetORB()->object_to_string( aGroup );
  builder->SetIOR( aGroupSO, ior.in() ); //  == aGroup->Register();
  //aGroup->Register();
  aGroupToRem->UnRegister();

  SMESH_CATCH( SMESH::throwCorbaException );

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
  TopoDS_Shape  myLocSubShape = _gen_i->GeomObjectToShape(theSubShapeObject);
  ::SMESH_subMesh * mySubMesh = _impl->GetSubMesh(myLocSubShape);
  const int         subMeshId = mySubMesh->GetId();

  SMESH_subMesh_i * subMeshServant = new SMESH_subMesh_i(myPOA, _gen_i, this, subMeshId);
  SMESH::SMESH_subMesh_var subMesh = subMeshServant->_this();

  _mapSubMesh   [subMeshId] = mySubMesh;
  _mapSubMesh_i [subMeshId] = subMeshServant;
  _mapSubMeshIor[subMeshId] = SMESH::SMESH_subMesh::_duplicate( subMesh );

  subMeshServant->Register();

  // register CORBA object for persistence
  int nextId = _gen_i->RegisterObject( subMesh );
  if(MYDEBUG) { MESSAGE( "Add submesh to map with id = "<< nextId); }
  else        { nextId = 0; } // avoid "unused variable" warning

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

bool SMESH_Mesh_i::removeSubMesh (SMESH::SMESH_subMesh_ptr theSubMesh,
                                  GEOM::GEOM_Object_ptr    theSubShapeObject )
{
  bool isHypChanged = false;
  if ( theSubMesh->_is_nil() /*|| theSubShapeObject->_is_nil()*/ )
    return isHypChanged;

  const int subMeshId = theSubMesh->GetId();

  if ( theSubShapeObject->_is_nil() )  // not published shape (IPAL13617)
  {
    if ( _mapSubMesh.find( subMeshId ) != _mapSubMesh.end())
    {
      TopoDS_Shape S = _mapSubMesh[ subMeshId ]->GetSubShape();
      if ( !S.IsNull() )
      {
        list<const SMESHDS_Hypothesis*> hyps = _impl->GetHypothesisList( S );
        isHypChanged = !hyps.empty();
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
      isHypChanged = ( aHypList->length() > 0 );
      for ( int i = 0, n = aHypList->length(); i < n; i++ ) {
        removeHypothesis( theSubShapeObject, aHypList[i] );
      }
    }
    catch( const SALOME::SALOME_Exception& ) {
      INFOS("SMESH_Mesh_i::removeSubMesh(): exception caught!");
    }
    removeGeomGroupData( theSubShapeObject );
  }

  // remove a servant
  std::map<int, SMESH_subMesh_i*>::iterator id_smi = _mapSubMesh_i.find( subMeshId );
  if ( id_smi != _mapSubMesh_i.end() )
    id_smi->second->UnRegister();

  // remove a CORBA object
  std::map<int, SMESH::SMESH_subMesh_ptr>::iterator id_smptr = _mapSubMeshIor.find( subMeshId );
  if ( id_smptr != _mapSubMeshIor.end() )
    SMESH::SMESH_subMesh_var( id_smptr->second );

  _mapSubMesh.erase(subMeshId);
  _mapSubMesh_i.erase(subMeshId);
  _mapSubMeshIor.erase(subMeshId);

  return isHypChanged;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

SMESH::SMESH_GroupBase_ptr SMESH_Mesh_i::createGroup (SMESH::ElementType        theElemType,
                                                      const char*               theName,
                                                      const TopoDS_Shape&       theShape,
                                                      const SMESH_PredicatePtr& thePredicate )
{
  std::string newName;
  if ( !theName || strlen( theName ) == 0 )
  {
    std::set< std::string > presentNames;
    std::map<int, SMESH::SMESH_GroupBase_ptr>::const_iterator i_gr = _mapGroups.begin();
    for ( ; i_gr != _mapGroups.end(); ++i_gr )
    {
      CORBA::String_var name = i_gr->second->GetName();
      presentNames.insert( name.in() );
    }
    do {
      newName = "noname_Group_" + SMESH_Comment( presentNames.size() + 1 );
    } while ( !presentNames.insert( newName ).second );
    theName = newName.c_str();
  }
  int anId;
  SMESH::SMESH_GroupBase_var aGroup;
  if ( _impl->AddGroup( (SMDSAbs_ElementType)theElemType, theName, anId, theShape, thePredicate ))
  {
    SMESH_GroupBase_i* aGroupImpl;
    if ( !theShape.IsNull() )
      aGroupImpl = new SMESH_GroupOnGeom_i( SMESH_Gen_i::GetPOA(), this, anId );
    else if ( thePredicate )
      aGroupImpl = new SMESH_GroupOnFilter_i( SMESH_Gen_i::GetPOA(), this, anId );
    else
      aGroupImpl = new SMESH_Group_i( SMESH_Gen_i::GetPOA(), this, anId );

    aGroup = aGroupImpl->_this();
    _mapGroups[anId] = SMESH::SMESH_GroupBase::_duplicate( aGroup );
    aGroupImpl->Register();

    // register CORBA object for persistence
    int nextId = _gen_i->RegisterObject( aGroup );
    if(MYDEBUG) { MESSAGE( "Add group to map with id = "<< nextId); }
    else        { nextId = 0; } // avoid "unused variable" warning in release mode

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
    SMESH::SMESH_GroupBase_var group = _mapGroups[theId];
    _mapGroups.erase( theId );
    removeGeomGroupData( group );
    if ( !_impl->RemoveGroup( theId ))
    {
      // it seems to be a call up from _impl caused by hyp modification (issue 0020918)
      RemoveGroup( group );
    }
    group->UnRegister();
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
  SMESH::log_array_var aLog;

  SMESH_TRY;
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

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

  SMESH_CATCH( SMESH::throwCorbaException );

  return aLog._retn();
}


//=============================================================================
/*!
 *
 */
//=============================================================================

void SMESH_Mesh_i::ClearLog() throw(SALOME::SALOME_Exception)
{
  SMESH_TRY;
  _impl->ClearLog();
  SMESH_CATCH( SMESH::throwCorbaException );
}

//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Long SMESH_Mesh_i::GetId()throw(SALOME::SALOME_Exception)
{
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
namespace
{
  //!< implementation of struct used to call methods of SMESH_Mesh_i from SMESH_Mesh
  // issue 0020918: groups removal is caused by hyp modification
  // issue 0021208: to forget not loaded mesh data at hyp modification
  struct TCallUp_i : public SMESH_Mesh::TCallUp
  {
    SMESH_Mesh_i* _mesh;
    TCallUp_i(SMESH_Mesh_i* mesh):_mesh(mesh) {}
    virtual void RemoveGroup (const int theGroupID) { _mesh->removeGroup( theGroupID ); }
    virtual void HypothesisModified ()              { _mesh->onHypothesisModified(); }
    virtual void Load ()                            { _mesh->Load(); }
  };
}

//================================================================================
/*!
 * \brief callback from _impl to forget not loaded mesh data (issue 0021208)
 */
//================================================================================

void SMESH_Mesh_i::onHypothesisModified()
{
  if ( _preMeshInfo )
    _preMeshInfo->ForgetOrLoad();
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
  if ( _impl )
    _impl->SetCallUp( new TCallUp_i(this));
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
  throw (SALOME::SALOME_Exception)
{
  SMESH::SMESH_MeshEditor_var aMeshEdVar;

  SMESH_TRY;
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  // Create MeshEditor
  if ( !_editor )
    _editor = new SMESH_MeshEditor_i( this, false );
  aMeshEdVar = _editor->_this();

  // Update Python script
  TPythonDump() << _editor << " = "
                << SMESH::SMESH_Mesh_var(_this()) << ".GetMeshEditor()";

  SMESH_CATCH( SMESH::throwCorbaException );

  return aMeshEdVar._retn();
}

//=============================================================================
/*!
 * Return mesh edition previewer
 */
//=============================================================================

SMESH::SMESH_MeshEditor_ptr SMESH_Mesh_i::GetMeshEditPreviewer()
  throw (SALOME::SALOME_Exception)
{
  SMESH::SMESH_MeshEditor_var aMeshEdVar;

  SMESH_TRY;
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  if ( !_previewEditor )
    _previewEditor = new SMESH_MeshEditor_i( this, true );
  aMeshEdVar = _previewEditor->_this();

  SMESH_CATCH( SMESH::throwCorbaException );

  return aMeshEdVar._retn();
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

//================================================================================
/*!
 * \brief Returns a random unique color
 */
//================================================================================

static SALOMEDS::Color getUniqueColor( const std::list<SALOMEDS::Color>& theReservedColors )
{
  const int MAX_ATTEMPTS = 100;
  int cnt = 0;
  double tolerance = 0.5;
  SALOMEDS::Color col;

  bool ok = false;
  while ( !ok ) {
    // generate random color
    double red    = (double)rand() / RAND_MAX;
    double green  = (double)rand() / RAND_MAX;
    double blue   = (double)rand() / RAND_MAX;
    // check existence in the list of the existing colors
    bool matched = false;
    std::list<SALOMEDS::Color>::const_iterator it;
    for ( it = theReservedColors.begin(); it != theReservedColors.end() && !matched; ++it ) {
      SALOMEDS::Color color = *it;
      double tol = fabs( color.R - red ) + fabs( color.G - green ) + fabs( color.B  - blue  );
      matched = tol < tolerance;
    }
    if ( (cnt+1) % 20 == 0 ) tolerance = tolerance/2;
    ok = ( ++cnt == MAX_ATTEMPTS ) || !matched;
    col.R = red;
    col.G = green;
    col.B = blue;
  }
  return col;
}

//=============================================================================
/*!
 * Sets auto-color mode. If it is on, groups get unique random colors
 */
//=============================================================================

void SMESH_Mesh_i::SetAutoColor(CORBA::Boolean theAutoColor) throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  _impl->SetAutoColor(theAutoColor);

  TPythonDump pyDump; // not to dump group->SetColor() from below code
  pyDump << SMESH::SMESH_Mesh_var(_this()) <<".SetAutoColor( "<<theAutoColor<<" )";

  std::list<SALOMEDS::Color> aReservedColors;
  map<int, SMESH::SMESH_GroupBase_ptr>::iterator it = _mapGroups.begin();
  for ( ; it != _mapGroups.end(); it++ ) {
    if ( CORBA::is_nil( it->second )) continue;
    SALOMEDS::Color aColor = getUniqueColor( aReservedColors );
    it->second->SetColor( aColor );
    aReservedColors.push_back( aColor );
  }
}

//=============================================================================
/*!
 * Returns true if auto-color mode is on
 */
//=============================================================================

CORBA::Boolean SMESH_Mesh_i::GetAutoColor() throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  return _impl->GetAutoColor();
}

//=============================================================================
/*!
 *  Checks if there are groups with equal names
 */
//=============================================================================

CORBA::Boolean SMESH_Mesh_i::HasDuplicatedGroupNamesMED()
{
  return _impl->HasDuplicatedGroupNamesMED();
}

//================================================================================
/*!
 * \brief Care of a file before exporting mesh into it
 */
//================================================================================

void SMESH_Mesh_i::PrepareForWriting (const char* file, bool overwrite)
{
  SMESH_File aFile( file );
  SMESH_Comment msg;
  if (aFile.exists()) {
    // existing filesystem node
    if ( !aFile.isDirectory() ) {
      if ( aFile.openForWriting() ) {
        if ( overwrite && ! aFile.remove()) {
          msg << "Can't replace " << aFile.getName();
        }
      } else {
        msg << "Can't write into " << aFile.getName();
      }
    } else {
      msg << "Location " << aFile.getName() << " is not a file";
    }
  }
  else {
    // nonexisting file; check if it can be created
    if ( !aFile.openForWriting() ) {
      msg << "You cannot create the file "
          << aFile.getName()
          << ". Check the directory existance and access rights";
    }
    aFile.remove();
  }

  if ( !msg.empty() )
  {
    msg << ".";
    THROW_SALOME_CORBA_EXCEPTION(msg.c_str(), SALOME::BAD_PARAM);
  }
}

//================================================================================
/*!
 * \brief Prepares a file for export and pass names of mesh groups from study to mesh DS
 *  \param file - file name
 *  \param overwrite - to erase the file or not
 *  \retval string - mesh name
 */
//================================================================================

string SMESH_Mesh_i::prepareMeshNameAndGroups(const char*    file,
                                              CORBA::Boolean overwrite)
{
  // Perform Export
  PrepareForWriting(file, overwrite);
  string aMeshName = "Mesh";
  SALOMEDS::Study_var aStudy = _gen_i->GetCurrentStudy();
  if ( !aStudy->_is_nil() ) {
    SALOMEDS::SObject_wrap aMeshSO = _gen_i->ObjectToSObject( aStudy, _this() );
    if ( !aMeshSO->_is_nil() ) {
      CORBA::String_var name = aMeshSO->GetName();
      aMeshName = name;
      // asv : 27.10.04 : fix of 6903: check for StudyLocked before adding attributes
      if ( !aStudy->GetProperties()->IsLocked() )
      {
        SALOMEDS::GenericAttribute_wrap anAttr;
        SALOMEDS::StudyBuilder_var aStudyBuilder = aStudy->NewBuilder();
        anAttr=aStudyBuilder->FindOrCreateAttribute(aMeshSO, "AttributeExternalFileDef");
        SALOMEDS::AttributeExternalFileDef_wrap aFileName = anAttr;
        ASSERT(!aFileName->_is_nil());
        aFileName->SetValue(file);
        anAttr=aStudyBuilder->FindOrCreateAttribute(aMeshSO, "AttributeFileType");
        SALOMEDS::AttributeFileType_wrap aFileType = anAttr;
        ASSERT(!aFileType->_is_nil());
        aFileType->SetValue("FICHIERMED");
      }
    }
  }
  // Update Python script
  // set name of mesh before export
  TPythonDump() << _gen_i << ".SetName("
                << SMESH::SMESH_Mesh_var(_this()) << ", '" << aMeshName.c_str() << "')";

  // check names of groups
  checkGroupNames();

  return aMeshName;
}

//================================================================================
/*!
 * \brief Export to med file
 */
//================================================================================

void SMESH_Mesh_i::ExportToMEDX (const char*        file,
                                 CORBA::Boolean     auto_groups,
                                 SMESH::MED_VERSION theVersion,
                                 CORBA::Boolean     overwrite,
                                 CORBA::Boolean     autoDimension)
  throw(SALOME::SALOME_Exception)
{
  SMESH_TRY;
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  string aMeshName = prepareMeshNameAndGroups(file, overwrite);
  _impl->ExportMED( file, aMeshName.c_str(), auto_groups, theVersion, 0, autoDimension );

  TPythonDump() << SMESH::SMESH_Mesh_var(_this()) << ".ExportToMEDX( r'"
                << file << "', " << auto_groups << ", "
                << theVersion << ", " << overwrite << ", "
                << autoDimension << " )";

  SMESH_CATCH( SMESH::throwCorbaException );
}

//================================================================================
/*!
 * \brief Export a mesh to a med file
 */
//================================================================================

void SMESH_Mesh_i::ExportToMED (const char*        file,
                                CORBA::Boolean     auto_groups,
                                SMESH::MED_VERSION theVersion)
  throw(SALOME::SALOME_Exception)
{
  ExportToMEDX(file,auto_groups,theVersion,true);
}

//================================================================================
/*!
 * \brief Export a mesh to a med file
 */
//================================================================================

void SMESH_Mesh_i::ExportMED (const char* file,
                              CORBA::Boolean auto_groups)
  throw(SALOME::SALOME_Exception)
{
  ExportToMEDX(file,auto_groups,SMESH::MED_V2_2,true);
}

//================================================================================
/*!
 * \brief Export a mesh to a SAUV file
 */
//================================================================================

void SMESH_Mesh_i::ExportSAUV (const char* file,
                               CORBA::Boolean auto_groups)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  string aMeshName = prepareMeshNameAndGroups(file, true);
  TPythonDump() << SMESH::SMESH_Mesh_var( _this())
                << ".ExportSAUV( r'" << file << "', " << auto_groups << " )";
  _impl->ExportSAUV(file, aMeshName.c_str(), auto_groups);
}


//================================================================================
/*!
 * \brief Export a mesh to a DAT file
 */
//================================================================================

void SMESH_Mesh_i::ExportDAT (const char *file)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  // Update Python script
  // check names of groups
  checkGroupNames();
  TPythonDump() << SMESH::SMESH_Mesh_var(_this()) << ".ExportDAT( r'" << file << "' )";

  // Perform Export
  PrepareForWriting(file);
  _impl->ExportDAT(file);
}

//================================================================================
/*!
 * \brief Export a mesh to an UNV file
 */
//================================================================================

void SMESH_Mesh_i::ExportUNV (const char *file)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  // Update Python script
  // check names of groups
  checkGroupNames();
  TPythonDump() << SMESH::SMESH_Mesh_var(_this()) << ".ExportUNV( r'" << file << "' )";

  // Perform Export
  PrepareForWriting(file);
  _impl->ExportUNV(file);
}

//================================================================================
/*!
 * \brief Export a mesh to an STL file
 */
//================================================================================

void SMESH_Mesh_i::ExportSTL (const char *file, const bool isascii)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  // Update Python script
  // check names of groups
  checkGroupNames();
  TPythonDump() << SMESH::SMESH_Mesh_var(_this())
                << ".ExportSTL( r'" << file << "', " << isascii << " )";

  // Perform Export
  PrepareForWriting(file);
  _impl->ExportSTL(file, isascii);
}

//================================================================================
/*!
 * \brief Export a part of mesh to a med file
 */
//================================================================================

void SMESH_Mesh_i::ExportPartToMED(SMESH::SMESH_IDSource_ptr meshPart,
                                   const char*               file,
                                   CORBA::Boolean            auto_groups,
                                   SMESH::MED_VERSION        version,
                                   CORBA::Boolean            overwrite,
                                   CORBA::Boolean            autoDimension,
                                   const GEOM::ListOfFields& fields,
                                   const char*               geomAssocFields)
  throw (SALOME::SALOME_Exception)
{
  SMESH_TRY;
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  // check fields
  bool have0dField = false;
  if ( fields.length() > 0 )
  {
    GEOM::GEOM_Object_var shapeToMesh = GetShapeToMesh();
    if ( shapeToMesh->_is_nil() )
      THROW_SALOME_CORBA_EXCEPTION( "No shape to mesh", SALOME::INTERNAL_ERROR );

    for ( size_t i = 0; i < fields.length(); ++i )
    {
      if ( fields[i]->GetDataType() == GEOM::FDT_String )
        THROW_SALOME_CORBA_EXCEPTION
          ( "Export of string fields is not supported", SALOME::BAD_PARAM);
      GEOM::GEOM_Object_var fieldShape = fields[i]->GetShape();
      if ( fieldShape->_is_nil() )
        THROW_SALOME_CORBA_EXCEPTION( "Null shape under a field", SALOME::INTERNAL_ERROR );
      if ( !fieldShape->IsSame( shapeToMesh ) )
        THROW_SALOME_CORBA_EXCEPTION
          ( "Field defined not on shape", SALOME::BAD_PARAM);
      if ( fields[i]->GetDimension() == 0 )
        have0dField = true;
    }
    if ( geomAssocFields )
      for ( int i = 0; geomAssocFields[i]; ++i )
        switch ( geomAssocFields[i] ) {
        case 'v':case 'e':case 'f':case 's': break;
        case 'V':case 'E':case 'F':case 'S': break;
        default: THROW_SALOME_CORBA_EXCEPTION
            ( "geomAssocFields can include only [vefs] characters", SALOME::BAD_PARAM);
        }
  }

  SMESHDS_Mesh* meshDS = _impl->GetMeshDS();

  // write mesh

  string aMeshName = "Mesh";
  SMESHUtils::Deleter< SMESH_MeshPartDS > tmpDSDeleter(0);
  if ( CORBA::is_nil( meshPart ) ||
       SMESH::DownCast< SMESH_Mesh_i* >( meshPart ))
  {
    aMeshName = prepareMeshNameAndGroups(file, overwrite);
    _impl->ExportMED( file, aMeshName.c_str(), auto_groups,
                      version, 0, autoDimension, /*addODOnVertices=*/have0dField);
    meshDS = _impl->GetMeshDS();
  }
  else
  {
    if ( _preMeshInfo )
      _preMeshInfo->FullLoadFromFile();

    PrepareForWriting(file, overwrite);

    SALOMEDS::Study_var aStudy = _gen_i->GetCurrentStudy();
    if ( !aStudy->_is_nil() ) {
      SALOMEDS::SObject_wrap SO = _gen_i->ObjectToSObject( aStudy, meshPart );
      if ( !SO->_is_nil() ) {
        CORBA::String_var name = SO->GetName();
        aMeshName = name;
      }
    }
    SMESH_MeshPartDS* partDS = new SMESH_MeshPartDS( meshPart );
    _impl->ExportMED( file, aMeshName.c_str(), auto_groups,
                      version, partDS, autoDimension, /*addODOnVertices=*/have0dField);
    meshDS = tmpDSDeleter._obj = partDS;
  }

  // write fields

  if ( _impl->HasShapeToMesh() )
  {
    DriverMED_W_Field fieldWriter;
    fieldWriter.SetFile( file );
    fieldWriter.SetMeshName( aMeshName );
    fieldWriter.AddODOnVertices( have0dField );

    exportMEDFields( fieldWriter, meshDS, fields, geomAssocFields );
  }

  // dump
  GEOM::ListOfGBO_var goList = new GEOM::ListOfGBO;
  goList->length( fields.length() );
  for ( size_t i = 0; i < fields.length(); ++i )
  {
    GEOM::GEOM_BaseObject_var gbo = GEOM::GEOM_BaseObject::_narrow( fields[i] );
    goList[i] = gbo;
  }
  TPythonDump() << _this() << ".ExportPartToMED( "
                << meshPart << ", r'" << file << "', "
                << auto_groups << ", " << version << ", " << overwrite << ", "
                << autoDimension << ", " << goList
                << ", '" << ( geomAssocFields ? geomAssocFields : "" ) << "'" << " )";

  SMESH_CATCH( SMESH::throwCorbaException );
}

//================================================================================
/*!
 * Write GEOM fields to MED file
 */
//================================================================================

void SMESH_Mesh_i::exportMEDFields( DriverMED_W_Field&        fieldWriter,
                                    SMESHDS_Mesh*             meshDS,
                                    const GEOM::ListOfFields& fields,
                                    const char*               geomAssocFields)
{
#define METH "SMESH_Mesh_i::exportMEDFields() "

  if (( fields.length() < 1 ) &&
      ( !geomAssocFields || !geomAssocFields[0] ))
    return;

  std::vector< std::vector< double > > dblVals;
  std::vector< std::vector< int > >    intVals;
  std::vector< int >                   subIdsByDim[ 4 ];
  const double noneDblValue = 0.;
  const double noneIntValue = 0;

  for ( size_t iF = 0; iF < fields.length(); ++iF )
  {
    // set field data

    int dim = fields[ iF ]->GetDimension();
    SMDSAbs_ElementType elemType;
    TopAbs_ShapeEnum    shapeType;
    switch ( dim ) {
    case 0: elemType = SMDSAbs_0DElement; shapeType = TopAbs_VERTEX; break;
    case 1: elemType = SMDSAbs_Edge;      shapeType = TopAbs_EDGE;   break;
    case 2: elemType = SMDSAbs_Face;      shapeType = TopAbs_FACE;   break;
    case 3: elemType = SMDSAbs_Volume;    shapeType = TopAbs_SOLID;  break;
    default:
      continue; // skip fields on whole shape
    }
    GEOM::field_data_type dataType = fields[ iF ]->GetDataType();
    if ( dataType == GEOM::FDT_String )
      continue;
    GEOM::ListOfLong_var stepIDs = fields[ iF ]->GetSteps();
    if ( stepIDs->length() < 1 )
      continue;
    GEOM::string_array_var comps = fields[ iF ]->GetComponents();
    if ( comps->length() < 1 )
      continue;
    CORBA::String_var       name = fields[ iF ]->GetName();

    if ( !fieldWriter.Set( meshDS,
                           name.in(),
                           elemType,
                           comps->length(),
                           /*isIntData=*/false ))//( dataType == GEOM::FDT_Int )))
      continue;

    for ( size_t iC = 0; iC < comps->length(); ++iC )
      fieldWriter.SetCompName( iC, comps[ iC ].in() );

    dblVals.resize( comps->length() );
    intVals.resize( comps->length() );

    // find sub-shape IDs

    std::vector< int >& subIds = subIdsByDim[ dim ];
    if ( subIds.empty() )
      for ( int id = 1; id <= meshDS->MaxShapeIndex(); ++id )
        if ( meshDS->IndexToShape( id ).ShapeType() == shapeType )
          subIds.push_back( id );

    // write steps

    SMDS_ElemIteratorPtr elemIt = fieldWriter.GetOrderedElems();
    if ( !elemIt )
      continue;

    for ( size_t iS = 0; iS < stepIDs->length(); ++iS )
    {
      GEOM::GEOM_FieldStep_var step = fields[ iF ]->GetStep( stepIDs[ iS ]);
      if ( step->_is_nil() )
        continue;

      CORBA::Long stamp = step->GetStamp();
      CORBA::Long id    = step->GetID();
      fieldWriter.SetDtIt( int( stamp ), int( id ));

      // fill dblVals or intVals
      for ( size_t iC = 0; iC < comps->length(); ++iC )
        if ( dataType == GEOM::FDT_Double )
        {
          dblVals[ iC ].clear();
          dblVals[ iC ].resize( meshDS->MaxShapeIndex()+1, 0 );
        }
        else
        {
          intVals[ iC ].clear();
          intVals[ iC ].resize( meshDS->MaxShapeIndex()+1, 0 );
        }
      switch ( dataType )
      {
      case GEOM::FDT_Double:
      {
        GEOM::GEOM_DoubleFieldStep_var dblStep = GEOM::GEOM_DoubleFieldStep::_narrow( step );
        if ( dblStep->_is_nil() ) continue;
        GEOM::ListOfDouble_var vv = dblStep->GetValues();
        if ( vv->length() != subIds.size() * comps->length() )
          THROW_SALOME_CORBA_EXCEPTION( METH "BUG: wrong nb subIds", SALOME::INTERNAL_ERROR );
        for ( size_t iS = 0, iV = 0; iS < subIds.size(); ++iS )
          for ( size_t iC = 0; iC < comps->length(); ++iC )
            dblVals[ iC ][ subIds[ iS ]] = vv[ iV++ ];
        break;
      }
      case GEOM::FDT_Int:
      {
        GEOM::GEOM_IntFieldStep_var intStep = GEOM::GEOM_IntFieldStep::_narrow( step );
        if ( intStep->_is_nil() ) continue;
        GEOM::ListOfLong_var vv = intStep->GetValues();
        if ( vv->length() != subIds.size() * comps->length() )
          THROW_SALOME_CORBA_EXCEPTION( METH "BUG: wrong nb subIds", SALOME::INTERNAL_ERROR );
        for ( size_t iS = 0, iV = 0; iS < subIds.size(); ++iS )
          for ( size_t iC = 0; iC < comps->length(); ++iC )
            intVals[ iC ][ subIds[ iS ]] = (int) vv[ iV++ ];
        break;
      }
      case GEOM::FDT_Bool:
      {
        GEOM::GEOM_BoolFieldStep_var boolStep = GEOM::GEOM_BoolFieldStep::_narrow( step );
        if ( boolStep->_is_nil() ) continue;
        GEOM::short_array_var vv = boolStep->GetValues();
        if ( vv->length() != subIds.size() * comps->length() )
          THROW_SALOME_CORBA_EXCEPTION( METH "BUG: wrong nb subIds", SALOME::INTERNAL_ERROR );
        for ( size_t iS = 0, iV = 0; iS < subIds.size(); ++iS )
          for ( size_t iC = 0; iC < comps->length(); ++iC )
            intVals[ iC ][ subIds[ iS ]] = (int) vv[ iV++ ];
        break;
      }
      default: continue;
      }

      // pass values to fieldWriter
      elemIt = fieldWriter.GetOrderedElems();
      if ( dataType == GEOM::FDT_Double )
        while ( elemIt->more() )
        {
          const SMDS_MeshElement* e = elemIt->next();
          const int shapeID = e->getshapeId();
          if ( shapeID < 1 || shapeID >= (int) dblVals[0].size() )
            for ( size_t iC = 0; iC < comps->length(); ++iC )
              fieldWriter.AddValue( noneDblValue );
          else
            for ( size_t iC = 0; iC < comps->length(); ++iC )
              fieldWriter.AddValue( dblVals[ iC ][ shapeID ]);
        }
      else
        while ( elemIt->more() )
        {
          const SMDS_MeshElement* e = elemIt->next();
          const int shapeID = e->getshapeId();
          if ( shapeID < 1 || shapeID >= (int) intVals[0].size() )
            for ( size_t iC = 0; iC < comps->length(); ++iC )
              fieldWriter.AddValue( (double) noneIntValue );
          else
            for ( size_t iC = 0; iC < comps->length(); ++iC )
              fieldWriter.AddValue( (double) intVals[ iC ][ shapeID ]);
        }

      // write a step
      fieldWriter.Perform();
      SMESH_ComputeErrorPtr res = fieldWriter.GetError();
      if ( res && res->IsKO() )
      {
        if ( res->myComment.empty() )
        { THROW_SALOME_CORBA_EXCEPTION( METH "Fatal error", SALOME::INTERNAL_ERROR ); }
        else
        { THROW_SALOME_CORBA_EXCEPTION( res->myComment.c_str(), SALOME::INTERNAL_ERROR ); }
      }

    } // loop on steps
  } // loop on fields

  if ( !geomAssocFields || !geomAssocFields[0] )
    return;

  // write geomAssocFields

  std::vector< int > shapeDim( TopAbs_SHAPE + 1 );
  shapeDim[ TopAbs_COMPOUND  ] = 3;
  shapeDim[ TopAbs_COMPSOLID ] = 3;
  shapeDim[ TopAbs_SOLID     ] = 3;
  shapeDim[ TopAbs_SHELL     ] = 2;
  shapeDim[ TopAbs_FACE      ] = 2;
  shapeDim[ TopAbs_WIRE      ] = 1;
  shapeDim[ TopAbs_EDGE      ] = 1;
  shapeDim[ TopAbs_VERTEX    ] = 0;
  shapeDim[ TopAbs_SHAPE     ] = 3;

  for ( int iF = 0; geomAssocFields[ iF ]; ++iF )
  {
    std::vector< std::string > compNames;
    switch ( geomAssocFields[ iF ]) {
    case 'v': case 'V':
      fieldWriter.Set( meshDS, "_vertices_", SMDSAbs_Node, /*nbComps=*/2, /*isInt=*/false );
      compNames.push_back( "dim" );
      break;
    case 'e': case 'E':
      fieldWriter.Set( meshDS, "_edges_", SMDSAbs_Edge, /*nbComps=*/1, /*isInt=*/false );
      break;
    case 'f': case 'F':
      fieldWriter.Set( meshDS, "_faces_", SMDSAbs_Face, /*nbComps=*/1, /*isInt=*/false );
      break;
    case 's': case 'S':
      fieldWriter.Set( meshDS, "_solids_", SMDSAbs_Volume, /*nbComps=*/1, /*isInt=*/false );
      break;
    default: continue;
    }
    compNames.push_back( "id" );
    for ( size_t iC = 0; iC < compNames.size(); ++iC )
      fieldWriter.SetCompName( iC, compNames[ iC ].c_str() );

    fieldWriter.SetDtIt( -1, -1 );

    SMDS_ElemIteratorPtr elemIt = fieldWriter.GetOrderedElems();
    if ( !elemIt )
      continue;

    if ( compNames.size() == 2 ) // _vertices_
      while ( elemIt->more() )
      {
        const SMDS_MeshElement* e = elemIt->next();
        const int shapeID = e->getshapeId();
        if ( shapeID < 1 )
        {
          fieldWriter.AddValue( (double) -1 );
          fieldWriter.AddValue( (double) -1 );
        }
        else
        {
          const TopoDS_Shape& S = meshDS->IndexToShape( shapeID );
          fieldWriter.AddValue( (double) ( S.IsNull() ? -1 : shapeDim[ S.ShapeType() ]));
          fieldWriter.AddValue( (double) shapeID );
        }
      }
    else
      while ( elemIt->more() )
      {
        const SMDS_MeshElement* e = elemIt->next();
        const int shapeID = e->getshapeId();
        if ( shapeID < 1 )
          fieldWriter.AddValue( (double) -1 );
        else
          fieldWriter.AddValue( (double) shapeID );
      }

    // write a step
    fieldWriter.Perform();
    SMESH_ComputeErrorPtr res = fieldWriter.GetError();
    if ( res && res->IsKO() )
    {
      if ( res->myComment.empty() )
      { THROW_SALOME_CORBA_EXCEPTION( METH "Fatal error", SALOME::INTERNAL_ERROR ); }
      else
      { THROW_SALOME_CORBA_EXCEPTION( res->myComment.c_str(), SALOME::INTERNAL_ERROR ); }
    }

  } // loop on geomAssocFields

#undef METH
}

//================================================================================
/*!
 * \brief Export a part of mesh to a DAT file
 */
//================================================================================

void SMESH_Mesh_i::ExportPartToDAT(::SMESH::SMESH_IDSource_ptr meshPart,
                                   const char*                 file)
  throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  PrepareForWriting(file);

  SMESH_MeshPartDS partDS( meshPart );
  _impl->ExportDAT(file,&partDS);

  TPythonDump() << SMESH::SMESH_Mesh_var(_this())
                << ".ExportPartToDAT( " << meshPart << ", r'" << file << "' )";
}
//================================================================================
/*!
 * \brief Export a part of mesh to an UNV file
 */
//================================================================================

void SMESH_Mesh_i::ExportPartToUNV(::SMESH::SMESH_IDSource_ptr meshPart,
                                   const char*                 file)
  throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  PrepareForWriting(file);

  SMESH_MeshPartDS partDS( meshPart );
  _impl->ExportUNV(file, &partDS);

  TPythonDump() << SMESH::SMESH_Mesh_var(_this())
                << ".ExportPartToUNV( " << meshPart<< ", r'" << file << "' )";
}
//================================================================================
/*!
 * \brief Export a part of mesh to an STL file
 */
//================================================================================

void SMESH_Mesh_i::ExportPartToSTL(::SMESH::SMESH_IDSource_ptr meshPart,
                                   const char*                 file,
                                   ::CORBA::Boolean            isascii)
  throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  PrepareForWriting(file);

  SMESH_MeshPartDS partDS( meshPart );
  _impl->ExportSTL(file, isascii, &partDS);

  TPythonDump() << SMESH::SMESH_Mesh_var(_this()) << ".ExportPartToSTL( "
                << meshPart<< ", r'" << file << "', " << isascii << ")";
}

//================================================================================
/*!
 * \brief Export a part of mesh to an STL file
 */
//================================================================================

void SMESH_Mesh_i::ExportCGNS(::SMESH::SMESH_IDSource_ptr meshPart,
                              const char*                 file,
                              CORBA::Boolean              overwrite)
  throw (SALOME::SALOME_Exception)
{
#ifdef WITH_CGNS
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  PrepareForWriting(file,overwrite);

  std::string meshName("");
  SALOMEDS::Study_var study = _gen_i->GetCurrentStudy();
  SALOMEDS::SObject_wrap so = _gen_i->ObjectToSObject( study, meshPart );
  if ( !so->_is_nil() )
  {
    CORBA::String_var name = so->GetName();
    meshName = name.in();
  }
  SMESH_MeshPartDS partDS( meshPart );
  _impl->ExportCGNS(file, &partDS, meshName.c_str() );

  TPythonDump() << SMESH::SMESH_Mesh_var(_this()) << ".ExportCGNS( "
                << meshPart<< ", r'" << file << "', " << overwrite << ")";
#else
  THROW_SALOME_CORBA_EXCEPTION("CGNS library is unavailable", SALOME::INTERNAL_ERROR);
#endif
}

//================================================================================
/*!
 * \brief Export a part of mesh to a GMF file
 */
//================================================================================

void SMESH_Mesh_i::ExportGMF(::SMESH::SMESH_IDSource_ptr meshPart,
                             const char*                 file,
                             bool                        withRequiredGroups)
  throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  PrepareForWriting(file,/*overwrite=*/true);

  SMESH_MeshPartDS partDS( meshPart );
  _impl->ExportGMF(file, &partDS, withRequiredGroups);

  TPythonDump() << SMESH::SMESH_Mesh_var(_this()) << ".ExportGMF( "
                << meshPart<< ", r'"
                << file << "', "
                << withRequiredGroups << ")";
}

//=============================================================================
/*!
 * Return computation progress [0.,1]
 */
//=============================================================================

CORBA::Double SMESH_Mesh_i::GetComputeProgress()
{
  SMESH_TRY;

  return _impl->GetComputeProgress();

  SMESH_CATCH( SMESH::doNothing );
  return 0.;
}

CORBA::Long SMESH_Mesh_i::NbNodes()throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbNodes();

  return _impl->NbNodes();
}

CORBA::Long SMESH_Mesh_i::NbElements()throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbElements();

  return Nb0DElements() + NbEdges() + NbFaces() + NbVolumes() + NbBalls();
}

CORBA::Long SMESH_Mesh_i::Nb0DElements()throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->Nb0DElements();

  return _impl->Nb0DElements();
}

CORBA::Long SMESH_Mesh_i::NbBalls() throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbBalls();

  return _impl->NbBalls();
}

CORBA::Long SMESH_Mesh_i::NbEdges()throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbEdges();

  return _impl->NbEdges();
}

CORBA::Long SMESH_Mesh_i::NbEdgesOfOrder(SMESH::ElementOrder order)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbEdges( (SMDSAbs_ElementOrder) order );

  return _impl->NbEdges( (SMDSAbs_ElementOrder) order);
}

//=============================================================================

CORBA::Long SMESH_Mesh_i::NbFaces()throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbFaces();

  return _impl->NbFaces();
}

CORBA::Long SMESH_Mesh_i::NbTriangles()throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbTriangles();

  return _impl->NbTriangles();
}

CORBA::Long SMESH_Mesh_i::NbBiQuadTriangles()throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbBiQuadTriangles();

  return _impl->NbBiQuadTriangles();
}

CORBA::Long SMESH_Mesh_i::NbQuadrangles()throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbQuadrangles();

  return _impl->NbQuadrangles();
}

CORBA::Long SMESH_Mesh_i::NbBiQuadQuadrangles()throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbBiQuadQuadrangles();

  return _impl->NbBiQuadQuadrangles();
}

CORBA::Long SMESH_Mesh_i::NbPolygons() throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbPolygons();

  return _impl->NbPolygons();
}

CORBA::Long SMESH_Mesh_i::NbPolygonsOfOrder(SMESH::ElementOrder order) throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbPolygons((SMDSAbs_ElementOrder) order);

  return _impl->NbPolygons((SMDSAbs_ElementOrder)order);
}

CORBA::Long SMESH_Mesh_i::NbFacesOfOrder(SMESH::ElementOrder order)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbFaces( (SMDSAbs_ElementOrder) order );

  return _impl->NbFaces( (SMDSAbs_ElementOrder) order);
}

CORBA::Long SMESH_Mesh_i::NbTrianglesOfOrder(SMESH::ElementOrder order)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbTriangles( (SMDSAbs_ElementOrder) order );

  return _impl->NbTriangles( (SMDSAbs_ElementOrder) order);
}

CORBA::Long SMESH_Mesh_i::NbQuadranglesOfOrder(SMESH::ElementOrder order)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbQuadrangles( (SMDSAbs_ElementOrder) order );

  return _impl->NbQuadrangles( (SMDSAbs_ElementOrder) order);
}

//=============================================================================

CORBA::Long SMESH_Mesh_i::NbVolumes()throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbVolumes();

  return _impl->NbVolumes();
}

CORBA::Long SMESH_Mesh_i::NbTetras()throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbTetras();

  return _impl->NbTetras();
}

CORBA::Long SMESH_Mesh_i::NbHexas()throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbHexas();

  return _impl->NbHexas();
}

CORBA::Long SMESH_Mesh_i::NbTriQuadraticHexas()throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbTriQuadHexas();

  return _impl->NbTriQuadraticHexas();
}

CORBA::Long SMESH_Mesh_i::NbPyramids()throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbPyramids();

  return _impl->NbPyramids();
}

CORBA::Long SMESH_Mesh_i::NbPrisms()throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbPrisms();

  return _impl->NbPrisms();
}

CORBA::Long SMESH_Mesh_i::NbHexagonalPrisms()throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbHexPrisms();

  return _impl->NbHexagonalPrisms();
}

CORBA::Long SMESH_Mesh_i::NbPolyhedrons()throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbPolyhedrons();

  return _impl->NbPolyhedrons();
}

CORBA::Long SMESH_Mesh_i::NbVolumesOfOrder(SMESH::ElementOrder order)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbVolumes( (SMDSAbs_ElementOrder) order );

  return _impl->NbVolumes( (SMDSAbs_ElementOrder) order);
}

CORBA::Long SMESH_Mesh_i::NbTetrasOfOrder(SMESH::ElementOrder order)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbTetras( (SMDSAbs_ElementOrder) order);

  return _impl->NbTetras( (SMDSAbs_ElementOrder) order);
}

CORBA::Long SMESH_Mesh_i::NbHexasOfOrder(SMESH::ElementOrder order)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbHexas( (SMDSAbs_ElementOrder) order);

  return _impl->NbHexas( (SMDSAbs_ElementOrder) order);
}

CORBA::Long SMESH_Mesh_i::NbPyramidsOfOrder(SMESH::ElementOrder order)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbPyramids( (SMDSAbs_ElementOrder) order);

  return _impl->NbPyramids( (SMDSAbs_ElementOrder) order);
}

CORBA::Long SMESH_Mesh_i::NbPrismsOfOrder(SMESH::ElementOrder order)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbPrisms( (SMDSAbs_ElementOrder) order);

  return _impl->NbPrisms( (SMDSAbs_ElementOrder) order);
}

//=============================================================================
/*!
 * Returns nb of published sub-meshes
 */
//=============================================================================

CORBA::Long SMESH_Mesh_i::NbSubMesh()throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  return _mapSubMesh_i.size();
}

//=============================================================================
/*!
 * Dumps mesh into a string
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
 * Method of SMESH_IDSource interface
 */
//=============================================================================

SMESH::long_array* SMESH_Mesh_i::GetIDs()
{
  return GetElementsId();
}

//=============================================================================
/*!
 * Returns ids of all elements
 */
//=============================================================================

SMESH::long_array* SMESH_Mesh_i::GetElementsId()
  throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

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
 * Returns ids of all elements of given type
 */
//=============================================================================

SMESH::long_array* SMESH_Mesh_i::GetElementsByType( SMESH::ElementType theElemType )
    throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESH::long_array_var aResult = new SMESH::long_array();
  SMESHDS_Mesh* aSMESHDS_Mesh = _impl->GetMeshDS();

  if ( aSMESHDS_Mesh == NULL )
    return aResult._retn();

  long nbElements = NbElements();

  // No sense in returning ids of elements along with ids of nodes:
  // when theElemType == SMESH::ALL, return node ids only if
  // there are no elements
  if ( theElemType == SMESH::NODE || (theElemType == SMESH::ALL && nbElements == 0) )
    return GetNodesId();

  aResult->length( nbElements );

  int i = 0;

  SMDS_ElemIteratorPtr anIt = aSMESHDS_Mesh->elementsIterator( (SMDSAbs_ElementType)theElemType );
  while ( i < nbElements && anIt->more() )
    aResult[i++] = anIt->next()->GetID();

  aResult->length( i );

  return aResult._retn();
}

//=============================================================================
/*!
 * Returns ids of all nodes
 */
//=============================================================================

SMESH::long_array* SMESH_Mesh_i::GetNodesId()
  throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESH::long_array_var aResult = new SMESH::long_array();
  SMESHDS_Mesh* aSMESHDS_Mesh = _impl->GetMeshDS();

  if ( aSMESHDS_Mesh == NULL )
    return aResult._retn();

  long nbNodes = NbNodes();
  aResult->length( nbNodes );
  SMDS_NodeIteratorPtr anIt = aSMESHDS_Mesh->nodesIterator(/*idInceasingOrder=*/true);
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
  SMESH::ElementType type = SMESH::ALL;
  SMESH_TRY;

  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  type = ( SMESH::ElementType ) _impl->GetElementType( id, iselem );

  SMESH_CATCH( SMESH::throwCorbaException );

  return type;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

SMESH::EntityType SMESH_Mesh_i::GetElementGeomType( const CORBA::Long id )
  throw (SALOME::SALOME_Exception)
{
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  const SMDS_MeshElement* e = _impl->GetMeshDS()->FindElement(id);
  if ( !e )
    THROW_SALOME_CORBA_EXCEPTION( "invalid element id", SALOME::BAD_PARAM );

  return ( SMESH::EntityType ) e->GetEntityType();
}

//=============================================================================
/*!
 *
 */
//=============================================================================

SMESH::GeometryType SMESH_Mesh_i::GetElementShape( const CORBA::Long id )
  throw (SALOME::SALOME_Exception)
{
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  const SMDS_MeshElement* e = _impl->GetMeshDS()->FindElement(id);
  if ( !e )
    THROW_SALOME_CORBA_EXCEPTION( "invalid element id", SALOME::BAD_PARAM );

  return ( SMESH::GeometryType ) e->GetGeomType();
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

  SMESH_TRY;
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

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

  SMESH_CATCH( SMESH::throwCorbaException );

  return aResult._retn();
}

//=============================================================================
/*!
 * Returns ID of nodes for given submesh
 * If param all==true - returns all nodes, else -
 * returns only nodes on shapes.
 */
//=============================================================================

SMESH::long_array* SMESH_Mesh_i::GetSubMeshNodesId(const CORBA::Long ShapeID,
                                                   CORBA::Boolean    all)
  throw (SALOME::SALOME_Exception)
{
  SMESH::long_array_var aResult = new SMESH::long_array();

  SMESH_TRY;
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

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

  SMESH_CATCH( SMESH::throwCorbaException );

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
  SMESH::ElementType type = SMESH::ALL;

  SMESH_TRY;
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESH_subMesh* SM = _impl->GetSubMeshContaining(ShapeID);
  if(!SM) return SMESH::ALL;

  SMESHDS_SubMesh* SDSM = SM->GetSubMeshDS();
  if(!SDSM) return SMESH::ALL;

  if(SDSM->NbElements()==0)
    return (SM->GetSubShape().ShapeType() == TopAbs_VERTEX) ? SMESH::NODE : SMESH::ALL;

  SMDS_ElemIteratorPtr eIt = SDSM->GetElements();
  const SMDS_MeshElement* anElem = eIt->next();

  type = ( SMESH::ElementType ) anElem->GetType();

  SMESH_CATCH( SMESH::throwCorbaException );

  return type; 
}
  

//=============================================================================
/*!
 * Returns pointer to _impl as an integer value. Is called from constructor of SMESH_Client
 */
//=============================================================================

CORBA::LongLong SMESH_Mesh_i::GetMeshPtr()
{
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  CORBA::LongLong pointeur = CORBA::LongLong(_impl);
  if ( MYDEBUG ) MESSAGE("CORBA::LongLong SMESH_Mesh_i::GetMeshPtr() "<<pointeur);
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
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

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
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

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
  aResult->length( aNode->NbInverseElements() );  
  for( int i = 0; eIt->more(); ++i )
  {
    const SMDS_MeshElement* elem = eIt->next();
    aResult[ i ] = elem->GetID();
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
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESH::NodePosition* aNodePosition = new SMESH::NodePosition();
  aNodePosition->shapeID = 0;
  aNodePosition->shapeType = GEOM::SHAPE;

  SMESHDS_Mesh* mesh = _impl->GetMeshDS();
  if ( !mesh ) return aNodePosition;

  if ( const SMDS_MeshNode* aNode = mesh->FindNode(NodeID) )
  {
    if ( SMDS_PositionPtr pos = aNode->GetPosition() )
    {
      aNodePosition->shapeID = aNode->getshapeId();
      switch ( pos->GetTypeOfPosition() ) {
      case SMDS_TOP_EDGE:
        aNodePosition->shapeType = GEOM::EDGE;
        aNodePosition->params.length(1);
        aNodePosition->params[0] =
          static_cast<SMDS_EdgePosition*>( pos )->GetUParameter();
        break;
      case SMDS_TOP_FACE:
        aNodePosition->shapeType = GEOM::FACE;
        aNodePosition->params.length(2);
        aNodePosition->params[0] =
          static_cast<SMDS_FacePosition*>( pos )->GetUParameter();
        aNodePosition->params[1] =
          static_cast<SMDS_FacePosition*>( pos )->GetVParameter();
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
 * \brief Return position of an element on shape
 */
//=============================================================================

SMESH::ElementPosition SMESH_Mesh_i::GetElementPosition(CORBA::Long ElemID)
{
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESH::ElementPosition anElementPosition;
  anElementPosition.shapeID = 0;
  anElementPosition.shapeType = GEOM::SHAPE;

  SMESHDS_Mesh* mesh = _impl->GetMeshDS();
  if ( !mesh ) return anElementPosition;

  if ( const SMDS_MeshElement* anElem = mesh->FindElement( ElemID ) )
  {
    anElementPosition.shapeID = anElem->getshapeId();
    const TopoDS_Shape& aSp = mesh->IndexToShape( anElem->getshapeId() );
    if ( !aSp.IsNull() ) {
      switch ( aSp.ShapeType() ) {
      case TopAbs_EDGE:
        anElementPosition.shapeType = GEOM::EDGE;
        break;
      case TopAbs_FACE:
        anElementPosition.shapeType = GEOM::FACE;
        break;
      case TopAbs_VERTEX:
        anElementPosition.shapeType = GEOM::VERTEX;
        break;
      case TopAbs_SOLID:
        anElementPosition.shapeType = GEOM::SOLID;
        break;
      case TopAbs_SHELL:
        anElementPosition.shapeType = GEOM::SHELL;
        break;
      default:;
      }
    }
  }
  return anElementPosition;
}

//=============================================================================
/*!
 * If given element is node returns IDs of shape from position
 * If there is not node for given ID - returns -1
 */
//=============================================================================

CORBA::Long SMESH_Mesh_i::GetShapeID(const CORBA::Long id)
{
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESHDS_Mesh* aSMESHDS_Mesh = _impl->GetMeshDS();
  if ( aSMESHDS_Mesh == NULL )
    return -1;

  // try to find node
  const SMDS_MeshNode* aNode = aSMESHDS_Mesh->FindNode(id);
  if(aNode) {
    return aNode->getshapeId();
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
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESHDS_Mesh* aSMESHDS_Mesh = _impl->GetMeshDS();
  if ( aSMESHDS_Mesh == NULL )
    return -1;

  // try to find element
  const SMDS_MeshElement* elem = aSMESHDS_Mesh->FindElement(id);
  if(!elem)
    return -1;

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
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

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
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

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
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

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
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

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
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

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
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

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
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

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
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

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
        for ( CORBA::ULong i = 0; i < aResult->length(); ++i )
          aResult[ i ] = nn[ i ]->GetID();
      }
    }
  }
  return aResult._retn();
}

//=======================================================================
//function : GetElemFaceNodes
//purpose  : Returns three components of normal of given mesh face.
//=======================================================================

SMESH::double_array* SMESH_Mesh_i::GetFaceNormal(CORBA::Long    elemId,
                                                 CORBA::Boolean normalized)
{
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESH::double_array_var aResult = new SMESH::double_array();

  if ( SMESHDS_Mesh* mesh = _impl->GetMeshDS() )
  {
    gp_XYZ normal;
    if ( SMESH_MeshAlgos::FaceNormal( mesh->FindElement(elemId), normal, normalized ))
    {
      aResult->length( 3 );
      aResult[ 0 ] = normal.X();
      aResult[ 1 ] = normal.Y();
      aResult[ 2 ] = normal.Z();
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
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  CORBA::Long elemID(0);
  if ( SMESHDS_Mesh* mesh = _impl->GetMeshDS() )
  {
    vector< const SMDS_MeshNode * > nn( nodes.length() );
    for ( CORBA::ULong i = 0; i < nodes.length(); ++i )
      if ( !( nn[i] = mesh->FindNode( nodes[i] )))
        return elemID;

    const SMDS_MeshElement* elem = mesh->FindElement( nn, SMDSAbs_All, /*noMedium=*/false );
    if ( !elem && ( _impl->NbEdges  ( ORDER_QUADRATIC ) ||
                    _impl->NbFaces  ( ORDER_QUADRATIC ) ||
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
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

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
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESHDS_Mesh* aSMESHDS_Mesh = _impl->GetMeshDS();
  if ( aSMESHDS_Mesh == NULL ) return false;
  const SMDS_MeshElement* elem = aSMESHDS_Mesh->FindElement(id);
  if(!elem) return false;
  return elem->IsQuadratic();
}

//=============================================================================
/*!
 * Returns diameter of ball discrete element or zero in case of an invalid \a id
 */
//=============================================================================

CORBA::Double SMESH_Mesh_i::GetBallDiameter(CORBA::Long id)
{
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  if ( const SMDS_BallElement* ball =
       dynamic_cast<const SMDS_BallElement*>( _impl->GetMeshDS()->FindElement( id )))
    return ball->GetDiameter();

  return 0;
}

//=============================================================================
/*!
 * Returns bary center for given element
 */
//=============================================================================

SMESH::double_array* SMESH_Mesh_i::BaryCenter(const CORBA::Long id)
{
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

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

//================================================================================
/*!
 * \brief Create a group of elements preventing computation of a sub-shape
 */
//================================================================================

SMESH::ListOfGroups*
SMESH_Mesh_i::MakeGroupsOfBadInputElements( int         theSubShapeID,
                                            const char* theGroupName )
  throw ( SALOME::SALOME_Exception )
{
  Unexpect aCatch(SALOME_SalomeException);

  if ( !theGroupName || strlen( theGroupName) == 0 )
    THROW_SALOME_CORBA_EXCEPTION( "empty group name",SALOME::BAD_PARAM );

  SMESH::ListOfGroups_var groups = new SMESH::ListOfGroups;
  ::SMESH_MeshEditor::ElemFeatures elemType;

  // submesh by subshape id
  if ( !_impl->HasShapeToMesh() ) theSubShapeID = 1;
  if ( SMESH_subMesh * sm = _impl->GetSubMeshContaining( theSubShapeID ))
  {
    // compute error
    SMESH_ComputeErrorPtr error = sm->GetComputeError();
    if ( error && !error->myBadElements.empty())
    {
      // sort bad elements by type
      vector< const SMDS_MeshElement* > elemsByType[ SMDSAbs_NbElementTypes ];
      list<const SMDS_MeshElement*>::iterator elemIt  = error->myBadElements.begin();
      list<const SMDS_MeshElement*>::iterator elemEnd = error->myBadElements.end();
      for ( ; elemIt != elemEnd; ++elemIt )
      {
        const SMDS_MeshElement* elem = *elemIt;
        if ( !elem ) continue;

        if ( elem->GetID() < 1 )
        {
          // elem is a temporary element, make a real element
          vector< const SMDS_MeshNode* > nodes;
          SMDS_NodeIteratorPtr nIt = elem->nodeIterator();
          while ( nIt->more() && elem )
          {
            nodes.push_back( nIt->next() );
            if ( nodes.back()->GetID() < 1 )
              elem = 0;  // a temporary element on temporary nodes
          }
          if ( elem )
          {
            ::SMESH_MeshEditor editor( _impl );
            elem = editor.AddElement( nodes, elemType.Init( elem ));
          }
        }
        if ( elem )
          elemsByType[ elem->GetType() ].push_back( elem );
      }

      // how many groups to create?
      int nbTypes = 0;
      for ( int i = 0; i < SMDSAbs_NbElementTypes; ++i )
        nbTypes += int( !elemsByType[ i ].empty() );
      groups->length( nbTypes );

      // create groups
      for ( int i = 0, iG = -1; i < SMDSAbs_NbElementTypes; ++i )
      {
        vector< const SMDS_MeshElement* >& elems = elemsByType[ i ];
        if ( elems.empty() ) continue;

        groups[ ++iG ] = createGroup( SMESH::ElementType(i), theGroupName );
        if ( _gen_i->CanPublishInStudy( groups[ iG ] ))
        {
          SALOMEDS::Study_var  study = _gen_i->GetCurrentStudy();
          SMESH::SMESH_Mesh_var mesh = _this();
          SALOMEDS::SObject_wrap aSO =
            _gen_i->PublishGroup( study, mesh, groups[ iG ],
                                 GEOM::GEOM_Object::_nil(), theGroupName);
        }
        SMESH_GroupBase_i* grp_i = SMESH::DownCast< SMESH_GroupBase_i* >( groups[ iG ]);
        if ( !grp_i ) continue;

        if ( SMESHDS_Group*  grpDS = dynamic_cast< SMESHDS_Group* >( grp_i->GetGroupDS() ))
          for ( size_t iE = 0; iE < elems.size(); ++iE )
            grpDS->SMDSGroup().Add( elems[ iE ]);
      }
    }
  }

  return groups._retn();
}

//=============================================================================
/*!
 * Create and publish group servants if any groups were imported or created anyhow
 */
//=============================================================================

void SMESH_Mesh_i::CreateGroupServants()
{
  SALOMEDS::Study_var  aStudy = _gen_i->GetCurrentStudy();
  SMESH::SMESH_Mesh_var aMesh = _this();

  set<int> addedIDs;
  ::SMESH_Mesh::GroupIteratorPtr groupIt = _impl->GetGroups();
  while ( groupIt->more() )
  {
    ::SMESH_Group* group = groupIt->next();
    int            anId = group->GetGroupDS()->GetID();

    map<int, SMESH::SMESH_GroupBase_ptr>::iterator it = _mapGroups.find(anId);
    if ( it != _mapGroups.end() && !CORBA::is_nil( it->second ))
      continue;
    addedIDs.insert( anId );

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

    SMESH::SMESH_GroupBase_var groupVar = aGroupImpl->_this();
    _mapGroups[anId] = SMESH::SMESH_GroupBase::_duplicate( groupVar );
    aGroupImpl->Register();

    // register CORBA object for persistence
    int nextId = _gen_i->RegisterObject( groupVar );
    if(MYDEBUG) { MESSAGE( "Add group to map with id = "<< nextId); }
    else        { nextId = 0; } // avoid "unused variable" warning in release mode

    // publishing the groups in the study
    if ( !aStudy->_is_nil() ) {
      GEOM::GEOM_Object_var shapeVar = _gen_i->ShapeToGeomObject( shape );
      _gen_i->PublishGroup( aStudy, aMesh, groupVar, shapeVar, group->GetName());
    }
  }
  if ( !addedIDs.empty() )
  {
    // python dump
    set<int>::iterator id = addedIDs.begin();
    for ( ; id != addedIDs.end(); ++id )
    {
      map<int, SMESH::SMESH_GroupBase_ptr>::iterator it = _mapGroups.find(*id);
      int i = std::distance( _mapGroups.begin(), it );
      TPythonDump() << it->second << " = " << aMesh << ".GetGroups()[ "<< i << " ]";
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

SMESH::MedFileInfo* SMESH_Mesh_i::GetMEDFileInfo()
{
  SMESH::MedFileInfo_var res( _medFileInfo );
  if ( !res.operator->() ) {
    res = new SMESH::MedFileInfo;
    res->fileName = "";
    res->fileSize = res->major = res->minor = res->release = -1;
  }
  return res._retn();
}

//=============================================================================
/*!
 * \brief Pass names of mesh groups from study to mesh DS
 */
//=============================================================================

void SMESH_Mesh_i::checkGroupNames()
{
  int nbGrp = NbGroups();
  if ( !nbGrp )
    return;

  SALOMEDS::Study_var aStudy = _gen_i->GetCurrentStudy();
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
    SALOMEDS::SObject_wrap aGrpSO = _gen_i->ObjectToSObject( aStudy, aGrp );
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
  SMESH_Gen_i::GetSMESHGen()->UpdateParameters( CORBA::Object_var( _this() ).in(),
                                                theParameters );
}

//=============================================================================
/*!
 * \brief Returns list of notebook variables used for Mesh operations separated by ":" symbol
 */
//=============================================================================

char* SMESH_Mesh_i::GetParameters()
{
  return SMESH_Gen_i::GetSMESHGen()->GetParameters( SMESH::SMESH_Mesh_var( _this()) );
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
    CORBA::String_var aParameters = GetParameters();
    SALOMEDS::Study_var    aStudy = gen->GetCurrentStudy();
    if ( !aStudy->_is_nil()) {
      SALOMEDS::ListOfListOfStrings_var aSections = aStudy->ParseVariables(aParameters); 
      if ( aSections->length() > 0 ) {
        SALOMEDS::ListOfStrings aVars = aSections[ aSections->length() - 1 ];
        aResult->length( aVars.length() );
        for ( CORBA::ULong i = 0;i < aVars.length(); i++ )
          aResult[i] = CORBA::string_dup( aVars[i] );
      }
    }
  }
  return aResult._retn();
}

//=======================================================================
//function : GetTypes
//purpose  : Returns types of elements it contains
//=======================================================================

SMESH::array_of_ElementType* SMESH_Mesh_i::GetTypes()
{
  if ( _preMeshInfo )
    return _preMeshInfo->GetTypes();

  SMESH::array_of_ElementType_var types = new SMESH::array_of_ElementType;

  types->length( 5 );
  int nbTypes = 0;
  if (_impl->NbEdges())      types[nbTypes++] = SMESH::EDGE;
  if (_impl->NbFaces())      types[nbTypes++] = SMESH::FACE;
  if (_impl->NbVolumes())    types[nbTypes++] = SMESH::VOLUME;
  if (_impl->Nb0DElements()) types[nbTypes++] = SMESH::ELEM0D;
  if (_impl->NbBalls())      types[nbTypes++] = SMESH::BALL;
  if (_impl->NbNodes() &&
      nbTypes == 0 )         types[nbTypes++] = SMESH::NODE;
  types->length( nbTypes );

  return types._retn();
}

//=======================================================================
//function : GetMesh
//purpose  : Returns self
//=======================================================================

SMESH::SMESH_Mesh_ptr SMESH_Mesh_i::GetMesh()
{
  return SMESH::SMESH_Mesh::_duplicate( _this() );
}

//=======================================================================
//function : IsMeshInfoCorrect
//purpose  : * Returns false if GetMeshInfo() returns incorrect information that may
//           * happen if mesh data is not yet fully loaded from the file of study.
//=======================================================================

bool SMESH_Mesh_i::IsMeshInfoCorrect()
{
  return _preMeshInfo ? _preMeshInfo->IsMeshInfoCorrect() : true;
}

//=============================================================================
/*!
 * \brief Returns number of mesh elements per each \a EntityType
 */
//=============================================================================

SMESH::long_array* SMESH_Mesh_i::GetMeshInfo()
{
  if ( _preMeshInfo )
    return _preMeshInfo->GetMeshInfo();

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
 * \brief Returns number of mesh elements per each \a ElementType
 */
//=============================================================================

SMESH::long_array* SMESH_Mesh_i::GetNbElementsByType()
{
  SMESH::long_array_var aRes = new SMESH::long_array();
  aRes->length(SMESH::NB_ELEMENT_TYPES);
  for (int i = 0; i < SMESH::NB_ELEMENT_TYPES; i++)
    aRes[ i ] = 0;

  const SMDS_MeshInfo* meshInfo = 0;
  if ( _preMeshInfo )
    meshInfo = _preMeshInfo;
  else if ( SMESHDS_Mesh* meshDS = _impl->GetMeshDS() )
    meshInfo = & meshDS->GetMeshInfo();

  if (meshInfo)
    for (int i = 0; i < SMESH::NB_ELEMENT_TYPES; i++)
      aRes[i] = meshInfo->NbElements((SMDSAbs_ElementType)i);

  return aRes._retn();
}

//=============================================================================
/*
 * Collect statistic of mesh elements given by iterator
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
/*
 * Returns mesh unstructed grid information.
 */
//=============================================================================

SALOMEDS::TMPFile* SMESH_Mesh_i::GetVtkUgStream()
{
  SALOMEDS::TMPFile_var SeqFile;
  if ( SMESHDS_Mesh* aMeshDS = _impl->GetMeshDS() ) {
    SMDS_UnstructuredGrid* aGrid = aMeshDS->getGrid();
    if(aGrid) {
      vtkUnstructuredGridWriter* aWriter = vtkUnstructuredGridWriter::New();
      aWriter->WriteToOutputStringOn();
      aWriter->SetInputData(aGrid);
      aWriter->SetFileTypeToBinary();
      aWriter->Write();
      char* str = aWriter->GetOutputString();
      int size = aWriter->GetOutputStringLength();
      
      //Allocate octect buffer of required size
      CORBA::Octet* OctetBuf = SALOMEDS::TMPFile::allocbuf(size);
      //Copy ostrstream content to the octect buffer
      memcpy(OctetBuf, str, size);
      //Create and return TMPFile
      SeqFile = new SALOMEDS::TMPFile(size, size, OctetBuf, 1);
      aWriter->Delete();
    }
  }
  return SeqFile._retn();
}

//=============================================================================
namespace /* Iterators used in SMESH_Mesh_i::GetElements(SMESH::SMESH_IDSource_var obj,
           *                                             SMESH::ElementType        type) */
{
  using namespace SMESH::Controls;
  //-----------------------------------------------------------------------------
  struct PredicateIterator : public SMDS_ElemIterator
  {
    SMDS_ElemIteratorPtr    _elemIter;
    PredicatePtr            _predicate;
    const SMDS_MeshElement* _elem;

    PredicateIterator( SMDS_ElemIteratorPtr   iterator,
                       PredicatePtr predicate):
      _elemIter(iterator), _predicate(predicate)
    {
      next();
    }
    virtual bool more()
    {
      return _elem;
    }
    virtual const SMDS_MeshElement* next()
    {
      const SMDS_MeshElement* res = _elem;
      _elem = 0;
      while ( _elemIter->more() && !_elem )
      {
        _elem = _elemIter->next();
        if ( _elem && ( !_predicate->IsSatisfy( _elem->GetID() )))
          _elem = 0;
      }
      return res;
    }
  };

  //-----------------------------------------------------------------------------
  struct IDSourceIterator : public SMDS_ElemIterator
  {
    const CORBA::Long*        _idPtr;
    const CORBA::Long*        _idEndPtr;
    SMESH::long_array_var     _idArray;
    const SMDS_Mesh*          _mesh;
    const SMDSAbs_ElementType _type;
    const SMDS_MeshElement*   _elem;

    IDSourceIterator( const SMDS_Mesh*    mesh,
                      const CORBA::Long*  ids,
                      const int           nbIds,
                      SMDSAbs_ElementType type):
      _idPtr( ids ), _idEndPtr( ids + nbIds ), _mesh( mesh ), _type( type ), _elem( 0 )
    {
      if ( _idPtr && nbIds && _mesh )
        next();
    }
    IDSourceIterator( const SMDS_Mesh*    mesh,
                      SMESH::long_array*  idArray,
                      SMDSAbs_ElementType type):
      _idPtr( 0 ), _idEndPtr( 0 ), _idArray( idArray), _mesh( mesh ), _type( type ), _elem( 0 )
    {
      if ( idArray && _mesh )
      {
        _idPtr    = &_idArray[0];
        _idEndPtr = _idPtr + _idArray->length();
        next();
      }
    }
    virtual bool more()
    {
      return _elem;
    }
    virtual const SMDS_MeshElement* next()
    {
      const SMDS_MeshElement* res = _elem;
      _elem = 0;
      while ( _idPtr < _idEndPtr && !_elem )
      {
        if ( _type == SMDSAbs_Node )
        {
          _elem = _mesh->FindNode( *_idPtr++ );
        }
        else if ((_elem = _mesh->FindElement( *_idPtr++ )) &&
                 (_elem->GetType() != _type && _type != SMDSAbs_All ))
        {
          _elem = 0;
        }
      }
      return res;
    }
  };
  //-----------------------------------------------------------------------------

  struct NodeOfElemIterator : public SMDS_ElemIterator
  {
    TColStd_MapOfInteger    _checkedNodeIDs;
    SMDS_ElemIteratorPtr    _elemIter;
    SMDS_ElemIteratorPtr    _nodeIter;
    const SMDS_MeshElement* _node;

    NodeOfElemIterator( SMDS_ElemIteratorPtr iter ): _elemIter( iter ), _node( 0 )
    {
      if ( _elemIter && _elemIter->more() )
      {
        _nodeIter = _elemIter->next()->nodesIterator();
        next();
      }
    }
    virtual bool more()
    {
      return _node;
    }
    virtual const SMDS_MeshElement* next()
    {
      const SMDS_MeshElement* res = _node;
      _node = 0;
      while (( _elemIter->more() || _nodeIter->more() ) && !_node )
      {
        if ( _nodeIter->more() )
        {
          _node = _nodeIter->next();
          if ( !_checkedNodeIDs.Add( _node->GetID() ))
            _node = 0;
        }
        else
        {
          _nodeIter = _elemIter->next()->nodesIterator();
        }
      }
      return res;
    }
  };
}

//=============================================================================
/*
 * Return iterator on elements of given type in given object
 */
//=============================================================================

SMDS_ElemIteratorPtr SMESH_Mesh_i::GetElements(SMESH::SMESH_IDSource_ptr theObject,
                                               SMESH::ElementType        theType)
{
  SMDS_ElemIteratorPtr  elemIt;
  bool                  typeOK = ( theType == SMESH::ALL );
  SMDSAbs_ElementType elemType = SMDSAbs_ElementType( theType );

  SMESH::SMESH_Mesh_var meshVar = theObject->GetMesh();
  SMESH_Mesh_i*          mesh_i = SMESH::DownCast<SMESH_Mesh_i*>( meshVar );
  if ( !mesh_i ) return elemIt;
  SMESHDS_Mesh*          meshDS = mesh_i->GetImpl().GetMeshDS();

  if ( SMESH::DownCast<SMESH_Mesh_i*>( theObject ))
  {
    elemIt = meshDS->elementsIterator( elemType );
    typeOK = true;
  }
  else if ( SMESH_subMesh_i* submesh_i = SMESH::DownCast<SMESH_subMesh_i*>( theObject ))
  {
    SMESHDS_SubMesh* sm = ((SMESHDS_Mesh*) meshDS)->MeshElements( submesh_i->GetId() );
    if ( sm )
    {
      elemIt = sm->GetElements();
      if ( elemType != SMDSAbs_Node && elemType != SMDSAbs_All )
      {
        typeOK = ( elemIt && elemIt->more() && elemIt->next()->GetType() == elemType );
        elemIt = typeOK ? sm->GetElements() : SMDS_ElemIteratorPtr();
      }
    }
  }
  else if ( SMESH_GroupBase_i* group_i = SMESH::DownCast<SMESH_GroupBase_i*>( theObject ))
  {
    SMESHDS_GroupBase* groupDS = group_i->GetGroupDS();
    if ( groupDS && ( elemType == groupDS->GetType()  ||
                      elemType == SMDSAbs_Node ||
                      elemType == SMDSAbs_All ))
    {
      elemIt = groupDS->GetElements();
      typeOK = ( groupDS->GetType() == elemType || elemType == SMDSAbs_All );
    }
  }
  else if ( SMESH::Filter_i* filter_i = SMESH::DownCast<SMESH::Filter_i*>( theObject ))
  {
    if ( filter_i->GetElementType() == theType ||
         elemType == SMDSAbs_Node ||
         elemType == SMDSAbs_All)
    {
      SMESH::Predicate_i* pred_i = filter_i->GetPredicate_i();
      if ( pred_i && pred_i->GetPredicate() )
      {
        SMDSAbs_ElementType filterType = SMDSAbs_ElementType( filter_i->GetElementType() );
        SMDS_ElemIteratorPtr allElemIt = meshDS->elementsIterator( filterType );
        elemIt = SMDS_ElemIteratorPtr( new PredicateIterator( allElemIt, pred_i->GetPredicate() ));
        typeOK = ( filterType == elemType || elemType == SMDSAbs_All );
      }
    }
  }
  else
  {
    SMESH::array_of_ElementType_var types = theObject->GetTypes();
    const bool                    isNodes = ( types->length() == 1 && types[0] == SMESH::NODE );
    if ( isNodes && elemType != SMDSAbs_Node && elemType != SMDSAbs_All )
      return elemIt;
    if ( SMESH_MeshEditor_i::IsTemporaryIDSource( theObject ))
    {
      int nbIds;
      if ( CORBA::Long* ids = SMESH_MeshEditor_i::GetTemporaryIDs( theObject, nbIds ))
        elemIt = SMDS_ElemIteratorPtr( new IDSourceIterator( meshDS, ids, nbIds, elemType ));
    }
    else
    {
      SMESH::long_array_var ids = theObject->GetIDs();
      elemIt = SMDS_ElemIteratorPtr( new IDSourceIterator( meshDS, ids._retn(), elemType ));
    }
    typeOK = ( isNodes == ( elemType == SMDSAbs_Node )) || ( elemType == SMDSAbs_All );
  }

  if ( elemIt && elemIt->more() && !typeOK )
  {
    if ( elemType == SMDSAbs_Node )
    {
      elemIt = SMDS_ElemIteratorPtr( new NodeOfElemIterator( elemIt ));
    }
    else
    {
      elemIt = SMDS_ElemIteratorPtr();
    }
  }
  return elemIt;
}

//=============================================================================
namespace // Finding concurrent hypotheses
//=============================================================================
{

/*!
 * \brief mapping of mesh dimension into shape type
 */
TopAbs_ShapeEnum shapeTypeByDim(const int theDim)
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

//-----------------------------------------------------------------------------
/*!
 * \brief Internal structure used to find concurent submeshes
 *
 * It represents a pair < submesh, concurent dimension >, where
 * 'concurrent dimension' is dimension of shape where the submesh can concurent
 *  with another submesh. In other words, it is dimension of a hypothesis assigned
 *  to submesh.
 */
class SMESH_DimHyp
{
 public:
  //! fileds
  int _dim;    //!< a dimension the algo can build (concurrent dimension)
  int _ownDim; //!< dimension of shape of _subMesh (>=_dim)
  TopTools_MapOfShape _shapeMap;
  SMESH_subMesh*      _subMesh;
  list<const SMESHDS_Hypothesis*> _hypotheses; //!< algo is first, then its parameters

  //-----------------------------------------------------------------------------
  // Return the algorithm
  const SMESH_Algo* GetAlgo() const
  { return _hypotheses.empty() ? 0 : dynamic_cast<const SMESH_Algo*>( _hypotheses.front() ); }

  //-----------------------------------------------------------------------------
  //! Constructors
  SMESH_DimHyp(const SMESH_subMesh* theSubMesh,
               const int            theDim,
               const TopoDS_Shape&  theShape)
  {
    _subMesh = (SMESH_subMesh*)theSubMesh;
    SetShape( theDim, theShape );
  }

  //-----------------------------------------------------------------------------
  //! set shape
  void SetShape(const int           theDim,
                const TopoDS_Shape& theShape)
  {
    _dim = theDim;
    _ownDim = SMESH_Gen::GetShapeDim(theShape);
    if (_dim >= _ownDim)
      _shapeMap.Add( theShape );
    else {
      TopExp_Explorer anExp( theShape, shapeTypeByDim(theDim) );
      for( ; anExp.More(); anExp.Next() )
        _shapeMap.Add( anExp.Current() );
    }
  }

  //-----------------------------------------------------------------------------
  //! Check sharing of sub-shapes
  static bool isShareSubShapes(const TopTools_MapOfShape& theToCheck,
                               const TopTools_MapOfShape& theToFind,
                               const TopAbs_ShapeEnum     theType)
  {
    bool isShared = false;
    TopTools_MapIteratorOfMapOfShape anItr( theToCheck );
    for (; !isShared && anItr.More(); anItr.Next() )
    {
      const TopoDS_Shape aSubSh = anItr.Key();
      // check for case when concurrent dimensions are same
      isShared = theToFind.Contains( aSubSh );
      // check for sub-shape with concurrent dimension
      TopExp_Explorer anExp( aSubSh, theType );
      for ( ; !isShared && anExp.More(); anExp.Next() )
        isShared = theToFind.Contains( anExp.Current() );
    }
    return isShared;
  }
  
  //-----------------------------------------------------------------------------
  //! check algorithms
  static bool checkAlgo(const SMESHDS_Hypothesis* theA1,
                        const SMESHDS_Hypothesis* theA2)
  {
    if ( !theA1 || !theA2 ||
         theA1->GetType() == SMESHDS_Hypothesis::PARAM_ALGO ||
         theA2->GetType() == SMESHDS_Hypothesis::PARAM_ALGO )
      return false; // one of the hypothesis is not algorithm
    // check algorithm names (should be equal)
    return strcmp( theA1->GetName(), theA2->GetName() ) == 0;
  }

  
  //-----------------------------------------------------------------------------
  //! Check if sub-shape hypotheses are concurrent
  bool IsConcurrent(const SMESH_DimHyp* theOther) const
  {
    if ( _subMesh == theOther->_subMesh )
      return false; // same sub-shape - should not be

    // if ( <own dim of either of submeshes> == <concurrent dim> &&
    //      any of the two submeshes is not on COMPOUND shape )
    //  -> no concurrency
    bool meIsCompound    = (_subMesh->GetSubMeshDS() &&
                            _subMesh->GetSubMeshDS()->IsComplexSubmesh());
    bool otherIsCompound = (theOther->_subMesh->GetSubMeshDS() &&
                            theOther->_subMesh->GetSubMeshDS()->IsComplexSubmesh());
    if ( (_ownDim == _dim  || theOther->_ownDim == _dim ) && (!meIsCompound || !otherIsCompound))
      return false;

//     bool checkSubShape = ( _dim >= theOther->_dim )
//       ? isShareSubShapes( _shapeMap, theOther->_shapeMap, shapeTypeByDim(theOther->_dim) )
//       : isShareSubShapes( theOther->_shapeMap, _shapeMap, shapeTypeByDim(_dim) ) ;
    bool checkSubShape = isShareSubShapes( _shapeMap, theOther->_shapeMap, shapeTypeByDim(_dim));
    if ( !checkSubShape )
        return false;

    // check algorithms to be same
    if ( !checkAlgo( this->GetAlgo(), theOther->GetAlgo() ))
      return true; // different algorithms -> concurrency !

    // check hypothesises for concurrence (skip first as algorithm)
    int nbSame = 0;
    // pointers should be same, because it is referened from mesh hypothesis partition
    list <const SMESHDS_Hypothesis*>::const_iterator hypIt = _hypotheses.begin();
    list <const SMESHDS_Hypothesis*>::const_iterator otheEndIt = theOther->_hypotheses.end();
    for ( hypIt++ /*skip first as algo*/; hypIt != _hypotheses.end(); hypIt++ )
      if ( find( theOther->_hypotheses.begin(), otheEndIt, *hypIt ) != otheEndIt )
        nbSame++;
    // the submeshes are concurrent if their algorithms has different parameters
    return nbSame != (int)theOther->_hypotheses.size() - 1;
  }

  // Return true if algorithm of this SMESH_DimHyp is used if no
  // sub-mesh order is imposed by the user
  bool IsHigherPriorityThan( const SMESH_DimHyp* theOther ) const
  {
    // NeedDiscreteBoundary() algo has a higher priority
    if ( this    ->GetAlgo()->NeedDiscreteBoundary() !=
         theOther->GetAlgo()->NeedDiscreteBoundary() )
      return !this->GetAlgo()->NeedDiscreteBoundary();

    return ( this->_subMesh->GetId() < theOther->_subMesh->GetId() );
  }
  
}; // end of SMESH_DimHyp
//-----------------------------------------------------------------------------

typedef list<const SMESH_DimHyp*> TDimHypList;

//-----------------------------------------------------------------------------

void addDimHypInstance(const int                               theDim, 
                       const TopoDS_Shape&                     theShape,
                       const SMESH_Algo*                       theAlgo,
                       const SMESH_subMesh*                    theSubMesh,
                       const list <const SMESHDS_Hypothesis*>& theHypList,
                       TDimHypList*                            theDimHypListArr )
{
  TDimHypList& listOfdimHyp = theDimHypListArr[theDim];
  if ( listOfdimHyp.empty() || listOfdimHyp.back()->_subMesh != theSubMesh ) {
    SMESH_DimHyp* dimHyp = new SMESH_DimHyp( theSubMesh, theDim, theShape );
    dimHyp->_hypotheses.push_front(theAlgo);
    listOfdimHyp.push_back( dimHyp );
  }
  
  SMESH_DimHyp* dimHyp = const_cast<SMESH_DimHyp*>( listOfdimHyp.back() );
  dimHyp->_hypotheses.insert( dimHyp->_hypotheses.end(),
                              theHypList.begin(), theHypList.end() );
}

//-----------------------------------------------------------------------------
void addInOrderOfPriority( const SMESH_DimHyp* theDimHyp,
                           TDimHypList&        theListOfConcurr)
{
  if ( theListOfConcurr.empty() )
  {
    theListOfConcurr.push_back( theDimHyp );
  }
  else
  {
    TDimHypList::iterator hypIt = theListOfConcurr.begin();
    while ( hypIt != theListOfConcurr.end() &&
            !theDimHyp->IsHigherPriorityThan( *hypIt ))
      ++hypIt;
    theListOfConcurr.insert( hypIt, theDimHyp );
  }
}

//-----------------------------------------------------------------------------
void findConcurrents(const SMESH_DimHyp* theDimHyp,
                     const TDimHypList&  theListOfDimHyp,
                     TDimHypList&        theListOfConcurrHyp,
                     set<int>&           theSetOfConcurrId )
{
  TDimHypList::const_reverse_iterator rIt = theListOfDimHyp.rbegin();
  for ( ; rIt != theListOfDimHyp.rend(); rIt++ )
  {
    const SMESH_DimHyp* curDimHyp = *rIt;
    if ( curDimHyp == theDimHyp )
      break; // meet own dimHyp pointer in same dimension

    if ( theDimHyp->IsConcurrent( curDimHyp ) &&
         theSetOfConcurrId.insert( curDimHyp->_subMesh->GetId() ).second )
    {
      addInOrderOfPriority( curDimHyp, theListOfConcurrHyp );
    }
  }
}

//-----------------------------------------------------------------------------
void unionLists(TListOfInt&       theListOfId,
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
//-----------------------------------------------------------------------------

//! free memory allocated for dimension-hypothesis objects
void removeDimHyps( TDimHypList* theArrOfList )
{
  for (int i = 0; i < 4; i++ ) {
    TDimHypList& listOfdimHyp = theArrOfList[i];
    TDimHypList::const_iterator it = listOfdimHyp.begin();
    for ( ; it != listOfdimHyp.end(); it++ )
      delete (*it);
  }
}

//-----------------------------------------------------------------------------
/*!
 * \brief find common submeshes with given submesh
 * \param theSubMeshList list of already collected submesh to check
 * \param theSubMesh given submesh to intersect with other
 * \param theCommonSubMeshes collected common submeshes
 */
void findCommonSubMesh (list<const SMESH_subMesh*>& theSubMeshList,
                        const SMESH_subMesh*        theSubMesh,
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

//-----------------------------------------------------------------------------
bool isSubMeshInList ( int smID, const TListOfListOfInt& smLists )
{
  TListOfListOfInt::const_iterator listsIt = smLists.begin();
  for ( ; listsIt != smLists.end(); ++listsIt )
  {
    const TListOfInt& smIDs = *listsIt;
    if ( std::find( smIDs.begin(), smIDs.end(), smID ) != smIDs.end() )
      return true;
  }
  return false;
}

} // namespace

//=============================================================================
/*!
 * \brief Return \c true if a meshing order not yet set for a concurrent sub-mesh
 */
//=============================================================================

CORBA::Boolean SMESH_Mesh_i::IsUnorderedSubMesh(CORBA::Long submeshID)
{
  TListOfListOfInt anOrder = GetImpl().GetMeshOrder(); // already defined order
  if ( isSubMeshInList( submeshID, anOrder ))
    return false;

  TListOfListOfInt allConurrent = findConcurrentSubMeshes();
  return isSubMeshInList( submeshID, allConurrent );
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

  TListOfListOfInt      anOrder = GetImpl().GetMeshOrder(); // already defined order
  TListOfListOfInt allConurrent = findConcurrentSubMeshes();
  anOrder.splice( anOrder.end(), allConurrent );

  int listIndx = 0;
  TListOfListOfInt::iterator listIt = anOrder.begin();
  for(; listIt != anOrder.end(); listIt++, listIndx++ )
    unionLists( *listIt,  anOrder, listIndx + 1 );

  // convert submesh ids into interface instances
  //  and dump command into python
  convertMeshOrder( anOrder, aResult, false );

  return aResult._retn();
}

//=============================================================================
/*!
 * \brief Finds concurrent sub-meshes
 */
//=============================================================================

TListOfListOfInt SMESH_Mesh_i::findConcurrentSubMeshes()
{
  TListOfListOfInt anOrder;
  ::SMESH_Mesh& mesh = GetImpl();
  {
    // collect submeshes and detect concurrent algorithms and hypothesises
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
          // try to find algorithm with help of sub-shapes
          TopExp_Explorer anExp( aSubMeshShape, shapeTypeByDim(hyp->GetDim()) );
          for ( ; !anAlgo && anExp.More(); anExp.Next() )
            anAlgo = mesh.GetGen()->GetAlgo( mesh, anExp.Current() );
        }
        if (!anAlgo)
          continue; // no algorithm assigned to a current submesh

        int dim = anAlgo->GetDim(); // top concurrent dimension (see comment to SMESH_DimHyp)
        // the submesh can concurrent at <dim> (or lower dims if !anAlgo->NeedDiscreteBoundary())

        // create instance of dimension-hypothesis for found concurrent dimension(s) and algorithm
        for ( int j = anAlgo->NeedDiscreteBoundary() ? dim : 1, jn = dim; j <= jn; j++ )
          addDimHypInstance( j, aSubMeshShape, anAlgo, sm, hypList, dimHypListArr );
      }
    } // end iterations on submesh
    
    // iterate on created dimension-hypotheses and check for concurrents
    for ( int i = 0; i < 4; i++ ) {
      const TDimHypList& listOfDimHyp = dimHypListArr[i];
      // check for concurrents in own and other dimensions (step-by-step)
      TDimHypList::const_iterator dhIt = listOfDimHyp.begin();
      for ( ; dhIt != listOfDimHyp.end(); dhIt++ ) {
        const SMESH_DimHyp* dimHyp = *dhIt;
        TDimHypList listOfConcurr;
        set<int>    setOfConcurrIds;
        // looking for concurrents and collect into own list
        for ( int j = i; j < 4; j++ )
          findConcurrents( dimHyp, dimHypListArr[j], listOfConcurr, setOfConcurrIds );
        // check if any concurrents found
        if ( listOfConcurr.size() > 0 ) {
          // add own submesh to list of concurrent
          addInOrderOfPriority( dimHyp, listOfConcurr );
          list<int> listOfConcurrIds;
          TDimHypList::iterator hypIt = listOfConcurr.begin();
          for ( ; hypIt != listOfConcurr.end(); ++hypIt )
            listOfConcurrIds.push_back( (*hypIt)->_subMesh->GetId() );
          anOrder.push_back( listOfConcurrIds );
        }
      }
    }
    
    removeDimHyps(dimHypListArr);
    
    // now, minimise the number of concurrent groups
    // Here we assume that lists of submeshes can have same submesh
    // in case of multi-dimension algorithms, as result
    //  list with common submesh has to be united into one list
    int listIndx = 0;
    TListOfListOfInt::iterator listIt = anOrder.begin();
    for(; listIt != anOrder.end(); listIt++, listIndx++ )
      unionLists( *listIt,  anOrder, listIndx + 1 );
  }

  return anOrder;
}

//=============================================================================
/*!
 * \brief Set submesh object order
 * \param theSubMeshArray submesh array order
 */
//=============================================================================

::CORBA::Boolean SMESH_Mesh_i::SetMeshOrder(const SMESH::submesh_array_array& theSubMeshArray)
{
  if ( _preMeshInfo )
    _preMeshInfo->ForgetOrLoad();

  bool res = false;
  ::SMESH_Mesh& mesh = GetImpl();

  TPythonDump aPythonDump; // prevent dump of called methods
  aPythonDump << "isDone = " << SMESH::SMESH_Mesh_var(_this()) << ".SetMeshOrder( [ ";

  TListOfListOfInt subMeshOrder;
  for ( int i = 0, n = theSubMeshArray.length(); i < n; i++ )
  {
    const SMESH::submesh_array& aSMArray = theSubMeshArray[i];
    TListOfInt subMeshIds;
    if ( i > 0 )
      aPythonDump << ", ";
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
    for ( ; clrIt != subMeshToClear.end(); clrIt++ )
      if ( SMESH_subMesh* sm = (SMESH_subMesh*)*clrIt )
        sm->ComputeStateEngine( SMESH_subMesh::CLEAN );
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

void SMESH_Mesh_i::convertMeshOrder (const TListOfListOfInt&     theIdsOrder,
                                     SMESH::submesh_array_array& theResOrder,
                                     const bool                  theIsDump)
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
    int j;
    for( j = 0; subIt != aSubOrder.end(); subIt++ ) {
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
    if ( j > 1 )
      theResOrder[ listIndx++ ] = aResSubSet;
  }
  // correct number of lists
  theResOrder.length( listIndx );

  if ( theIsDump ) {
    // finilise python dump
    aPythonDump << " ]";
    aPythonDump << " = " << SMESH::SMESH_Mesh_var(_this()) << ".GetMeshOrder()";
  }
}

//================================================================================
//
// Implementation of SMESH_MeshPartDS
//
SMESH_MeshPartDS::SMESH_MeshPartDS(SMESH::SMESH_IDSource_ptr meshPart):
  SMESHDS_Mesh( /*meshID=*/-1, /*isEmbeddedMode=*/true)
{
  SMESH::SMESH_Mesh_var mesh = meshPart->GetMesh();
  SMESH_Mesh_i*       mesh_i = SMESH::DownCast<SMESH_Mesh_i*>( mesh );

  _meshDS = mesh_i->GetImpl().GetMeshDS();

  SetPersistentId( _meshDS->GetPersistentId() );

  if ( mesh_i == SMESH::DownCast<SMESH_Mesh_i*>( meshPart ))
  {
    // <meshPart> is the whole mesh
    myInfo = _meshDS->GetMeshInfo(); // copy mesh info;
    // copy groups
    set<SMESHDS_GroupBase*>& myGroupSet = const_cast<set<SMESHDS_GroupBase*>&>( GetGroups() );
    myGroupSet = _meshDS->GetGroups();
  }
  else
  {
    TMeshInfo tmpInfo;
    SMESH::long_array_var           anIDs = meshPart->GetIDs();
    SMESH::array_of_ElementType_var types = meshPart->GetTypes();
    if ( types->length() == 1 && types[0] == SMESH::NODE ) // group of nodes
    {
      for ( CORBA::ULong i=0; i < anIDs->length(); i++ )
        if ( const SMDS_MeshNode * n = _meshDS->FindNode( anIDs[i] ))
          if ( _elements[ SMDSAbs_Node ].insert( n ).second )
            tmpInfo.Add( n );
    }
    else
    {
      for ( CORBA::ULong i=0; i < anIDs->length(); i++ )
        if ( const SMDS_MeshElement * e = _meshDS->FindElement(anIDs[i]))
          if ( _elements[ e->GetType() ].insert( e ).second )
          {
            tmpInfo.Add( e );
            SMDS_ElemIteratorPtr nIt = e->nodesIterator();
            while ( nIt->more() )
            {
              const SMDS_MeshNode * n = (const SMDS_MeshNode*) nIt->next();
              if ( _elements[ SMDSAbs_Node ].insert( n ).second )
                tmpInfo.Add( n );
            }
          }
    }
    myInfo = tmpInfo;

    ShapeToMesh( _meshDS->ShapeToMesh() );

    _meshDS = 0; // to enforce iteration on _elements and _nodes
  }
}
// -------------------------------------------------------------------------------------
SMESH_MeshPartDS::SMESH_MeshPartDS(const std::list< const SMDS_MeshElement* > & meshPart):
  SMESHDS_Mesh( /*meshID=*/-1, /*isEmbeddedMode=*/true), _meshDS(0)
{
  TMeshInfo tmpInfo;
  list< const SMDS_MeshElement* >::const_iterator partIt = meshPart.begin();
  for ( ; partIt != meshPart.end(); ++partIt )
    if ( const SMDS_MeshElement * e = *partIt )
      if ( _elements[ e->GetType() ].insert( e ).second )
      {
        tmpInfo.Add( e );
        SMDS_ElemIteratorPtr nIt = e->nodesIterator();
        while ( nIt->more() )
        {
          const SMDS_MeshNode * n = (const SMDS_MeshNode*) nIt->next();
          if ( _elements[ SMDSAbs_Node ].insert( n ).second )
            tmpInfo.Add( n );
        }
      }
  myInfo = tmpInfo;
}
// -------------------------------------------------------------------------------------
SMDS_ElemIteratorPtr SMESH_MeshPartDS::elementGeomIterator(SMDSAbs_GeometryType geomType) const
{
  if ( _meshDS ) return _meshDS->elementGeomIterator( geomType );

  typedef SMDS_SetIterator
    <const SMDS_MeshElement*,
    TIDSortedElemSet::const_iterator,
    SMDS::SimpleAccessor<const SMDS_MeshElement*, TIDSortedElemSet::const_iterator>,
    SMDS_MeshElement::GeomFilter
    > TIter;

  SMDSAbs_ElementType type = SMDS_MeshCell::toSmdsType( geomType );

  return SMDS_ElemIteratorPtr( new TIter( _elements[type].begin(),
                                          _elements[type].end(),
                                          SMDS_MeshElement::GeomFilter( geomType )));
}
// -------------------------------------------------------------------------------------
SMDS_ElemIteratorPtr SMESH_MeshPartDS::elementEntityIterator(SMDSAbs_EntityType entity) const
{
  if ( _meshDS ) return _meshDS->elementEntityIterator( entity );

  typedef SMDS_SetIterator
    <const SMDS_MeshElement*,
    TIDSortedElemSet::const_iterator,
    SMDS::SimpleAccessor<const SMDS_MeshElement*, TIDSortedElemSet::const_iterator>,
    SMDS_MeshElement::EntityFilter
    > TIter;

  SMDSAbs_ElementType type = SMDS_MeshCell::toSmdsType( entity );

  return SMDS_ElemIteratorPtr( new TIter( _elements[type].begin(),
                                          _elements[type].end(),
                                          SMDS_MeshElement::EntityFilter( entity )));
}
// -------------------------------------------------------------------------------------
SMDS_ElemIteratorPtr SMESH_MeshPartDS::elementsIterator(SMDSAbs_ElementType type) const
{
  typedef SMDS_SetIterator<const SMDS_MeshElement*, TIDSortedElemSet::const_iterator > TIter;
  if ( type == SMDSAbs_All && !_meshDS )
  {
    typedef vector< SMDS_ElemIteratorPtr > TIterVec;
    TIterVec iterVec;
    for ( int i = 0; i < SMDSAbs_NbElementTypes; ++i )
      if ( !_elements[i].empty() && i != SMDSAbs_Node )
        iterVec.push_back
          ( SMDS_ElemIteratorPtr( new TIter( _elements[i].begin(), _elements[i].end() )));

    typedef SMDS_IteratorOnIterators<const SMDS_MeshElement*, TIterVec > TIterOnIters;
    return SMDS_ElemIteratorPtr( new TIterOnIters( iterVec ));
  }
  return _meshDS ? _meshDS->elementsIterator(type) : SMDS_ElemIteratorPtr
      ( new TIter( _elements[type].begin(), _elements[type].end() ));
}
// -------------------------------------------------------------------------------------
#define _GET_ITER_DEFINE( iterType, methName, elem, elemType)                       \
  iterType SMESH_MeshPartDS::methName( bool idInceasingOrder) const                 \
  {                                                                                 \
    typedef SMDS_SetIterator<const elem*, TIDSortedElemSet::const_iterator > TIter; \
    return _meshDS ? _meshDS->methName(idInceasingOrder) : iterType                 \
      ( new TIter( _elements[elemType].begin(), _elements[elemType].end() ));       \
  }
// -------------------------------------------------------------------------------------
_GET_ITER_DEFINE( SMDS_NodeIteratorPtr, nodesIterator, SMDS_MeshNode, SMDSAbs_Node )
_GET_ITER_DEFINE( SMDS_EdgeIteratorPtr, edgesIterator, SMDS_MeshEdge, SMDSAbs_Edge )
_GET_ITER_DEFINE( SMDS_FaceIteratorPtr, facesIterator, SMDS_MeshFace, SMDSAbs_Face )
_GET_ITER_DEFINE( SMDS_VolumeIteratorPtr, volumesIterator, SMDS_MeshVolume, SMDSAbs_Volume)
#undef _GET_ITER_DEFINE
//
// END Implementation of SMESH_MeshPartDS
//
//================================================================================


