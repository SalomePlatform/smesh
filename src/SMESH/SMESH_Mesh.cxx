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
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
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
#include "SMESH_HypoFilter.hxx"
#include "SMESHDS_Group.hxx"
#include "SMESHDS_Script.hxx"
#include "SMESHDS_GroupOnGeom.hxx"
#include "SMDS_MeshVolume.hxx"

#include "utilities.h"

#include "DriverMED_W_SMESHDS_Mesh.h"
#include "DriverDAT_W_SMDS_Mesh.h"
#include "DriverUNV_W_SMDS_Mesh.h"
#include "DriverSTL_W_SMDS_Mesh.h"

#include "DriverMED_R_SMESHDS_Mesh.h"
#include "DriverUNV_R_SMDS_Mesh.h"
#include "DriverSTL_R_SMDS_Mesh.h"

#include <BRepTools_WireExplorer.hxx>
#include <BRep_Builder.hxx>
#include <gp_Pnt.hxx>

#include <TCollection_AsciiString.hxx>
#include <TopExp.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_Array1OfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_MapOfShape.hxx>

#include <memory>

#include "Utils_ExceptHandlers.hxx"

// maximum stored group name length in MED file
#define MAX_MED_GROUP_NAME_LENGTH 80

#ifdef _DEBUG_
static int MYDEBUG = 0;
#else
static int MYDEBUG = 0;
#endif


//=============================================================================
/*!
 * 
 */
//=============================================================================

