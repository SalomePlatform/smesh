//  SMESH SMESH : implementaion of SMESH idl descriptions
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
//  File   : SMESH_Mesh.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

#include "SMESH_Mesh.hxx"
#include "SMESH_subMesh.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_Hypothesis.hxx"
#include "SMESH_Group.hxx"
#include "SMESHDS_Group.hxx"
#include "SMESHDS_Script.hxx"
#include "SMDS_MeshVolume.hxx"

#include "utilities.h"

#include "Mesh_Writer.h"
#include "DriverMED_W_SMESHDS_Mesh.h"
#include "DriverDAT_W_SMESHDS_Mesh.h"
#include "DriverUNV_W_SMESHDS_Mesh.h"

#include "DriverMED_R_SMESHDS_Mesh.h"

#include <TCollection_AsciiString.hxx>
#include <memory>
#include <TopTools_ListOfShape.hxx>
#include <TopExp.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include "Utils_ExceptHandlers.hxx"


//=============================================================================
/*!
 * 
 */
//=============================================================================

SMESH_Mesh::SMESH_Mesh()
{
	MESSAGE("SMESH_Mesh::SMESH_Mesh");
	_id = -1;
	ASSERT(0);
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

SMESH_Mesh::SMESH_Mesh(int localId, int studyId, SMESH_Gen * gen, SMESHDS_Document * myDocument)
: _groupId( 0 )
{
	MESSAGE("SMESH_Mesh::SMESH_Mesh(int localId)");
	_id = localId;
	_studyId = studyId;
	_gen = gen;
	_myDocument = myDocument;
	_idDoc = _myDocument->NewMesh();
	_myMeshDS = _myDocument->GetMesh(_idDoc);
	_isShapeToMesh = false;
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

SMESH_Mesh::~SMESH_Mesh()
{
  MESSAGE("SMESH_Mesh::~SMESH_Mesh");

  // delete groups
  map < int, SMESH_Group * >::iterator itg;
  for (itg = _mapGroup.begin(); itg != _mapGroup.end(); itg++) {
    SMESH_Group *aGroup = (*itg).second;
    delete aGroup;
  }
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

void SMESH_Mesh::ShapeToMesh(const TopoDS_Shape & aShape){
	MESSAGE("SMESH_Mesh::ShapeToMesh");
	if (_isShapeToMesh)
		throw
			SALOME_Exception(LOCALIZED
		("a shape to mesh as already been defined"));
	_isShapeToMesh = true;
	_myMeshDS->ShapeToMesh(aShape);

	// NRI : 24/02/03
	TopExp::MapShapes(aShape, _subShapes);
}

//=======================================================================
//function : MEDToMesh
//purpose  : 
//=======================================================================

int SMESH_Mesh::MEDToMesh(const char* theFileName, const char* theMeshName)
{
  MESSAGE("MEDToMesh - theFileName = "<<theFileName<<", mesh name = "<<theMeshName);
  if(_isShapeToMesh)
    throw SALOME_Exception(LOCALIZED("a shape to mesh as already been defined"));
  _isShapeToMesh = true;
  std::auto_ptr<DriverMED_R_SMESHDS_Mesh> myReader(new DriverMED_R_SMESHDS_Mesh);
  myReader->SetMesh(_myMeshDS);
  myReader->SetMeshId(-1);
  myReader->SetFile(theFileName);
  myReader->SetMeshName(theMeshName);
  DriverMED_R_SMESHDS_Mesh::ReadStatus status = myReader->ReadMySelf();
  MESSAGE("MEDToMesh - _myMeshDS->NbNodes() = "<<_myMeshDS->NbNodes());
  MESSAGE("MEDToMesh - _myMeshDS->NbEdges() = "<<_myMeshDS->NbEdges());
  MESSAGE("MEDToMesh - _myMeshDS->NbFaces() = "<<_myMeshDS->NbFaces());
  MESSAGE("MEDToMesh - _myMeshDS->NbVolumes() = "<<_myMeshDS->NbVolumes());

  // Reading groups (sub-meshes are out of scope of MED import functionality)
  list<string> aGroupNames = myReader->GetGroupNames();
  MESSAGE("MEDToMesh - Nb groups = "<<aGroupNames.size()); 
  int anId;
  for ( list<string>::iterator it = aGroupNames.begin(); it != aGroupNames.end(); it++ ) {
    SMESH_Group* aGroup = AddGroup( SMDSAbs_All, it->c_str(), anId );
    if ( aGroup ) {
      MESSAGE("MEDToMesh - group added: "<<it->c_str());      
      SMESHDS_Group* aGroupDS = aGroup->GetGroupDS();
      aGroupDS->SetStoreName( it->c_str() );
      myReader->GetGroup( aGroupDS );
    }
  }
  return (int) status;
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

SMESH_Hypothesis::Hypothesis_Status
  SMESH_Mesh::AddHypothesis(const TopoDS_Shape & aSubShape,
                            int anHypId) throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
  MESSAGE("SMESH_Mesh::AddHypothesis");

  StudyContextStruct *sc = _gen->GetStudyContext(_studyId);
  if (sc->mapHypothesis.find(anHypId) == sc->mapHypothesis.end())
  {
    MESSAGE("Hypothesis ID does not give an hypothesis");
    SCRUTE(_studyId);
    SCRUTE(anHypId);
    throw SALOME_Exception(LOCALIZED("hypothesis does not exist"));
  }

  SMESH_subMesh *subMesh = GetSubMesh(aSubShape);
  SMESH_Hypothesis *anHyp = sc->mapHypothesis[anHypId];
  SCRUTE( anHyp->GetName() );
  int event;

  bool isGlobalHyp = IsMainShape( aSubShape );

  // NotConformAllowed can be only global
  if ( !isGlobalHyp )
  {
    string hypName = anHyp->GetName();
    if ( hypName == "NotConformAllowed" )
    {
      MESSAGE( "Hypotesis <NotConformAllowed> can be only global" );
      return SMESH_Hypothesis::HYP_INCOMPATIBLE;
    }
  }

  // shape 

  if (anHyp->GetType() == SMESHDS_Hypothesis::PARAM_ALGO)
    event = SMESH_subMesh::ADD_HYP;
  else
    event = SMESH_subMesh::ADD_ALGO;
  SMESH_Hypothesis::Hypothesis_Status ret = subMesh->AlgoStateEngine(event, anHyp);

  // subShapes
  if (!SMESH_Hypothesis::IsStatusFatal(ret) &&
      !subMesh->IsApplicableHypotesis( anHyp )) // is added on father
  {
    if (anHyp->GetType() == SMESHDS_Hypothesis::PARAM_ALGO)
      event = SMESH_subMesh::ADD_FATHER_HYP;
    else
      event = SMESH_subMesh::ADD_FATHER_ALGO;
    SMESH_Hypothesis::Hypothesis_Status ret2 =
      subMesh->SubMeshesAlgoStateEngine(event, anHyp);
    if (ret2 > ret)
      ret = ret2;
  }

  subMesh->DumpAlgoState(true);
  SCRUTE(ret);
  return ret;
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

SMESH_Hypothesis::Hypothesis_Status
  SMESH_Mesh::RemoveHypothesis(const TopoDS_Shape & aSubShape,
                               int anHypId)throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
	MESSAGE("SMESH_Mesh::RemoveHypothesis");

	StudyContextStruct *sc = _gen->GetStudyContext(_studyId);
	if (sc->mapHypothesis.find(anHypId) == sc->mapHypothesis.end())
		throw SALOME_Exception(LOCALIZED("hypothesis does not exist"));

	SMESH_subMesh *subMesh = GetSubMesh(aSubShape);
	SMESH_Hypothesis *anHyp = sc->mapHypothesis[anHypId];
	int hypType = anHyp->GetType();
	SCRUTE(hypType);
	int event;

	// shape 

	if (anHyp->GetType() == SMESHDS_Hypothesis::PARAM_ALGO)
		event = SMESH_subMesh::REMOVE_HYP;
	else
		event = SMESH_subMesh::REMOVE_ALGO;
	SMESH_Hypothesis::Hypothesis_Status ret = subMesh->AlgoStateEngine(event, anHyp);

	// subShapes
        if (!SMESH_Hypothesis::IsStatusFatal(ret) &&
            !subMesh->IsApplicableHypotesis( anHyp )) // is removed from father
	{
		if (anHyp->GetType() == SMESHDS_Hypothesis::PARAM_ALGO)
			event = SMESH_subMesh::REMOVE_FATHER_HYP;
		else
			event = SMESH_subMesh::REMOVE_FATHER_ALGO;
                SMESH_Hypothesis::Hypothesis_Status ret2 =
                  subMesh->SubMeshesAlgoStateEngine(event, anHyp);
                if (ret2 > ret) // more severe
                  ret = ret2;
	}

	subMesh->DumpAlgoState(true);
	SCRUTE(ret);
	return ret;
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

SMESHDS_Mesh * SMESH_Mesh::GetMeshDS()
{
	return _myMeshDS;
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

const list<const SMESHDS_Hypothesis*>&
	SMESH_Mesh::GetHypothesisList(const TopoDS_Shape & aSubShape) const
	throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
// 	MESSAGE("SMESH_Mesh::GetHypothesisList");
// 	_subShapeHypothesisList.clear();
// 	const list<const SMESHDS_Hypothesis*>& listHyp =
// 		_myMeshDS->GetHypothesis(aSubShape);

// 	list<const SMESHDS_Hypothesis*>::const_iterator it=listHyp.begin();
// 	while (it!=listHyp.end())
// 	{
// 		const SMESHDS_Hypothesis *anHyp = *it;
// 		_subShapeHypothesisList.push_back(anHyp);
// 		it++;
// 	}
// 	return _subShapeHypothesisList;
  return _myMeshDS->GetHypothesis(aSubShape);
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

const list<SMESHDS_Command*> & SMESH_Mesh::GetLog() throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
	MESSAGE("SMESH_Mesh::GetLog");
	return _myMeshDS->GetScript()->GetCommands();
}

//=============================================================================
/*!
 * 
 */
//=============================================================================
void SMESH_Mesh::ClearLog() throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
	MESSAGE("SMESH_Mesh::ClearLog");
	_myMeshDS->GetScript()->Clear();
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

int SMESH_Mesh::GetId()
{
	MESSAGE("SMESH_Mesh::GetId");
	return _id;
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

SMESH_Gen *SMESH_Mesh::GetGen()
{
	return _gen;
}

//=============================================================================
/*!
 * Get or Create the SMESH_subMesh object implementation
 */
//=============================================================================

SMESH_subMesh *SMESH_Mesh::GetSubMesh(const TopoDS_Shape & aSubShape)
throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
	//MESSAGE("SMESH_Mesh::GetSubMesh");
	SMESH_subMesh *aSubMesh;
	int index = _subShapes.FindIndex(aSubShape);

	if (_mapSubMesh.find(index) != _mapSubMesh.end())
	{
		aSubMesh = _mapSubMesh[index];
	}
	else
	{
		aSubMesh = new SMESH_subMesh(index, this, _myMeshDS, aSubShape);
		_mapSubMesh[index] = aSubMesh;
	}

	/* NRI 24/02/2003
	 * int index = -1;
	 * if (_subShapes.Contains(aSubShape))
	 * {
	 * index = _subShapes.FindIndex(aSubShape);
	 * ASSERT(_mapSubMesh.find(index) != _mapSubMesh.end());
	 * aSubMesh = _mapSubMesh[index];
	 * //MESSAGE("found submesh " << index);
	 * }
	 * else
	 * {
	 * index = _subShapes.Add(aSubShape);
	 * aSubMesh = new SMESH_subMesh(index, this, _myMeshDS, aSubShape);
	 * _mapSubMesh[index] = aSubMesh;
	 * //MESSAGE("created submesh " << index);
	 * }
	 */
	return aSubMesh;
}

//=============================================================================
/*!
 * Get the SMESH_subMesh object implementation. Dont create it, return null
 * if it does not exist.
 */
//=============================================================================
//
//  * Given a subShape, find if there is a subMesh associated to this subShape
//  * or to a collection of shapes containing this subShape. Collection =
//  * compsolid, shell, wire.
//  *
//  * WARNING : with arg = compsolid, shell or wire returns always NULL.
//  * with a face inside a shell, and submesh created for both, if arg is face,
//  * returns first created submesh of the two. 
//  * subMesh is not created, return may be NULL.

SMESH_subMesh *SMESH_Mesh::GetSubMeshContaining(const TopoDS_Shape & aSubShape)
throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
	//MESSAGE("SMESH_Mesh::GetSubMeshContaining");
	bool isFound = false;
	SMESH_subMesh *aSubMesh = NULL;

	int index = _subShapes.FindIndex(aSubShape);
	if (_mapSubMesh.find(index) != _mapSubMesh.end())
	{
		aSubMesh = _mapSubMesh[index];
		isFound = true;
	}

	/* NRI 24/02/2003
	 * int index = -1;
	 * if (_subShapes.Contains(aSubShape))
	 * {
	 * index = _subShapes.FindIndex(aSubShape);
	 * ASSERT(_mapSubMesh.find(index) != _mapSubMesh.end());
	 * aSubMesh = _mapSubMesh[index];
	 * isFound = true;
	 * //MESSAGE("found submesh " << index);
	 * }
	 */

//   map<int, SMESH_subMesh*>::iterator itsm;
//   for (itsm = _mapSubMesh.begin(); itsm != _mapSubMesh.end(); itsm++)
//     {
//       aSubMesh = (*itsm).second;
//       isFound = aSubMesh->Contains(aSubShape);
//       if (isFound) break;
//     }

	if (!isFound)
		aSubMesh = NULL;
	return aSubMesh;
}

//=======================================================================
//function : IsUsedHypothesis
//purpose  : Return True if anHyp is used to mesh aSubShape
//=======================================================================

bool SMESH_Mesh::IsUsedHypothesis(SMESHDS_Hypothesis * anHyp,
                                  const TopoDS_Shape & aSubShape)
{
  //MESSAGE( "SMESH_Mesh::IsUsedHypothesis" );

  // check if anHyp is applicable to aSubShape
  SMESH_subMesh * subMesh = GetSubMeshContaining( aSubShape );
  if (!subMesh ||
      !subMesh->IsApplicableHypotesis(static_cast<SMESH_Hypothesis*>(anHyp)))
    return false;

  SMESH_Algo *algo = _gen->GetAlgo(*this, aSubShape);

  // algorithm
  if (anHyp->GetType() > SMESHDS_Hypothesis::PARAM_ALGO)
    return ( anHyp == algo );

  // algorithm parameter
  if (algo)
  {
    // look trough hypotheses used by algo
    const list <const SMESHDS_Hypothesis * >&usedHyps =
      algo->GetUsedHypothesis(*this, aSubShape);
    list <const SMESHDS_Hypothesis * >::const_iterator itl;
    for (itl = usedHyps.begin(); itl != usedHyps.end(); itl++)
      if (anHyp == (*itl))
        return true;
  }
  else
  {
    // look through all assigned hypotheses
    {
      const list <const SMESHDS_Hypothesis * >&usedHyps =
        _myMeshDS->GetHypothesis( aSubShape );
      list <const SMESHDS_Hypothesis * >::const_iterator itl;
      for (itl = usedHyps.begin(); itl != usedHyps.end(); itl++)
        if (anHyp == (*itl))
          return true;
    }

    // on ancestors
    TopTools_ListIteratorOfListOfShape it( GetAncestors( aSubShape ));
    for (; it.More(); it.Next())
    {
      const list <const SMESHDS_Hypothesis * >&usedHyps =
        _myMeshDS->GetHypothesis( aSubShape );
      list <const SMESHDS_Hypothesis * >::const_iterator itl;
      for (itl = usedHyps.begin(); itl != usedHyps.end(); itl++)
        if (anHyp == (*itl))
          return true;
    }
  }
    
  return false;
}


//=============================================================================
/*!
 *
 */
//=============================================================================

const list < SMESH_subMesh * >&
	SMESH_Mesh::GetSubMeshUsingHypothesis(SMESHDS_Hypothesis * anHyp)
throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
	MESSAGE("SMESH_Mesh::GetSubMeshUsingHypothesis");
	map < int, SMESH_subMesh * >::iterator itsm;
	_subMeshesUsingHypothesisList.clear();
	for (itsm = _mapSubMesh.begin(); itsm != _mapSubMesh.end(); itsm++)
	{
		SMESH_subMesh *aSubMesh = (*itsm).second;
		if ( IsUsedHypothesis ( anHyp, aSubMesh->GetSubShape() ))
			_subMeshesUsingHypothesisList.push_back(aSubMesh);
	}
	return _subMeshesUsingHypothesisList;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

void SMESH_Mesh::ExportMED(const char *file, const char* theMeshName, bool theAutoGroups) throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
  std::auto_ptr<DriverMED_W_SMESHDS_Mesh> myWriter(new DriverMED_W_SMESHDS_Mesh);
  myWriter  ->SetFile    ( file        );
  myWriter  ->SetMesh    ( _myMeshDS   );
  if ( !theMeshName ) 
    myWriter->SetMeshId  ( _idDoc      );
  else {
    myWriter->SetMeshId  ( -1          );
    myWriter->SetMeshName( theMeshName );
  }

  if ( theAutoGroups ) {
    myWriter->AddGroupOfNodes();
    myWriter->AddGroupOfEdges();
    myWriter->AddGroupOfFaces();
    myWriter->AddGroupOfVolumes();
  }

  for ( map<int, SMESH_Group*>::iterator it = _mapGroup.begin(); it != _mapGroup.end(); it++ ) {
    SMESH_Group*     aGroup = it->second;
    SMESHDS_Group* aGroupDS = aGroup->GetGroupDS();
    aGroupDS->SetStoreName( aGroup->GetName() );
    myWriter->AddGroup( aGroupDS );
  }

  myWriter->Add();
}

void SMESH_Mesh::ExportDAT(const char *file) throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
	Mesh_Writer *myWriter = new DriverDAT_W_SMESHDS_Mesh;
	myWriter->SetFile(string(file));
	myWriter->SetMesh(_myMeshDS);
	myWriter->SetMeshId(_idDoc);
	myWriter->Add();
}

void SMESH_Mesh::ExportUNV(const char *file) throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
	Mesh_Writer *myWriter = new DriverUNV_W_SMESHDS_Mesh;
	myWriter->SetFile(string(file));
	myWriter->SetMesh(_myMeshDS);
	myWriter->SetMeshId(_idDoc);
	myWriter->Add();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
int SMESH_Mesh::NbNodes() throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
	return _myMeshDS->NbNodes();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
int SMESH_Mesh::NbEdges() throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
	return _myMeshDS->NbEdges();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
int SMESH_Mesh::NbFaces() throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
	return _myMeshDS->NbFaces();
}

