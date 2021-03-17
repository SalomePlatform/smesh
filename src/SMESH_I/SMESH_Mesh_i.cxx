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
//  File   : SMESH_Mesh_i.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH

#include "SMESH_Mesh_i.hxx"

#include "DriverMED_R_SMESHDS_Mesh.h"
#include "DriverMED_W_Field.h"
#include "DriverMED_W_SMESHDS_Mesh.h"
#include "MED_Factory.hxx"
#include "SMDS_LinearEdge.hxx"
#include "SMDS_EdgePosition.hxx"
#include "SMDS_ElemIterator.hxx"
#include "SMDS_FacePosition.hxx"
#include "SMDS_IteratorOnIterators.hxx"
#include "SMDS_MeshGroup.hxx"
#include "SMDS_SetIterator.hxx"
#include "SMDS_StdIterator.hxx"
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
#include <TopTools_DataMapOfShapeShape.hxx>
#include <TopTools_MapIteratorOfMapOfShape.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopoDS_Compound.hxx>

// STL Includes
#include <algorithm>
#include <iostream>
#include <sstream>

#include <vtkUnstructuredGridWriter.h>

// to pass CORBA exception and TooLargeForExport exception through SMESH_TRY
#define SMY_OWN_CATCH                                                                           \
  catch( SALOME::SALOME_Exception& se ) { throw se; }                                           \
  catch( ::SMESH_Mesh::TooLargeForExport& ex )                                                  \
  { SALOME::ExceptionStruct se = {                                                              \
      SALOME::COMM,                                                                             \
      CORBA::string_dup(SMESH_Comment("Mesh is too large for export in format ") << ex.what()), \
      CORBA::string_dup(SMESH_Comment("format=") <<  ex.what() ), 0 };                          \
    throw SALOME::SALOME_Exception( se );  }

#include "SMESH_TryCatch.hxx" // include after OCCT headers!

#ifdef _DEBUG_
static int MYDEBUG = 0;
#else
static int MYDEBUG = 0;
#endif

using namespace std;
using SMESH::TPythonDump;
using SMESH::TVar;

int SMESH_Mesh_i::_idGenerator = 0;

//=============================================================================
/*!
 *  Constructor
 */
//=============================================================================

SMESH_Mesh_i::SMESH_Mesh_i( PortableServer::POA_ptr thePOA,
                            SMESH_Gen_i*            gen_i )
: SALOME::GenericObj_i( thePOA )
{
  _impl          = NULL;
  _gen_i         = gen_i;
  _id            = _idGenerator++;
  _nbInvalidHypos= -1;
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

  // clear cached shapes if no more meshes remain; (the cache is blame,
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
 *  Associate <this> mesh with <theShape> and put a reference
 *  to <theShape> into the current study;
 *  the previous shape is substituted by the new one.
 */
//=============================================================================

void SMESH_Mesh_i::SetShape( GEOM::GEOM_Object_ptr theShapeObject )
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
 * \brief Return true if mesh has a shape to build a shape on
 */
//================================================================================

CORBA::Boolean SMESH_Mesh_i::HasShapeToMesh()
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

//================================================================================
/*!
 * \brief Return the shape to mesh
 */
//================================================================================

GEOM::GEOM_Object_ptr SMESH_Mesh_i::GetShapeToMesh()
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
            SALOMEDS::SObject_wrap so = _gen_i->getStudyServant()->FindObjectID( data->_groupEntry.c_str() );
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

CORBA::Boolean SMESH_Mesh_i::IsLoaded()
{
  Unexpect aCatch(SALOME_SalomeException);
  return !_preMeshInfo;
}

//================================================================================
/*!
 * \brief Load full mesh data from the study file
 */
//================================================================================

void SMESH_Mesh_i::Load()
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

void SMESH_Mesh_i::Clear()
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

  SMESH::SMESH_Mesh_var mesh = _this();
  _gen_i->UpdateIcons( mesh );
}

//================================================================================
/*!
 * \brief Remove all nodes and elements for indicated shape
 */
//================================================================================

void SMESH_Mesh_i::ClearSubMesh(CORBA::Long ShapeID)
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
 *  Import mesh data from MED file
 */
//=============================================================================

SMESH::DriverMED_ReadStatus
SMESH_Mesh_i::ImportMEDFile( const char* theFileName, const char* theMeshName )
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
  major = minor = release = 0;
  MED::GetMEDVersion(theFileName, major, minor, release);
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
 * \brief Import mesh data from the CGNS file
 */
//================================================================================

SMESH::DriverMED_ReadStatus SMESH_Mesh_i::ImportCGNSFile( const char*  theFileName,
                                                          const int    theMeshIndex,
                                                          std::string& theMeshName )
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

  _medFileInfo           = new SMESH::MedFileInfo();
  _medFileInfo->fileName = theFileName;
  _medFileInfo->major    = 0;
  _medFileInfo->minor    = 0;
  _medFileInfo->release  = 0;
  _medFileInfo->fileSize = SMESH_File( theFileName ).size();

  return ConvertDriverMEDReadStatus(status);
}

//================================================================================
/*!
 * \brief Return string representation of a MED file version comprising nbDigits
 */
//================================================================================

char* SMESH_Mesh_i::GetVersionString(CORBA::Long minor, CORBA::Short nbDigits)
{
  string ver = DriverMED_W_SMESHDS_Mesh::GetVersionString(minor, nbDigits);
  return CORBA::string_dup( ver.c_str() );
}

//================================================================================
/*!
 *  Return the list of med versions compatibles for write/append,
 *  encoded in 10*major+minor (for instance, code for med 3.2.1 is 32)
 */
//================================================================================

SMESH::long_array* SMESH_Mesh_i::GetMEDVersionsCompatibleForAppend()
{
  SMESH::long_array_var aResult = new SMESH::long_array();
  std::vector<int> mvok = MED::GetMEDVersionsAppendCompatible();
  long nbver = mvok.size();
  aResult->length( nbver );
  for ( int i = 0; i < nbver; i++ )
    aResult[i] = mvok[i];
  return aResult._retn();
}

//=============================================================================
/*!
 *  ImportUNVFile
 *
 *  Import mesh data from MED file
 */
//=============================================================================

int SMESH_Mesh_i::ImportUNVFile( const char* theFileName )
{
  SMESH_TRY;

  // Read mesh with name = <theMeshName> into SMESH_Mesh
  _impl->UNVToMesh( theFileName );

  CreateGroupServants();

  _medFileInfo           = new SMESH::MedFileInfo();
  _medFileInfo->fileName = theFileName;
  _medFileInfo->major    = 0;
  _medFileInfo->minor    = 0;
  _medFileInfo->release  = 0;
  _medFileInfo->fileSize = SMESH_File( theFileName ).size();

  SMESH_CATCH( SMESH::throwCorbaException );

  return 1;
}

//=============================================================================
/*!
 *  ImportSTLFile
 *
 *  Import mesh data from STL file
 */
//=============================================================================