SMESH_Mesh::SMESH_Mesh(int theLocalId, 
		       int theStudyId, 
		       SMESH_Gen* theGen,
		       bool theIsEmbeddedMode,
		       SMESHDS_Document* theDocument):
  _groupId( 0 )
{
  INFOS("SMESH_Mesh::SMESH_Mesh(int localId)");
  _id = theLocalId;
  _studyId = theStudyId;
  _gen = theGen;
  _myDocument = theDocument;
  _idDoc = theDocument->NewMesh(theIsEmbeddedMode);
  _myMeshDS = theDocument->GetMesh(_idDoc);
  _isShapeToMesh = false;
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

SMESH_Mesh::~SMESH_Mesh()
{
  INFOS("SMESH_Mesh::~SMESH_Mesh");

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

void SMESH_Mesh::ShapeToMesh(const TopoDS_Shape & aShape)
{
  if(MYDEBUG) MESSAGE("SMESH_Mesh::ShapeToMesh");

  if ( !_myMeshDS->ShapeToMesh().IsNull() && aShape.IsNull() )
  {
    // removal of a shape to mesh, delete objects referring to sub-shapes:
    // - sub-meshes
    map <int, SMESH_subMesh *>::iterator i_sm = _mapSubMesh.begin();
    for ( ; i_sm != _mapSubMesh.end(); ++i_sm )
      delete i_sm->second;
    _mapSubMesh.clear();
    //  - groups on geometry
    map <int, SMESH_Group *>::iterator i_gr = _mapGroup.begin();
    while ( i_gr != _mapGroup.end() ) {
      if ( dynamic_cast<SMESHDS_GroupOnGeom*>( i_gr->second->GetGroupDS() )) {
        _myMeshDS->RemoveGroup( i_gr->second->GetGroupDS() );
        delete i_gr->second;
        _mapGroup.erase( i_gr++ );
      }
      else
        i_gr++;
    }
    _mapPropagationChains.Clear();
  }
  else
  {
    if (_isShapeToMesh)
      throw SALOME_Exception(LOCALIZED ("a shape to mesh has already been defined"));
  }
  _isShapeToMesh = true;
  _myMeshDS->ShapeToMesh(aShape);

  // fill _mapAncestors
  _mapAncestors.Clear();
  int desType, ancType;
  for ( desType = TopAbs_VERTEX; desType > TopAbs_COMPOUND; desType-- )
    for ( ancType = desType - 1; ancType >= TopAbs_COMPOUND; ancType-- )
      TopExp::MapShapesAndAncestors ( aShape,
                                     (TopAbs_ShapeEnum) desType,
                                     (TopAbs_ShapeEnum) ancType,
                                     _mapAncestors );

  // NRI : 24/02/03
  //EAP: 1/9/04 TopExp::MapShapes(aShape, _subShapes); USE the same map of _myMeshDS
}

//=======================================================================
//function : UNVToMesh
//purpose  : 
//=======================================================================

int SMESH_Mesh::UNVToMesh(const char* theFileName)
{
  if(MYDEBUG) MESSAGE("UNVToMesh - theFileName = "<<theFileName);
  if(_isShapeToMesh)
    throw SALOME_Exception(LOCALIZED("a shape to mesh has already been defined"));
  _isShapeToMesh = true;
  DriverUNV_R_SMDS_Mesh myReader;
  myReader.SetMesh(_myMeshDS);
  myReader.SetFile(theFileName);
  myReader.SetMeshId(-1);
  myReader.Perform();
  if(MYDEBUG){
    MESSAGE("UNVToMesh - _myMeshDS->NbNodes() = "<<_myMeshDS->NbNodes());
    MESSAGE("UNVToMesh - _myMeshDS->NbEdges() = "<<_myMeshDS->NbEdges());
    MESSAGE("UNVToMesh - _myMeshDS->NbFaces() = "<<_myMeshDS->NbFaces());
    MESSAGE("UNVToMesh - _myMeshDS->NbVolumes() = "<<_myMeshDS->NbVolumes());
  }
  SMDS_MeshGroup* aGroup = (SMDS_MeshGroup*) myReader.GetGroup();
  if (aGroup != 0) {
    TGroupNamesMap aGroupNames = myReader.GetGroupNamesMap();
    //const TGroupIdMap& aGroupId = myReader.GetGroupIdMap();
    aGroup->InitSubGroupsIterator();
    while (aGroup->MoreSubGroups()) {
      SMDS_MeshGroup* aSubGroup = (SMDS_MeshGroup*) aGroup->NextSubGroup();
      std::string aName = aGroupNames[aSubGroup];
      int aId;

      SMESH_Group* aSMESHGroup = AddGroup( aSubGroup->GetType(), aName.c_str(), aId );
      if ( aSMESHGroup ) {
	if(MYDEBUG) MESSAGE("UNVToMesh - group added: "<<aName);      
	SMESHDS_Group* aGroupDS = dynamic_cast<SMESHDS_Group*>( aSMESHGroup->GetGroupDS() );
	if ( aGroupDS ) {
	  aGroupDS->SetStoreName(aName.c_str());
	  aSubGroup->InitIterator();
	  const SMDS_MeshElement* aElement = 0;
	  while (aSubGroup->More()) {
	    aElement = aSubGroup->Next();
	    if (aElement) {
	      aGroupDS->SMDSGroup().Add(aElement);
	    }
	  }
	  if (aElement)
	    aGroupDS->SetType(aElement->GetType());
	}
      }
    }
  }
  return 1;
}

//=======================================================================
//function : MEDToMesh
//purpose  : 
//=======================================================================

int SMESH_Mesh::MEDToMesh(const char* theFileName, const char* theMeshName)
{
  if(MYDEBUG) MESSAGE("MEDToMesh - theFileName = "<<theFileName<<", mesh name = "<<theMeshName);
  if(_isShapeToMesh)
    throw SALOME_Exception(LOCALIZED("a shape to mesh has already been defined"));
  _isShapeToMesh = true;
  DriverMED_R_SMESHDS_Mesh myReader;
  myReader.SetMesh(_myMeshDS);
  myReader.SetMeshId(-1);
  myReader.SetFile(theFileName);
  myReader.SetMeshName(theMeshName);
  Driver_Mesh::Status status = myReader.Perform();
  if(MYDEBUG){
    MESSAGE("MEDToMesh - _myMeshDS->NbNodes() = "<<_myMeshDS->NbNodes());
    MESSAGE("MEDToMesh - _myMeshDS->NbEdges() = "<<_myMeshDS->NbEdges());
    MESSAGE("MEDToMesh - _myMeshDS->NbFaces() = "<<_myMeshDS->NbFaces());
    MESSAGE("MEDToMesh - _myMeshDS->NbVolumes() = "<<_myMeshDS->NbVolumes());
  }

  // Reading groups (sub-meshes are out of scope of MED import functionality)
  list<TNameAndType> aGroupNames = myReader.GetGroupNamesAndTypes();
  if(MYDEBUG) MESSAGE("MEDToMesh - Nb groups = "<<aGroupNames.size()); 
  int anId;
  list<TNameAndType>::iterator name_type = aGroupNames.begin();
  for ( ; name_type != aGroupNames.end(); name_type++ ) {
    SMESH_Group* aGroup = AddGroup( name_type->second, name_type->first.c_str(), anId );
    if ( aGroup ) {
      if(MYDEBUG) MESSAGE("MEDToMesh - group added: "<<name_type->first.c_str());      
      SMESHDS_Group* aGroupDS = dynamic_cast<SMESHDS_Group*>( aGroup->GetGroupDS() );
      if ( aGroupDS ) {
        aGroupDS->SetStoreName( name_type->first.c_str() );
        myReader.GetGroup( aGroupDS );
      }
    }
  }
  return (int) status;
}

//=======================================================================
//function : STLToMesh
//purpose  : 
//=======================================================================

int SMESH_Mesh::STLToMesh(const char* theFileName)
{
  if(MYDEBUG) MESSAGE("STLToMesh - theFileName = "<<theFileName);
  if(_isShapeToMesh)
    throw SALOME_Exception(LOCALIZED("a shape to mesh has already been defined"));
  _isShapeToMesh = true;
  DriverSTL_R_SMDS_Mesh myReader;
  myReader.SetMesh(_myMeshDS);
  myReader.SetFile(theFileName);
  myReader.SetMeshId(-1);
  myReader.Perform();
  if(MYDEBUG){
    MESSAGE("STLToMesh - _myMeshDS->NbNodes() = "<<_myMeshDS->NbNodes());
    MESSAGE("STLToMesh - _myMeshDS->NbEdges() = "<<_myMeshDS->NbEdges());
    MESSAGE("STLToMesh - _myMeshDS->NbFaces() = "<<_myMeshDS->NbFaces());
    MESSAGE("STLToMesh - _myMeshDS->NbVolumes() = "<<_myMeshDS->NbVolumes());
  }
  return 1;
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

SMESH_Hypothesis::Hypothesis_Status
  SMESH_Mesh::AddHypothesis(const TopoDS_Shape & aSubShape,
                            int                  anHypId  ) throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
  if(MYDEBUG) MESSAGE("SMESH_Mesh::AddHypothesis");

  SMESH_subMesh *subMesh = GetSubMesh(aSubShape);
  if ( !subMesh || !subMesh->GetId())
    return SMESH_Hypothesis::HYP_BAD_SUBSHAPE;

  SMESHDS_SubMesh *subMeshDS = subMesh->GetSubMeshDS();
  if ( subMeshDS && subMeshDS->IsComplexSubmesh() ) // group of sub-shapes and maybe of not sub-
  {
    MESSAGE("AddHypothesis() to complex submesh");
    // return the worst but not fatal state of all group memebers
    SMESH_Hypothesis::Hypothesis_Status aBestRet, aWorstNotFatal, ret;
    aBestRet = SMESH_Hypothesis::HYP_BAD_DIM;
    aWorstNotFatal = SMESH_Hypothesis::HYP_OK;
    for ( TopoDS_Iterator itS ( aSubShape ); itS.More(); itS.Next())
    {
      if ( !GetMeshDS()->ShapeToIndex( itS.Value() ))
        continue; // not sub-shape
      ret = AddHypothesis( itS.Value(), anHypId );
      if ( !SMESH_Hypothesis::IsStatusFatal( ret ) && ret > aWorstNotFatal )
        aWorstNotFatal = ret;
      if ( ret < aBestRet )
        aBestRet = ret;
    }
    if ( SMESH_Hypothesis::IsStatusFatal( aBestRet ))
      return aBestRet;
    return aWorstNotFatal;
  }

  StudyContextStruct *sc = _gen->GetStudyContext(_studyId);
  if (sc->mapHypothesis.find(anHypId) == sc->mapHypothesis.end())
  {
    if(MYDEBUG) MESSAGE("Hypothesis ID does not give an hypothesis");
    if(MYDEBUG) {
      SCRUTE(_studyId);
      SCRUTE(anHypId);
    }
    throw SALOME_Exception(LOCALIZED("hypothesis does not exist"));
  }

  SMESH_Hypothesis *anHyp = sc->mapHypothesis[anHypId];
  MESSAGE( "SMESH_Mesh::AddHypothesis " << anHyp->GetName() );

  bool isGlobalHyp = IsMainShape( aSubShape );

  // NotConformAllowed can be only global
  if ( !isGlobalHyp )
  {
    string hypName = anHyp->GetName();
    if ( hypName == "NotConformAllowed" )
    {
      if(MYDEBUG) MESSAGE( "Hypotesis <NotConformAllowed> can be only global" );
      return SMESH_Hypothesis::HYP_INCOMPATIBLE;
    }
  }

  // shape 

  int event;
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

    // check concurent hypotheses on ansestors
    if (ret < SMESH_Hypothesis::HYP_CONCURENT && !isGlobalHyp )
    {
      const map < int, SMESH_subMesh * >& smMap = subMesh->DependsOn();
      map < int, SMESH_subMesh * >::const_iterator smIt = smMap.begin();
      for ( ; smIt != smMap.end(); smIt++ ) {
        if ( smIt->second->IsApplicableHypotesis( anHyp )) {
          ret2 = smIt->second->CheckConcurentHypothesis( anHyp->GetType() );
          if (ret2 > ret) {
            ret = ret2;
            break;
          }
        }
      }
    }
  }

  if(MYDEBUG) subMesh->DumpAlgoState(true);
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
  if(MYDEBUG) MESSAGE("SMESH_Mesh::RemoveHypothesis");
  
  SMESH_subMesh *subMesh = GetSubMesh(aSubShape);
  SMESHDS_SubMesh *subMeshDS = subMesh->GetSubMeshDS();
  if ( subMeshDS && subMeshDS->IsComplexSubmesh() )
  {
    // return the worst but not fatal state of all group memebers
    SMESH_Hypothesis::Hypothesis_Status aBestRet, aWorstNotFatal, ret;
    aBestRet = SMESH_Hypothesis::HYP_BAD_DIM;
    aWorstNotFatal = SMESH_Hypothesis::HYP_OK;
    for ( TopoDS_Iterator itS ( aSubShape ); itS.More(); itS.Next())
    {
      if ( !GetMeshDS()->ShapeToIndex( itS.Value() ))
        continue; // not sub-shape
      ret = RemoveHypothesis( itS.Value(), anHypId );
      if ( !SMESH_Hypothesis::IsStatusFatal( ret ) && ret > aWorstNotFatal )
        aWorstNotFatal = ret;
      if ( ret < aBestRet )
        aBestRet = ret;
    }
    if ( SMESH_Hypothesis::IsStatusFatal( aBestRet ))
      return aBestRet;
    return aWorstNotFatal;
  }

  StudyContextStruct *sc = _gen->GetStudyContext(_studyId);
  if (sc->mapHypothesis.find(anHypId) == sc->mapHypothesis.end())
    throw SALOME_Exception(LOCALIZED("hypothesis does not exist"));
  
  SMESH_Hypothesis *anHyp = sc->mapHypothesis[anHypId];
  int hypType = anHyp->GetType();
  if(MYDEBUG) SCRUTE(hypType);
  int event;
  
  // shape 
  
  if (anHyp->GetType() == SMESHDS_Hypothesis::PARAM_ALGO)
    event = SMESH_subMesh::REMOVE_HYP;
  else
    event = SMESH_subMesh::REMOVE_ALGO;
  SMESH_Hypothesis::Hypothesis_Status ret = subMesh->AlgoStateEngine(event, anHyp);

  // there may appear concurrent hyps that were covered by the removed hyp
  if (ret < SMESH_Hypothesis::HYP_CONCURENT &&
      subMesh->IsApplicableHypotesis( anHyp ) &&
      subMesh->CheckConcurentHypothesis( anHyp->GetType() ) != SMESH_Hypothesis::HYP_OK)
    ret = SMESH_Hypothesis::HYP_CONCURENT;

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

    // check concurent hypotheses on ansestors
    if (ret < SMESH_Hypothesis::HYP_CONCURENT && !IsMainShape( aSubShape ) )
    {
      const map < int, SMESH_subMesh * >& smMap = subMesh->DependsOn();
      map < int, SMESH_subMesh * >::const_iterator smIt = smMap.begin();
      for ( ; smIt != smMap.end(); smIt++ ) {
        if ( smIt->second->IsApplicableHypotesis( anHyp )) {
          ret2 = smIt->second->CheckConcurentHypothesis( anHyp->GetType() );
          if (ret2 > ret) {
            ret = ret2;
            break;
          }
        }
      }
    }
  }
  
  if(MYDEBUG) subMesh->DumpAlgoState(true);
  if(MYDEBUG) SCRUTE(ret);
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
  return _myMeshDS->GetHypothesis(aSubShape);
}