///////////////////////////////////////////////////////////////////////////////
/// Return the number of 3 nodes faces in the mesh. This method run in O(n)
///////////////////////////////////////////////////////////////////////////////
int SMESH_Mesh::NbTriangles() throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
	int Nb = 0;

	SMDS_FaceIteratorPtr itFaces=_myMeshDS->facesIterator();
	while(itFaces->more()) if(itFaces->next()->NbNodes()==3) Nb++;
	return Nb;
}

///////////////////////////////////////////////////////////////////////////////
/// Return the number of 4 nodes faces in the mesh. This method run in O(n)
///////////////////////////////////////////////////////////////////////////////
int SMESH_Mesh::NbQuadrangles() throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
	int Nb = 0;

	SMDS_FaceIteratorPtr itFaces=_myMeshDS->facesIterator();
	while(itFaces->more()) if(itFaces->next()->NbNodes()==4) Nb++;
	return Nb;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
int SMESH_Mesh::NbVolumes() throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
	return _myMeshDS->NbVolumes();
}

int SMESH_Mesh::NbTetras() throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
	int Nb = 0;
	SMDS_VolumeIteratorPtr itVolumes=_myMeshDS->volumesIterator();
	while(itVolumes->more()) if(itVolumes->next()->NbNodes()==4) Nb++;
	return Nb;
}

