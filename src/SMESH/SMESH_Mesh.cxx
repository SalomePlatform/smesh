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

using namespace std;
using namespace std;
#include "SMESH_Mesh.hxx"
#include "SMESH_subMesh.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_Hypothesis.hxx"
#include "SMESHDS_Script.hxx"
//#include "SMESHDS_ListOfAsciiString.hxx"
//#include "SMESHDS_ListIteratorOfListOfAsciiString.hxx"
#include "SMESHDS_ListOfPtrHypothesis.hxx"
#include "SMESHDS_ListIteratorOfListOfPtrHypothesis.hxx"
#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshFacesIterator.hxx"
#include "SMDS_MeshVolumesIterator.hxx"
#include "TCollection_AsciiString.hxx"

#include "utilities.h"

#include "Mesh_Writer.h"
#include "DriverMED_W_SMESHDS_Mesh.h"
#include "DriverDAT_W_SMESHDS_Mesh.h"
#include "DriverUNV_W_SMESHDS_Mesh.h"

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

SMESH_Mesh::SMESH_Mesh(int localId,
		       int studyId,
		       SMESH_Gen* gen,
		       const Handle(SMESHDS_Document)& myDocument)
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
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

void SMESH_Mesh::ShapeToMesh(const TopoDS_Shape& aShape)
  throw (SALOME_Exception)
{
  MESSAGE("SMESH_Mesh::ShapeToMesh");
  if (_isShapeToMesh)
    throw SALOME_Exception(LOCALIZED("a shape to mesh as already been defined"));
  _isShapeToMesh = true;
  _myMeshDS->ShapeToMesh(aShape);

  // NRI : 24/02/03
  TopExp::MapShapes(aShape,_subShapes);
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

bool SMESH_Mesh::AddHypothesis(const TopoDS_Shape& aSubShape,
			       int anHypId)
  throw (SALOME_Exception)
{
  MESSAGE("SMESH_Mesh::AddHypothesis");

  StudyContextStruct* sc = _gen->GetStudyContext(_studyId);
  if (sc->mapHypothesis.find(anHypId) == sc->mapHypothesis.end())
    {
      MESSAGE("Hypothesis ID does not give an hypothesis");
      SCRUTE(_studyId);
      SCRUTE(anHypId);
      throw SALOME_Exception(LOCALIZED("hypothesis does not exist"));
    }

  SMESH_subMesh* subMesh = GetSubMesh(aSubShape);
  SMESH_Hypothesis* anHyp = sc->mapHypothesis[anHypId];
  int event;

  // shape 

  if (anHyp->GetType() == SMESHDS_Hypothesis::PARAM_ALGO)
    event = SMESH_subMesh::ADD_HYP;
  else
    event = SMESH_subMesh::ADD_ALGO;
  int ret = subMesh->AlgoStateEngine(event, anHyp);

  // subShapes (only when shape is mainShape)
  TopoDS_Shape mainShape = _myMeshDS->ShapeToMesh();
  if (aSubShape.IsSame(mainShape))
    {
      if (anHyp->GetType() == SMESHDS_Hypothesis::PARAM_ALGO)
	event = SMESH_subMesh::ADD_FATHER_HYP;
      else
	event = SMESH_subMesh::ADD_FATHER_ALGO;
      subMesh->SubMeshesAlgoStateEngine(event, anHyp);
    }

  subMesh->DumpAlgoState(true);
  //SCRUTE(ret);
  return ret;
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

bool SMESH_Mesh::RemoveHypothesis(const TopoDS_Shape& aSubShape,
				  int anHypId)
  throw (SALOME_Exception)
{
  MESSAGE("SMESH_Mesh::RemoveHypothesis");

  StudyContextStruct* sc = _gen->GetStudyContext(_studyId);
  if (sc->mapHypothesis.find(anHypId) == sc->mapHypothesis.end())
    throw SALOME_Exception(LOCALIZED("hypothesis does not exist"));

  SMESH_subMesh* subMesh = GetSubMesh(aSubShape);
  SMESH_Hypothesis* anHyp = sc->mapHypothesis[anHypId];
  int hypType = anHyp->GetType();
  SCRUTE(hypType);
  int event;

  // shape 

  if (anHyp->GetType() == SMESHDS_Hypothesis::PARAM_ALGO)
    event = SMESH_subMesh::REMOVE_HYP;
  else
    event = SMESH_subMesh::REMOVE_ALGO;
  int ret = subMesh->AlgoStateEngine(event, anHyp);

  // subShapes (only when shape is mainShape)

  TopoDS_Shape mainShape = _myMeshDS->ShapeToMesh();
  if (aSubShape.IsSame(mainShape))
    {
      if (anHyp->GetType() == SMESHDS_Hypothesis::PARAM_ALGO)
	event = SMESH_subMesh::REMOVE_FATHER_HYP;
      else
	event = SMESH_subMesh::REMOVE_FATHER_ALGO;
      subMesh->SubMeshesAlgoStateEngine(event, anHyp);
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

const Handle(SMESHDS_Mesh)& SMESH_Mesh::GetMeshDS()
{
  return _myMeshDS;
}


//=============================================================================
/*!
 * 
 */
//=============================================================================

const list<SMESHDS_Hypothesis*>&
SMESH_Mesh::GetHypothesisList(const TopoDS_Shape& aSubShape)
  throw (SALOME_Exception)
{
  MESSAGE("SMESH_Mesh::GetHypothesisList");
  _subShapeHypothesisList.clear();
  const SMESHDS_ListOfPtrHypothesis& listHyp
    = _myMeshDS->GetHypothesis(aSubShape);
  SMESHDS_ListIteratorOfListOfPtrHypothesis it(listHyp);
  while (it.More())
    {
      SMESHDS_Hypothesis* anHyp = it.Value();
      _subShapeHypothesisList.push_back(anHyp);
      it.Next();
    }
  return _subShapeHypothesisList;
}

//=============================================================================
/*!
 * 
 */
//=============================================================================

const SMESHDS_ListOfCommand& SMESH_Mesh::GetLog()
  throw (SALOME_Exception)
{
  MESSAGE("SMESH_Mesh::GetLog");
  Handle (SMESHDS_Script) scriptDS = _myMeshDS->GetScript();
  const SMESHDS_ListOfCommand& logDS = scriptDS->GetCommands();
//   SMESHDS_ListIteratorOfListOfCommand its;
//   const SMESHDS_ListOfAsciiString& logDS = scriptDS->GetCommands();
//   SMESHDS_ListIteratorOfListOfAsciiString its;
//   for (its.Initialize(logDS); its.More(); its.Next())
//     {
//       SCRUTE(its.Value().ToCString());
//     }
  return logDS;
}

//=============================================================================
/*!
 * 
 */
//=============================================================================
void SMESH_Mesh::ClearLog()
  throw (SALOME_Exception)
{
  MESSAGE("SMESH_Mesh::ClearLog");
  Handle (SMESHDS_Script) scriptDS = _myMeshDS->GetScript();
  scriptDS->Clear();
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

SMESH_Gen* SMESH_Mesh::GetGen()
{
  return _gen;
}

//=============================================================================
/*!
 * Get or Create the SMESH_subMesh object implementation
 */
//=============================================================================

SMESH_subMesh* SMESH_Mesh::GetSubMesh(const TopoDS_Shape & aSubShape)
  throw (SALOME_Exception)
{
  //MESSAGE("SMESH_Mesh::GetSubMesh");
  SMESH_subMesh* aSubMesh;
  int index = _subShapes.FindIndex(aSubShape);
  if ( _mapSubMesh.find(index) != _mapSubMesh.end() ) {
    aSubMesh = _mapSubMesh[index];
  } else {
    aSubMesh = new SMESH_subMesh(index, this, _myMeshDS, aSubShape);
    _mapSubMesh[index] = aSubMesh;
  }

  /* NRI 24/02/2003
  int index = -1;
  if (_subShapes.Contains(aSubShape))
    {
      index = _subShapes.FindIndex(aSubShape);
      ASSERT(_mapSubMesh.find(index) != _mapSubMesh.end());
      aSubMesh = _mapSubMesh[index];
      //MESSAGE("found submesh " << index);
    }
  else
    {
      index = _subShapes.Add(aSubShape);
      aSubMesh = new SMESH_subMesh(index, this, _myMeshDS, aSubShape);
      _mapSubMesh[index] = aSubMesh;
      //MESSAGE("created submesh " << index);
    }
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

SMESH_subMesh* SMESH_Mesh::GetSubMeshContaining(const TopoDS_Shape & aSubShape)
  throw (SALOME_Exception)
{
  //MESSAGE("SMESH_Mesh::GetSubMeshContaining");
  bool isFound = false;
  SMESH_subMesh* aSubMesh = NULL;

  int index = _subShapes.FindIndex(aSubShape);
  if ( _mapSubMesh.find(index) != _mapSubMesh.end() ) {
    aSubMesh = _mapSubMesh[index];
    isFound = true;
  } 

  /* NRI 24/02/2003
  int index = -1;
  if (_subShapes.Contains(aSubShape))
    {
      index = _subShapes.FindIndex(aSubShape);
      ASSERT(_mapSubMesh.find(index) != _mapSubMesh.end());
      aSubMesh = _mapSubMesh[index];
      isFound = true;
      //MESSAGE("found submesh " << index);
    }
  */

//   map<int, SMESH_subMesh*>::iterator itsm;
//   for (itsm = _mapSubMesh.begin(); itsm != _mapSubMesh.end(); itsm++)
//     {
//       aSubMesh = (*itsm).second;
//       isFound = aSubMesh->Contains(aSubShape);
//       if (isFound) break;
//     }

  if (! isFound) aSubMesh = NULL;
  return aSubMesh;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

const list <SMESH_subMesh*>&
SMESH_Mesh::GetSubMeshUsingHypothesis(SMESHDS_Hypothesis* anHyp)
  throw (SALOME_Exception)
{
  MESSAGE("SMESH_Mesh::GetSubMeshUsingHypothesis");
  map<int, SMESH_subMesh*>::iterator itsm;
  _subMeshesUsingHypothesisList.clear();
  for (itsm = _mapSubMesh.begin(); itsm != _mapSubMesh.end(); itsm++)
    {
      SMESH_subMesh* aSubMesh = (*itsm).second;
      bool usesHyp = false;
      SMESH_Algo* algo = _gen->GetAlgo(*this, aSubMesh->GetSubShape());
      if (algo != NULL)
	{
	  const list<SMESHDS_Hypothesis*>& usedHyps
	    = algo->GetUsedHypothesis(*this, aSubMesh->GetSubShape());
	  list<SMESHDS_Hypothesis*>::const_iterator itl;
	  for(itl=usedHyps.begin(); itl != usedHyps.end(); itl++)
	    if (anHyp == (*itl))
	      {
		usesHyp = true;
		break;
	      }	    
	}
      if (usesHyp) _subMeshesUsingHypothesisList.push_back(aSubMesh);
    }
  return _subMeshesUsingHypothesisList;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

void SMESH_Mesh::ExportMED( const char* file )
  throw (SALOME_Exception)
{
  Mesh_Writer* myWriter = new DriverMED_W_SMESHDS_Mesh;
  myWriter->SetFile( string(file) );
  myWriter->SetMesh( _myMeshDS );
  MESSAGE ( " _idDoc " << _idDoc )
  myWriter->SetMeshId( _idDoc );
  myWriter->Add();
}

void SMESH_Mesh::ExportDAT( const char* file )
  throw (SALOME_Exception)
{
  Mesh_Writer* myWriter = new DriverDAT_W_SMESHDS_Mesh;
  myWriter->SetFile( string(file) );
  myWriter->SetMesh( _myMeshDS );
  myWriter->SetMeshId( _idDoc );
  myWriter->Add();
}

void SMESH_Mesh::ExportUNV( const char* file )
  throw (SALOME_Exception)
{
  Mesh_Writer* myWriter = new DriverUNV_W_SMESHDS_Mesh;
  myWriter->SetFile( string(file) );
  myWriter->SetMesh( _myMeshDS );
  myWriter->SetMeshId( _idDoc );
  myWriter->Add();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
int SMESH_Mesh::NbNodes()
  throw (SALOME_Exception)
{
  return _myMeshDS->NbNodes();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
int SMESH_Mesh::NbEdges()
  throw (SALOME_Exception)
{
  return _myMeshDS->NbEdges();
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
int SMESH_Mesh::NbFaces()
  throw (SALOME_Exception)
{
  return _myMeshDS->NbFaces();
}
int SMESH_Mesh::NbTriangles()
  throw (SALOME_Exception)
{
  SMDS_MeshFacesIterator itFaces(_myMeshDS);
  int Nb = 0;
  for (;itFaces.More();itFaces.Next()) {
    const Handle(SMDS_MeshElement)& elem = itFaces.Value();

    switch (elem->NbNodes()) {
    case 3 : {
      Nb++;
      break;
    }
    }
  }
  return Nb;
}
int SMESH_Mesh::NbQuadrangles()
  throw (SALOME_Exception)
{
  SMDS_MeshFacesIterator itFaces(_myMeshDS);
  int Nb = 0;
  for (;itFaces.More();itFaces.Next()) {
    const Handle(SMDS_MeshElement)& elem = itFaces.Value();
    
    switch (elem->NbNodes()) {
    case 4 : {
      Nb++;
      break;
    }
    }
  }
  return Nb;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
int SMESH_Mesh::NbVolumes()
  throw (SALOME_Exception)
{
  return _myMeshDS->NbVolumes();
}
int SMESH_Mesh::NbTetras()
  throw (SALOME_Exception)
{
  int Nb = 0;
  SMDS_MeshVolumesIterator itVolumes(_myMeshDS);
  for (;itVolumes.More();itVolumes.Next()) {
    const Handle(SMDS_MeshElement)& elem = itVolumes.Value();

    switch (elem->NbNodes()) {
    case 4 : {
      Nb++;
      break;
    }
    }
  }
  return Nb;
}
int SMESH_Mesh::NbHexas()
  throw (SALOME_Exception)
{
  int Nb = 0;
  SMDS_MeshVolumesIterator itVolumes(_myMeshDS);
  for (;itVolumes.More();itVolumes.Next()) {
    const Handle(SMDS_MeshElement)& elem = itVolumes.Value();

    switch (elem->NbNodes()) {
    case 8 : {
      Nb++;
      break;
    }
    }
  }
  return Nb;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
int SMESH_Mesh::NbSubMesh()
  throw (SALOME_Exception)
{
  return _myMeshDS->NbSubMesh();
}