//=======================================================================
//function : GetHypothesis
//purpose  : 
//=======================================================================

const SMESH_Hypothesis * SMESH_Mesh::GetHypothesis(const TopoDS_Shape &    aSubShape,
                                                   const SMESH_HypoFilter& aFilter,
                                                   const bool              andAncestors) const
{
  {
    const list<const SMESHDS_Hypothesis*>& hypList = _myMeshDS->GetHypothesis(aSubShape);
    list<const SMESHDS_Hypothesis*>::const_iterator hyp = hypList.begin();
    for ( ; hyp != hypList.end(); hyp++ ) {
      const SMESH_Hypothesis * h = static_cast<const SMESH_Hypothesis*>( *hyp );
      if ( aFilter.IsOk( h, aSubShape))
        return h;
    }
  }
  if ( andAncestors )
  {
    TopTools_ListIteratorOfListOfShape it( GetAncestors( aSubShape ));
    for (; it.More(); it.Next() )
    {
      const list<const SMESHDS_Hypothesis*>& hypList = _myMeshDS->GetHypothesis(it.Value());
      list<const SMESHDS_Hypothesis*>::const_iterator hyp = hypList.begin();
      for ( ; hyp != hypList.end(); hyp++ ) {
        const SMESH_Hypothesis * h = static_cast<const SMESH_Hypothesis*>( *hyp );
        if (aFilter.IsOk( h, it.Value() ))
          return h;
      }
    }
  }
  return 0;
}

//=======================================================================
//function : GetHypotheses
//purpose  : 
//=======================================================================

//================================================================================
/*!
 * \brief Return hypothesis assigned to the shape
  * \param aSubShape - the shape to check
  * \param aFilter - the hypothesis filter
  * \param aHypList - the list of the found hypotheses
  * \param andAncestors - flag to check hypos assigned to ancestors of the shape
  * \retval int - number of unique hypos in aHypList
 */
//================================================================================