int SMESH_Mesh::NbHexas() throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
	int Nb = 0;
	SMDS_VolumeIteratorPtr itVolumes=_myMeshDS->volumesIterator();
	while(itVolumes->more()) if(itVolumes->next()->NbNodes()==8) Nb++;
	return Nb;
}

int SMESH_Mesh::NbPyramids() throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
	int Nb = 0;
	SMDS_VolumeIteratorPtr itVolumes=_myMeshDS->volumesIterator();
	while(itVolumes->more()) if(itVolumes->next()->NbNodes()==5) Nb++;
	return Nb;
}

int SMESH_Mesh::NbPrisms() throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
	int Nb = 0;
	SMDS_VolumeIteratorPtr itVolumes=_myMeshDS->volumesIterator();
	while(itVolumes->more()) if(itVolumes->next()->NbNodes()==6) Nb++;
	return Nb;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
int SMESH_Mesh::NbSubMesh() throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
	return _myMeshDS->NbSubMesh();
}

//=======================================================================
//function : IsNotConformAllowed
//purpose  : check if a hypothesis alowing notconform mesh is present
//=======================================================================

bool SMESH_Mesh::IsNotConformAllowed() const
{
  MESSAGE("SMESH_Mesh::IsNotConformAllowed");

  const list<const SMESHDS_Hypothesis*>& listHyp =
    _myMeshDS->GetHypothesis( _myMeshDS->ShapeToMesh() );
  list<const SMESHDS_Hypothesis*>::const_iterator it=listHyp.begin();
  while (it!=listHyp.end())
  {
    const SMESHDS_Hypothesis *aHyp = *it;
    string hypName = aHyp->GetName();
    if ( hypName == "NotConformAllowed" )
      return true;
    it++;
  }
  return false;
}