int SMESH_Mesh_i::ImportSTLFile( const char* theFileName )
{
  SMESH_TRY;

  // Read mesh with name = <theMeshName> into SMESH_Mesh
  std::string name = _impl->STLToMesh( theFileName );
  if ( !name.empty() )
  {
    SALOMEDS::SObject_wrap meshSO = _gen_i->ObjectToSObject( _this() );
    _gen_i->SetName( meshSO, name.c_str() );
  }
  _medFileInfo           = new SMESH::MedFileInfo();
  _medFileInfo->fileName = theFileName;
  _medFileInfo->major    = 0;
  _medFileInfo->minor    = 0;
  _medFileInfo->release  = 0;
  _medFileInfo->fileSize = SMESH_File( theFileName ).size();

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
 * \brief Import data from a GMF file and return an error description
 */
//================================================================================

SMESH::ComputeError* SMESH_Mesh_i::ImportGMFFile( const char* theFileName,
                                                  bool        theMakeRequiredGroups )
{
  SMESH_ComputeErrorPtr error;

#undef SMESH_CAUGHT
#define SMESH_CAUGHT error =
  SMESH_TRY;

  error = _impl->GMFToMesh( theFileName, theMakeRequiredGroups );

  _medFileInfo           = new SMESH::MedFileInfo();
  _medFileInfo->fileName = theFileName;
  _medFileInfo->major    = 0;
  _medFileInfo->minor    = 0;
  _medFileInfo->release  = 0;
  _medFileInfo->fileSize = SMESH_File( theFileName ).size();

  SMESH_CATCH( exceptionToComputeError );
#undef SMESH_CAUGHT
#define SMESH_CAUGHT

  CreateGroupServants();

  return ConvertComputeError( error );
}

//=============================================================================
/*!
 * \brief Convert SMESH_Hypothesis::Hypothesis_Status into SMESH::Hypothesis_Status
 */
//=============================================================================

#define RETURNCASE(hyp_stat) case SMESH_Hypothesis::hyp_stat: return SMESH::hyp_stat;

SMESH::Hypothesis_Status SMESH_Mesh_i::ConvertHypothesisStatus
                         (SMESH_Hypothesis::Hypothesis_Status theStatus)
{
  switch (theStatus) {
  RETURNCASE( HYP_OK            );
  RETURNCASE( HYP_MISSING       );
  RETURNCASE( HYP_CONCURRENT    );
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
 *  Call internal addHypothesis() and then add a reference to <anHyp> under
 *  the SObject actually having a reference to <aSubShape>.
 *  NB: For this method to work, it is necessary to add a reference to sub-shape first.
 */
//=============================================================================

SMESH::Hypothesis_Status
SMESH_Mesh_i::AddHypothesis(GEOM::GEOM_Object_ptr       aSubShape,
                            SMESH::SMESH_Hypothesis_ptr anHyp,
                            CORBA::String_out           anErrorText)
{
  Unexpect aCatch(SALOME_SalomeException);

  const smIdType prevNbMeshEnt = NbNodes() + NbElements();

  if ( _preMeshInfo )
    _preMeshInfo->ForgetOrLoad();

  std::string error;
  SMESH_Hypothesis::Hypothesis_Status status = addHypothesis( aSubShape, anHyp, &error );
  anErrorText = error.c_str();

  SMESH::SMESH_Mesh_var mesh( _this() );
  if ( !SMESH_Hypothesis::IsStatusFatal(status) )
  {
    _gen_i->AddHypothesisToShape( mesh, aSubShape, anHyp );

    //int newNbMeshEnt = _impl->NbNodes() + _impl->GetMeshDS()->NbElements();
    if ( prevNbMeshEnt > 0 /*newNbMeshEnt != prevNbMeshEnt*/ )
      _gen_i->UpdateIcons( mesh );
  }
  if(MYDEBUG) MESSAGE( " AddHypothesis(): status = " << status );

  // Update Python script
  TPythonDump() << "status = " << mesh << ".AddHypothesis( "
                << aSubShape << ", " << anHyp << " )";

  return ConvertHypothesisStatus(status);
}

//================================================================================
/*!
 * \brief Create  a sub-mesh and add a hypothesis to it
 */
//================================================================================

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

//================================================================================
/*!
 * \brief Un-assign a hypothesis from a sub-mesh dedicate to the given sub-shape
 */
//================================================================================

SMESH::Hypothesis_Status SMESH_Mesh_i::RemoveHypothesis(GEOM::GEOM_Object_ptr       aSubShape,
                                                        SMESH::SMESH_Hypothesis_ptr anHyp)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    _preMeshInfo->ForgetOrLoad();

  SMESH_Hypothesis::Hypothesis_Status status = removeHypothesis( aSubShape, anHyp );
  SMESH::SMESH_Mesh_var mesh = _this();

  if ( !SMESH_Hypothesis::IsStatusFatal(status) )
  {
    _gen_i->RemoveHypothesisFromShape( mesh, aSubShape, anHyp );
    _gen_i->UpdateIcons( mesh );
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
 * \brief Un-assign a hypothesis from a sub-mesh dedicate to the given sub-shape
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

  if ( _preMeshInfo )
    _preMeshInfo->ForgetOrLoad();

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

//================================================================================
/*!
 * \brief Return hypotheses assigned to a given sub-shape
 */
//================================================================================

SMESH::ListOfHypothesis *
SMESH_Mesh_i::GetHypothesisList(GEOM::GEOM_Object_ptr aSubShape)
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

//================================================================================
/*!
 * \brief Return sub-meshes
 */
//================================================================================

SMESH::submesh_array* SMESH_Mesh_i::GetSubMeshes()
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

//================================================================================
/*!
 * \brief Create and return a sub-mesh on the given sub-shape
 */
//================================================================================

SMESH::SMESH_subMesh_ptr SMESH_Mesh_i::GetSubMesh(GEOM::GEOM_Object_ptr aSubShape,
                                                  const char*           theName )
{
  Unexpect aCatch(SALOME_SalomeException);
  if (CORBA::is_nil(aSubShape))
    THROW_SALOME_CORBA_EXCEPTION("bad Sub-shape reference", SALOME::BAD_PARAM);

  SMESH::SMESH_subMesh_var subMesh;
  SMESH::SMESH_Mesh_var    aMesh = _this();
  try {
    TopoDS_Shape myLocSubShape = _gen_i->GeomObjectToShape(aSubShape);

    //Get or Create the SMESH_subMesh object implementation

    TopoDS_Iterator it( myLocSubShape );
    int   subMeshId = _impl->GetMeshDS()->ShapeToIndex( myLocSubShape );
    bool isValidSub = ( subMeshId || _impl->GetMeshDS()->IsGroupOfSubShapes( myLocSubShape ));
    if ( isValidSub && myLocSubShape.ShapeType() == TopAbs_COMPOUND )
      isValidSub = !it.Value().IsSame( _impl->GetShapeToMesh() );
    if ( !isValidSub )
    {
      if ( it.More() )
        THROW_SALOME_CORBA_EXCEPTION("Not a sub-shape of the main shape", SALOME::BAD_PARAM);
    }
    subMesh = getSubMesh( subMeshId );

    // create a new subMesh object servant if there is none for the shape
    if ( subMesh->_is_nil() )
      subMesh = createSubMesh( aSubShape );
    if ( _gen_i->CanPublishInStudy( subMesh ))
    {
      SALOMEDS::SObject_wrap aSO =
        _gen_i->PublishSubMesh( aMesh, subMesh, aSubShape, theName );
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

//================================================================================
/*!
 * \brief Remove a sub-mesh
 */
//================================================================================

void SMESH_Mesh_i::RemoveSubMesh( SMESH::SMESH_subMesh_ptr theSubMesh )
{
  SMESH_TRY;

  if ( theSubMesh->_is_nil() )
    return;

  GEOM::GEOM_Object_var aSubShape;
  // Remove submesh's SObject
  SALOMEDS::SObject_wrap anSO = _gen_i->ObjectToSObject( theSubMesh );
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

    SALOMEDS::StudyBuilder_var builder = _gen_i->getStudyServant()->NewBuilder();
    builder->RemoveObjectWithChildren( anSO );

    // Update Python script
    TPythonDump() << SMESH::SMESH_Mesh_var( _this() ) << ".RemoveSubMesh( " << anSO << " )";
  }

  if ( removeSubMesh( theSubMesh, aSubShape.in() ))
    if ( _preMeshInfo )
      _preMeshInfo->ForgetOrLoad();

  SMESH_CATCH( SMESH::throwCorbaException );
}

//================================================================================
/*!
 * \brief Create a standalone group
 */
//================================================================================

SMESH::SMESH_Group_ptr SMESH_Mesh_i::CreateGroup( SMESH::ElementType theElemType,
                                                  const char*        theName )
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESH::SMESH_Group_var aNewGroup =
    SMESH::SMESH_Group::_narrow( createGroup( theElemType, theName ));

  if ( _gen_i->CanPublishInStudy( aNewGroup ) )
  {
    SMESH::SMESH_Mesh_var mesh = _this();
    SALOMEDS::SObject_wrap aSO =
      _gen_i->PublishGroup( mesh, aNewGroup, GEOM::GEOM_Object::_nil(), theName);
    if ( !aSO->_is_nil())
      // Update Python script
      TPythonDump() << aSO << " = " << mesh << ".CreateGroup( "
                    << theElemType << ", '" << theName << "' )";
  }
  return aNewGroup._retn();
}

//================================================================================
/*!
 * \brief Create a group based on the given geometry
 */
//================================================================================

SMESH::SMESH_GroupOnGeom_ptr
SMESH_Mesh_i::CreateGroupFromGEOM (SMESH::ElementType    theElemType,
                                   const char*           theName,
                                   GEOM::GEOM_Object_ptr theGeomObj)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESH::SMESH_GroupOnGeom_var aNewGroup;

  TopoDS_Shape aShape = _gen_i->GeomObjectToShape( theGeomObj );
  if ( !aShape.IsNull() )
  {
    aNewGroup =
      SMESH::SMESH_GroupOnGeom::_narrow( createGroup( theElemType, theName, /*id=*/-1, aShape ));

    if ( _gen_i->CanPublishInStudy( aNewGroup ) )
    {
      SMESH::SMESH_Mesh_var mesh = _this();
      SALOMEDS::SObject_wrap aSO =
        _gen_i->PublishGroup( mesh, aNewGroup, theGeomObj, theName );
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
    ( createGroup( theElemType, theName, /*id=*/-1, TopoDS_Shape(), predicate ));

  TPythonDump pd;
  if ( !aNewGroup->_is_nil() )
    aNewGroup->SetFilter( theFilter );

  if ( _gen_i->CanPublishInStudy( aNewGroup ) )
  {
    SMESH::SMESH_Mesh_var mesh = _this();
    SALOMEDS::SObject_wrap aSO =
      _gen_i->PublishGroup( mesh, aNewGroup, GEOM::GEOM_Object::_nil(), theName );

    if ( !aSO->_is_nil())
      pd << aSO << " = " << mesh << ".CreateGroupFromFilter( "
         << theElemType << ", '" << theName << "', " << theFilter << " )";
  }
  return aNewGroup._retn();
}

//================================================================================
/*!
 * \brief Remove a group
 */
//================================================================================

void SMESH_Mesh_i::RemoveGroup( SMESH::SMESH_GroupBase_ptr theGroup )
{
  if ( theGroup->_is_nil() )
    return;

  SMESH_TRY;

  SMESH_GroupBase_i* aGroup = SMESH::DownCast<SMESH_GroupBase_i*>( theGroup );
  if ( !aGroup )
    return;

  if ( aGroup->GetMeshServant() != this )
    THROW_SALOME_CORBA_EXCEPTION( "RemoveGroup(): group does not belong to this mesh",
                                  SALOME::BAD_PARAM );

  SALOMEDS::SObject_wrap aGroupSO = _gen_i->ObjectToSObject( theGroup );
  if ( !aGroupSO->_is_nil() )
  {
    // Update Python script
    TPythonDump() << SMESH::SMESH_Mesh_var(_this()) << ".RemoveGroup( " << aGroupSO << " )";

    // Remove group's SObject
    SALOMEDS::StudyBuilder_var builder = SMESH_Gen_i::GetSMESHGen()->getStudyServant()->NewBuilder();
    builder->RemoveObjectWithChildren( aGroupSO );
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
{
  SMESH_TRY;
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  if ( theGroup->_is_nil() )
    return;

  SMESH_GroupBase_i* groupImpl = SMESH::DownCast< SMESH_GroupBase_i* >( theGroup );
  if ( !groupImpl || groupImpl->GetMeshServant() != this )
    THROW_SALOME_CORBA_EXCEPTION( "RemoveGroupWithContents(): group does not belong to this mesh",
                                  SALOME::BAD_PARAM);

  vector<smIdType> nodeIds; // to remove nodes becoming free
  bool isNodal = ( theGroup->GetType() == SMESH::NODE );
  if ( !isNodal && !theGroup->IsEmpty() )
  {
    SMESH::smIdType elemID = theGroup->GetID( 1 );
    int nbElemNodes = GetElemNbNodes( elemID );
    if ( nbElemNodes > 0 )
      nodeIds.reserve( theGroup->Size() * nbElemNodes );
  }

  // Retrieve contents
  SMESH::SMESH_IDSource_var idSrc = SMESH::SMESH_IDSource::_narrow( theGroup );
  SMDS_ElemIteratorPtr     elemIt = GetElements( idSrc, theGroup->GetType() );
  SMDS_StdIterator< const SMDS_MeshElement*, SMDS_ElemIteratorPtr > elemBeg( elemIt ), elemEnd;
  std::vector< const SMDS_MeshElement* > elems( theGroup->Size() );
  elems.assign( elemBeg, elemEnd );

  TPythonDump pyDump; // Suppress dump from RemoveGroup()

  // Remove group
  RemoveGroup( theGroup );

  // Remove contents
  for ( size_t i = 0; i < elems.size(); ++i )
  {
    // if ( !_impl->GetMeshDS()->Contains( elems[i] ))
    //   continue;
    if ( !isNodal )
    {
      for ( SMDS_ElemIteratorPtr nIt = elems[i]->nodesIterator(); nIt->more(); )
        nodeIds.push_back( nIt->next()->GetID() );

      _impl->GetMeshDS()->RemoveFreeElement( elems[i], /*sm=*/0 );
    }
    else
    {
      _impl->GetMeshDS()->RemoveElement( elems[i] );
    }
  }

  // Remove free nodes
  for ( size_t i = 0 ; i < nodeIds.size(); ++i )
    if ( const SMDS_MeshNode* n = _impl->GetMeshDS()->FindNode( nodeIds[i] ))
      if ( n->NbInverseElements() == 0 )
        _impl->GetMeshDS()->RemoveFreeNode( n, /*sm=*/0 );

  _impl->GetMeshDS()->Modified();
  _impl->SetIsModified( true );

  // Update Python script (theGroup must be alive for this)
  pyDump << SMESH::SMESH_Mesh_var(_this())
         << ".RemoveGroupWithContents( " << theGroup << " )";

  SMESH_CATCH( SMESH::throwCorbaException );
}

//================================================================================
/*!
 * \brief Get the list of groups existing in the mesh
 *  \retval SMESH::ListOfGroups * - list of groups
 */
//================================================================================

SMESH::ListOfGroups * SMESH_Mesh_i::GetGroups()
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

CORBA::Long SMESH_Mesh_i::NbGroups()
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
  bool isAllNodesCommon(int nbChecked, int nbCommon, int nbNodes, int /*nbCorners*/,
                        bool & toStopChecking )
  {
    toStopChecking = ( nbCommon < nbChecked );
    return nbCommon == nbNodes;
  }
  bool isMainNodesCommon(int nbChecked, int nbCommon, int /*nbNodes*/, int nbCorners,
                         bool & toStopChecking )
  {
    toStopChecking = ( nbCommon < nbChecked || nbChecked >= nbCorners );
    return nbCommon == nbCorners;
  }
  bool isAtLeastOneNodeCommon(int /*nbChecked*/, int nbCommon, int /*nbNodes*/, int /*nbCorners*/,
                              bool & /*toStopChecking*/ )
  {
    return nbCommon > 0;
  }
  bool isMajorityOfNodesCommon(int /*nbChecked*/, int nbCommon, int nbNodes, int /*nbCorners*/,
                               bool & /*toStopChecking*/ )
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
          const smIdType nID = nIt->next()->GetID();
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
 * \brief Distribute all faces of the mesh between groups using sharp edges and optionally
 *        existing 1D elements as group boundaries.
 *  \param [in] theSharpAngle - edge is considered sharp if an angle between normals of
 *              adjacent faces is more than \a sharpAngle in degrees.
 *  \param [in] theCreateEdges - to create 1D elements for detected sharp edges.
 *  \param [in] theUseExistingEdges - to use existing edges as group boundaries
 *  \return ListOfGroups - the created groups
 */
//================================================================================

SMESH::ListOfGroups*
SMESH_Mesh_i::FaceGroupsSeparatedByEdges( CORBA::Double  theSharpAngle,
                                          CORBA::Boolean theCreateEdges,
                                          CORBA::Boolean theUseExistingEdges )
{
  if ( theSharpAngle < 0 || theSharpAngle > 180 )
    THROW_SALOME_CORBA_EXCEPTION("Invalid sharp angle, it must be between 0 and 180 degrees",
                                 SALOME::BAD_PARAM);

  SMESH::ListOfGroups_var resultGroups = new SMESH::ListOfGroups;

  TPythonDump pyDump;

  SMESH_TRY;
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESHDS_Mesh* meshDS = _impl->GetMeshDS();

  std::vector< SMESH_MeshAlgos::Edge > edges =
    SMESH_MeshAlgos::FindSharpEdges( meshDS, theSharpAngle, theUseExistingEdges );

  if ( theCreateEdges )
  {
    std::vector<const SMDS_MeshNode *> nodes(2);
    for ( size_t i = 0; i < edges.size(); ++i )
    {
      nodes[0] = edges[i]._node1;
      nodes[1] = edges[i]._node2;
      if ( meshDS->FindElement( nodes, SMDSAbs_Edge ))
        continue;
      if ( edges[i]._medium )
        meshDS->AddEdge( edges[i]._node1, edges[i]._node2, edges[i]._medium );
      else
        meshDS->AddEdge( edges[i]._node1, edges[i]._node2 );
    }
  }

  std::vector< std::vector< const SMDS_MeshElement* > > faceGroups =
    SMESH_MeshAlgos::SeparateFacesByEdges( meshDS, edges );

  SMESH::SMESH_MeshEditor_var editor = GetMeshEditor(); // create _editor

  resultGroups->length( faceGroups.size() );
  for ( size_t iG = 0; iG < faceGroups.size(); ++iG )
  {
    SMESH::SMESH_Group_var group = CreateGroup( SMESH::FACE,
                                                _editor->GenerateGroupName("Group").c_str());
    resultGroups[iG] = SMESH::SMESH_Group::_duplicate( group );

    SMESHDS_GroupBase* groupBaseDS =
      SMESH::DownCast<SMESH_GroupBase_i*>( group )->GetGroupDS();
    SMDS_MeshGroup& groupCore = static_cast< SMESHDS_Group* >( groupBaseDS )->SMDSGroup();

    std::vector< const SMDS_MeshElement* >& faces = faceGroups[ iG ];
    for ( size_t i = 0; i < faces.size(); ++i )
      groupCore.Add( faces[i] );
  }

  pyDump << resultGroups << " = " << SMESH::SMESH_Mesh_var(_this())
         << ".FaceGroupsSeparatedByEdges( "
         << TVar( theSharpAngle ) << ", "
         << theCreateEdges << ", "
         << theUseExistingEdges << " )";

  SMESH_CATCH( SMESH::throwCorbaException );
  return resultGroups._retn();

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
  SALOMEDS::SObject_wrap groupSO = _gen_i->ObjectToSObject( theGeomObj );
  if ( groupSO->_is_nil() )
    return;
  // group indices
  GEOM::GEOM_Gen_var               geomGen = _gen_i->GetGeomEngine( theGeomObj );
  GEOM::GEOM_IGroupOperations_wrap groupOp = geomGen->GetIGroupOperations();
  GEOM::ListOfLong_var                 ids = groupOp->GetObjects( theGeomObj );

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
* \brief Replace a shape in the mesh upon Break Link
*/
//================================================================================

void SMESH_Mesh_i::ReplaceShape(GEOM::GEOM_Object_ptr theNewGeom)
{
  // check if geometry changed
  bool geomChanged = true;
  GEOM::GEOM_Object_var oldGeom = GetShapeToMesh();
  if ( !theNewGeom->_is_nil() && !oldGeom->_is_nil() )
    geomChanged = ( oldGeom->GetTick() != theNewGeom->GetTick() );

  TopoDS_Shape S = _impl->GetShapeToMesh();
  GEOM_Client* geomClient = _gen_i->GetShapeReader();
  TCollection_AsciiString aIOR;
  if ( geomClient->Find( S, aIOR ))
    geomClient->RemoveShapeFromBuffer( aIOR );

  // clear buffer also for sub-groups
  SMESHDS_Mesh* meshDS = _impl->GetMeshDS();
  const std::set<SMESHDS_GroupBase*>& groups = meshDS->GetGroups();
  std::set<SMESHDS_GroupBase*>::const_iterator g = groups.begin();
  for (; g != groups.end(); ++g)
    if (const SMESHDS_GroupOnGeom* group = dynamic_cast<SMESHDS_GroupOnGeom*>(*g))
    {
      const TopoDS_Shape& s = group->GetShape();
      if ( geomClient->Find( s, aIOR ))
        geomClient->RemoveShapeFromBuffer( aIOR );
    }

  // clear buffer also for sub-meshes
  std::map<int, SMESH_subMesh_i*>::const_iterator aSubMeshIter = _mapSubMesh_i.cbegin();
  for(; aSubMeshIter != _mapSubMesh_i.cend(); aSubMeshIter++) {
    int aShapeID = aSubMeshIter->first;
    const TopoDS_Shape& aSubShape = meshDS->IndexToShape(aShapeID);
    TCollection_AsciiString aShapeIOR;
    if ( geomClient->Find( aSubShape, aShapeIOR ))
      geomClient->RemoveShapeFromBuffer( aShapeIOR );
  }

  typedef struct {
    int shapeID; smIdType fromID, toID; // indices of elements of a sub-mesh
  } TRange;
  std::vector< TRange > elemRanges, nodeRanges; // elements of sub-meshes
  std::vector< SMDS_PositionPtr > positions; // node positions
  if ( !geomChanged )
  {
    // store positions of elements on geometry
    Load();
    if ( meshDS->MaxNodeID()    > meshDS->NbNodes() ||
         meshDS->MaxElementID() > meshDS->NbElements() )
    {
      meshDS->Modified();
      meshDS->CompactMesh();
    }
    positions.resize( meshDS->NbNodes() + 1 );
    for ( SMDS_NodeIteratorPtr nodeIt = meshDS->nodesIterator(); nodeIt->more(); )
    {
      const SMDS_MeshNode* n = nodeIt->next();
      positions[ n->GetID() ] = n->GetPosition();
    }

    // remove elements from sub-meshes to avoid their removal at hypotheses addition
    for ( int isNode = 0; isNode < 2; ++isNode )
    {
      std::vector< TRange > & ranges = isNode ? nodeRanges : elemRanges;
      ranges.reserve( meshDS->MaxShapeIndex() + 10 );
      ranges.push_back( TRange{ 0,0,0 });
      SMDS_ElemIteratorPtr elemIt = meshDS->elementsIterator( isNode ? SMDSAbs_Node : SMDSAbs_All );
      while ( elemIt->more() )
      {
        const SMDS_MeshElement* e = elemIt->next();
        const smIdType     elemID = e->GetID();
        const int         shapeID = e->GetShapeID();
        TRange &        lastRange = ranges.back();
        if ( lastRange.shapeID != shapeID ||
             lastRange.toID    != elemID )
          ranges.push_back( TRange{ shapeID, elemID, elemID + 1 });
        else
          lastRange.toID = elemID + 1;

        if ( SMESHDS_SubMesh* sm = meshDS->MeshElements( shapeID ))
        {
          if ( isNode ) sm->RemoveNode( static_cast< const SMDS_MeshNode *>( e ));
          else          sm->RemoveElement( e );
        }
      }
    }
  }


  // update the reference to theNewGeom (needed for correct execution of a dumped python script)
  SMESH::SMESH_Mesh_var   me = _this();
  SALOMEDS::SObject_wrap aSO = _gen_i->ObjectToSObject( me );
  CORBA::String_var    entry = theNewGeom->GetStudyEntry();
  if ( !aSO->_is_nil() )
  {
    SALOMEDS::SObject_wrap aShapeRefSO;
    if ( aSO->FindSubObject( _gen_i->GetRefOnShapeTag(), aShapeRefSO.inout() ))
    {
      SALOMEDS::SObject_wrap    aShapeSO = _gen_i->getStudyServant()->FindObjectID( entry );
      SALOMEDS::StudyBuilder_var builder = _gen_i->getStudyServant()->NewBuilder();
      builder->Addreference( aShapeRefSO, aShapeSO );
    }
  }

  // re-assign global hypotheses to the new shape
  _mainShapeTick = geomChanged ? -1 : theNewGeom->GetTick();
  CheckGeomModif( true );

  if ( !geomChanged )
  {
    // restore positions of elements on geometry
    for ( int isNode = 0; isNode < 2; ++isNode )
    {
      std::vector< TRange > & ranges = isNode ? nodeRanges : elemRanges;
      for ( size_t i = 1; i < ranges.size(); ++i )
      {
        int elemID = ranges[ i ].fromID;
        int   toID = ranges[ i ].toID;
        SMESHDS_SubMesh * smDS = meshDS->NewSubMesh( ranges[ i ].shapeID );
        if ( isNode )
          for ( ; elemID < toID; ++elemID )
            smDS->AddNode( meshDS->FindNode( elemID ));
        else
          for ( ; elemID < toID; ++elemID )
            smDS->AddElement( meshDS->FindElement( elemID ));

        if ( SMESH_subMesh* sm = _impl->GetSubMeshContaining( ranges[ i ].shapeID ))
          sm->ComputeStateEngine( SMESH_subMesh::CHECK_COMPUTE_STATE );
      }
    }
    for ( unsigned int nodeID = 1; nodeID < positions.size(); ++nodeID )
      if ( positions[ nodeID ])
        if ( SMDS_MeshNode* n = const_cast< SMDS_MeshNode*>( meshDS->FindNode( nodeID )))
          n->SetPosition( positions[ nodeID ], n->GetShapeID() );

    // restore icons
    _gen_i->UpdateIcons( SMESH::SMESH_Mesh_var( _this() ));
  }

  TPythonDump() << "SHAPERSTUDY.breakLinkForSubElements(salome.ObjectToSObject("
                << me <<".GetMesh()), " << entry.in() << ")";

  TPythonDump() <<  me << ".ReplaceShape( " << entry.in() << " )";

}

//================================================================================
/*!
 * \brief Return new group contents if it has been changed and update group data
 */
//================================================================================

enum { ONLY_IF_CHANGED, IS_BREAK_LINK, MAIN_TRANSFORMED };

TopoDS_Shape SMESH_Mesh_i::newGroupShape( TGeomGroupData & groupData, int how )
{
  TopoDS_Shape newShape;
  SALOMEDS::SObject_wrap groupSO;

  if ( how == IS_BREAK_LINK )
  {
    SALOMEDS::SObject_wrap meshSO = _gen_i->ObjectToSObject( groupData._smeshObject );
    SALOMEDS::SObject_wrap geomRefSO;
    if ( !meshSO->_is_nil() &&
         meshSO->FindSubObject( SMESH::Tag_RefOnShape, geomRefSO.inout() ))
    {
      geomRefSO->ReferencedObject( groupSO.inout() );
    }
  }
  else
  {
    // get geom group
    groupSO = _gen_i->getStudyServant()->FindObjectID( groupData._groupEntry.c_str() );
  }

  if ( groupSO->_is_nil() )
    return newShape;

  CORBA::Object_var      groupObj = _gen_i->SObjectToObject( groupSO );
  GEOM::GEOM_Object_var geomGroup = GEOM::GEOM_Object::_narrow( groupObj );
  if ( geomGroup->_is_nil() )
    return newShape;

  // get indices of group items
  set<int> curIndices;
  GEOM::GEOM_Gen_var               geomGen = _gen_i->GetGeomEngine( geomGroup );
  GEOM::GEOM_IGroupOperations_wrap groupOp = geomGen->GetIGroupOperations();
  GEOM::ListOfLong_var                 ids = groupOp->GetObjects( geomGroup );
  for ( CORBA::ULong i = 0; i < ids->length(); ++i )
    curIndices.insert( ids[i] );

  bool sameIndices = ( groupData._indices == curIndices );
  if ( how == ONLY_IF_CHANGED && sameIndices )
    return newShape; // group not changed

  // update data
  CORBA::String_var entry = geomGroup->GetStudyEntry();
  groupData._groupEntry = entry.in();
  groupData._indices = curIndices;

  newShape = _gen_i->GeomObjectToShape( geomGroup );

  // check if newShape is up-to-date
  if ( !newShape.IsNull() && ids->length() > 0 )
  {
    bool toUpdate = ! _impl->GetMeshDS()->IsGroupOfSubShapes( newShape );
    if ( !toUpdate )
    {
      TopExp_Explorer exp( newShape, (TopAbs_ShapeEnum)( groupOp->GetType( geomGroup )));
      for ( ; exp.More() && !toUpdate; exp.Next() )
      {
        int ind = _impl->GetMeshDS()->ShapeToIndex( exp.Current() );
        toUpdate = ( curIndices.erase( ind ) == 0 );
      }
      if ( !curIndices.empty() )
        toUpdate = true;
    }
    if ( toUpdate )
    {
      GEOM_Client*    geomClient = _gen_i->GetShapeReader();
      CORBA::String_var groupIOR = geomGen->GetStringFromIOR( geomGroup );
      geomClient->RemoveShapeFromBuffer( groupIOR.in() );
      newShape = _gen_i->GeomObjectToShape( geomGroup );
    }
  }
  else
  {
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
    TopoDS_Shape        _shape;
    SMDSAbs_ElementType _type;
    std::string         _name;
    Quantity_Color      _color;

    TGroupOnGeomData( const SMESHDS_GroupOnGeom* group )
    {
      _oldID = group->GetID();
      _type  = group->GetType();
      _name  = group->GetStoreName();
      _color = group->GetColor();
    }
  };

  //-----------------------------------------------------------------------------
  /*!
   * \brief Check if a filter is still valid after geometry removal
   */
  bool isValidGeomFilter( SMESH::Filter_var theFilter )
  {
    if ( theFilter->_is_nil() )
      return false;
    SMESH::Filter::Criteria_var criteria;
    theFilter->GetCriteria( criteria.out() );

    for ( CORBA::ULong iCr = 0; iCr < criteria->length(); ++iCr )
    {
      const char* thresholdID = criteria[ iCr ].ThresholdID.in();
      std::string entry;
      switch ( criteria[ iCr ].Type )
      {
      case SMESH::FT_BelongToGeom:
      case SMESH::FT_BelongToPlane:
      case SMESH::FT_BelongToCylinder:
      case SMESH::FT_BelongToGenSurface:
      case SMESH::FT_LyingOnGeom:
        entry = thresholdID;
        break;
      case SMESH::FT_ConnectedElements:
        if ( thresholdID )
        {
          entry = thresholdID;
          break;
        }
      default:
        continue;
      }
      SMESH_Gen_i*           gen = SMESH_Gen_i::GetSMESHGen();
      SALOMEDS::SObject_wrap  so = gen->getStudyServant()->FindObjectID( entry.c_str() );
      if ( so->_is_nil() )
        return false;
      CORBA::Object_var      obj = so->GetObject();
      GEOM::GEOM_Object_var geom = GEOM::GEOM_Object::_narrow( obj );
      if ( gen->GeomObjectToShape( geom ).IsNull() )
        return false;

    } // loop on criteria

    return true;
  }
}

//=============================================================================
/*!
 * \brief Update data if geometry changes
 *
 * Issue 0022501
 */
//=============================================================================

void SMESH_Mesh_i::CheckGeomModif( bool theIsBreakLink )
{
  SMESH::SMESH_Mesh_var me = _this();
  GEOM::GEOM_Object_var mainGO = GetShapeToMesh();

  TPythonDump dumpNothing; // prevent any dump

  //bool removedFromClient = false;

  if ( mainGO->_is_nil() ) // GEOM_Client cleared or geometry removed? (IPAL52735, PAL23636)
  {
    //removedFromClient = _impl->HasShapeToMesh();

    // try to find geometry by study reference
    SALOMEDS::SObject_wrap meshSO = _gen_i->ObjectToSObject( me );
    SALOMEDS::SObject_wrap geomRefSO, geomSO;
    if ( !meshSO->_is_nil() &&
         meshSO->FindSubObject( SMESH::Tag_RefOnShape, geomRefSO.inout() ) &&
         geomRefSO->ReferencedObject( geomSO.inout() ))
    {
      CORBA::Object_var geomObj = _gen_i->SObjectToObject( geomSO );
      mainGO = GEOM::GEOM_Object::_narrow( geomObj );
    }

    if ( mainGO->_is_nil() &&    // geometry removed ==>
         !geomRefSO->_is_nil() ) // remove geom dependent data: sub-meshes etc.
    {
      // convert geom dependent groups into standalone ones
      CheckGeomGroupModif();

      _impl->ShapeToMesh( TopoDS_Shape() );

      // remove sub-meshes
      std::map<int, SMESH::SMESH_subMesh_ptr>::iterator i_sm = _mapSubMeshIor.begin();
      while ( i_sm != _mapSubMeshIor.end() )
      {
        SMESH::SMESH_subMesh_ptr sm = i_sm->second;
        ++i_sm;
        RemoveSubMesh( sm );
      }
      // remove all children except groups in the study
      SALOMEDS::StudyBuilder_var builder = _gen_i->getStudyServant()->NewBuilder();
      SALOMEDS::SObject_wrap so;
      for ( CORBA::Long tag = SMESH::Tag_RefOnShape; tag <= SMESH::Tag_LastSubMesh; ++tag )
        if ( meshSO->FindSubObject( tag, so.inout() ))
          builder->RemoveObjectWithChildren( so );

      _gen_i->SetPixMap( meshSO, "ICON_SMESH_TREE_MESH_IMPORTED" );

      return;
    }
  }

  if ( !_impl->HasShapeToMesh() ) return;


  // Update after group modification

  const bool geomChanged = ( mainGO->GetTick() != _mainShapeTick );
  if ( !theIsBreakLink )
    if ( mainGO->GetType() == GEOM_GROUP || !geomChanged )  // is group or not modified
    {
      smIdType nb = NbNodes() + NbElements();
      CheckGeomGroupModif();
      if ( nb != NbNodes() + NbElements() ) // something removed due to hypotheses change
        _gen_i->UpdateIcons( me );
      return;
    }

  // Update after shape modification or breakLink w/o geometry change

  GEOM_Client* geomClient = _gen_i->GetShapeReader();
  if ( !geomClient ) return;
  GEOM::GEOM_Gen_var geomGen = _gen_i->GetGeomEngine( mainGO );
  if ( geomGen->_is_nil() ) return;
  CORBA::String_var geomComponentType = geomGen->ComponentDataType();
  bool isShaper = ( strcmp( geomComponentType.in(), "SHAPERSTUDY" ) == 0 );

  SMESHDS_Mesh * meshDS = _impl->GetMeshDS();

  TopoDS_Shape newShape = _gen_i->GeomObjectToShape( mainGO );
  if ( meshDS->ShapeToIndex( newShape ) == 1 ) // not yet updated
  {
    CORBA::String_var ior = geomGen->GetStringFromIOR( mainGO );
    geomClient->RemoveShapeFromBuffer( ior.in() );
    newShape = _gen_i->GeomObjectToShape( mainGO );
  }

  // Update data taking into account that if topology doesn't change
  // all sub-shapes change but IDs of sub-shapes remain (except for geom groups)

  if ( _preMeshInfo )
    _preMeshInfo->ForgetAllData();

  if ( geomChanged || !isShaper )
    _impl->Clear();
  if ( newShape.IsNull() )
    return;

  _mainShapeTick = mainGO->GetTick();

  // store data of groups on geometry including new TopoDS_Shape's
  std::vector< TGroupOnGeomData > groupsData;
  const std::set<SMESHDS_GroupBase*>& groups = meshDS->GetGroups();
  groupsData.reserve( groups.size() );
  TopTools_DataMapOfShapeShape old2newShapeMap;
  std::set<SMESHDS_GroupBase*>::const_iterator g = groups.begin();
  for ( ; g != groups.end(); ++g )
  {
    if ( const SMESHDS_GroupOnGeom* group = dynamic_cast< SMESHDS_GroupOnGeom* >( *g ))
    {
      groupsData.push_back( TGroupOnGeomData( group ));

      // get a new shape
      SMESH::SMESH_GroupOnGeom_var gog;
      std::map<int, SMESH::SMESH_GroupBase_ptr>::iterator i_grp = _mapGroups.find( group->GetID() );
      if ( i_grp != _mapGroups.end() )
        gog = SMESH::SMESH_GroupOnGeom::_narrow( i_grp->second );

      GEOM::GEOM_Object_var geom;
      if ( !gog->_is_nil() )
      {
        if ( !theIsBreakLink )
          geom = gog->GetShape();

        if ( theIsBreakLink || geom->_is_nil() )
        {
          SALOMEDS::SObject_wrap grpSO = _gen_i->ObjectToSObject( gog );
          SALOMEDS::SObject_wrap geomRefSO, geomSO;
          if ( !grpSO->_is_nil() &&
               grpSO->FindSubObject( SMESH::Tag_RefOnShape, geomRefSO.inout() ) &&
               geomRefSO->ReferencedObject( geomSO.inout() ))
          {
            CORBA::Object_var geomObj = _gen_i->SObjectToObject( geomSO );
            geom = GEOM::GEOM_Object::_narrow( geomObj );
          }
        }
      }
      if ( old2newShapeMap.IsBound( group->GetShape() ))
      {
        groupsData.back()._shape = old2newShapeMap( group->GetShape() );
      }
      else if ( !geom->_is_nil() )
      {
        groupsData.back()._shape = _gen_i->GeomObjectToShape( geom );
        if ( meshDS->IsGroupOfSubShapes( groupsData.back()._shape ))
        {
          CORBA::String_var ior = geomGen->GetStringFromIOR( geom );
          geomClient->RemoveShapeFromBuffer( ior.in() );
          groupsData.back()._shape = _gen_i->GeomObjectToShape( geom );
        }
        old2newShapeMap.Bind( group->GetShape(), groupsData.back()._shape );
      }
      
    }
  }
  // store assigned hypotheses
  std::vector< pair< int, THypList > > ids2Hyps;
  const ShapeToHypothesis & hyps = meshDS->GetHypotheses();
  for ( ShapeToHypothesis::Iterator s2hyps( hyps ); s2hyps.More(); s2hyps.Next() )
  {
    const TopoDS_Shape& s = s2hyps.Key();
    const THypList&  hyps = s2hyps.ChangeValue();
    ids2Hyps.push_back( make_pair( meshDS->ShapeToIndex( s ), hyps ));
  }

  std::multimap< std::set<int>, int > ii2iMap; // group sub-ids to group id in SMESHDS

  // count shapes excluding compounds corresponding to geom groups
  int oldNbSubShapes = meshDS->MaxShapeIndex();
  for ( ; oldNbSubShapes > 0; --oldNbSubShapes )
  {
    const TopoDS_Shape& s = meshDS->IndexToShape( oldNbSubShapes );
    if ( s.IsNull() || s.ShapeType() != TopAbs_COMPOUND )
      break;
    // fill ii2iMap
    std::set<int> subIds;
    for ( TopoDS_Iterator it( s ); it.More(); it.Next() )
      subIds.insert( meshDS->ShapeToIndex( it.Value() ));
    ii2iMap.insert( std::make_pair( subIds, oldNbSubShapes ));
  }

  // check if shape topology changes - save shape type per shape ID
  std::vector< TopAbs_ShapeEnum > shapeTypes( Max( oldNbSubShapes + 1, 1 ));
  for ( int shapeID = oldNbSubShapes; shapeID > 0; --shapeID )
    shapeTypes[ shapeID ] = meshDS->IndexToShape( shapeID ).ShapeType();

  // change shape to mesh
  _impl->ShapeToMesh( TopoDS_Shape() );
  _impl->ShapeToMesh( newShape );

  // check if shape topology changes - check new shape types
  bool sameTopology = ( oldNbSubShapes == meshDS->MaxShapeIndex() );
  for ( int shapeID = oldNbSubShapes; shapeID > 0 &&  sameTopology; --shapeID )
  {
    const TopoDS_Shape& s = meshDS->IndexToShape( shapeID );
    sameTopology = ( !s.IsNull() && s.ShapeType() == shapeTypes[ shapeID ]);
  }

  // re-add shapes (compounds) of geom groups
  typedef std::map< std::vector< int >, TGeomGroupData* > TIndices2GroupData;
  TIndices2GroupData ii2grData;
  std::vector< int > ii;
  std::map< int, int > old2newIDs; // group IDs
  std::list<TGeomGroupData>::iterator dataIt = _geomGroupData.begin();
  for ( ; dataIt != _geomGroupData.end(); ++dataIt )
  {
    TGeomGroupData* data = &(*dataIt);
    ii.reserve( data->_indices.size() );
    ii.assign( data->_indices.begin(), data->_indices.end() );
    TIndices2GroupData::iterator ii2gd = ii2grData.insert( std::make_pair( ii, data )).first;
    if ( ii2gd->second != data )
    {
      data->_groupEntry = ii2gd->second->_groupEntry;
      data->_indices    = ii2gd->second->_indices;
      continue;
    }
    const int  oldNbSub = data->_indices.size();
    const int soleOldID = oldNbSub == 1 ? *data->_indices.begin() : 0;
    int oldID = 0;
    std::multimap< std::set<int>, int >::iterator ii2i = ii2iMap.find( data->_indices );
    if ( ii2i != ii2iMap.end() )
    {
      oldID = ii2i->second;
      ii2iMap.erase( ii2i );
    }
    if ( !oldID && oldNbSub == 1 )
      oldID = soleOldID;
    if ( old2newIDs.count( oldID ))
      continue;

    int how = ( theIsBreakLink || !sameTopology ) ? IS_BREAK_LINK : MAIN_TRANSFORMED;
    newShape = newGroupShape( *data, how );

    if ( !newShape.IsNull() )
    {
      if ( oldNbSub > 1 && meshDS->ShapeToIndex( newShape ) > 0 ) // group reduced to one sub-shape
      {
        TopoDS_Compound compound;
        BRep_Builder().MakeCompound( compound );
        BRep_Builder().Add( compound, newShape );
        newShape = compound;
      }
      int newID = _impl->GetSubMesh( newShape )->GetId();
      if ( oldID /*&& oldID != newID*/ )
        old2newIDs.insert( std::make_pair( oldID, newID ));
      if ( oldNbSub == 1 )
        old2newIDs.insert( std::make_pair( soleOldID, newID ));
    }
  }

  // re-assign hypotheses
  for ( size_t i = 0; i < ids2Hyps.size(); ++i )
  {
    int sID = ids2Hyps[i].first;
    if ( sID != 1 )
    {
      std::map< int, int >::iterator o2n = old2newIDs.find( sID );
      if ( o2n != old2newIDs.end() )
        sID = o2n->second;
      else if ( !sameTopology )
        continue;
    }
    const TopoDS_Shape& s = meshDS->IndexToShape( sID );
    if ( s.IsNull() )
      continue;
    const THypList& hyps = ids2Hyps[i].second;
    THypList::const_iterator h = hyps.begin();
    for ( ; h != hyps.end(); ++h )
      _impl->AddHypothesis( s, (*h)->GetID() );
  }

  {
    // restore groups on geometry
    for ( size_t i = 0; i < groupsData.size(); ++i )
    {
      const TGroupOnGeomData& data = groupsData[i];
      if ( data._shape.IsNull() )
        continue;

      std::map<int, SMESH::SMESH_GroupBase_ptr>::iterator i2g = _mapGroups.find( data._oldID );
      if ( i2g == _mapGroups.end() ) continue;

      SMESH_GroupBase_i* gr_i = SMESH::DownCast<SMESH_GroupBase_i*>( i2g->second );
      if ( !gr_i ) continue;

      SMESH_Group* g = _impl->AddGroup( data._type, data._name.c_str(), data._oldID, data._shape );
      if ( !g )
        _mapGroups.erase( i2g );
      else
        g->GetGroupDS()->SetColor( data._color );
    }

    if ( !sameTopology )
    {
      std::map< int, int >::iterator o2n = old2newIDs.begin();
      for ( ; o2n != old2newIDs.end(); ++o2n )
      {
        int newID = o2n->second, oldID = o2n->first;
        if ( newID == oldID || !_mapSubMesh.count( oldID ))
          continue;
        if ( newID > 0 )
        {
          _mapSubMesh   [ newID ] = _impl->GetSubMeshContaining( newID );
          _mapSubMesh_i [ newID ] = _mapSubMesh_i [ oldID ];
          _mapSubMeshIor[ newID ] = _mapSubMeshIor[ oldID ];
        }
        _mapSubMesh.   erase(oldID);
        _mapSubMesh_i. erase(oldID);
        _mapSubMeshIor.erase(oldID);
        if ( newID > 0 )
          _mapSubMesh_i [ newID ]->changeLocalId( newID );
      }
    }

    // update _mapSubMesh
    std::map<int, ::SMESH_subMesh*>::iterator i_sm = _mapSubMesh.begin();
    for ( ; i_sm != _mapSubMesh.end(); ++i_sm )
      i_sm->second = _impl->GetSubMesh( meshDS->IndexToShape( i_sm->first ));
  }

  if ( !sameTopology )
  {
    // remove invalid study sub-objects
    CheckGeomGroupModif();
  }

  _gen_i->UpdateIcons( me );

  if ( !theIsBreakLink && isShaper )
  {
    SALOMEDS::SObject_wrap meshSO = _gen_i->ObjectToSObject( me );
    if ( !meshSO->_is_nil() )
      _gen_i->SetPixMap(meshSO, "ICON_SMESH_TREE_GEOM_MODIF");
  }
}

//=============================================================================
/*!
 * \brief Update objects depending on changed geom groups
 *
 * NPAL16168: geometrical group edition from a submesh don't modify mesh computation
 * issue 0020210: Update of a smesh group after modification of the associated geom group
 */
//=============================================================================

void SMESH_Mesh_i::CheckGeomGroupModif()
{
  // remove sub-meshes referring a removed sub-shapes (if main shape still exists)
  SALOMEDS::StudyBuilder_var builder = _gen_i->getStudyServant()->NewBuilder();
  GEOM::GEOM_Object_var  mainGO = GetShapeToMesh();
  SALOMEDS::SObject_wrap meshSO = _gen_i->ObjectToSObject( SMESH::SMESH_Mesh_var( _this() ));
  if ( !mainGO->_is_nil() && !meshSO->_is_nil() )
  {
    SALOMEDS::SObject_wrap rootSO, geomRefSO, geomSO;
    for ( CORBA::Long tag = SMESH::Tag_FirstSubMesh; tag <= SMESH::Tag_LastSubMesh; ++tag )
      if ( meshSO->FindSubObject( tag, rootSO.inout() ))
      {
        int nbValid = 0, nbRemoved = 0;
        SALOMEDS::ChildIterator_wrap chItr = _gen_i->getStudyServant()->NewChildIterator( rootSO );
        for ( ; chItr->More(); chItr->Next() )
        {
          SALOMEDS::SObject_wrap smSO = chItr->Value(); // sub-mesh SO
          if ( !smSO->_is_nil() &&
               smSO->FindSubObject( SMESH::Tag_RefOnShape, geomRefSO.inout() ) &&
               geomRefSO->ReferencedObject( geomSO.inout() )) // find geometry by reference
          {
            CORBA::Object_var  geomObj = _gen_i->SObjectToObject( geomSO );
            GEOM::GEOM_Object_var geom = GEOM::GEOM_Object::_narrow( geomObj );
            if ( !geom->_non_existent() )
            {
              ++nbValid;
              continue; // keep the sub-mesh
            }
          }
          CORBA::Object_var     smObj = _gen_i->SObjectToObject( smSO );
          SMESH::SMESH_subMesh_var sm = SMESH::SMESH_subMesh::_narrow( smObj );
          if ( !sm->_is_nil() && !sm->_non_existent() )
          {
            GEOM::GEOM_Object_var smGeom = sm->GetSubShape();
            if ( smGeom->_is_nil() )
            {
              RemoveSubMesh( sm );
              ++nbRemoved;
            }
          }
          else
          {
            if ( _preMeshInfo )
              _preMeshInfo->ForgetAllData(); // unknown hypothesis modified
            builder->RemoveObjectWithChildren( smSO ); // sub-shape removed before loading SMESH
            ++nbRemoved;
          }
        }
        if ( /*nbRemoved > 0 &&*/ nbValid == 0 )
          builder->RemoveObjectWithChildren( rootSO );
      }
  }

  // check for removed sub-shapes and convert geom dependent groups into standalone ones
  std::map<int, SMESH::SMESH_GroupBase_ptr>::iterator i_gr = _mapGroups.begin();
  while ( i_gr != _mapGroups.end())
  {
    SMESH::SMESH_GroupBase_ptr group = i_gr->second;
    ++i_gr;
    SALOMEDS::SObject_wrap        groupSO = _gen_i->ObjectToSObject( group ), refSO, geomSO;
    SMESH::SMESH_GroupOnGeom_var   onGeom = SMESH::SMESH_GroupOnGeom::_narrow  ( group );
    SMESH::SMESH_GroupOnFilter_var onFilt = SMESH::SMESH_GroupOnFilter::_narrow( group );
    bool isValidGeom = false;
    if ( !onGeom->_is_nil() )
    {
      isValidGeom = ( ! GEOM::GEOM_Object_var( onGeom->GetShape() )->_is_nil() ); // check TopoDS
      if ( !isValidGeom ) // check reference
      {
        isValidGeom = ( ! groupSO->_is_nil() &&
                        groupSO->FindSubObject( SMESH::Tag_RefOnShape, refSO.inout() ) &&
                        refSO->ReferencedObject( geomSO.inout() ) &&
                        ! geomSO->_is_nil() &&
                        !CORBA::is_nil( CORBA::Object_var( geomSO->GetObject() )));
      }
    }
    else if ( !onFilt->_is_nil() )
    {
      isValidGeom = isValidGeomFilter( onFilt->GetFilter() );
    }
    else // standalone
    {
      isValidGeom = ( !groupSO->_is_nil() &&
                      !groupSO->FindSubObject( SMESH::Tag_RefOnShape, refSO.inout() ));
    }
    if ( !isValidGeom )
    {
      if ( !IsLoaded() || group->IsEmpty() )
      {
        RemoveGroup( group );
      }
      else if ( !onGeom->_is_nil() || !onFilt->_is_nil() )
      {
        SMESH::SMESH_Group_var ( ConvertToStandalone( group ));
      }
      else // is it possible?
      {
        builder->RemoveObjectWithChildren( refSO );
      }
    }
  }


  if ( !_impl->HasShapeToMesh() ) return;

  SMESH::smIdType nbEntities = NbNodes() + NbElements();

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
      newShape = newGroupShape( *data, ONLY_IF_CHANGED );
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
      // set new shape to mesh -> DS of sub-meshes and geom groups is deleted
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
        SALOMEDS::SObject_wrap groupSO = _gen_i->ObjectToSObject( _mapGroups[oldID] );
        CORBA::String_var      name    = groupSO->GetName();
        // update
        if ( SMESH_GroupBase_i* group_i = SMESH::DownCast<SMESH_GroupBase_i*>(_mapGroups[oldID]))
          if ( SMESH_Group* group = _impl->AddGroup( geomType->second, name.in(),
                                                     /*id=*/-1, geom._shape ))
            group_i->changeLocalId( group->GetID() );
      }

      break; // everything has been updated

    } // update mesh
  } // loop on group data

  // Update icons

  SMESH::smIdType newNbEntities = NbNodes() + NbElements();
  list< SALOMEDS::SObject_wrap > soToUpdateIcons;
  if ( newNbEntities != nbEntities )
  {
    // Add all SObjects with icons to soToUpdateIcons
    soToUpdateIcons.push_back( _gen_i->ObjectToSObject( _this() )); // mesh

    for (map<int, SMESH::SMESH_subMesh_ptr>::iterator i_sm = _mapSubMeshIor.begin();
         i_sm != _mapSubMeshIor.end(); ++i_sm ) // submeshes
      soToUpdateIcons.push_back( _gen_i->ObjectToSObject( i_sm->second ));

    for ( map<int, SMESH::SMESH_GroupBase_ptr>::iterator i_gr = _mapGroups.begin();
          i_gr != _mapGroups.end(); ++i_gr ) // groups
      soToUpdateIcons.push_back( _gen_i->ObjectToSObject( i_gr->second ));
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
  SALOMEDS::Study_var        aStudy = SMESH_Gen_i::GetSMESHGen()->getStudyServant();
  if ( !aStudy->_is_nil() ) {
    builder  = aStudy->NewBuilder();
    aGroupSO = _gen_i->ObjectToSObject( theGroup );
    if ( !aGroupSO->_is_nil() )
    {
      // remove reference to geometry
      SALOMEDS::ChildIterator_wrap chItr = aStudy->NewChildIterator(aGroupSO);
      for ( ; chItr->More(); chItr->Next() )
      {
        // Remove group's child SObject
        SALOMEDS::SObject_wrap so = chItr->Value();
        builder->RemoveObject( so );
      }
      // Update Python script
      TPythonDump() << aGroupSO << " = " << SMESH::SMESH_Mesh_var(_this())
                    << ".ConvertToStandalone( " << aGroupSO << " )";

      // change icon of Group on Filter
      if ( isOnFilter )
      {
        // SMESH::array_of_ElementType_var elemTypes = aGroupImpl->GetTypes();
        // const int isEmpty = ( elemTypes->length() == 0 );
        // if ( !isEmpty )
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

//================================================================================
/*!
 * \brief Create a sub-mesh on a given sub-shape
 */
//================================================================================

SMESH::SMESH_subMesh_ptr SMESH_Mesh_i::createSubMesh( GEOM::GEOM_Object_ptr theSubShapeObject )
{
  if(MYDEBUG) MESSAGE( "createSubMesh" );
  TopoDS_Shape  myLocSubShape = _gen_i->GeomObjectToShape(theSubShapeObject);
  ::SMESH_subMesh * mySubMesh = _impl->GetSubMesh(myLocSubShape);
  int               subMeshId = 0;

  SMESH_subMesh_i * subMeshServant;
  if ( mySubMesh )
  {
    subMeshId = mySubMesh->GetId();
    subMeshServant = new SMESH_subMesh_i(myPOA, _gen_i, this, subMeshId);
  }
  else // "invalid sub-mesh"
  {
    // The invalid sub-mesh is created for the case where a valid sub-shape not found
    // by SMESH_Gen_i::CopyMeshWithGeom(). The invalid sub-mesh has GetId() < 0.
    if ( _mapSubMesh.empty() )
      subMeshId = -1;
    else
      subMeshId = _mapSubMesh.begin()->first - 1;
    subMeshServant = new SMESH_Invalid_subMesh_i(myPOA, _gen_i, this, subMeshId, theSubShapeObject);
  }

  SMESH::SMESH_subMesh_var subMesh = subMeshServant->_this();

  _mapSubMesh   [subMeshId] = mySubMesh;
  _mapSubMesh_i [subMeshId] = subMeshServant;
  _mapSubMeshIor[subMeshId] = SMESH::SMESH_subMesh::_duplicate( subMesh );

  subMeshServant->Register();

  // register CORBA object for persistence
  int nextId = _gen_i->RegisterObject( subMesh );
  if(MYDEBUG) { MESSAGE( "Add submesh to map with id = "<< nextId); }
  else        { (void)nextId; } // avoid "unused variable" warning

  // to track changes of GEOM groups
  if ( subMeshId > 0 )
    addGeomGroupData( theSubShapeObject, subMesh );

  return subMesh._retn();
}

//================================================================================
/*!
 * \brief Return an existing sub-mesh based on a sub-shape with the given ID
 */
//================================================================================

SMESH::SMESH_subMesh_ptr SMESH_Mesh_i::getSubMesh(int shapeID)
{
  map<int, SMESH::SMESH_subMesh_ptr>::iterator it = _mapSubMeshIor.find( shapeID );
  if ( it == _mapSubMeshIor.end() )
    return SMESH::SMESH_subMesh::_nil();

  return SMESH::SMESH_subMesh::_duplicate( (*it).second );
}

//================================================================================
/*!
 * \brief Remove a sub-mesh based on the given sub-shape
 */
//================================================================================

bool SMESH_Mesh_i::removeSubMesh (SMESH::SMESH_subMesh_ptr theSubMesh,
                                  GEOM::GEOM_Object_ptr    theSubShapeObject )
{
  bool isHypChanged = false;
  if ( theSubMesh->_is_nil() /*|| theSubShapeObject->_is_nil()*/ )
    return isHypChanged;

  const int subMeshId = theSubMesh->GetId();

  if ( theSubShapeObject->_is_nil() )  // not published shape (IPAL13617)
  {
    SMESH_subMesh* sm;
    if (( _mapSubMesh.count( subMeshId )) &&
        ( sm = _impl->GetSubMeshContaining( subMeshId )))
    {
      TopoDS_Shape S = sm->GetSubShape();
      if ( !S.IsNull() )
      {
        list<const SMESHDS_Hypothesis*> hyps = _impl->GetHypothesisList( S );
        isHypChanged = !hyps.empty();
        if ( isHypChanged && _preMeshInfo )
          _preMeshInfo->ForgetOrLoad();
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

//================================================================================
/*!
 * \brief Create a group. Group type depends on given arguments
 */
//================================================================================

SMESH::SMESH_GroupBase_ptr SMESH_Mesh_i::createGroup (SMESH::ElementType        theElemType,
                                                      const char*               theName,
                                                      const int                 theID,
                                                      const TopoDS_Shape&       theShape,
                                                      const SMESH_PredicatePtr& thePredicate )
{
  std::string newName;
  if ( !theName || !theName[0] )
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
  SMESH::SMESH_GroupBase_var aGroup;
  if ( SMESH_Group* g = _impl->AddGroup( (SMDSAbs_ElementType)theElemType, theName,
                                         theID, theShape, thePredicate ))
  {
    int anId = g->GetID();
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
    else        { nextId = ( nextId > 0 ); } // avoid "unused variable" warning in release mode

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

//================================================================================
/*!
 * \brief Return a log that can be used to move another mesh to the same state as this one
 */
//================================================================================

SMESH::log_array * SMESH_Mesh_i::GetLog(CORBA::Boolean clearAfterGet)
{
  SMESH::log_array_var aLog;

  SMESH_TRY;
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  list < SMESHDS_Command * >logDS = _impl->GetLog();
  aLog = new SMESH::log_array;
  int indexLog = 0;
  int lg = logDS.size();
  aLog->length(lg);
  list < SMESHDS_Command * >::iterator its = logDS.begin();
  while(its != logDS.end()){
    SMESHDS_Command *com = *its;
    int comType = com->GetType();
    smIdType lgcom = com->GetNumber();
    const list < smIdType >&intList = com->GetIndexes();
    int inum = intList.size();
    list < smIdType >::const_iterator ii = intList.begin();
    const list < double >&coordList = com->GetCoords();
    int rnum = coordList.size();
    list < double >::const_iterator ir = coordList.begin();
    aLog[indexLog].commandType = comType;
    aLog[indexLog].number = lgcom;
    aLog[indexLog].coords.length(rnum);
    aLog[indexLog].indexes.length(inum);
    for(int i = 0; i < rnum; i++){
      aLog[indexLog].coords[i] = *ir;
      ir++;
    }
    for(int i = 0; i < inum; i++){
      aLog[indexLog].indexes[i] = *ii;
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

//================================================================================
/*!
 * \brief Remove the log of commands
 */
//================================================================================

void SMESH_Mesh_i::ClearLog()
{
  SMESH_TRY;
  _impl->ClearLog();
  SMESH_CATCH( SMESH::throwCorbaException );
}

//================================================================================
/*!
 * \brief Return a mesh ID
 */
//================================================================================

CORBA::Long SMESH_Mesh_i::GetId()
{
  return _id;
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
    void RemoveGroup (const int theGroupID) override { _mesh->removeGroup( theGroupID ); }
    void HypothesisModified( int hypID,
                             bool updIcons) override { _mesh->onHypothesisModified( hypID,
                                                                                    updIcons ); }
    void Load ()                            override { _mesh->Load(); }
    bool IsLoaded()                         override { return _mesh->IsLoaded(); }
    TopoDS_Shape GetShapeByEntry(const std::string& entry) override
    {
      GEOM::GEOM_Object_var go = SMESH_Gen_i::GetGeomObjectByEntry( entry );
      return SMESH_Gen_i::GeomObjectToShape( go );
    }
  };
}

//================================================================================
/*!
 * \brief callback from _impl to
 *     1) forget not loaded mesh data (issue 0021208)
 *     2) mark hypothesis as valid
 */
//================================================================================

void SMESH_Mesh_i::onHypothesisModified(int theHypID, bool theUpdateIcons)
{
  if ( _preMeshInfo )
    _preMeshInfo->ForgetOrLoad();

  if ( theUpdateIcons )
  {
    SMESH::SMESH_Mesh_var mesh = _this();
    _gen_i->UpdateIcons( mesh );
  }

  if ( _nbInvalidHypos != 0 )
  {
    // mark a hypothesis as valid after edition
    int nbInvalid = 0;
    SALOMEDS::SComponent_wrap smeshComp = _gen_i->PublishComponent();
    SALOMEDS::SObject_wrap hypRoot;
    if ( !smeshComp->_is_nil() &&
         smeshComp->FindSubObject( _gen_i->GetHypothesisRootTag(), hypRoot.inout() ))
    {
      SALOMEDS::ChildIterator_wrap anIter = _gen_i->getStudyServant()->NewChildIterator( hypRoot );
      for ( ; anIter->More(); anIter->Next() )
      {
        SALOMEDS::SObject_wrap    hypSO = anIter->Value();
        CORBA::Object_var           obj = _gen_i->SObjectToObject( hypSO );
        SMESH::SMESH_Hypothesis_var hyp = SMESH::SMESH_Hypothesis::_narrow( obj );
        if ( !hyp->_is_nil() && hyp->GetId() == theHypID )
          _gen_i->HighLightInvalid( hyp, false );
        else
          nbInvalid += _gen_i->IsInvalid( hypSO );
      }
    }
    _nbInvalidHypos = nbInvalid;
  }
}

//================================================================================
/*!
 * \brief Set mesh implementation
 */
//================================================================================

void SMESH_Mesh_i::SetImpl(::SMESH_Mesh * impl)
{
  if(MYDEBUG) MESSAGE("SMESH_Mesh_i::SetImpl");
  _impl = impl;
  if ( _impl )
    _impl->SetCallUp( new TCallUp_i(this));
}

//=============================================================================
/*!
 * Return a mesh implementation
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

CORBA::Boolean SMESH_Mesh_i::HasModificationsToDiscard()
{
  Unexpect aCatch(SALOME_SalomeException);
  return _impl->HasModificationsToDiscard();
}

//================================================================================
/*!
 * \brief Return a random unique color
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
 * Set auto-color mode. If it is on, groups get unique random colors
 */
//=============================================================================

void SMESH_Mesh_i::SetAutoColor(CORBA::Boolean theAutoColor)
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
 * Return true if auto-color mode is on
 */
//=============================================================================

CORBA::Boolean SMESH_Mesh_i::GetAutoColor()
{
  Unexpect aCatch(SALOME_SalomeException);
  return _impl->GetAutoColor();
}

//=============================================================================
/*!
 *  Check if there are groups with equal names
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
  SMESH_File aFile( file, false );
  SMESH_Comment msg;
  if ( aFile.exists() ) {
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
          << ". Check the directory existence and access rights";
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
 * \brief Prepare a file for export and pass names of mesh groups from study to mesh DS
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
  SALOMEDS::Study_var aStudy = SMESH_Gen_i::GetSMESHGen()->getStudyServant();
  if ( !aStudy->_is_nil() ) {
    SALOMEDS::SObject_wrap aMeshSO = _gen_i->ObjectToSObject(  _this() );
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
 * \brief Export to MED file
 */
//================================================================================

void SMESH_Mesh_i::ExportMED(const char*    file,
                             CORBA::Boolean auto_groups,
                             CORBA::Long    version,
                             CORBA::Boolean overwrite,
                             CORBA::Boolean autoDimension)
{
  //MESSAGE("MED minor version: "<< minor);
  SMESH_TRY;
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  string aMeshName = prepareMeshNameAndGroups(file, overwrite);
  _impl->ExportMED( file, aMeshName.c_str(), auto_groups, version, 0, autoDimension );

  TPythonDump() << SMESH::SMESH_Mesh_var(_this()) << ".ExportMED( r'"
                << file << "', "
                << "auto_groups=" <<auto_groups << ", "
                << "version=" << version <<  ", "
                << "overwrite=" << overwrite << ", "
                << "meshPart=None, "
                << "autoDimension=" << autoDimension << " )";

  SMESH_CATCH( SMESH::throwCorbaException );
}

//================================================================================
/*!
 * \brief Export a mesh to a SAUV file
 */
//================================================================================

void SMESH_Mesh_i::ExportSAUV( const char* file, CORBA::Boolean auto_groups )
{
  SMESH_TRY;
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  string aMeshName = prepareMeshNameAndGroups(file, true);
  TPythonDump() << SMESH::SMESH_Mesh_var( _this())
                << ".ExportSAUV( r'" << file << "', " << auto_groups << " )";
  _impl->ExportSAUV(file, aMeshName.c_str(), auto_groups);

  SMESH_CATCH( SMESH::throwCorbaException );
}


//================================================================================
/*!
 * \brief Export a mesh to a DAT file
 */
//================================================================================

void SMESH_Mesh_i::ExportDAT (const char *file)
{
  SMESH_TRY;
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  // check names of groups
  checkGroupNames();
  // Update Python script
  TPythonDump() << SMESH::SMESH_Mesh_var(_this()) << ".ExportDAT( r'" << file << "' )";

  // Perform Export
  PrepareForWriting(file);
  _impl->ExportDAT(file);

  SMESH_CATCH( SMESH::throwCorbaException );
}

//================================================================================
/*!
 * \brief Export a mesh to an UNV file
 */
//================================================================================

void SMESH_Mesh_i::ExportUNV (const char *file)
{
  SMESH_TRY;
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  // check names of groups
  checkGroupNames();
  // Update Python script
  TPythonDump() << SMESH::SMESH_Mesh_var(_this()) << ".ExportUNV( r'" << file << "' )";

  // Perform Export
  PrepareForWriting(file);
  _impl->ExportUNV(file);

  SMESH_CATCH( SMESH::throwCorbaException );
}

//================================================================================
/*!
 * \brief Export a mesh to an STL file
 */
//================================================================================

void SMESH_Mesh_i::ExportSTL (const char *file, const bool isascii)
{
  SMESH_TRY;
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  // check names of groups
  checkGroupNames();
  // Update Python script
  TPythonDump() << SMESH::SMESH_Mesh_var(_this())
                << ".ExportSTL( r'" << file << "', " << isascii << " )";

  CORBA::String_var name;
  SALOMEDS::SObject_wrap so = _gen_i->ObjectToSObject( _this() );
  if ( !so->_is_nil() )
    name = so->GetName();

  // Perform Export
  PrepareForWriting( file );
  _impl->ExportSTL( file, isascii, name.in() );

  SMESH_CATCH( SMESH::throwCorbaException );
}

//================================================================================
/*!
 * \brief Export a part of mesh to a med file
 */
//================================================================================

void SMESH_Mesh_i::ExportPartToMED(SMESH::SMESH_IDSource_ptr meshPart,
                                   const char*               file,
                                   CORBA::Boolean            auto_groups,
                                   CORBA::Long               version,
                                   CORBA::Boolean            overwrite,
                                   CORBA::Boolean            autoDimension,
                                   const GEOM::ListOfFields& fields,
                                   const char*               geomAssocFields,
                                   CORBA::Double             ZTolerance)
{
  MESSAGE("MED version: "<< version);
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
    _impl->ExportMED( file, aMeshName.c_str(), auto_groups, version,
                      0, autoDimension, /*addODOnVertices=*/have0dField,
                      ZTolerance);
    meshDS = _impl->GetMeshDS();
  }
  else
  {
    if ( _preMeshInfo )
      _preMeshInfo->FullLoadFromFile();

    PrepareForWriting(file, overwrite);

    SALOMEDS::SObject_wrap SO = _gen_i->ObjectToSObject( meshPart );
    if ( !SO->_is_nil() ) {
      CORBA::String_var name = SO->GetName();
      aMeshName = name;
    }

    SMESH_MeshPartDS* partDS = new SMESH_MeshPartDS( meshPart );
    _impl->ExportMED( file, aMeshName.c_str(), auto_groups, version,
                      partDS, autoDimension, /*addODOnVertices=*/have0dField, ZTolerance);
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
                << meshPart << ", r'"
                << file << "', "
                << auto_groups << ", "
                << version << ", "
                << overwrite << ", "
                << autoDimension << ", "
                << goList << ", '"
                << ( geomAssocFields ? geomAssocFields : "" ) << "',"
                << TVar( ZTolerance )
                << " )";

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
{
  SMESH_TRY;
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  PrepareForWriting(file);

  SMESH_MeshPartDS partDS( meshPart );
  _impl->ExportDAT(file,&partDS);

  TPythonDump() << SMESH::SMESH_Mesh_var(_this())
                << ".ExportPartToDAT( " << meshPart << ", r'" << file << "' )";

  SMESH_CATCH( SMESH::throwCorbaException );
}
//================================================================================
/*!
 * \brief Export a part of mesh to an UNV file
 */
//================================================================================

void SMESH_Mesh_i::ExportPartToUNV(::SMESH::SMESH_IDSource_ptr meshPart,
                                   const char*                 file)
{
  SMESH_TRY;
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  PrepareForWriting(file);

  SMESH_MeshPartDS partDS( meshPart );
  _impl->ExportUNV(file, &partDS);

  TPythonDump() << SMESH::SMESH_Mesh_var(_this())
                << ".ExportPartToUNV( " << meshPart<< ", r'" << file << "' )";

  SMESH_CATCH( SMESH::throwCorbaException );
}
//================================================================================
/*!
 * \brief Export a part of mesh to an STL file
 */
//================================================================================

void SMESH_Mesh_i::ExportPartToSTL(::SMESH::SMESH_IDSource_ptr meshPart,
                                   const char*                 file,
                                   ::CORBA::Boolean            isascii)
{
  SMESH_TRY;
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  PrepareForWriting(file);

  CORBA::String_var name;
  SALOMEDS::SObject_wrap so = _gen_i->ObjectToSObject( meshPart );
  if ( !so->_is_nil() )
    name = so->GetName();

  SMESH_MeshPartDS partDS( meshPart );
  _impl->ExportSTL( file, isascii, name.in(), &partDS );

  TPythonDump() << SMESH::SMESH_Mesh_var(_this()) << ".ExportPartToSTL( "
                << meshPart<< ", r'" << file << "', " << isascii << ")";

  SMESH_CATCH( SMESH::throwCorbaException );
}

//================================================================================
/*!
 * \brief Export a part of mesh to an STL file
 */
//================================================================================

void SMESH_Mesh_i::ExportCGNS(::SMESH::SMESH_IDSource_ptr meshPart,
                              const char*                 file,
                              CORBA::Boolean              overwrite,
                              CORBA::Boolean              groupElemsByType)
{
#ifdef WITH_CGNS
  SMESH_TRY;
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  PrepareForWriting(file,overwrite);

  std::string meshName("");
  SALOMEDS::SObject_wrap so = _gen_i->ObjectToSObject( meshPart );
  if ( !so->_is_nil() )
  {
    CORBA::String_var name = so->GetName();
    meshName = name.in();
  }
  SMESH_TRY;

  SMESH_MeshPartDS partDS( meshPart );
  _impl->ExportCGNS(file, &partDS, meshName.c_str(), groupElemsByType );

  SMESH_CATCH( SMESH::throwCorbaException );

  TPythonDump() << SMESH::SMESH_Mesh_var(_this()) << ".ExportCGNS( "
                << meshPart<< ", r'" << file << "', " << overwrite << ")";

  SMESH_CATCH( SMESH::throwCorbaException );

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
{
  SMESH_TRY;
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  PrepareForWriting(file,/*overwrite=*/true);

  SMESH_MeshPartDS partDS( meshPart );
  _impl->ExportGMF(file, &partDS, withRequiredGroups);

  TPythonDump() << SMESH::SMESH_Mesh_var(_this()) << ".ExportGMF( "
                << meshPart<< ", r'"
                << file << "', "
                << withRequiredGroups << ")";

  SMESH_CATCH( SMESH::throwCorbaException );
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

//================================================================================
/*!
 * \brief Return nb of nodes
 */
//================================================================================

SMESH::smIdType SMESH_Mesh_i::NbNodes()
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbNodes();

  return _impl->NbNodes();
}

//================================================================================
/*!
 * \brief Return nb of elements
 */
//================================================================================

SMESH::smIdType SMESH_Mesh_i::NbElements()
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbElements();

  return Nb0DElements() + NbEdges() + NbFaces() + NbVolumes() + NbBalls();
}

//================================================================================
/*!
 * \brief Return nb of 0D elements
 */
//================================================================================

SMESH::smIdType SMESH_Mesh_i::Nb0DElements()
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->Nb0DElements();

  return _impl->Nb0DElements();
}

//================================================================================
/*!
 * \brief Return nb of BALL elements
 */
//================================================================================

SMESH::smIdType SMESH_Mesh_i::NbBalls()
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbBalls();

  return _impl->NbBalls();
}

//================================================================================
/*!
 * \brief Return nb of 1D elements
 */
//================================================================================

SMESH::smIdType SMESH_Mesh_i::NbEdges()
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbEdges();

  return _impl->NbEdges();
}

//================================================================================
/*!
 * \brief Return nb of edges
 */
//================================================================================

SMESH::smIdType SMESH_Mesh_i::NbEdgesOfOrder(SMESH::ElementOrder order)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbEdges( (SMDSAbs_ElementOrder) order );

  return _impl->NbEdges( (SMDSAbs_ElementOrder) order);
}

//================================================================================
/*!
 * \brief Return nb of faces
 */
//================================================================================

SMESH::smIdType SMESH_Mesh_i::NbFaces()
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbFaces();

  return _impl->NbFaces();
}

//================================================================================
/*!
 * \brief Return nb of tringles
 */
//================================================================================

SMESH::smIdType SMESH_Mesh_i::NbTriangles()
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbTriangles();

  return _impl->NbTriangles();
}

//================================================================================
/*!
 * \brief Return nb of bi-quadratic triangles
 */
//================================================================================

SMESH::smIdType SMESH_Mesh_i::NbBiQuadTriangles()
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbBiQuadTriangles();

  return _impl->NbBiQuadTriangles();
}

SMESH::smIdType SMESH_Mesh_i::NbQuadrangles()
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbQuadrangles();

  return _impl->NbQuadrangles();
}

SMESH::smIdType SMESH_Mesh_i::NbBiQuadQuadrangles()
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbBiQuadQuadrangles();

  return _impl->NbBiQuadQuadrangles();
}

SMESH::smIdType SMESH_Mesh_i::NbPolygons()
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbPolygons();

  return _impl->NbPolygons();
}

SMESH::smIdType SMESH_Mesh_i::NbPolygonsOfOrder(SMESH::ElementOrder order)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbPolygons((SMDSAbs_ElementOrder) order);

  return _impl->NbPolygons((SMDSAbs_ElementOrder)order);
}

SMESH::smIdType SMESH_Mesh_i::NbFacesOfOrder(SMESH::ElementOrder order)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbFaces( (SMDSAbs_ElementOrder) order );

  return _impl->NbFaces( (SMDSAbs_ElementOrder) order);
}

SMESH::smIdType SMESH_Mesh_i::NbTrianglesOfOrder(SMESH::ElementOrder order)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbTriangles( (SMDSAbs_ElementOrder) order );

  return _impl->NbTriangles( (SMDSAbs_ElementOrder) order);
}

SMESH::smIdType SMESH_Mesh_i::NbQuadranglesOfOrder(SMESH::ElementOrder order)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbQuadrangles( (SMDSAbs_ElementOrder) order );

  return _impl->NbQuadrangles( (SMDSAbs_ElementOrder) order);
}

//=============================================================================

SMESH::smIdType SMESH_Mesh_i::NbVolumes()
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbVolumes();

  return _impl->NbVolumes();
}

SMESH::smIdType SMESH_Mesh_i::NbTetras()
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbTetras();

  return _impl->NbTetras();
}

SMESH::smIdType SMESH_Mesh_i::NbHexas()
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbHexas();

  return _impl->NbHexas();
}

SMESH::smIdType SMESH_Mesh_i::NbTriQuadraticHexas()
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbTriQuadHexas();

  return _impl->NbTriQuadraticHexas();
}

SMESH::smIdType SMESH_Mesh_i::NbPyramids()
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbPyramids();

  return _impl->NbPyramids();
}

SMESH::smIdType SMESH_Mesh_i::NbPrisms()
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbPrisms();

  return _impl->NbPrisms();
}