int SMESH_Mesh::GetHypotheses(const TopoDS_Shape &                aSubShape,
                              const SMESH_HypoFilter&             aFilter,
                              list <const SMESHDS_Hypothesis * >& aHypList,
                              const bool                          andAncestors) const
{
  set<string> hypTypes; // to exclude same type hypos from the result list
  int nbHyps = 0;

  // fill in hypTypes
  list<const SMESHDS_Hypothesis*>::const_iterator hyp;
  for ( hyp = aHypList.begin(); hyp != aHypList.end(); hyp++ )
    if ( hypTypes.insert( (*hyp)->GetName() ).second )
      nbHyps++;

  // get hypos from aSubShape
  {
    const list<const SMESHDS_Hypothesis*>& hypList = _myMeshDS->GetHypothesis(aSubShape);
    for ( hyp = hypList.begin(); hyp != hypList.end(); hyp++ )
      if ( aFilter.IsOk (static_cast<const SMESH_Hypothesis*>( *hyp ), aSubShape) &&
           hypTypes.insert( (*hyp)->GetName() ).second )
      {
        aHypList.push_back( *hyp );
        nbHyps++;
      }
  }

  // get hypos from ancestors of aSubShape
  if ( andAncestors )
  {
    TopTools_MapOfShape map;
    TopTools_ListIteratorOfListOfShape it( GetAncestors( aSubShape ));
    for (; it.More(); it.Next() )
    {
     if ( !map.Add( it.Value() ))
        continue;
      const list<const SMESHDS_Hypothesis*>& hypList = _myMeshDS->GetHypothesis(it.Value());
      for ( hyp = hypList.begin(); hyp != hypList.end(); hyp++ )
        if (aFilter.IsOk( static_cast<const SMESH_Hypothesis*>( *hyp ), it.Value() ) &&
            hypTypes.insert( (*hyp)->GetName() ).second ) {
          aHypList.push_back( *hyp );
          nbHyps++;
        }
    }
  }
  return nbHyps;
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

const list<SMESHDS_Command*> & SMESH_Mesh::GetLog() throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
  if(MYDEBUG) MESSAGE("SMESH_Mesh::GetLog");
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
  if(MYDEBUG) MESSAGE("SMESH_Mesh::ClearLog");
  _myMeshDS->GetScript()->Clear();
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

int SMESH_Mesh::GetId()
{
  if(MYDEBUG) MESSAGE("SMESH_Mesh::GetId");
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
  SMESH_subMesh *aSubMesh;
  int index = _myMeshDS->ShapeToIndex(aSubShape);

  // for submeshes on GEOM Group
  if ( !index && aSubShape.ShapeType() == TopAbs_COMPOUND ) {
    TopoDS_Iterator it( aSubShape );
    if ( it.More() )
      index = _myMeshDS->AddCompoundSubmesh( aSubShape, it.Value().ShapeType() );
  }
//   if ( !index )
//     return NULL; // neither sub-shape nor a group

  map <int, SMESH_subMesh *>::iterator i_sm = _mapSubMesh.find(index);
  if ( i_sm != _mapSubMesh.end())
  {
    aSubMesh = i_sm->second;
  }
  else
  {
    aSubMesh = new SMESH_subMesh(index, this, _myMeshDS, aSubShape);
    _mapSubMesh[index] = aSubMesh;
  }
  return aSubMesh;
}

//=============================================================================
/*!
 * Get the SMESH_subMesh object implementation. Dont create it, return null
 * if it does not exist.
 */
//=============================================================================

SMESH_subMesh *SMESH_Mesh::GetSubMeshContaining(const TopoDS_Shape & aSubShape)
  throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
  SMESH_subMesh *aSubMesh = NULL;
  
  int index = _myMeshDS->ShapeToIndex(aSubShape);

  map <int, SMESH_subMesh *>::iterator i_sm = _mapSubMesh.find(index);
  if ( i_sm != _mapSubMesh.end())
    aSubMesh = i_sm->second;

  return aSubMesh;
}

//=============================================================================
/*!
 * Get the SMESH_subMesh object implementation. Dont create it, return null
 * if it does not exist.
 */
//=============================================================================

SMESH_subMesh *SMESH_Mesh::GetSubMeshContaining(const int aShapeID)
throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
  
  map <int, SMESH_subMesh *>::iterator i_sm = _mapSubMesh.find(aShapeID);
  if (i_sm == _mapSubMesh.end())
    return NULL;
  return i_sm->second;
}

//=======================================================================
//function : IsUsedHypothesis
//purpose  : Return True if anHyp is used to mesh aSubShape
//=======================================================================

bool SMESH_Mesh::IsUsedHypothesis(SMESHDS_Hypothesis * anHyp,
                                  const SMESH_subMesh* aSubMesh)
{
  SMESH_Hypothesis* hyp = static_cast<SMESH_Hypothesis*>(anHyp);

  // check if anHyp can be used to mesh aSubMesh
  if ( !aSubMesh || !aSubMesh->IsApplicableHypotesis( hyp ))
    return false;

  const TopoDS_Shape & aSubShape = const_cast<SMESH_subMesh*>( aSubMesh )->GetSubShape();

  SMESH_Algo *algo = _gen->GetAlgo(*this, aSubShape );

  // algorithm
  if (anHyp->GetType() > SMESHDS_Hypothesis::PARAM_ALGO)
    return ( anHyp == algo );

  // algorithm parameter
  if (algo)
  {
    // look trough hypotheses used by algo
    SMESH_HypoFilter hypoKind;
    if ( algo->InitCompatibleHypoFilter( hypoKind, !hyp->IsAuxiliary() )) {
      list <const SMESHDS_Hypothesis * > usedHyps;
      if ( GetHypotheses( aSubShape, hypoKind, usedHyps, true ))
        return ( find( usedHyps.begin(), usedHyps.end(), anHyp ) != usedHyps.end() );
    }
  }

  // look through all assigned hypotheses
  //SMESH_HypoFilter filter( SMESH_HypoFilter::Is( hyp ));
  return false; //GetHypothesis( aSubShape, filter, true );
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
  if(MYDEBUG) MESSAGE("SMESH_Mesh::GetSubMeshUsingHypothesis");
  map < int, SMESH_subMesh * >::iterator itsm;
  _subMeshesUsingHypothesisList.clear();
  for (itsm = _mapSubMesh.begin(); itsm != _mapSubMesh.end(); itsm++)
  {
    SMESH_subMesh *aSubMesh = (*itsm).second;
    if ( IsUsedHypothesis ( anHyp, aSubMesh ))
      _subMeshesUsingHypothesisList.push_back(aSubMesh);
  }
  return _subMeshesUsingHypothesisList;
}

//=======================================================================
//function : NotifySubMeshesHypothesisModification
//purpose  : Say all submeshes using theChangedHyp that it has been modified
//=======================================================================

void SMESH_Mesh::NotifySubMeshesHypothesisModification(const SMESH_Hypothesis* theChangedHyp)
{
  Unexpect aCatch(SalomeException);

  const SMESH_Hypothesis* hyp = static_cast<const SMESH_Hypothesis*>(theChangedHyp);

  const SMESH_Algo *foundAlgo = 0;
  SMESH_HypoFilter algoKind( SMESH_HypoFilter::IsAlgo() );
  SMESH_HypoFilter compatibleHypoKind;
  list <const SMESHDS_Hypothesis * > usedHyps;


  map < int, SMESH_subMesh * >::iterator itsm;
  for (itsm = _mapSubMesh.begin(); itsm != _mapSubMesh.end(); itsm++)
  {
    SMESH_subMesh *aSubMesh = (*itsm).second;
    if ( aSubMesh->IsApplicableHypotesis( hyp ))
    {
      const TopoDS_Shape & aSubShape = aSubMesh->GetSubShape();

      if ( !foundAlgo ) // init filter for algo search
        algoKind.And( algoKind.IsApplicableTo( aSubShape ));
      
      const SMESH_Algo *algo = static_cast<const SMESH_Algo*>
        ( GetHypothesis( aSubShape, algoKind, true ));

      if ( algo )
      {
        bool sameAlgo = ( algo == foundAlgo );
        if ( !sameAlgo && foundAlgo )
          sameAlgo = ( strcmp( algo->GetName(), foundAlgo->GetName() ) == 0);

        if ( !sameAlgo ) { // init filter for used hypos search
          if ( !algo->InitCompatibleHypoFilter( compatibleHypoKind, !hyp->IsAuxiliary() ))
            continue; // algo does not use any hypothesis
          foundAlgo = algo;
        }

        // check if hyp is used by algo
        usedHyps.clear();
        if ( GetHypotheses( aSubShape, compatibleHypoKind, usedHyps, true ) &&
             find( usedHyps.begin(), usedHyps.end(), hyp ) != usedHyps.end() )
        {
          aSubMesh->ComputeStateEngine(SMESH_subMesh::MODIF_HYP);

          if ( algo->GetDim() == 1 && IsPropagationHypothesis( aSubShape ))
            CleanMeshOnPropagationChain( aSubShape );
        }
      }
    }
  }
}

