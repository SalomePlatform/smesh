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

using namespace std;
#include "SMESH_Mesh_i.hxx"
#include "SMESH_subMesh_i.hxx"
#include "SMESH_MEDMesh_i.hxx"
#include "SMESH_Group_i.hxx"

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

#include <string>
#include <iostream>


//**** SMESHDS en champ

int SMESH_Mesh_i::myIdGenerator = 0;

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_Mesh_i::SMESH_Mesh_i()
     : SALOME::GenericObj_i( PortableServer::POA::_nil() )
{
	MESSAGE("SMESH_Mesh_i: default constructor, not for use");
	ASSERT(0);
}

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
  _gen_i = gen_i;
  _id = myIdGenerator++;
  _studyId = studyId;
  thePOA->activate_object( this );
}
/*
SMESH_Mesh_i::SMESH_Mesh_i(SMESH_Gen_i* gen_i,
			   CORBA::Long studyId, 
			   int localId)
{
  MESSAGE("SMESH_Mesh_i");
  _gen_i = gen_i;
  _id = localId;
  _studyId = studyId;
}
*/
//=============================================================================
/*!
 *  Destructor
 */
//=============================================================================

SMESH_Mesh_i::~SMESH_Mesh_i()
{
  MESSAGE("~SMESH_Mesh_i");
  map<int, SMESH::SMESH_Group_ptr>::iterator it;
  for ( it = _mapGroups.begin(); it != _mapGroups.end(); it++ ) {
    SMESH_Group_i* aGroup = dynamic_cast<SMESH_Group_i*>( SMESH_Gen_i::GetServant( it->second ).in() );
    if ( aGroup ) {
      _impl->RemoveGroup( aGroup->GetLocalID() );
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

void SMESH_Mesh_i::SetShape( GEOM::GEOM_Shape_ptr theShape )
    throw (SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  try {
    setShape( theShape );
  }
  catch(SALOME_Exception & S_ex) {
    THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
  }  

  SALOMEDS::Study_ptr aStudy = _gen_i->GetCurrentStudy();
  if ( aStudy->_is_nil() ) 
    return;

  // Create a reference to <theShape> 
  SALOMEDS::SObject_var aMeshSO  = SALOMEDS::SObject::_narrow( aStudy->FindObjectIOR( ( SMESH_Gen_i::GetORB()->object_to_string( _this() ) ) ) );
  SALOMEDS::SObject_var aShapeSO = aStudy->FindObjectIOR( SMESH_Gen_i::GetORB()->object_to_string( theShape ) );
  
  SALOMEDS::SObject_var          anObj, aRef;
  SALOMEDS::GenericAttribute_var anAttr;
  SALOMEDS::AttributeIOR_var     anIOR;
  SALOMEDS::StudyBuilder_var     aBuilder = aStudy->NewBuilder();
  long                           aTag = SMESH_Gen_i::GetRefOnShapeTag();      
  
  if ( aMeshSO->FindSubObject( aTag, anObj ) ) {
    if ( anObj->ReferencedObject( aRef ) ) {
      if ( strcmp( aRef->GetID(), aShapeSO->GetID() ) == 0 ) {
	// Setting the same shape twice forbidden
	return;
      }
    }
  }
  else {
    anObj = aBuilder->NewObjectToTag( aMeshSO, aTag );
  }
  aBuilder->Addreference( anObj, aShapeSO );
}

//=============================================================================
/*!
 *  setShape
 *
 *  Sets shape to the mesh implementation
 */
//=============================================================================

bool SMESH_Mesh_i::setShape( GEOM::GEOM_Shape_ptr theShape )
{
  TopoDS_Shape aLocShape  = _gen_i->GetShapeReader()->GetShape( SMESH_Gen_i::GetGeomEngine(), theShape );
  _impl->ShapeToMesh( aLocShape );
  return true;
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
  if ( aStudy->_is_nil() ) 
    return ConvertDriverMEDReadStatus(status);
  
  // publishing of the groups in the study (sub-meshes are out of scope of MED import)
  map<int, SMESH::SMESH_Group_ptr>::iterator it = _mapGroups.begin();
  for (; it != _mapGroups.end(); it++ ) {
    SMESH::SMESH_Group_var aGroup = SMESH::SMESH_Group::_duplicate( it->second );
    if ( _gen_i->CanPublishInStudy( aGroup ) )
      _gen_i->PublishInStudy( aStudy, 
			      SALOMEDS::SObject::_nil(),
			      aGroup,
			      aGroup->GetName() );
  }
  return ConvertDriverMEDReadStatus(status);
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
    SMESH::SMESH_Group_var aGroup = SMESH::SMESH_Group::_narrow( aGroupImpl->_this() );
    _mapGroups[*it]               = SMESH::SMESH_Group::_duplicate( aGroup );

    // register CORBA object for persistence
    StudyContext* myStudyContext = _gen_i->GetCurrentStudyContext();
    string iorString = SMESH_Gen_i::GetORB()->object_to_string( aGroup );
    int nextId = myStudyContext->addObject( iorString );
    MESSAGE( "Add group to map with id = "<< nextId << " and IOR = " << iorString.c_str() );
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

SMESH::Hypothesis_Status SMESH_Mesh_i::AddHypothesis(GEOM::GEOM_Shape_ptr aSubShape,
                                                     SMESH::SMESH_Hypothesis_ptr anHyp)
  throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  SMESH_Hypothesis::Hypothesis_Status status = addHypothesis( aSubShape, anHyp );

  if ( !SMESH_Hypothesis::IsStatusFatal(status) ) {
    SALOMEDS::Study_ptr aStudy = _gen_i->GetCurrentStudy();

    if ( !aStudy->_is_nil() ) {
      // Detect whether <aSubShape> refers to this mesh or its sub-mesh
      SALOMEDS::GenericAttribute_var anAttr;
      SALOMEDS::AttributeIOR_var     anIOR;
      SALOMEDS::SObject_var aMeshSO  = SALOMEDS::SObject::_narrow( aStudy->FindObjectIOR( ( SMESH_Gen_i::GetORB()->object_to_string( _this() ) ) ) );
      SALOMEDS::SObject_var aMorSM, aRef;
      CORBA::String_var aShapeIOR    = CORBA::string_dup( SMESH_Gen_i::GetORB()->object_to_string( aSubShape ) );
      SALOMEDS::ChildIterator_var it = aStudy->NewChildIterator( aMeshSO );

      for ( it->InitEx( true ); it->More(); it->Next() ) {
	SALOMEDS::SObject_var anObj = it->Value();
	if ( anObj->ReferencedObject( aRef ) ) {
	  if ( aRef->FindAttribute( anAttr, "AttributeIOR" ) ) {
	    anIOR = SALOMEDS::AttributeIOR::_narrow( anAttr );
	    if ( strcmp( anIOR->Value(), aShapeIOR ) == 0 ) {
	      aMorSM = anObj->GetFather();
	      break;
	    }
	  }
	}
      }

      bool aIsAlgo = !SMESH::SMESH_Algo::_narrow( anHyp )->_is_nil();
      SALOMEDS::SObject_var aHypSO  = SALOMEDS::SObject::_narrow( aStudy->FindObjectIOR( ( SMESH_Gen_i::GetORB()->object_to_string( anHyp   ) ) ) );
      if ( !aMorSM->_is_nil() && !aHypSO->_is_nil() ) {
	//Find or Create Applied Hypothesis root
	SALOMEDS::SObject_var             AHR;
	SALOMEDS::AttributeName_var       aName;
	SALOMEDS::AttributeSelectable_var aSelAttr;
	SALOMEDS::AttributePixMap_var     aPixmap;
	SALOMEDS::StudyBuilder_var        aBuilder = aStudy->NewBuilder();
	long                              aTag = aIsAlgo ? SMESH_Gen_i::GetRefOnAppliedAlgorithmsTag() : SMESH_Gen_i::GetRefOnAppliedHypothesisTag();

	if ( !aMorSM->FindSubObject( aTag, AHR ) ) {
	  AHR      = aBuilder->NewObjectToTag( aMorSM, aTag );
	  anAttr   = aBuilder->FindOrCreateAttribute( AHR, "AttributeName" );
	  aName    = SALOMEDS::AttributeName::_narrow( anAttr );
	  aName    ->SetValue( aIsAlgo ? "Applied algorithms" : "Applied hypotheses" );
	  anAttr   = aBuilder->FindOrCreateAttribute( AHR, "AttributeSelectable" );
	  aSelAttr = SALOMEDS::AttributeSelectable::_narrow( anAttr );
	  aSelAttr ->SetSelectable( false );
	  anAttr   = aBuilder->FindOrCreateAttribute( AHR, "AttributePixMap" );
	  aPixmap  = SALOMEDS::AttributePixMap::_narrow( anAttr );
	  aPixmap  ->SetPixMap( aIsAlgo ? "ICON_SMESH_TREE_ALGO" : "ICON_SMESH_TREE_HYPO" );
	}

	SALOMEDS::SObject_var SO = aBuilder->NewObject( AHR );
	aBuilder->Addreference( SO, aHypSO );
      }
    }
  }

  return ConvertHypothesisStatus(status);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_Hypothesis::Hypothesis_Status
  SMESH_Mesh_i::addHypothesis(GEOM::GEOM_Shape_ptr aSubShape,
                              SMESH::SMESH_Hypothesis_ptr anHyp)
{
	MESSAGE("addHypothesis");
	// **** proposer liste de subShape (selection multiple)

	GEOM::GEOM_Shape_var mySubShape = GEOM::GEOM_Shape::_narrow(aSubShape);
	if (CORBA::is_nil(mySubShape))
		THROW_SALOME_CORBA_EXCEPTION("bad subShape reference",
			SALOME::BAD_PARAM);

	SMESH::SMESH_Hypothesis_var myHyp = SMESH::SMESH_Hypothesis::_narrow(anHyp);
	if (CORBA::is_nil(myHyp))
		THROW_SALOME_CORBA_EXCEPTION("bad hypothesis reference",
			SALOME::BAD_PARAM);

	SMESH_Hypothesis::Hypothesis_Status status = SMESH_Hypothesis::HYP_OK;
	try
	{
		TopoDS_Shape myLocSubShape =
			_gen_i->GetShapeReader()->GetShape(SMESH_Gen_i::GetGeomEngine(), mySubShape);
		int hypId = myHyp->GetId();
		status = _impl->AddHypothesis(myLocSubShape, hypId);
                if ( !SMESH_Hypothesis::IsStatusFatal(status) )
                  _mapHypo[hypId] = myHyp;
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

SMESH::Hypothesis_Status SMESH_Mesh_i::RemoveHypothesis(GEOM::GEOM_Shape_ptr aSubShape,
                                                        SMESH::SMESH_Hypothesis_ptr anHyp)
     throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  SMESH_Hypothesis::Hypothesis_Status status = removeHypothesis( aSubShape, anHyp );

  if ( !SMESH_Hypothesis::IsStatusFatal(status) ) {
    SALOMEDS::Study_ptr aStudy = _gen_i->GetCurrentStudy();

    if ( !aStudy->_is_nil() ) {
      // Detect whether <aSubShape> refers to this mesh or its sub-mesh
      SALOMEDS::GenericAttribute_var anAttr;
      SALOMEDS::AttributeIOR_var     anIOR;
      SALOMEDS::SObject_var aMeshSO  = SALOMEDS::SObject::_narrow( aStudy->FindObjectIOR( ( SMESH_Gen_i::GetORB()->object_to_string( _this() ) ) ) );
      if ( aMeshSO->_is_nil() )
	return SMESH::HYP_UNKNOWN_FATAL;

      SALOMEDS::SObject_var aMorSM, aRef;
      CORBA::String_var aShapeIOR    = CORBA::string_dup( SMESH_Gen_i::GetORB()->object_to_string( aSubShape ) );
      SALOMEDS::ChildIterator_var it = aStudy->NewChildIterator( aMeshSO );

      for ( it->InitEx( true ); it->More(); it->Next() ) {
	SALOMEDS::SObject_var anObj = it->Value();
	if ( anObj->ReferencedObject( aRef ) ) {
	  if ( aRef->FindAttribute( anAttr, "AttributeIOR" ) ) {
	    anIOR = SALOMEDS::AttributeIOR::_narrow( anAttr );
	    if ( strcmp( anIOR->Value(), aShapeIOR ) == 0 ) {
	      aMorSM = anObj->GetFather();
	      break;
	    }
	  }
	}
      }

      bool aIsAlgo = !SMESH::SMESH_Algo::_narrow( anHyp )->_is_nil();
      SALOMEDS::SObject_var aHypSO  = SALOMEDS::SObject::_narrow( aStudy->FindObjectIOR( ( SMESH_Gen_i::GetORB()->object_to_string( anHyp ) ) ) );
      if ( !aMorSM->_is_nil() && !aHypSO->_is_nil() ) {
	// Remove a refernce to hypothesis or algorithm
	SALOMEDS::SObject_var             AHR;
	SALOMEDS::AttributeName_var       aName;
	SALOMEDS::AttributeSelectable_var aSelAttr;
	SALOMEDS::AttributePixMap_var     aPixmap;
	SALOMEDS::StudyBuilder_var        aBuilder = aStudy->NewBuilder();
	CORBA::String_var                 aHypIOR  = CORBA::string_dup( SMESH_Gen_i::GetORB()->object_to_string( anHyp ) );
	long                              aTag     = aIsAlgo ? SMESH_Gen_i::GetRefOnAppliedAlgorithmsTag() : SMESH_Gen_i::GetRefOnAppliedHypothesisTag();

	if ( aMorSM->FindSubObject( aTag, AHR ) ) {
	  SALOMEDS::ChildIterator_var it = aStudy->NewChildIterator( AHR );
	  for ( ; it->More(); it->Next() ) {
	    SALOMEDS::SObject_var anObj = it->Value();
	    if ( anObj->ReferencedObject( aRef ) ) {
	      if ( aRef->FindAttribute( anAttr, "AttributeIOR" ) ) {
		anIOR = SALOMEDS::AttributeIOR::_narrow(anAttr);
		if ( strcmp( anIOR->Value(), aHypIOR ) == 0 ) {
		  aBuilder->RemoveObject( anObj );
		  break;
		}
	      }
	    }
	  }
	}
      }
    }
  }

  return ConvertHypothesisStatus(status);
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_Hypothesis::Hypothesis_Status
  SMESH_Mesh_i::removeHypothesis(GEOM::GEOM_Shape_ptr aSubShape,
                                 SMESH::SMESH_Hypothesis_ptr anHyp)
{
	MESSAGE("removeHypothesis()");
	// **** proposer liste de subShape (selection multiple)

	GEOM::GEOM_Shape_var mySubShape = GEOM::GEOM_Shape::_narrow(aSubShape);
	if (CORBA::is_nil(mySubShape))
		THROW_SALOME_CORBA_EXCEPTION("bad subShape reference",
			SALOME::BAD_PARAM);

	SMESH::SMESH_Hypothesis_var myHyp = SMESH::SMESH_Hypothesis::_narrow(anHyp);
	if (CORBA::is_nil(myHyp))
	  THROW_SALOME_CORBA_EXCEPTION("bad hypothesis reference",
			SALOME::BAD_PARAM);

	SMESH_Hypothesis::Hypothesis_Status status = SMESH_Hypothesis::HYP_OK;
	try
	{
		TopoDS_Shape myLocSubShape =
			_gen_i->GetShapeReader()->GetShape(SMESH_Gen_i::GetGeomEngine(), mySubShape);
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
	SMESH_Mesh_i::GetHypothesisList(GEOM::GEOM_Shape_ptr aSubShape)
throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  MESSAGE("GetHypothesisList");
  GEOM::GEOM_Shape_var mySubShape = GEOM::GEOM_Shape::_narrow(aSubShape);
  if (CORBA::is_nil(mySubShape))
    THROW_SALOME_CORBA_EXCEPTION("bad subShape reference",
				 SALOME::BAD_PARAM);
  
  SMESH::ListOfHypothesis_var aList = new SMESH::ListOfHypothesis();

  try {
    TopoDS_Shape myLocSubShape
      = _gen_i->GetShapeReader()->GetShape(SMESH_Gen_i::GetGeomEngine(), mySubShape);
    
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
SMESH::SMESH_subMesh_ptr SMESH_Mesh_i::GetSubMesh(GEOM::GEOM_Shape_ptr aSubShape,
						  const char*          theName ) 
     throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  MESSAGE("SMESH_Mesh_i::GetElementsOnShape");
  GEOM::GEOM_Shape_var mySubShape = GEOM::GEOM_Shape::_narrow(aSubShape);
  if (CORBA::is_nil(mySubShape))
    THROW_SALOME_CORBA_EXCEPTION("bad subShape reference",
				 SALOME::BAD_PARAM);
  
  int subMeshId = 0;
  try {
    TopoDS_Shape myLocSubShape
      = _gen_i->GetShapeReader()->GetShape(SMESH_Gen_i::GetGeomEngine(), mySubShape);
    
    //Get or Create the SMESH_subMesh object implementation
    
    ::SMESH_subMesh * mySubMesh = _impl->GetSubMesh(myLocSubShape);
    subMeshId = mySubMesh->GetId();
    
    // create a new subMesh object servant if there is none for the shape
    
    if (_mapSubMesh.find(subMeshId) == _mapSubMesh.end()) {
      SMESH::SMESH_subMesh_var subMesh = createSubMesh( aSubShape );
      if ( _gen_i->CanPublishInStudy( subMesh ) ) {
	SALOMEDS::SObject_var aSubmeshSO = _gen_i->PublishInStudy( _gen_i->GetCurrentStudy(), 
								   SALOMEDS::SObject::_nil(),
								   subMesh,
								   theName );
	  
	// Add reference to <aSubShape> to the study
	SALOMEDS::Study_var aStudy = _gen_i->GetCurrentStudy();
	SALOMEDS::SObject_var aShapeSO = aStudy->FindObjectIOR( SMESH_Gen_i::GetORB()->object_to_string( aSubShape ) );
	if ( !aSubmeshSO->_is_nil() && !aShapeSO->_is_nil() ) {
	  MESSAGE( "********** SMESH_Mesh_i::GetSubMesh(): adding shape reference..." )
	  SALOMEDS::StudyBuilder_var aBuilder = aStudy->NewBuilder();
	  SALOMEDS::SObject_var SO = aBuilder->NewObjectToTag( aSubmeshSO, SMESH_Gen_i::GetRefOnShapeTag() );
	  aBuilder->Addreference( SO, aShapeSO );
	  MESSAGE( "********** SMESH_Mesh_i::GetSubMesh(): shape reference added" )
	}
      }
    }
  }
  catch(SALOME_Exception & S_ex) {
    THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
  }
    
  ASSERT(_mapSubMeshIor.find(subMeshId) != _mapSubMeshIor.end());
  return SMESH::SMESH_subMesh::_duplicate(_mapSubMeshIor[subMeshId]);
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_Mesh_i::RemoveSubMesh( SMESH::SMESH_subMesh_ptr theSubMesh )
     throw (SALOME::SALOME_Exception)
{
  MESSAGE("SMESH_Mesh_i::RemoveSubMesh");
  if ( theSubMesh->_is_nil() )
    return;

  GEOM::GEOM_Shape_var aSubShape;
  SALOMEDS::Study_ptr aStudy = _gen_i->GetCurrentStudy();
  if ( !aStudy->_is_nil() )  {
    // Remove submesh's SObject
    SALOMEDS::SObject_var anSO = SALOMEDS::SObject::_narrow( aStudy->FindObjectIOR( ( SMESH_Gen_i::GetORB()->object_to_string( theSubMesh ) ) ) );
    if ( !anSO->_is_nil() ) {
      long aTag = SMESH_Gen_i::GetRefOnShapeTag(); 
      SALOMEDS::SObject_var anObj, aRef;
      if ( anSO->FindSubObject( aTag, anObj ) && anObj->ReferencedObject( aRef ) )
	aSubShape = GEOM::GEOM_Shape::_narrow( aRef->GetObject() );

      aStudy->NewBuilder()->RemoveObjectWithChildren( anSO );
    }
  }

  removeSubMesh( theSubMesh, aSubShape.in() );
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
  SMESH::SMESH_Group_var aNewGroup = createGroup( theElemType, theName );

  // Groups should be put under separate roots according to their type (nodes, edges, faces, volumes)
  if ( _gen_i->CanPublishInStudy( aNewGroup ) ) {
    SALOMEDS::SObject_var aGroupSO = _gen_i->PublishInStudy( _gen_i->GetCurrentStudy(), 
							     SALOMEDS::SObject::_nil(),
							     aNewGroup,
							     theName );
  }

  return aNewGroup._retn();
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_Mesh_i::RemoveGroup( SMESH::SMESH_Group_ptr theGroup )
    throw (SALOME::SALOME_Exception)
{
  if ( theGroup->_is_nil() )
    return;

  SMESH_Group_i* aGroup = dynamic_cast<SMESH_Group_i*>( SMESH_Gen_i::GetServant( theGroup ).in() );
  if ( !aGroup )
    return;

  SALOMEDS::Study_ptr aStudy = _gen_i->GetCurrentStudy();
  if ( !aStudy->_is_nil() )  {
    // Remove group's SObject
    SALOMEDS::SObject_var aGroupSO = SALOMEDS::SObject::_narrow( aStudy->FindObjectIOR( ( SMESH_Gen_i::GetORB()->object_to_string( theGroup ) ) ) );
    if ( !aGroupSO->_is_nil() )
      aStudy->NewBuilder()->RemoveObject( aGroupSO );
  }

  // Remove the group from SMESH data structures
  removeGroup( aGroup->GetLocalID() );
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH::SMESH_subMesh_ptr SMESH_Mesh_i::createSubMesh( GEOM::GEOM_Shape_ptr theSubShape ) {
 
  TopoDS_Shape myLocSubShape = _gen_i->GetShapeReader()->GetShape(SMESH_Gen_i::GetGeomEngine(), theSubShape);

  ::SMESH_subMesh * mySubMesh = _impl->GetSubMesh(myLocSubShape);
  int subMeshId = mySubMesh->GetId();
  SMESH_subMesh_i *subMeshServant = new SMESH_subMesh_i(myPOA, _gen_i, this, subMeshId);
  SMESH::SMESH_subMesh_var subMesh
    = SMESH::SMESH_subMesh::_narrow(subMeshServant->_this());

  _mapSubMesh[subMeshId] = mySubMesh;
  _mapSubMesh_i[subMeshId] = subMeshServant;
  _mapSubMeshIor[subMeshId]
    = SMESH::SMESH_subMesh::_duplicate(subMesh);

  // register CORBA object for persistence
  StudyContext* myStudyContext = _gen_i->GetCurrentStudyContext();
  string iorString = SMESH_Gen_i::GetORB()->object_to_string( subMesh );
  int nextId = myStudyContext->addObject( iorString );
  MESSAGE( "Add submesh to map with id = "<< nextId << " and IOR = " << iorString.c_str() );

  return subMesh._retn(); 
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_Mesh_i::removeSubMesh( SMESH::SMESH_subMesh_ptr theSubMesh, GEOM::GEOM_Shape_ptr theSubShape )
{
  MESSAGE("SMESH_Mesh_i::removeSubMesh()");
  if ( theSubMesh->_is_nil() || theSubShape->_is_nil() )
    return;

  try {
    SMESH::ListOfHypothesis_var aHypList = GetHypothesisList( theSubShape );
    for ( int i = 0, n = aHypList->length(); i < n; i++ ) {
      removeHypothesis( theSubShape, aHypList[i] );
    }
  }
  catch( const SALOME::SALOME_Exception& ) {
    MESSAGE("SMESH_Mesh_i::removeSubMesh(): exception caught!");
  }

  int subMeshId = theSubMesh->GetId();

  _mapSubMesh.erase(subMeshId);
  _mapSubMesh_i.erase(subMeshId);
  _mapSubMeshIor.erase(subMeshId);
  MESSAGE("SMESH_Mesh_i::removeSubMesh() completed");
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH::SMESH_Group_ptr SMESH_Mesh_i::createGroup( SMESH::ElementType theElemType, const char* theName )
{
  int anId;
  SMESH::SMESH_Group_var aGroup;
  if ( _impl->AddGroup( (SMDSAbs_ElementType)theElemType, theName, anId ) ) {
    SMESH_Group_i* aGroupImpl = new SMESH_Group_i( SMESH_Gen_i::GetPOA(), this, anId );
    aGroup = SMESH::SMESH_Group::_narrow( aGroupImpl->_this() );
    _mapGroups[anId] = SMESH::SMESH_Group::_duplicate( aGroup );

    // register CORBA object for persistence
    StudyContext* myStudyContext = _gen_i->GetCurrentStudyContext();
    string iorString = SMESH_Gen_i::GetORB()->object_to_string( aGroup );
    int nextId = myStudyContext->addObject( iorString );
    MESSAGE( "Add group to map with id = "<< nextId << " and IOR = " << iorString.c_str() );
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
  MESSAGE("SMESH_Mesh_i::removeGroup()" );  
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
	MESSAGE("SMESH_Mesh_i::GetLog");

	SMESH::log_array_var aLog;
	try
	{
		list < SMESHDS_Command * >logDS = _impl->GetLog();
		aLog = new SMESH::log_array;
		int indexLog = 0;
		int lg = logDS.size();
		SCRUTE(lg);
		aLog->length(lg);
		list < SMESHDS_Command * >::iterator its = logDS.begin();
		while (its != logDS.end())
		{
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
			for (int i = 0; i < rnum; i++)
			{
				aLog[indexLog].coords[i] = *ir;
				//MESSAGE(" "<<i<<" "<<ir.Value());
				ir++;
			}
			for (int i = 0; i < inum; i++)
			{
				aLog[indexLog].indexes[i] = *ii;
				//MESSAGE(" "<<i<<" "<<ii.Value());
				ii++;
			}
			indexLog++;
			its++;
		}
		if (clearAfterGet)
			_impl->ClearLog();
	}
	catch(SALOME_Exception & S_ex)
	{
		THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
	}
	return aLog._retn();
}

//  SMESH::string_array* SMESH_Mesh_i::GetLog(CORBA::Boolean clearAfterGet)
//    throw (SALOME::SALOME_Exception)
//  {
//    MESSAGE("SMESH_Mesh_i::GetLog");

//    SMESH::string_array_var aLog;
//    try
//      {
//        const SMESHDS_ListOfCommand& logDS =_impl->GetLog();
//        aLog = new SMESH::string_array;
//        int logSize = 0;
//        int indexLog = 0;
//        int lg = logDS.Extent();
//        SCRUTE(lg);
//        SMESHDS_ListIteratorOfListOfCommand its(logDS);
//        while(its.More())
//        {
//      Handle(SMESHDS_Command) com = its.Value();
//      int comType = com->GetType();
//      SCRUTE(comType);
//      int lgcom = com->GetNumber();
//      SCRUTE(lgcom);
//      logSize += lgcom;
//      aLog->length(logSize);
//      SCRUTE(logSize);
//      const TColStd_ListOfInteger& intList = com->GetIndexes();
//      TColStd_ListIteratorOfListOfInteger ii(intList);
//      const TColStd_ListOfReal& coordList = com->GetCoords();
//      TColStd_ListIteratorOfListOfReal ir(coordList);
//      for (int icom = 1; icom <= lgcom; icom++)
//        {
//          ostringstream S;
//          switch (comType)
//            {
//            case SMESHDS_AddNode:
//          S << "AddNode " << ii.Value(); ii.Next();
//          S << " " << ir.Value(); ir.Next(); 
//          S << " " << ir.Value(); ir.Next();
//          S << " " << ir.Value(); ir.Next();
//          break;
//            case SMESHDS_AddEdge:
//          S << "AddEdge " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          break;
//            case SMESHDS_AddTriangle:
//          S << "AddFace " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          break;
//            case SMESHDS_AddQuadrangle:
//          S << "AddFace " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          break;
//            case SMESHDS_AddTetrahedron:
//          S << "AddVolume " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          break;
//            case SMESHDS_AddPyramid:
//          S << "AddVolume " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          break;
//            case SMESHDS_AddPrism:
//          S << "AddVolume " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          break;
//            case SMESHDS_AddHexahedron:
//          S << "AddVolume " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          S << " " << ii.Value(); ii.Next();
//          break;
//            case SMESHDS_RemoveNode:
//          S << "RemoveNode " << ii.Value(); ii.Next();
//          break;
//            case SMESHDS_RemoveElement:
//          S << "RemoveElement " << ii.Value(); ii.Next();
//          break;
//            default:
//          ASSERT(0);
//          break;
//            }
//          string ch = S.str();
//          SCRUTE(ch);
//          aLog[indexLog++] = CORBA::string_dup(ch.c_str());
//        }
//      its.Next();
//        }
//        if (clearAfterGet) _impl->ClearLog();
//      }
//    catch (SALOME_Exception& S_ex)
//      {
//        THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
//      }
//    return aLog._retn();
//  }

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_Mesh_i::ClearLog() throw(SALOME::SALOME_Exception)
{
	MESSAGE("SMESH_Mesh_i::ClearLog");
	// ****
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

CORBA::Long SMESH_Mesh_i::GetId()throw(SALOME::SALOME_Exception)
{
	MESSAGE("SMESH_Mesh_i::GetId");
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
	MESSAGE("SMESH_Mesh_i::SetImpl");
	_impl = impl;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

::SMESH_Mesh & SMESH_Mesh_i::GetImpl()
{
	MESSAGE("SMESH_Mesh_i::GetImpl()");
	return *_impl;
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH::SMESH_MeshEditor_ptr SMESH_Mesh_i::GetMeshEditor()
{
	SMESH_MeshEditor_i *aMeshEditor =
		new SMESH_MeshEditor_i(_impl->GetMeshDS());
	SMESH::SMESH_MeshEditor_var aMesh = aMeshEditor->_this();
	return aMesh._retn();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_Mesh_i::ExportMED(const char *file, CORBA::Boolean auto_groups) throw(SALOME::SALOME_Exception)
{
  Unexpect aCatch(SALOME_SalomeException);
  SALOMEDS::Study_ptr aStudy = _gen_i->GetCurrentStudy();
  if ( aStudy->_is_nil() ) 
    return;

  char* aMeshName = NULL;
  SALOMEDS::SObject_var aMeshSO = SALOMEDS::SObject::_narrow( aStudy->FindObjectIOR( ( SMESH_Gen_i::GetORB()->object_to_string( _this() ) ) ) );
  if ( !aMeshSO->_is_nil() )
    {
      aMeshName = aMeshSO->GetName();
      //SCRUTE(file);
      //SCRUTE(aMeshName);
      //SCRUTE(aMeshSO->GetID());
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
  _impl->ExportMED( file, aMeshName, auto_groups );
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