SMESH::smIdType SMESH_Mesh_i::NbHexagonalPrisms()
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbHexPrisms();

  return _impl->NbHexagonalPrisms();
}

SMESH::smIdType SMESH_Mesh_i::NbPolyhedrons()
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbPolyhedrons();

  return _impl->NbPolyhedrons();
}

SMESH::smIdType SMESH_Mesh_i::NbVolumesOfOrder(SMESH::ElementOrder order)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbVolumes( (SMDSAbs_ElementOrder) order );

  return _impl->NbVolumes( (SMDSAbs_ElementOrder) order);
}

SMESH::smIdType SMESH_Mesh_i::NbTetrasOfOrder(SMESH::ElementOrder order)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbTetras( (SMDSAbs_ElementOrder) order);

  return _impl->NbTetras( (SMDSAbs_ElementOrder) order);
}

SMESH::smIdType SMESH_Mesh_i::NbHexasOfOrder(SMESH::ElementOrder order)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbHexas( (SMDSAbs_ElementOrder) order);

  return _impl->NbHexas( (SMDSAbs_ElementOrder) order);
}

SMESH::smIdType SMESH_Mesh_i::NbPyramidsOfOrder(SMESH::ElementOrder order)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbPyramids( (SMDSAbs_ElementOrder) order);

  return _impl->NbPyramids( (SMDSAbs_ElementOrder) order);
}