//=============================================================================
/*! Export* methods.
 *  To store mesh contents on disk in different formats.
 */
//=============================================================================

bool SMESH_Mesh::HasDuplicatedGroupNamesMED()
{
  set<string> aGroupNames;
  for ( map<int, SMESH_Group*>::iterator it = _mapGroup.begin(); it != _mapGroup.end(); it++ ) {
    SMESH_Group* aGroup = it->second;
    string aGroupName = aGroup->GetName();
    aGroupName.resize(MAX_MED_GROUP_NAME_LENGTH);
    if (!aGroupNames.insert(aGroupName).second)
      return true;
  }

  return false;
}

void SMESH_Mesh::ExportMED(const char *file, 
			   const char* theMeshName, 
			   bool theAutoGroups,
			   int theVersion) 
  throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);

  DriverMED_W_SMESHDS_Mesh myWriter;
  myWriter.SetFile    ( file, MED::EVersion(theVersion) );
  myWriter.SetMesh    ( _myMeshDS   );
  if ( !theMeshName ) 
    myWriter.SetMeshId  ( _idDoc      );
  else {
    myWriter.SetMeshId  ( -1          );
    myWriter.SetMeshName( theMeshName );
  }

  if ( theAutoGroups ) {
    myWriter.AddGroupOfNodes();
    myWriter.AddGroupOfEdges();
    myWriter.AddGroupOfFaces();
    myWriter.AddGroupOfVolumes();
  }

  // Pass groups to writer. Provide unique group names.
  set<string> aGroupNames;
  char aString [256];
  int maxNbIter = 10000; // to guarantee cycle finish
  for ( map<int, SMESH_Group*>::iterator it = _mapGroup.begin(); it != _mapGroup.end(); it++ ) {
    SMESH_Group*       aGroup   = it->second;
    SMESHDS_GroupBase* aGroupDS = aGroup->GetGroupDS();
    if ( aGroupDS ) {
      string aGroupName0 = aGroup->GetName();
      aGroupName0.resize(MAX_MED_GROUP_NAME_LENGTH);
      string aGroupName = aGroupName0;
      for (int i = 1; !aGroupNames.insert(aGroupName).second && i < maxNbIter; i++) {
        sprintf(&aString[0], "GR_%d_%s", i, aGroupName0.c_str());
        aGroupName = aString;
        aGroupName.resize(MAX_MED_GROUP_NAME_LENGTH);
      }
      aGroupDS->SetStoreName( aGroupName.c_str() );
      myWriter.AddGroup( aGroupDS );
    }
  }

  // Perform export
  myWriter.Perform();
}

void SMESH_Mesh::ExportDAT(const char *file) throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
  DriverDAT_W_SMDS_Mesh myWriter;
  myWriter.SetFile(string(file));
  myWriter.SetMesh(_myMeshDS);
  myWriter.SetMeshId(_idDoc);
  myWriter.Perform();
}

void SMESH_Mesh::ExportUNV(const char *file) throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
  DriverUNV_W_SMDS_Mesh myWriter;
  myWriter.SetFile(string(file));
  myWriter.SetMesh(_myMeshDS);
  myWriter.SetMeshId(_idDoc);
  //  myWriter.SetGroups(_mapGroup);

  for ( map<int, SMESH_Group*>::iterator it = _mapGroup.begin(); it != _mapGroup.end(); it++ ) {
    SMESH_Group*       aGroup   = it->second;
    SMESHDS_GroupBase* aGroupDS = aGroup->GetGroupDS();
    if ( aGroupDS ) {
      string aGroupName = aGroup->GetName();
      aGroupDS->SetStoreName( aGroupName.c_str() );
      myWriter.AddGroup( aGroupDS );
    }
  }
  myWriter.Perform();
}

void SMESH_Mesh::ExportSTL(const char *file, const bool isascii) throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
  DriverSTL_W_SMDS_Mesh myWriter;
  myWriter.SetFile(string(file));
  myWriter.SetIsAscii( isascii );
  myWriter.SetMesh(_myMeshDS);
  myWriter.SetMeshId(_idDoc);
  myWriter.Perform();
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
int SMESH_Mesh::NbEdges(ElementOrder order) throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
  if (order == ORDER_ANY)
    return _myMeshDS->NbEdges();

  int Nb = 0;
  SMDS_EdgeIteratorPtr it = _myMeshDS->edgesIterator();
  while (it->more()) {
    const SMDS_MeshEdge* cur = it->next();
    if ( order == ORDER_LINEAR && !cur->IsQuadratic() ||
         order == ORDER_QUADRATIC && cur->IsQuadratic() )
      Nb++;
  }
  return Nb;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
int SMESH_Mesh::NbFaces(ElementOrder order) throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
  if (order == ORDER_ANY)
    return _myMeshDS->NbFaces();

  int Nb = 0;
  SMDS_FaceIteratorPtr it = _myMeshDS->facesIterator();
  while (it->more()) {
    const SMDS_MeshFace* cur = it->next();
    if ( order == ORDER_LINEAR && !cur->IsQuadratic() ||
         order == ORDER_QUADRATIC && cur->IsQuadratic() )
      Nb++;
  }
  return Nb;
}

///////////////////////////////////////////////////////////////////////////////
/// Return the number of 3 nodes faces in the mesh. This method run in O(n)
///////////////////////////////////////////////////////////////////////////////
int SMESH_Mesh::NbTriangles(ElementOrder order) throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
  int Nb = 0;
  
  SMDS_FaceIteratorPtr itFaces=_myMeshDS->facesIterator();
  while (itFaces->more()) {
    const SMDS_MeshFace* curFace = itFaces->next();
    int nbnod = curFace->NbNodes();
    if ( !curFace->IsPoly() && 
	 ( order == ORDER_ANY && (nbnod==3 || nbnod==6) ||
           order == ORDER_LINEAR && nbnod==3 ||
           order == ORDER_QUADRATIC && nbnod==6 ) )
      Nb++;
  }
  return Nb;
}