//=======================================================================
//function : IsMainShape
//purpose  : 
//=======================================================================

bool SMESH_Mesh::IsMainShape(const TopoDS_Shape& theShape) const
{
  return theShape.IsSame(_myMeshDS->ShapeToMesh() );
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_Group* SMESH_Mesh::AddGroup (const SMDSAbs_ElementType theType,
                                   const char*               theName,
				   int&                      theId)
{
  if (_mapGroup.find(_groupId) != _mapGroup.end())
    return NULL;
  SMESH_Group* aGroup = new SMESH_Group (this, theType, theName);
  theId = _groupId;
  _mapGroup[_groupId++] = aGroup;
  return aGroup;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

SMESH_Group* SMESH_Mesh::GetGroup (const int theGroupID)
{
  if (_mapGroup.find(theGroupID) == _mapGroup.end())
    return NULL;
  return _mapGroup[theGroupID];
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

list<int> SMESH_Mesh::GetGroupIds()
{
  list<int> anIds;
  for ( map<int, SMESH_Group*>::const_iterator it = _mapGroup.begin(); it != _mapGroup.end(); it++ )
    anIds.push_back( it->first );
  
  return anIds;
}


//=============================================================================
/*!
 *  
 */
//=============================================================================

void SMESH_Mesh::RemoveGroup (const int theGroupID)
{
  if (_mapGroup.find(theGroupID) == _mapGroup.end())
    return;
  delete _mapGroup[theGroupID];
  _mapGroup.erase (theGroupID);
}

//=======================================================================
//function : GetAncestors
//purpose  : return list of ancestors of theSubShape in the order
//           that lower dimention shapes come first.
//=======================================================================

const TopTools_ListOfShape& SMESH_Mesh::GetAncestors(const TopoDS_Shape& theS)
{
  if ( _mapAncestors.IsEmpty() )
  {
    // fill _mapAncestors
    int desType, ancType;
    for ( desType = TopAbs_EDGE; desType > TopAbs_COMPOUND; desType-- )
      for ( ancType = desType - 1; ancType >= TopAbs_COMPOUND; ancType-- )
        TopExp::MapShapesAndAncestors (_myMeshDS->ShapeToMesh(),
                                       (TopAbs_ShapeEnum) desType,
                                       (TopAbs_ShapeEnum) ancType,
                                       _mapAncestors );
  }

  if ( _mapAncestors.Contains( theS ) )
    return _mapAncestors.FindFromKey( theS );

  static TopTools_ListOfShape emptyList;
  return emptyList;
}

//=======================================================================
//function : Dump
//purpose  : dumps contents of mesh to stream [ debug purposes ]
//=======================================================================
ostream& SMESH_Mesh::Dump(ostream& save)
{
  save << "========================== Dump contents of mesh ==========================" << endl;
  save << "1) Total number of nodes:     " << NbNodes() << endl;
  save << "2) Total number of edges:     " << NbEdges() << endl;
  save << "3) Total number of faces:     " << NbFaces() << endl;
  if ( NbFaces() > 0 ) {
    int nb3 = NbTriangles();
    int nb4 = NbQuadrangles();
    save << "3.1.) Number of triangles:    " << nb3 << endl;
    save << "3.2.) Number of quadrangles:  " << nb4 << endl;
    if ( nb3 + nb4 !=  NbFaces() ) {
      map<int,int> myFaceMap;
      SMDS_FaceIteratorPtr itFaces=_myMeshDS->facesIterator();
      while( itFaces->more( ) ) {
	int nbNodes = itFaces->next()->NbNodes();
	if ( myFaceMap.find( nbNodes ) == myFaceMap.end() )
	  myFaceMap[ nbNodes ] = 0;
	myFaceMap[ nbNodes ] = myFaceMap[ nbNodes ] + 1;
      }
      save << "3.3.) Faces in detail: " << endl;
      map <int,int>::iterator itF;
      for (itF = myFaceMap.begin(); itF != myFaceMap.end(); itF++)
	save << "--> nb nodes: " << itF->first << " - nb elemens: " << itF->second << endl;
    }
  }
  save << "4) Total number of volumes:   " << NbVolumes() << endl;
  if ( NbVolumes() > 0 ) {
    int nb8 = NbHexas();
    int nb4 = NbTetras();
    int nb5 = NbPyramids();
    int nb6 = NbPrisms();
    save << "4.1.) Number of hexahedrons:  " << nb8 << endl;
    save << "4.2.) Number of tetrahedrons: " << nb4 << endl;
    save << "4.3.) Number of prisms:       " << nb6 << endl;
    save << "4.4.) Number of pyramides:    " << nb5 << endl;
    if ( nb8 + nb4 + nb5 + nb6 != NbVolumes() ) {
      map<int,int> myVolumesMap;
      SMDS_VolumeIteratorPtr itVolumes=_myMeshDS->volumesIterator();
      while( itVolumes->more( ) ) {
	int nbNodes = itVolumes->next()->NbNodes();
	if ( myVolumesMap.find( nbNodes ) == myVolumesMap.end() )
	  myVolumesMap[ nbNodes ] = 0;
	myVolumesMap[ nbNodes ] = myVolumesMap[ nbNodes ] + 1;
      }
      save << "4.5.) Volumes in detail: " << endl;
      map <int,int>::iterator itV;
      for (itV = myVolumesMap.begin(); itV != myVolumesMap.end(); itV++)
	save << "--> nb nodes: " << itV->first << " - nb elemens: " << itV->second << endl;
    }
  }
  save << "===========================================================================" << endl;
  return save;
}