SMESH::smIdType SMESH_Mesh_i::NbPrismsOfOrder(SMESH::ElementOrder order)
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    return _preMeshInfo->NbPrisms( (SMDSAbs_ElementOrder) order);

  return _impl->NbPrisms( (SMDSAbs_ElementOrder) order);
}

//=============================================================================
/*!
 * Return nb of published sub-meshes
 */
//=============================================================================

SMESH::smIdType SMESH_Mesh_i::NbSubMesh()
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

SMESH::smIdType_array* SMESH_Mesh_i::GetIDs()
{
  return GetElementsId();
}

//=============================================================================
/*!
 * Return ids of all elements
 */
//=============================================================================

SMESH::smIdType_array* SMESH_Mesh_i::GetElementsId()
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESH::smIdType_array_var aResult = new SMESH::smIdType_array();
  SMESHDS_Mesh* aSMESHDS_Mesh = _impl->GetMeshDS();

  if ( aSMESHDS_Mesh == NULL )
    return aResult._retn();

  smIdType nbElements = NbElements();
  aResult->length( nbElements );
  SMDS_ElemIteratorPtr anIt = aSMESHDS_Mesh->elementsIterator();
  for ( smIdType i = 0, n = nbElements; i < n && anIt->more(); i++ )
    aResult[i] = anIt->next()->GetID();

  return aResult._retn();
}