///////////////////////////////////////////////////////////////////////////////
/// Return the number of 4 nodes faces in the mesh. This method run in O(n)
///////////////////////////////////////////////////////////////////////////////
int SMESH_Mesh::NbQuadrangles(ElementOrder order) throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
  int Nb = 0;
  
  SMDS_FaceIteratorPtr itFaces=_myMeshDS->facesIterator();
  while (itFaces->more()) {
    const SMDS_MeshFace* curFace = itFaces->next();
    int nbnod = curFace->NbNodes();
    if ( !curFace->IsPoly() && 
	 ( order == ORDER_ANY && (nbnod==4 || nbnod==8) ||
           order == ORDER_LINEAR && nbnod==4 ||
           order == ORDER_QUADRATIC && nbnod==8 ) )
      Nb++;
  }
  return Nb;
}

///////////////////////////////////////////////////////////////////////////////
/// Return the number of polygonal faces in the mesh. This method run in O(n)
///////////////////////////////////////////////////////////////////////////////
int SMESH_Mesh::NbPolygons() throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
  int Nb = 0;
  SMDS_FaceIteratorPtr itFaces = _myMeshDS->facesIterator();
  while (itFaces->more())
    if (itFaces->next()->IsPoly()) Nb++;
  return Nb;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
int SMESH_Mesh::NbVolumes(ElementOrder order) throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
  if (order == ORDER_ANY)
    return _myMeshDS->NbVolumes();

  int Nb = 0;
  SMDS_VolumeIteratorPtr it = _myMeshDS->volumesIterator();
  while (it->more()) {
    const SMDS_MeshVolume* cur = it->next();
    if ( order == ORDER_LINEAR && !cur->IsQuadratic() ||
         order == ORDER_QUADRATIC && cur->IsQuadratic() )
      Nb++;
  }
  return Nb;
}

int SMESH_Mesh::NbTetras(ElementOrder order) throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
  int Nb = 0;
  SMDS_VolumeIteratorPtr itVolumes=_myMeshDS->volumesIterator();
  while (itVolumes->more()) {
    const SMDS_MeshVolume* curVolume = itVolumes->next();
    int nbnod = curVolume->NbNodes();
    if ( !curVolume->IsPoly() && 
	 ( order == ORDER_ANY && (nbnod==4 || nbnod==10) ||
           order == ORDER_LINEAR && nbnod==4 ||
           order == ORDER_QUADRATIC && nbnod==10 ) )
      Nb++;
  }
  return Nb;
}

int SMESH_Mesh::NbHexas(ElementOrder order) throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
  int Nb = 0;
  SMDS_VolumeIteratorPtr itVolumes=_myMeshDS->volumesIterator();
  while (itVolumes->more()) {
    const SMDS_MeshVolume* curVolume = itVolumes->next();
    int nbnod = curVolume->NbNodes();
    if ( !curVolume->IsPoly() && 
	 ( order == ORDER_ANY && (nbnod==8 || nbnod==20) ||
           order == ORDER_LINEAR && nbnod==8 ||
           order == ORDER_QUADRATIC && nbnod==20 ) )
      Nb++;
  }
  return Nb;
}

int SMESH_Mesh::NbPyramids(ElementOrder order) throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
  int Nb = 0;
  SMDS_VolumeIteratorPtr itVolumes=_myMeshDS->volumesIterator();
  while (itVolumes->more()) {
    const SMDS_MeshVolume* curVolume = itVolumes->next();
    int nbnod = curVolume->NbNodes();
    if ( !curVolume->IsPoly() && 
	 ( order == ORDER_ANY && (nbnod==5 || nbnod==13) ||
           order == ORDER_LINEAR && nbnod==5 ||
           order == ORDER_QUADRATIC && nbnod==13 ) )
      Nb++;
  }
  return Nb;
}

int SMESH_Mesh::NbPrisms(ElementOrder order) throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
  int Nb = 0;
  SMDS_VolumeIteratorPtr itVolumes=_myMeshDS->volumesIterator();
  while (itVolumes->more()) {
    const SMDS_MeshVolume* curVolume = itVolumes->next();
    int nbnod = curVolume->NbNodes();
    if ( !curVolume->IsPoly() && 
	 ( order == ORDER_ANY && (nbnod==6 || nbnod==15) ||
           order == ORDER_LINEAR && nbnod==6 ||
           order == ORDER_QUADRATIC && nbnod==15 ) )
      Nb++;
  }
  return Nb;
}

int SMESH_Mesh::NbPolyhedrons() throw(SALOME_Exception)
{
  Unexpect aCatch(SalomeException);
  int Nb = 0;
  SMDS_VolumeIteratorPtr itVolumes = _myMeshDS->volumesIterator();
  while (itVolumes->more())
    if (itVolumes->next()->IsPoly()) Nb++;
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
  if(MYDEBUG) MESSAGE("SMESH_Mesh::IsNotConformAllowed");

  SMESH_HypoFilter filter( SMESH_HypoFilter::HasName( "NotConformAllowed" ));
  return GetHypothesis( _myMeshDS->ShapeToMesh(), filter, false );
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
				   int&                      theId,
                                   const TopoDS_Shape&       theShape)
{
  if (_mapGroup.find(_groupId) != _mapGroup.end())
    return NULL;
  theId = _groupId;
  SMESH_Group* aGroup = new SMESH_Group (theId, this, theType, theName, theShape);
  GetMeshDS()->AddGroup( aGroup->GetGroupDS() );
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
  GetMeshDS()->RemoveGroup( _mapGroup[theGroupID]->GetGroupDS() );
  delete _mapGroup[theGroupID];
  _mapGroup.erase (theGroupID);
}

//=============================================================================
/*!
 *  IsLocal1DHypothesis
 *  Returns a local 1D hypothesis used for theEdge
 */
//=============================================================================
const SMESH_Hypothesis* SMESH_Mesh::IsLocal1DHypothesis (const TopoDS_Shape& theEdge)
{
  SMESH_HypoFilter hypo ( SMESH_HypoFilter::HasDim( 1 ));
  hypo.AndNot( hypo.IsAlgo() ).AndNot( hypo.IsAssignedTo( GetMeshDS()->ShapeToMesh() ));

  return GetHypothesis( theEdge, hypo, true );
}

//=============================================================================
/*!
 *  IsPropagationHypothesis
 */
//=============================================================================
bool SMESH_Mesh::IsPropagationHypothesis (const TopoDS_Shape& theEdge)
{
  return _mapPropagationChains.Contains(theEdge);
}

//=============================================================================
/*!
 *  IsPropagatedHypothesis
 */