//=============================================================================
/*!
 * Return ids of all elements of given type
 */
//=============================================================================

SMESH::smIdType_array* SMESH_Mesh_i::GetElementsByType( SMESH::ElementType theElemType )
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESH::smIdType_array_var aResult = new SMESH::smIdType_array();
  SMESHDS_Mesh* aSMESHDS_Mesh = _impl->GetMeshDS();

  if ( aSMESHDS_Mesh == NULL )
    return aResult._retn();

  smIdType nbElements = NbElements();

  // No sense in returning ids of elements along with ids of nodes:
  // when theElemType == SMESH::ALL, return node ids only if
  // there are no elements
  if ( theElemType == SMESH::NODE || (theElemType == SMESH::ALL && nbElements == 0) )
    return GetNodesId();

  aResult->length( nbElements );

  smIdType i = 0;

  SMDS_ElemIteratorPtr anIt = aSMESHDS_Mesh->elementsIterator( (SMDSAbs_ElementType)theElemType );
  while ( i < nbElements && anIt->more() )
    aResult[i++] = anIt->next()->GetID();

  aResult->length( i );

  return aResult._retn();
}

//=============================================================================
/*!
 * Return ids of all nodes
 */
//=============================================================================

SMESH::smIdType_array* SMESH_Mesh_i::GetNodesId()
{
  Unexpect aCatch(SALOME_SalomeException);
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESH::smIdType_array_var aResult = new SMESH::smIdType_array();
  SMESHDS_Mesh* aMeshDS = _impl->GetMeshDS();

  if ( aMeshDS == NULL )
    return aResult._retn();

  smIdType nbNodes = NbNodes();
  aResult->length( nbNodes );
  SMDS_NodeIteratorPtr anIt = aMeshDS->nodesIterator();
  for ( smIdType i = 0, n = nbNodes; i < n && anIt->more(); i++ )
    aResult[i] = anIt->next()->GetID();

  return aResult._retn();
}

//=============================================================================
/*!
 * Return type of the given element
 */
//=============================================================================

SMESH::ElementType SMESH_Mesh_i::GetElementType( const SMESH::smIdType id, const bool iselem )
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
 * Return geometric type of the given element
 */
//=============================================================================

SMESH::EntityType SMESH_Mesh_i::GetElementGeomType( const SMESH::smIdType id )
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
 * Return type of the given element
 */
//=============================================================================

SMESH::GeometryType SMESH_Mesh_i::GetElementShape( const SMESH::smIdType id )
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
 * Return ID of elements for given submesh
 */
//=============================================================================

SMESH::smIdType_array* SMESH_Mesh_i::GetSubMeshElementsId(const CORBA::Long ShapeID)
{
  SMESH::smIdType_array_var aResult = new SMESH::smIdType_array();

  SMESH_TRY;
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESH_subMesh* SM = _impl->GetSubMeshContaining(ShapeID);
  if(!SM) return aResult._retn();

  SMESHDS_SubMesh* SDSM = SM->GetSubMeshDS();
  if(!SDSM) return aResult._retn();

  aResult->length(SDSM->NbElements());

  SMDS_ElemIteratorPtr eIt = SDSM->GetElements();
  smIdType i = 0;
  while ( eIt->more() ) {
    aResult[i++] = eIt->next()->GetID();
  }

  SMESH_CATCH( SMESH::throwCorbaException );

  return aResult._retn();
}

//=============================================================================
/*!
 * Return ID of nodes for given sub-mesh
 * If param all==true - return all nodes, else -
 * Return only nodes on shapes.
 */
//=============================================================================

SMESH::smIdType_array* SMESH_Mesh_i::GetSubMeshNodesId(const CORBA::Long ShapeID,
                                                   CORBA::Boolean    all)
{
  SMESH::smIdType_array_var aResult = new SMESH::smIdType_array();

  SMESH_TRY;
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESH_subMesh* SM = _impl->GetSubMeshContaining(ShapeID);
  if(!SM) return aResult._retn();

  SMESHDS_SubMesh* SDSM = SM->GetSubMeshDS();
  if(!SDSM) return aResult._retn();

  set<smIdType> theElems;
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
  set<smIdType>::iterator itElem;
  smIdType i = 0;
  for ( itElem = theElems.begin(); itElem != theElems.end(); itElem++ )
    aResult[i++] = *itElem;

  SMESH_CATCH( SMESH::throwCorbaException );

  return aResult._retn();
}

//=============================================================================
/*!
 * Return type of elements for given sub-mesh
 */
//=============================================================================

SMESH::ElementType SMESH_Mesh_i::GetSubMeshElementType(const CORBA::Long ShapeID)
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
 * Return pointer to _impl as an integer value. Is called from constructor of SMESH_Client
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
 * If there is not node for given ID - return empty list
 */
//=============================================================================