//=============================================================================
bool SMESH_Mesh::IsPropagatedHypothesis (const TopoDS_Shape& theEdge,
                                         TopoDS_Shape&       theMainEdge)
{
  int nbChains = _mapPropagationChains.Extent();
  for (int i = 1; i <= nbChains; i++) {
    //const TopTools_IndexedMapOfShape& aChain = _mapPropagationChains.FindFromIndex(i);
    const SMESH_IndexedMapOfShape& aChain = _mapPropagationChains.FindFromIndex(i);
    if (aChain.Contains(theEdge)) {
      theMainEdge = _mapPropagationChains.FindKey(i);
      return true;
    }
  }

  return false;
}
//=============================================================================
/*!
 *  IsReversedInChain
 */
//=============================================================================

bool SMESH_Mesh::IsReversedInChain (const TopoDS_Shape& theEdge,
                                    const TopoDS_Shape& theMainEdge)
{
  if ( !theMainEdge.IsNull() && !theEdge.IsNull() &&
      _mapPropagationChains.Contains( theMainEdge ))
  {
    const SMESH_IndexedMapOfShape& aChain =
      _mapPropagationChains.FindFromKey( theMainEdge );
    int index = aChain.FindIndex( theEdge );
    if ( index )
      return aChain(index).Orientation() == TopAbs_REVERSED;
  }
  return false;
}

//=============================================================================
/*!
 *  CleanMeshOnPropagationChain
 */
//=============================================================================
void SMESH_Mesh::CleanMeshOnPropagationChain (const TopoDS_Shape& theMainEdge)
{
  const SMESH_IndexedMapOfShape& aChain = _mapPropagationChains.FindFromKey(theMainEdge);
  int i, nbEdges = aChain.Extent();
  for (i = 1; i <= nbEdges; i++) {
    TopoDS_Shape anEdge = aChain.FindKey(i);
    SMESH_subMesh *subMesh = GetSubMesh(anEdge);
    SMESHDS_SubMesh *subMeshDS = subMesh->GetSubMeshDS();
    if (subMeshDS && subMeshDS->NbElements() > 0) {
      subMesh->ComputeStateEngine(SMESH_subMesh::CLEAN);
    }
  }
}

//=============================================================================
/*!
 *  RebuildPropagationChains
 *  Rebuild all existing propagation chains.
 *  Have to be used, if 1D hypothesis have been assigned/removed to/from any edge
 */
//=============================================================================
bool SMESH_Mesh::RebuildPropagationChains()
{
  bool ret = true;

  // Clean all chains, because they can be not up-to-date
  int i, nbChains = _mapPropagationChains.Extent();
  for (i = 1; i <= nbChains; i++) {
    TopoDS_Shape aMainEdge = _mapPropagationChains.FindKey(i);
    CleanMeshOnPropagationChain(aMainEdge);
    _mapPropagationChains.ChangeFromIndex(i).Clear();
  }

  // Build all chains
  for (i = 1; i <= nbChains; i++) {
    TopoDS_Shape aMainEdge = _mapPropagationChains.FindKey(i);
    if (!BuildPropagationChain(aMainEdge))
      ret = false;
    CleanMeshOnPropagationChain(aMainEdge);
  }

  return ret;
}

//=============================================================================
/*!
 *  RemovePropagationChain
 *  Have to be used, if Propagation hypothesis is removed from <theMainEdge>
 */
//=============================================================================
bool SMESH_Mesh::RemovePropagationChain (const TopoDS_Shape& theMainEdge)
{
  if (!_mapPropagationChains.Contains(theMainEdge))
    return false;

  // Clean mesh elements and nodes, built on the chain
  CleanMeshOnPropagationChain(theMainEdge);

  // Clean the chain
  _mapPropagationChains.ChangeFromKey(theMainEdge).Clear();

  // Remove the chain from the map
  int i = _mapPropagationChains.FindIndex(theMainEdge);
  if ( i == _mapPropagationChains.Extent() )
    _mapPropagationChains.RemoveLast();
  else {
    TopoDS_Vertex anEmptyShape;
    BRep_Builder BB;
    BB.MakeVertex(anEmptyShape, gp_Pnt(0,0,0), 0.1);
    SMESH_IndexedMapOfShape anEmptyMap;
    _mapPropagationChains.Substitute(i, anEmptyShape, anEmptyMap);
  }

  return true;
}

//=============================================================================
/*!
 *  BuildPropagationChain
 */
//=============================================================================
bool SMESH_Mesh::BuildPropagationChain (const TopoDS_Shape& theMainEdge)
{
  if (theMainEdge.ShapeType() != TopAbs_EDGE) return true;

  // Add new chain, if there is no
  if (!_mapPropagationChains.Contains(theMainEdge)) {
    SMESH_IndexedMapOfShape aNewChain;
    _mapPropagationChains.Add(theMainEdge, aNewChain);
  }

  // Check presence of 1D hypothesis to be propagated
  const SMESH_Hypothesis* aMainHyp = IsLocal1DHypothesis(theMainEdge);
  if (!aMainHyp) {
    MESSAGE("Warning: There is no 1D hypothesis to propagate. Please, assign.");
    return true;
  }

  // Edges, on which the 1D hypothesis will be propagated from <theMainEdge>
  SMESH_IndexedMapOfShape& aChain = _mapPropagationChains.ChangeFromKey(theMainEdge);
  if (aChain.Extent() > 0) {
    CleanMeshOnPropagationChain(theMainEdge);
    aChain.Clear();
  }

  // At first put <theMainEdge> in the chain
  aChain.Add(theMainEdge);

  // List of edges, added to chain on the previous cycle pass
  TopTools_ListOfShape listPrevEdges;
  listPrevEdges.Append(theMainEdge.Oriented( TopAbs_FORWARD ));

//   5____4____3____4____5____6
//   |    |    |    |    |    |
//   |    |    |    |    |    |
//   4____3____2____3____4____5
//   |    |    |    |    |    |      Number in the each knot of
//   |    |    |    |    |    |      grid indicates cycle pass,
//   3____2____1____2____3____4      on which corresponding edge
//   |    |    |    |    |    |      (perpendicular to the plane
//   |    |    |    |    |    |      of view) will be found.
//   2____1____0____1____2____3
//   |    |    |    |    |    |
//   |    |    |    |    |    |
//   3____2____1____2____3____4

  // Collect all edges pass by pass
  while (listPrevEdges.Extent() > 0) {
    // List of edges, added to chain on this cycle pass
    TopTools_ListOfShape listCurEdges;

    // Find the next portion of edges
    TopTools_ListIteratorOfListOfShape itE (listPrevEdges);
    for (; itE.More(); itE.Next()) {
      TopoDS_Shape anE = itE.Value();

      // Iterate on faces, having edge <anE>
      TopTools_ListIteratorOfListOfShape itA (GetAncestors(anE));
      for (; itA.More(); itA.Next()) {
        TopoDS_Shape aW = itA.Value();

        // There are objects of different type among the ancestors of edge
        if (aW.ShapeType() == TopAbs_WIRE) {
          TopoDS_Shape anOppE;

          BRepTools_WireExplorer aWE (TopoDS::Wire(aW));
          Standard_Integer nb = 1, found = 0;
          TopTools_Array1OfShape anEdges (1,4);
          for (; aWE.More(); aWE.Next(), nb++) {
            if (nb > 4) {
              found = 0;
              break;
            }
            anEdges(nb) = aWE.Current();
            if (!_mapAncestors.Contains(anEdges(nb))) {
              MESSAGE("WIRE EXPLORER HAVE GIVEN AN INVALID EDGE !!!");
              break;
            }
            if (anEdges(nb).IsSame(anE)) found = nb;
          }

          if (nb == 5 && found > 0) {
            // Quadrangle face found, get an opposite edge
            Standard_Integer opp = found + 2;
            if (opp > 4) opp -= 4;
            anOppE = anEdges(opp);

            // add anOppE to aChain if ...
            if (!aChain.Contains(anOppE)) { // ... anOppE is not in aChain
              if (!IsLocal1DHypothesis(anOppE)) { // ... no other 1d hyp on anOppE
                TopoDS_Shape aMainEdgeForOppEdge; // ... no other hyp is propagated to anOppE
                if (!IsPropagatedHypothesis(anOppE, aMainEdgeForOppEdge))
                {
                  // Add found edge to the chain oriented so that to
                  // have it co-directed with a forward MainEdge
                  TopAbs_Orientation ori = anE.Orientation();
                  if ( anEdges(opp).Orientation() == anEdges(found).Orientation() )
                    ori = TopAbs::Reverse( ori );
                  anOppE.Orientation( ori );
                  aChain.Add(anOppE);
                  listCurEdges.Append(anOppE);
                }
                else {
                  // Collision!
                  MESSAGE("Error: Collision between propagated hypotheses");
                  CleanMeshOnPropagationChain(theMainEdge);
                  aChain.Clear();
                  return ( aMainHyp == IsLocal1DHypothesis(aMainEdgeForOppEdge) );
                }
              }
            }
          } // if (nb == 5 && found > 0)
        } // if (aF.ShapeType() == TopAbs_WIRE)
      } // for (; itF.More(); itF.Next())
    } // for (; itE.More(); itE.Next())

    listPrevEdges = listCurEdges;
  } // while (listPrevEdges.Extent() > 0)

  CleanMeshOnPropagationChain(theMainEdge);
  return true;
}

//=======================================================================
//function : GetAncestors
//purpose  : return list of ancestors of theSubShape in the order
//           that lower dimention shapes come first.
//=======================================================================

const TopTools_ListOfShape& SMESH_Mesh::GetAncestors(const TopoDS_Shape& theS) const
{
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
  int clause = 0;
  save << "========================== Dump contents of mesh ==========================" << endl << endl;
  save << ++clause << ") Total number of nodes:   \t"    << NbNodes() << endl;
  save << ++clause << ") Total number of edges:   \t"    << NbEdges() << endl;
  save << ++clause << ") Total number of faces:   \t"    << NbFaces() << endl;
  save << ++clause << ") Total number of polygons:\t"    << NbPolygons() << endl;
  save << ++clause << ") Total number of volumes:\t"     << NbVolumes() << endl;
  save << ++clause << ") Total number of polyhedrons:\t" << NbPolyhedrons() << endl << endl;
  for ( int isQuadratic = 0; isQuadratic < 2; ++isQuadratic )
  {
    string orderStr = isQuadratic ? "quadratic" : "linear";
    ElementOrder order  = isQuadratic ? ORDER_QUADRATIC : ORDER_LINEAR;

    save << ++clause << ") Total number of " << orderStr << " edges:\t" << NbEdges(order) << endl;
    save << ++clause << ") Total number of " << orderStr << " faces:\t" << NbFaces(order) << endl;
    if ( NbFaces(order) > 0 ) {
      int nb3 = NbTriangles(order);
      int nb4 = NbQuadrangles(order);
      save << clause << ".1) Number of " << orderStr << " triangles:  \t" << nb3 << endl;
      save << clause << ".2) Number of " << orderStr << " quadrangles:\t" << nb4 << endl;
      if ( nb3 + nb4 !=  NbFaces(order) ) {
        map<int,int> myFaceMap;
        SMDS_FaceIteratorPtr itFaces=_myMeshDS->facesIterator();
        while( itFaces->more( ) ) {
          int nbNodes = itFaces->next()->NbNodes();
          if ( myFaceMap.find( nbNodes ) == myFaceMap.end() )
            myFaceMap[ nbNodes ] = 0;
          myFaceMap[ nbNodes ] = myFaceMap[ nbNodes ] + 1;
        }
        save << clause << ".3) Faces in detail: " << endl;
        map <int,int>::iterator itF;
        for (itF = myFaceMap.begin(); itF != myFaceMap.end(); itF++)
          save << "--> nb nodes: " << itF->first << " - nb elemens:\t" << itF->second << endl;
      }
    }
    save << ++clause << ") Total number of " << orderStr << " volumes:\t" << NbVolumes(order) << endl;
    if ( NbVolumes(order) > 0 ) {
      int nb8 = NbHexas(order);
      int nb4 = NbTetras(order);
      int nb5 = NbPyramids(order);
      int nb6 = NbPrisms(order);
      save << clause << ".1) Number of " << orderStr << " hexahedrons:\t" << nb8 << endl;
      save << clause << ".2) Number of " << orderStr << " tetrahedrons:\t" << nb4 << endl;
      save << clause << ".3) Number of " << orderStr << " prisms:      \t" << nb6 << endl;
      save << clause << ".4) Number of " << orderStr << " pyramids:\t" << nb5 << endl;
      if ( nb8 + nb4 + nb5 + nb6 != NbVolumes(order) ) {
        map<int,int> myVolumesMap;
        SMDS_VolumeIteratorPtr itVolumes=_myMeshDS->volumesIterator();
        while( itVolumes->more( ) ) {
          int nbNodes = itVolumes->next()->NbNodes();
          if ( myVolumesMap.find( nbNodes ) == myVolumesMap.end() )
            myVolumesMap[ nbNodes ] = 0;
          myVolumesMap[ nbNodes ] = myVolumesMap[ nbNodes ] + 1;
        }
        save << clause << ".5) Volumes in detail: " << endl;
        map <int,int>::iterator itV;
        for (itV = myVolumesMap.begin(); itV != myVolumesMap.end(); itV++)
          save << "--> nb nodes: " << itV->first << " - nb elemens:\t" << itV->second << endl;
      }
    }
    save << endl;
  }
  save << "===========================================================================" << endl;
  return save;
}

//=======================================================================
//function : GetElementType
//purpose  : Returns type of mesh element with certain id
//=======================================================================
SMDSAbs_ElementType SMESH_Mesh::GetElementType( const int id, const bool iselem )
{
  return _myMeshDS->GetElementType( id, iselem );
}