SMESH::double_array* SMESH_Mesh_i::GetNodeXYZ(const SMESH::smIdType id)
{
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESH::double_array_var aResult = new SMESH::double_array();
  SMESHDS_Mesh* aMeshDS = _impl->GetMeshDS();
  if ( aMeshDS == NULL )
    return aResult._retn();

  // find node
  const SMDS_MeshNode* aNode = aMeshDS->FindNode(id);
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
 * For given node return list of IDs of inverse elements
 * If there is not node for given ID - return empty list
 */
//=============================================================================

SMESH::smIdType_array* SMESH_Mesh_i::GetNodeInverseElements(const SMESH::smIdType  id,
                                                            SMESH::ElementType elemType)
{
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESH::smIdType_array_var aResult = new SMESH::smIdType_array();
  SMESHDS_Mesh* aMeshDS = _impl->GetMeshDS();
  if ( aMeshDS == NULL )
    return aResult._retn();

  // find node
  const SMDS_MeshNode* aNode = aMeshDS->FindNode( id );
  if ( !aNode )
    return aResult._retn();

  // find inverse elements
  SMDSAbs_ElementType type = SMDSAbs_ElementType( elemType );
  SMDS_ElemIteratorPtr eIt = aNode->GetInverseElementIterator( type );
  aResult->length( aNode->NbInverseElements( type ));
  for( smIdType i = 0; eIt->more(); ++i )
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

SMESH::NodePosition* SMESH_Mesh_i::GetNodePosition(SMESH::smIdType NodeID)
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
        aNodePosition->params[0] = SMDS_EdgePositionPtr( pos )->GetUParameter();
        break;
      case SMDS_TOP_FACE: {
        SMDS_FacePositionPtr fPos = pos;
        aNodePosition->shapeType = GEOM::FACE;
        aNodePosition->params.length(2);
        aNodePosition->params[0] = fPos->GetUParameter();
        aNodePosition->params[1] = fPos->GetVParameter();
        break;
      }
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

SMESH::ElementPosition SMESH_Mesh_i::GetElementPosition(SMESH::smIdType ElemID)
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
 * If given element is node return IDs of shape from position
 * If there is not node for given ID - return -1
 */
//=============================================================================

CORBA::Long SMESH_Mesh_i::GetShapeID(const SMESH::smIdType id)
{
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESHDS_Mesh* aMeshDS = _impl->GetMeshDS();
  if ( aMeshDS == NULL )
    return -1;

  // try to find node
  const SMDS_MeshNode* aNode = aMeshDS->FindNode(id);
  if(aNode) {
    return aNode->getshapeId();
  }

  return -1;
}


//=============================================================================
/*!
 * For given element return ID of result shape after
 * ::FindShape() from SMESH_MeshEditor
 * If there is not element for given ID - return -1
 */
//=============================================================================

CORBA::Long SMESH_Mesh_i::GetShapeIDForElem(const SMESH::smIdType id)
{
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESHDS_Mesh* aMeshDS = _impl->GetMeshDS();
  if ( aMeshDS == NULL )
    return -1;

  // try to find element
  const SMDS_MeshElement* elem = aMeshDS->FindElement(id);
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
 * Return number of nodes for given element
 * If there is not element for given ID - return -1
 */
//=============================================================================

CORBA::Short SMESH_Mesh_i::GetElemNbNodes(const SMESH::smIdType id)
{
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESHDS_Mesh* aMeshDS = _impl->GetMeshDS();
  if ( aMeshDS == NULL ) return -1;
  // try to find element
  const SMDS_MeshElement* elem = aMeshDS->FindElement(id);
  if(!elem) return -1;
  return elem->NbNodes();
}


//=============================================================================
/*!
 * Return ID of node by given index for given element
 * If there is not element for given ID - return -1
 * If there is not node for given index - return -2
 */
//=============================================================================

SMESH::smIdType SMESH_Mesh_i::GetElemNode(const SMESH::smIdType id, const CORBA::Short index)
{
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESHDS_Mesh* aMeshDS = _impl->GetMeshDS();
  if ( aMeshDS == NULL ) return -1;
  const SMDS_MeshElement* elem = aMeshDS->FindElement(id);
  if(!elem) return -1;
  if( index>=elem->NbNodes() || index<0 ) return -1;
  return elem->GetNode(index)->GetID();
}

//=============================================================================
/*!
 * Return IDs of nodes of given element
 */
//=============================================================================

SMESH::smIdType_array* SMESH_Mesh_i::GetElemNodes(const SMESH::smIdType id)
{
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESH::smIdType_array_var aResult = new SMESH::smIdType_array();
  if ( SMESHDS_Mesh* aMeshDS = _impl->GetMeshDS() )
  {
    if ( const SMDS_MeshElement* elem = aMeshDS->FindElement(id) )
    {
      aResult->length( elem->NbNodes() );
      for ( SMESH::smIdType i = 0; i < aResult->length(); ++i )
        if ( const SMDS_MeshNode* n = elem->GetNode( i ))
          aResult[ i ] = n->GetID();
    }
  }
  return aResult._retn();
}

//=============================================================================
/*!
 * Return true if given node is medium node
 * in given quadratic element
 */
//=============================================================================

CORBA::Boolean SMESH_Mesh_i::IsMediumNode(const SMESH::smIdType ide, const SMESH::smIdType idn)
{
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESHDS_Mesh* aMeshDS = _impl->GetMeshDS();
  if ( aMeshDS == NULL ) return false;
  // try to find node
  const SMDS_MeshNode* aNode = aMeshDS->FindNode(idn);
  if(!aNode) return false;
  // try to find element
  const SMDS_MeshElement* elem = aMeshDS->FindElement(ide);
  if(!elem) return false;

  return elem->IsMediumNode(aNode);
}


//=============================================================================
/*!
 * Return true if given node is medium node
 * in one of quadratic elements
 */
//=============================================================================

CORBA::Boolean SMESH_Mesh_i::IsMediumNodeOfAnyElem(const SMESH::smIdType idn,
                                                   SMESH::ElementType theElemType)
{
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESHDS_Mesh* aMeshDS = _impl->GetMeshDS();
  if ( aMeshDS == NULL ) return false;

  // try to find node
  const SMDS_MeshNode* aNode = aMeshDS->FindNode(idn);
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
 * Return number of edges for given element
 */
//=============================================================================

CORBA::Long SMESH_Mesh_i::ElemNbEdges(const SMESH::smIdType id)
{
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESHDS_Mesh* aMeshDS = _impl->GetMeshDS();
  if ( aMeshDS == NULL ) return -1;
  const SMDS_MeshElement* elem = aMeshDS->FindElement(id);
  if(!elem) return -1;
  return elem->NbEdges();
}


//=============================================================================
/*!
 * Return number of faces for given element
 */
//=============================================================================

CORBA::Long SMESH_Mesh_i::ElemNbFaces(const SMESH::smIdType id)
{
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESHDS_Mesh* aMeshDS = _impl->GetMeshDS();
  if ( aMeshDS == NULL ) return -1;
  const SMDS_MeshElement* elem = aMeshDS->FindElement(id);
  if(!elem) return -1;
  return elem->NbFaces();
}

//================================================================================
/*!
 * \brief Return nodes of given face (counted from zero) for given element.
 */
//================================================================================

SMESH::smIdType_array* SMESH_Mesh_i::GetElemFaceNodes(SMESH::smIdType  elemId,
                                                      CORBA::Short     faceIndex)
{
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESH::smIdType_array_var aResult = new SMESH::smIdType_array();
  if ( SMESHDS_Mesh* aMeshDS = _impl->GetMeshDS() )
  {
    if ( const SMDS_MeshElement* elem = aMeshDS->FindElement(elemId) )
    {
      SMDS_VolumeTool vtool( elem, /*skipCentralNodes = */false );
      if ( faceIndex < vtool.NbFaces() )
      {
        aResult->length( vtool.NbFaceNodes( faceIndex ));
        const SMDS_MeshNode** nn = vtool.GetFaceNodes( faceIndex );
        for ( SMESH::smIdType i = 0; i < aResult->length(); ++i )
          aResult[ i ] = nn[ i ]->GetID();
      }
    }
  }
  return aResult._retn();
}

//================================================================================
/*!
 * \brief Return three components of normal of given mesh face.
 */
//================================================================================

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

//================================================================================
/*!
 * \brief Return an element based on all given nodes.
 */
//================================================================================

SMESH::smIdType SMESH_Mesh_i::FindElementByNodes(const SMESH::smIdType_array& nodes)
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

//================================================================================
/*!
 * \brief Return elements including all given nodes.
 */
//================================================================================

SMESH::smIdType_array* SMESH_Mesh_i::GetElementsByNodes(const SMESH::smIdType_array& nodes,
                                                        SMESH::ElementType       elemType)
{
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESH::smIdType_array_var result = new SMESH::smIdType_array();

  if ( SMESHDS_Mesh* mesh = _impl->GetMeshDS() )
  {
    vector< const SMDS_MeshNode * > nn( nodes.length() );
    for ( SMESH::smIdType i = 0; i < nodes.length(); ++i )
      nn[i] = mesh->FindNode( nodes[i] );

    std::vector<const SMDS_MeshElement *> elems;
    mesh->GetElementsByNodes( nn, elems, (SMDSAbs_ElementType) elemType );
    result->length( elems.size() );
    for ( size_t i = 0; i < elems.size(); ++i )
      result[i] = elems[i]->GetID();
  }
  return result._retn();
}

//=============================================================================
/*!
 * Return true if given element is polygon
 */
//=============================================================================

CORBA::Boolean SMESH_Mesh_i::IsPoly(const SMESH::smIdType id)
{
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESHDS_Mesh* aMeshDS = _impl->GetMeshDS();
  if ( aMeshDS == NULL ) return false;
  const SMDS_MeshElement* elem = aMeshDS->FindElement(id);
  if(!elem) return false;
  return elem->IsPoly();
}


//=============================================================================
/*!
 * Return true if given element is quadratic
 */
//=============================================================================

CORBA::Boolean SMESH_Mesh_i::IsQuadratic(const SMESH::smIdType id)
{
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESHDS_Mesh* aMeshDS = _impl->GetMeshDS();
  if ( aMeshDS == NULL ) return false;
  const SMDS_MeshElement* elem = aMeshDS->FindElement(id);
  if(!elem) return false;
  return elem->IsQuadratic();
}

//=============================================================================
/*!
 * Return diameter of ball discrete element or zero in case of an invalid \a id
 */
//=============================================================================

CORBA::Double SMESH_Mesh_i::GetBallDiameter(SMESH::smIdType id)
{
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  if ( const SMDS_BallElement* ball =
       SMDS_Mesh::DownCast<SMDS_BallElement>( _impl->GetMeshDS()->FindElement( id )))
    return ball->GetDiameter();

  return 0;
}

//=============================================================================
/*!
 * Return bary center for given element
 */
//=============================================================================

SMESH::double_array* SMESH_Mesh_i::BaryCenter(const SMESH::smIdType id)
{
  if ( _preMeshInfo )
    _preMeshInfo->FullLoadFromFile();

  SMESH::double_array_var aResult = new SMESH::double_array();
  SMESHDS_Mesh* aMeshDS = _impl->GetMeshDS();
  if ( aMeshDS == NULL )
    return aResult._retn();

  const SMDS_MeshElement* elem = aMeshDS->FindElement(id);
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
    if ( error && error->HasBadElems() )
    {
      // sort bad elements by type
      vector< const SMDS_MeshElement* > elemsByType[ SMDSAbs_NbElementTypes ];
      const list<const SMDS_MeshElement*>& badElems =
        static_cast<SMESH_BadInputElements*>( error.get() )->myBadElements;
      list<const SMDS_MeshElement*>::const_iterator elemIt  = badElems.begin();
      list<const SMDS_MeshElement*>::const_iterator elemEnd = badElems.end();
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
          SMESH::SMESH_Mesh_var mesh = _this();
          SALOMEDS::SObject_wrap aSO =
            _gen_i->PublishGroup( mesh, groups[ iG ],
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
  SMESH::SMESH_Mesh_var aMesh = _this();

  set<int> addedIDs;
  ::SMESH_Mesh::GroupIteratorPtr groupIt = _impl->GetGroups();
  while ( groupIt->more() )
  {
    ::SMESH_Group* group = groupIt->next();
    int             anId = group->GetID();

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
    else        { (void)nextId; } // avoid "unused variable" warning in release mode

    // publishing the groups in the study
    GEOM::GEOM_Object_var shapeVar = _gen_i->ShapeToGeomObject( shape );
    _gen_i->PublishGroup( aMesh, groupVar, shapeVar, group->GetName());
  }

  if ( !addedIDs.empty() )
  {
    // python dump
    map<int, SMESH::SMESH_GroupBase_ptr>::iterator i_grp = _mapGroups.begin();
    for ( int index = 0; i_grp != _mapGroups.end(); ++index, ++i_grp )
    {
      set<int>::iterator it = addedIDs.find( i_grp->first );
      if ( it != addedIDs.end() )
      {
        TPythonDump() << i_grp->second << " = " << aMesh << ".GetGroups()[ "<< index << " ]";
        addedIDs.erase( it );
        if ( addedIDs.empty() )
          break;
      }
    }
  }
}

//=============================================================================
/*!
 * \brief Return true if all sub-meshes are computed OK - to update an icon
 */
//=============================================================================

bool SMESH_Mesh_i::IsComputedOK()
{
  return _impl->IsComputedOK();
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

//=======================================================================
//function : FileInfoToString
//purpose  : Persistence of file info
//=======================================================================

std::string SMESH_Mesh_i::FileInfoToString()
{
  std::string s;
  if ( &_medFileInfo.in() && _medFileInfo->fileName[0] )
  {
    s = SMESH_Comment( _medFileInfo->fileSize )
      << " " << _medFileInfo->major
      << " " << _medFileInfo->minor
      << " " << _medFileInfo->release
      << " " << _medFileInfo->fileName;
  }
  return s;
}

//=======================================================================
//function : FileInfoFromString
//purpose  : Persistence of file info
//=======================================================================

void SMESH_Mesh_i::FileInfoFromString(const std::string& info)
{
  std::string size, major, minor, release, fileName;
  std::istringstream is(info);
  is >> size >> major >> minor >> release;
  fileName = info.data() + ( size.size()   + 1 +
                             major.size()  + 1 +
                             minor.size()  + 1 +
                             release.size()+ 1 );

  _medFileInfo           = new SMESH::MedFileInfo();
  _medFileInfo->fileName = fileName.c_str();
  _medFileInfo->fileSize = atoi( size.c_str() );
  _medFileInfo->major    = atoi( major.c_str() );
  _medFileInfo->minor    = atoi( minor.c_str() );
  _medFileInfo->release  = atoi( release.c_str() );
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
    SALOMEDS::SObject_wrap aGrpSO = _gen_i->ObjectToSObject( aGrp );
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
 * \brief Set list of notebook variables used for Mesh operations separated by ":" symbol
 */
//=============================================================================
void SMESH_Mesh_i::SetParameters(const char* theParameters)
{
  SMESH_Gen_i::GetSMESHGen()->UpdateParameters( CORBA::Object_var( _this() ).in(),
                                                theParameters );
}

//=============================================================================
/*!
 * \brief Return list of notebook variables used for Mesh operations separated by ":" symbol
 */
//=============================================================================

char* SMESH_Mesh_i::GetParameters()
{
  return SMESH_Gen_i::GetSMESHGen()->GetParameters( SMESH::SMESH_Mesh_var( _this()) );
}

//=============================================================================
/*!
 * \brief Return list of notebook variables used for last Mesh operation
 */
//=============================================================================
SMESH::string_array* SMESH_Mesh_i::GetLastParameters()
{
  SMESH::string_array_var aResult = new SMESH::string_array();
  SMESH_Gen_i *gen = SMESH_Gen_i::GetSMESHGen();
  if(gen) {
    CORBA::String_var aParameters = GetParameters();
    SALOMEDS::ListOfListOfStrings_var aSections = SMESH_Gen_i::GetSMESHGen()->getStudyServant()->ParseVariables(aParameters);
    if ( aSections->length() > 0 ) {
      SALOMEDS::ListOfStrings aVars = aSections[ aSections->length() - 1 ];
      aResult->length( aVars.length() );
      for ( CORBA::ULong i = 0;i < aVars.length(); i++ )
        aResult[i] = CORBA::string_dup( aVars[i] );
    }
  }
  return aResult._retn();
}

//================================================================================
/*!
 * \brief Return types of elements it contains
 */
//================================================================================

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

//================================================================================
/*!
 * \brief Return self
 */
//================================================================================

SMESH::SMESH_Mesh_ptr SMESH_Mesh_i::GetMesh()
{
  return SMESH::SMESH_Mesh::_duplicate( _this() );
}

//================================================================================
/*!
 * \brief Return false if GetMeshInfo() return incorrect information that may
 *        happen if mesh data is not yet fully loaded from the file of study.
 * 
 * 
 */
//================================================================================

bool SMESH_Mesh_i::IsMeshInfoCorrect()
{
  return _preMeshInfo ? _preMeshInfo->IsMeshInfoCorrect() : true;
}

//=============================================================================
/*!
 * \brief Return number of mesh elements per each \a EntityType
 */
//=============================================================================

SMESH::smIdType_array* SMESH_Mesh_i::GetMeshInfo()
{
  if ( _preMeshInfo )
    return _preMeshInfo->GetMeshInfo();

  SMESH::smIdType_array_var aRes = new SMESH::smIdType_array();
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
 * \brief Return number of mesh elements per each \a ElementType
 */
//=============================================================================

SMESH::smIdType_array* SMESH_Mesh_i::GetNbElementsByType()
{
  SMESH::smIdType_array_var aRes = new SMESH::smIdType_array();
  aRes->length(SMESH::NB_ELEMENT_TYPES);
  for (smIdType i = 0; i < SMESH::NB_ELEMENT_TYPES; i++)
    aRes[ i ] = 0;

  const SMDS_MeshInfo* meshInfo = 0;
  if ( _preMeshInfo )
    meshInfo = _preMeshInfo;
  else if ( SMESHDS_Mesh* meshDS = _impl->GetMeshDS() )
    meshInfo = & meshDS->GetMeshInfo();

  if (meshInfo)
    for (smIdType i = 0; i < SMESH::NB_ELEMENT_TYPES; i++)
      aRes[i] = meshInfo->NbElements((SMDSAbs_ElementType)i);

  return aRes._retn();
}

//=============================================================================
/*
 * Collect statistic of mesh elements given by iterator
 */
//=============================================================================

void SMESH_Mesh_i::CollectMeshInfo(const SMDS_ElemIteratorPtr theItr,
                                   SMESH::smIdType_array&     theInfo)
{
  if (!theItr) return;
  while (theItr->more())
    theInfo[ theItr->next()->GetEntityType() ]++;
}
//=============================================================================
/*
 * Return mesh unstructed grid information.
 */
//=============================================================================

SALOMEDS::TMPFile* SMESH_Mesh_i::GetVtkUgStream()
{
  SALOMEDS::TMPFile_var SeqFile;
  if ( SMESHDS_Mesh* aMeshDS = _impl->GetMeshDS() ) {
    SMDS_UnstructuredGrid* aGrid = aMeshDS->GetGrid();
    if(aGrid) {
      vtkUnstructuredGridWriter* aWriter = vtkUnstructuredGridWriter::New();
      aWriter->WriteToOutputStringOn();
      aWriter->SetInputData(aGrid);
      aWriter->SetFileTypeToBinary();
      aWriter->Write();
      char* str = aWriter->GetOutputString();
      int size = aWriter->GetOutputStringLength();

      //Allocate octet buffer of required size
      CORBA::Octet* OctetBuf = SALOMEDS::TMPFile::allocbuf(size);
      //Copy ostrstream content to the octet buffer
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
    SMDSAbs_ElementType     _type;

    PredicateIterator( SMDS_ElemIteratorPtr iterator,
                       PredicatePtr         predicate,
                       SMDSAbs_ElementType  type):
      _elemIter(iterator), _predicate(predicate), _type(type)
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
        if ((_elem = _elemIter->next()) &&
            (( _type != SMDSAbs_All && _type != _elem->GetType() ) ||
             ( !_predicate->IsSatisfy( _elem->GetID() ))))
          _elem = 0;
      }
      return res;
    }
  };

  //-----------------------------------------------------------------------------
  struct IDSourceIterator : public SMDS_ElemIterator
  {
    const SMESH::smIdType*        _idPtr;
    const SMESH::smIdType*        _idEndPtr;
    SMESH::smIdType_array_var     _idArray;
    const SMDS_Mesh*              _mesh;
    const SMDSAbs_ElementType     _type;
    const SMDS_MeshElement*       _elem;

    IDSourceIterator( const SMDS_Mesh*       mesh,
                      const SMESH::smIdType* ids,
                      const smIdType         nbIds,
                      SMDSAbs_ElementType    type):
      _idPtr( ids ), _idEndPtr( ids + nbIds ), _mesh( mesh ), _type( type ), _elem( 0 )
    {
      if ( _idPtr && nbIds && _mesh )
        next();
    }
    IDSourceIterator( const SMDS_Mesh*       mesh,
                      SMESH::smIdType_array* idArray,
                      SMDSAbs_ElementType    type):
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
      while ( !_node && ( _elemIter->more() || _nodeIter->more() ))
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
         filter_i->GetElementType() == SMESH::ALL ||
         elemType == SMDSAbs_Node ||
         elemType == SMDSAbs_All)
    {
      SMESH::Predicate_i* pred_i = filter_i->GetPredicate_i();
      if ( pred_i && pred_i->GetPredicate() )
      {
        SMDSAbs_ElementType filterType = SMDSAbs_ElementType( filter_i->GetElementType() );
        SMDS_ElemIteratorPtr allElemIt = meshDS->elementsIterator( filterType );
        SMDSAbs_ElementType   iterType = elemType == SMDSAbs_Node ? filterType : elemType;
        elemIt = SMDS_ElemIteratorPtr
          ( new PredicateIterator( allElemIt, pred_i->GetPredicate(), iterType ));
        typeOK = ( elemType == SMDSAbs_Node ? filterType == SMDSAbs_Node : elemIt->more() );
      }
    }
  }
  else
  {
    SMESH::array_of_ElementType_var types = theObject->GetTypes();
    const bool                    isNodes = ( types->length() == 1 && types[0] == SMESH::NODE );
    if ( isNodes && elemType != SMDSAbs_Node && elemType != SMDSAbs_All )
      return elemIt;
    SMDSAbs_ElementType iterType = isNodes ? SMDSAbs_Node : elemType;
    if ( SMESH_MeshEditor_i::IsTemporaryIDSource( theObject ))
    {
      SMESH::smIdType nbIds;
      if ( SMESH::smIdType* ids = SMESH_MeshEditor_i::GetTemporaryIDs( theObject, nbIds ))
        elemIt = SMDS_ElemIteratorPtr( new IDSourceIterator( meshDS, ids, nbIds, iterType ));
    }
    else
    {
      SMESH::smIdType_array_var ids = theObject->GetIDs();
      elemIt = SMDS_ElemIteratorPtr( new IDSourceIterator( meshDS, ids._retn(), iterType ));
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
 * \brief Internal structure used to find concurrent submeshes
 *
 * It represents a pair < submesh, concurrent dimension >, where
 * 'concurrent dimension' is dimension of shape where the submesh can concurrent
 *  with another submesh. In other words, it is dimension of a hypothesis assigned
 *  to submesh.
 */
class SMESH_DimHyp
{
 public:
  //! fields
  int _dim;    //!< a dimension the algo can build (concurrent dimension)
  int _ownDim; //!< dimension of shape of _subMesh (>=_dim)
  TopTools_MapOfShape  _shapeMap; //!< [sub-]shapes of dimension == _dim
  const SMESH_subMesh* _subMesh;
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
    _subMesh = theSubMesh;
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
      const TopoDS_Shape& aSubSh = anItr.Key();
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

    bool checkSubShape = isShareSubShapes( _shapeMap, theOther->_shapeMap, shapeTypeByDim(_dim));
    if ( !checkSubShape )
      return false;

    // check algorithms to be same
    const SMESH_Algo* a1 = this->GetAlgo();
    const SMESH_Algo* a2 = theOther->GetAlgo();
    bool isSame = checkAlgo( a1, a2 );
    if ( !isSame )
    {
      return true;
      // commented off for IPAL54678
      // if ( !a1 || !a2 )
      //   return false; // pb?
      // return a1->GetDim() == a2->GetDim(); // different algorithms of same dim -> concurrency !
    }

    // check hypothesises for concurrence (skip first as algorithm)
    size_t nbSame = 0;
    // pointers should be same, because it is referened from mesh hypothesis partition
    list <const SMESHDS_Hypothesis*>::const_iterator hypIt = _hypotheses.begin();
    list <const SMESHDS_Hypothesis*>::const_iterator otheEndIt = theOther->_hypotheses.end();
    for ( hypIt++ /*skip first as algo*/; hypIt != _hypotheses.end(); hypIt++ )
      if ( find( theOther->_hypotheses.begin(), otheEndIt, *hypIt ) != otheEndIt )
        nbSame++;
    // the submeshes are concurrent if their algorithms has different parameters
    return nbSame != theOther->_hypotheses.size() - 1;
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
  if ( !theAlgo->NeedDiscreteBoundary() &&
       theAlgo->NeedLowerHyps( theDim )) // IPAL54678
    return;
  TDimHypList& listOfdimHyp = theDimHypListArr[theDim];
  if ( listOfdimHyp.empty() || listOfdimHyp.back()->_subMesh != theSubMesh )
  {
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
bool unionLists(TListOfInt&       theListOfId,
                TListOfListOfInt& theListOfListOfId,
                const int         theIndx )
{
  bool changed = false;
  if ( theListOfId.empty() )
    return changed;
  TListOfListOfInt::iterator it = theListOfListOfId.begin();
  for ( int i = 0; it != theListOfListOfId.end(); it++, i++ )
  {
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
      {
        theListOfId.push_back(*it2);
        changed = true;
      }
    }
    // clear source list
    otherListOfId.clear();
  }
  return changed;
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
 * \brief Return sub-mesh objects list in meshing order
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

  bool changed;
  do {
    changed = false;
    TListOfListOfInt::iterator listIt = anOrder.begin();
    for ( int listIndx = 1; listIt != anOrder.end(); listIt++, listIndx++ )
      if ( unionLists( *listIt,  anOrder, listIndx ))
        changed = true;
  }
  while ( changed );

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

  // collect submeshes and detect concurrent algorithms and hypothesises
  TDimHypList dimHypListArr[4]; // dimHyp list for each shape dimension

  map<int, ::SMESH_subMesh*>::iterator i_sm = _mapSubMesh.begin();
  for ( ; i_sm != _mapSubMesh.end(); i_sm++ )
  {
    ::SMESH_subMesh* sm = (*i_sm).second;
    // shape of submesh
    const TopoDS_Shape& aSubMeshShape = sm->GetSubShape();

    // list of assigned hypothesises
    const list <const SMESHDS_Hypothesis*>& hypList = mesh.GetHypothesisList(aSubMeshShape);
    // Find out dimensions where the submesh can be concurrent.
    // We define the dimensions by algo of each of hypotheses in hypList
    list <const SMESHDS_Hypothesis*>::const_iterator hypIt = hypList.begin();
    for( ; hypIt != hypList.end(); hypIt++ )
    {
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
      // the submesh can concurrent at <dim> (or lower dims if !anAlgo->NeedDiscreteBoundary()
      // and !anAlgo->NeedLowerHyps( dim ))

      // create instance of dimension-hypothesis for found concurrent dimension(s) and algorithm
      for ( int j = anAlgo->NeedDiscreteBoundary() ? dim : 1, jn = dim; j <= jn; j++ )
        addDimHypInstance( j, aSubMeshShape, anAlgo, sm, hypList, dimHypListArr );
    }
  } // end iterations on submesh

    // iterate on created dimension-hypotheses and check for concurrents
  for ( int i = 0; i < 4; i++ )
  {
    const TDimHypList& listOfDimHyp = dimHypListArr[i];
    // check for concurrents in own and other dimensions (step-by-step)
    TDimHypList::const_iterator dhIt = listOfDimHyp.begin();
    for ( ; dhIt != listOfDimHyp.end(); dhIt++ )
    {
      const SMESH_DimHyp* dimHyp = *dhIt;
      TDimHypList listOfConcurr;
      set<int>    setOfConcurrIds;
      // looking for concurrents and collect into own list
      for ( int j = i; j < 4; j++ )
        findConcurrents( dimHyp, dimHypListArr[j], listOfConcurr, setOfConcurrIds );
      // check if any concurrents found
      if ( listOfConcurr.size() > 0 )
      {
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

  // now, minimize the number of concurrent groups
  // Here we assume that lists of submeshes can have same submesh
  // in case of multi-dimension algorithms, as result
  //  list with common submesh has to be united into one list
  int listIndx = 0;
  TListOfListOfInt::iterator listIt = anOrder.begin();
  for(; listIt != anOrder.end(); listIt++, listIndx++ )
    unionLists( *listIt,  anOrder, listIndx + 1 );

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

    // clear collected sub-meshes
    set<const SMESH_subMesh*>::iterator clrIt = subMeshToClear.begin();
    for ( ; clrIt != subMeshToClear.end(); clrIt++ )
      if ( SMESH_subMesh* sm = (SMESH_subMesh*)*clrIt )
      {
        sm->ComputeStateEngine( SMESH_subMesh::CLEAN );
        if ( SMESH_Algo* algo = sm->GetAlgo() ) // #16748
          sm->AlgoStateEngine( SMESH_subMesh::MODIF_HYP, algo ); // to clear a cached algo
      }
  }
  aPythonDump << " ])";

  mesh.SetMeshOrder( subMeshOrder );
  res = true;

  SMESH::SMESH_Mesh_var me = _this();
  _gen_i->UpdateIcons( me );

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
  for( ; it != theIdsOrder.end(); it++ )
  {
    // translate submesh identificators into submesh objects
    //  takeing into account real number of concurrent lists
    const TListOfInt& aSubOrder = (*it);
    if (!aSubOrder.size())
      continue;
    if ( theIsDump )
      aPythonDump << "[ ";
    // convert shape indices into interfaces
    SMESH::submesh_array_var aResSubSet = new SMESH::submesh_array();
    aResSubSet->length(aSubOrder.size());
    TListOfInt::const_iterator subIt = aSubOrder.begin();
    int j;
    for( j = 0; subIt != aSubOrder.end(); subIt++ )
    {
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

namespace // utils used by SMESH_MeshPartDS
{
  /*!
   * \brief Class used to access to protected data of SMDS_MeshInfo
   */
  struct TMeshInfo : public SMDS_MeshInfo
  {
    void Add(const SMDS_MeshElement* e) { SMDS_MeshInfo::addWithPoly( e ); }
  };
  /*!
   * \brief Element holing its ID only
   */
  struct TElemID : public SMDS_LinearEdge
  {
    TElemID(int ID) : SMDS_LinearEdge(0,0) { setID( ID ); }
  };
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

  mesh_i->Load();
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
    SMESH::smIdType_array_var       anIDs = meshPart->GetIDs();
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
const SMDS_MeshElement * SMESH_MeshPartDS::FindElement(int IDelem) const
{
  if ( _meshDS ) return _meshDS->FindElement( IDelem );

  TElemID elem( IDelem );
  for ( int iType = SMDSAbs_Edge; iType < SMDSAbs_NbElementTypes; ++iType )
    if ( !_elements[ iType ].empty() )
    {
      TIDSortedElemSet::const_iterator it = _elements[ iType ].find( &elem );
      if ( it != _elements[ iType ].end() )
        return *it;
    }
  return 0;
}
// -------------------------------------------------------------------------------------
bool SMESH_MeshPartDS::HasNumerationHoles()
{
  if ( _meshDS ) return _meshDS->HasNumerationHoles();

  return ( MinNodeID() != 1 ||
           MaxNodeID() != NbNodes() ||
           MinElementID() != 1 ||
           MaxElementID() != NbElements() );
}
// -------------------------------------------------------------------------------------
smIdType SMESH_MeshPartDS::MaxNodeID() const
{
  if ( _meshDS ) return _meshDS->MaxNodeID();
  return NbNodes() == 0 ? 0 : (*_elements[ SMDSAbs_Node ].rbegin())->GetID();
}
// -------------------------------------------------------------------------------------
smIdType SMESH_MeshPartDS::MinNodeID() const
{
  if ( _meshDS ) return _meshDS->MinNodeID();
  return NbNodes() == 0 ? 0 : (*_elements[ SMDSAbs_Node ].begin())->GetID();
}  
// -------------------------------------------------------------------------------------
smIdType SMESH_MeshPartDS::MaxElementID() const
{
  if ( _meshDS ) return _meshDS->MaxElementID();
  smIdType maxID = 0;
  for ( int iType = SMDSAbs_Edge; iType < SMDSAbs_NbElementTypes; ++iType )
    if ( !_elements[ iType ].empty() )
      maxID = std::max( maxID, (*_elements[ iType ].rbegin())->GetID() );
  return maxID;
}
// -------------------------------------------------------------------------------------
smIdType SMESH_MeshPartDS::MinElementID() const
{
  if ( _meshDS ) return _meshDS->MinElementID();
  smIdType minID = 0;
  for ( int iType = SMDSAbs_Edge; iType < SMDSAbs_NbElementTypes; ++iType )
    if ( !_elements[ iType ].empty() )
      minID = std::min( minID, (*_elements[ iType ].begin())->GetID() );
  return minID;
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

  SMDSAbs_ElementType type = SMDS_MeshCell::ElemType( geomType );

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

  SMDSAbs_ElementType type = SMDS_MeshCell::ElemType( entity );

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
  iterType SMESH_MeshPartDS::methName() const                 \
  {                                                                                 \
    typedef SMDS_SetIterator<const elem*, TIDSortedElemSet::const_iterator > TIter; \
    return _meshDS ? _meshDS->methName() : iterType                 \
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
