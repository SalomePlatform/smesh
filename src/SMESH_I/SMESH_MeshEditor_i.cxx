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
//  File   : SMESH_MeshEditor_i.cxx
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#include "SMESH_MeshEditor_i.hxx"

#include "SMDS_MeshEdge.hxx"
#include "SMDS_MeshFace.hxx"
#include "SMDS_MeshVolume.hxx"

#include "SMESH_MeshEditor.hxx"

#include "SMESH_Gen_i.hxx"
#include "SMESH_Filter_i.hxx"
#include "SMESH_PythonDump.hxx"
#include "CASCatch.hxx"

#include "utilities.h"

#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <gp_Vec.hxx>

#include <sstream>

typedef map<const SMDS_MeshElement*,
            list<const SMDS_MeshElement*> > TElemOfElemListMap;

using namespace std;
using SMESH::TPythonDump;

//=============================================================================
/*!
 *
 */
//=============================================================================

SMESH_MeshEditor_i::SMESH_MeshEditor_i(SMESH_Mesh* theMesh)
{
	_myMesh = theMesh;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Boolean
  SMESH_MeshEditor_i::RemoveElements(const SMESH::long_array & IDsOfElements)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  ::SMESH_MeshEditor anEditor( _myMesh );
  list< int > IdList;

  for (int i = 0; i < IDsOfElements.length(); i++)
    IdList.push_back( IDsOfElements[i] );

  // Update Python script
  TPythonDump() << "isDone = " << this << ".RemoveElements( " << IDsOfElements << " )";
#ifdef _DEBUG_
  TPythonDump() << "print 'RemoveElements: ', isDone";
#endif
  // Remove Elements
  return anEditor.Remove( IdList, false );
}

//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::RemoveNodes(const SMESH::long_array & IDsOfNodes)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  ::SMESH_MeshEditor anEditor( _myMesh );
  list< int > IdList;
  for (int i = 0; i < IDsOfNodes.length(); i++)
    IdList.push_back( IDsOfNodes[i] );

  // Update Python script
  TPythonDump() << "isDone = " << this << ".RemoveNodes( " << IDsOfNodes << " )";
#ifdef _DEBUG_
  TPythonDump() << "print 'RemoveNodes: ', isDone";
#endif

  return anEditor.Remove( IdList, true );
}

//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Long SMESH_MeshEditor_i::AddEdge(const SMESH::long_array & IDsOfNodes)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  int NbNodes = IDsOfNodes.length();
  SMDS_MeshElement* elem = 0;
  if (NbNodes == 2)
  {
    CORBA::Long index1 = IDsOfNodes[0];
    CORBA::Long index2 = IDsOfNodes[1];
    elem = GetMeshDS()->AddEdge(GetMeshDS()->FindNode(index1), GetMeshDS()->FindNode(index2));

    // Update Python script
    TPythonDump() << "edge = " << this << ".AddEdge([ "
                  << index1 << ", " << index2 <<" ])";
  }
  if (NbNodes == 3) {
    CORBA::Long n1 = IDsOfNodes[0];
    CORBA::Long n2 = IDsOfNodes[1];
    CORBA::Long n12 = IDsOfNodes[2];
    elem = GetMeshDS()->AddEdge(GetMeshDS()->FindNode(n1),
                                GetMeshDS()->FindNode(n2),
                                GetMeshDS()->FindNode(n12));
    // Update Python script
    TPythonDump() << "edgeID = " << this << ".AddEdge([ "
                  <<n1<<", "<<n2<<", "<<n12<<" ])";
  }

  if(elem)
    return elem->GetID();

  return 0;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Long SMESH_MeshEditor_i::AddNode(CORBA::Double x,
                                        CORBA::Double y, CORBA::Double z)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  const SMDS_MeshNode* N = GetMeshDS()->AddNode(x, y, z);

  // Update Python script
  TPythonDump() << "nodeID = " << this << ".AddNode( "
                << x << ", " << y << ", " << z << " )";

  return N->GetID();
}

//=============================================================================
/*!
 *  AddFace
 */
//=============================================================================

CORBA::Long SMESH_MeshEditor_i::AddFace(const SMESH::long_array & IDsOfNodes)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  int NbNodes = IDsOfNodes.length();
  if (NbNodes < 3)
  {
    return false;
  }

  std::vector<const SMDS_MeshNode*> nodes (NbNodes);
  for (int i = 0; i < NbNodes; i++)
    nodes[i] = GetMeshDS()->FindNode(IDsOfNodes[i]);

  SMDS_MeshElement* elem = 0;
  if (NbNodes == 3) {
    elem = GetMeshDS()->AddFace(nodes[0], nodes[1], nodes[2]);
  }
  else if (NbNodes == 4) {
    elem = GetMeshDS()->AddFace(nodes[0], nodes[1], nodes[2], nodes[3]);
  }
  else if (NbNodes == 6) {
    elem = GetMeshDS()->AddFace(nodes[0], nodes[1], nodes[2], nodes[3],
                                nodes[4], nodes[5]);
  }
  else if (NbNodes == 8) {
    elem = GetMeshDS()->AddFace(nodes[0], nodes[1], nodes[2], nodes[3],
                                nodes[4], nodes[5], nodes[6], nodes[7]);
  }

  // Update Python script
  TPythonDump() << "faceID = " << this << ".AddFace( " << IDsOfNodes << " )";

  if(elem)
    return elem->GetID();

  return 0;
}

//=============================================================================
/*!
 *  AddPolygonalFace
 */
//=============================================================================
CORBA::Long SMESH_MeshEditor_i::AddPolygonalFace
                                   (const SMESH::long_array & IDsOfNodes)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  int NbNodes = IDsOfNodes.length();
  std::vector<const SMDS_MeshNode*> nodes (NbNodes);
  for (int i = 0; i < NbNodes; i++)
    nodes[i] = GetMeshDS()->FindNode(IDsOfNodes[i]);

  const SMDS_MeshElement* elem = GetMeshDS()->AddPolygonalFace(nodes);
  
  // Update Python script
  TPythonDump() <<"faceID = "<<this<<".AddPolygonalFace( "<<IDsOfNodes<<" )";
#ifdef _DEBUG_
  TPythonDump() << "print 'AddPolygonalFace: ', faceID";
#endif

  if(elem)
    return elem->GetID();

  return 0;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Long SMESH_MeshEditor_i::AddVolume(const SMESH::long_array & IDsOfNodes)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  int NbNodes = IDsOfNodes.length();
  vector< const SMDS_MeshNode*> n(NbNodes);
  for(int i=0;i<NbNodes;i++)
    n[i]=GetMeshDS()->FindNode(IDsOfNodes[i]);

  SMDS_MeshElement* elem = 0;
  switch(NbNodes)
  {
  case 4 :elem = GetMeshDS()->AddVolume(n[0],n[1],n[2],n[3]); break;
  case 5 :elem = GetMeshDS()->AddVolume(n[0],n[1],n[2],n[3],n[4]); break;
  case 6 :elem = GetMeshDS()->AddVolume(n[0],n[1],n[2],n[3],n[4],n[5]); break;
  case 8 :elem = GetMeshDS()->AddVolume(n[0],n[1],n[2],n[3],n[4],n[5],n[6],n[7]); break;
  case 10:elem = GetMeshDS()->AddVolume(n[0],n[1],n[2],n[3],n[4],n[5],
                                        n[6],n[7],n[8],n[9]);
    break;
  case 13:elem = GetMeshDS()->AddVolume(n[0],n[1],n[2],n[3],n[4],n[5],n[6],
                                        n[7],n[8],n[9],n[10],n[11],n[12]);
    break;
  case 15:elem = GetMeshDS()->AddVolume(n[0],n[1],n[2],n[3],n[4],n[5],n[6],n[7],n[8],
                                        n[9],n[10],n[11],n[12],n[13],n[14]);
    break;
  case 20:elem = GetMeshDS()->AddVolume(n[0],n[1],n[2],n[3],n[4],n[5],n[6],n[7],
                                        n[8],n[9],n[10],n[11],n[12],n[13],n[14],
                                        n[15],n[16],n[17],n[18],n[19]);
    break;
  }

  // Update Python script
  TPythonDump() << "volID = " << this << ".AddVolume( " << IDsOfNodes << " )";
#ifdef _DEBUG_
  TPythonDump() << "print 'AddVolume: ', volID";
#endif

  if(elem)
    return elem->GetID();

  return 0;
}

//=============================================================================
/*!
 *  AddPolyhedralVolume
 */
//=============================================================================
CORBA::Long SMESH_MeshEditor_i::AddPolyhedralVolume
                                   (const SMESH::long_array & IDsOfNodes,
                                    const SMESH::long_array & Quantities)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  int NbNodes = IDsOfNodes.length();
  std::vector<const SMDS_MeshNode*> n (NbNodes);
  for (int i = 0; i < NbNodes; i++)
    n[i] = GetMeshDS()->FindNode(IDsOfNodes[i]);

  int NbFaces = Quantities.length();
  std::vector<int> q (NbFaces);
  for (int j = 0; j < NbFaces; j++)
    q[j] = Quantities[j];

  const SMDS_MeshElement* elem = GetMeshDS()->AddPolyhedralVolume(n, q);

  // Update Python script
  TPythonDump() << "volID = " << this << ".AddPolyhedralVolume( "
                << IDsOfNodes << ", " << Quantities << " )";
#ifdef _DEBUG_
  TPythonDump() << "print 'AddPolyhedralVolume: ', volID";
#endif

  if(elem)
    return elem->GetID();

  return 0;
}

//=============================================================================
/*!
 *  AddPolyhedralVolumeByFaces
 */
//=============================================================================
CORBA::Long SMESH_MeshEditor_i::AddPolyhedralVolumeByFaces
                                   (const SMESH::long_array & IdsOfFaces)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  int NbFaces = IdsOfFaces.length();
  std::vector<const SMDS_MeshNode*> poly_nodes;
  std::vector<int> quantities (NbFaces);

  for (int i = 0; i < NbFaces; i++) {
    const SMDS_MeshElement* aFace = GetMeshDS()->FindElement(IdsOfFaces[i]);
    quantities[i] = aFace->NbNodes();

    SMDS_ElemIteratorPtr It = aFace->nodesIterator();
    while (It->more()) {
      poly_nodes.push_back(static_cast<const SMDS_MeshNode *>(It->next()));
    }
  }

  const SMDS_MeshElement* elem = GetMeshDS()->AddPolyhedralVolume(poly_nodes, quantities);

  // Update Python script
  TPythonDump() << "volID = " << this << ".AddPolyhedralVolumeByFaces( "
                << IdsOfFaces << " )";
#ifdef _DEBUG_
  TPythonDump() << "print 'AddPolyhedralVolume: ', volID";
#endif

  if(elem)
    return elem->GetID();

  return 0;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::MoveNode(CORBA::Long   NodeID,
                                            CORBA::Double x,
                                            CORBA::Double y,
                                            CORBA::Double z)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  const SMDS_MeshNode * node = GetMeshDS()->FindNode( NodeID );
  if ( !node )
    return false;

  GetMeshDS()->MoveNode(node, x, y, z);

  // Update Python script
  TPythonDump() << "isDone = " << this << ".MoveNode( "
                << NodeID << ", " << x << ", " << y << ", " << z << " )";

  return true;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::InverseDiag(CORBA::Long NodeID1,
                                               CORBA::Long NodeID2)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  const SMDS_MeshNode * n1 = GetMeshDS()->FindNode( NodeID1 );
  const SMDS_MeshNode * n2 = GetMeshDS()->FindNode( NodeID2 );
  if ( !n1 || !n2 )
    return false;

  // Update Python script
  TPythonDump() << "isDone = " << this << ".InverseDiag( "
                << NodeID1 << ", " << NodeID2 << " )";

  ::SMESH_MeshEditor aMeshEditor( _myMesh );
  return aMeshEditor.InverseDiag ( n1, n2 );
}

//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::DeleteDiag(CORBA::Long NodeID1,
                                              CORBA::Long NodeID2)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  const SMDS_MeshNode * n1 = GetMeshDS()->FindNode( NodeID1 );
  const SMDS_MeshNode * n2 = GetMeshDS()->FindNode( NodeID2 );
  if ( !n1 || !n2 )
    return false;

  // Update Python script
  TPythonDump() << "isDone = " << this << ".DeleteDiag( "
                << NodeID1 << ", " << NodeID2 <<  " )";

  ::SMESH_MeshEditor aMeshEditor( _myMesh );

  bool stat = aMeshEditor.DeleteDiag ( n1, n2 );

  UpdateLastResult(aMeshEditor);

  return stat;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::Reorient(const SMESH::long_array & IDsOfElements)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  ::SMESH_MeshEditor anEditor( _myMesh );
  for (int i = 0; i < IDsOfElements.length(); i++)
  {
    CORBA::Long index = IDsOfElements[i];
    const SMDS_MeshElement * elem = GetMeshDS()->FindElement(index);
    if ( elem )
      anEditor.Reorient( elem );
  }
  // Update Python script
  TPythonDump() << "isDone = " << this << ".Reorient( " << IDsOfElements << " )";

  return true;
}


//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Boolean SMESH_MeshEditor_i::ReorientObject(SMESH::SMESH_IDSource_ptr theObject)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  SMESH::long_array_var anElementsId = theObject->GetIDs();
  CORBA::Boolean isDone = Reorient(anElementsId);

  // Clear python line, created by Reorient()
  SMESH_Gen_i* aSMESHGen = SMESH_Gen_i::GetSMESHGen();
  aSMESHGen->RemoveLastFromPythonScript(aSMESHGen->GetCurrentStudyID());

  // Update Python script
  TPythonDump() << "isDone = " << this << ".ReorientObject( " << theObject << " )";

  return isDone;
}


//=======================================================================
//function : ToMap
//purpose  : auxilary function for conversion long_array to std::map<>
//           which is used in some methods
//=======================================================================
static void ToMap(const SMESH::long_array & IDs,
                  const SMESHDS_Mesh* aMesh,
                  std::map<int,const SMDS_MeshElement*>& aMap)
{ 
  for (int i=0; i<IDs.length(); i++) {
    CORBA::Long ind = IDs[i];
    std::map<int,const SMDS_MeshElement*>::iterator It = aMap.find(ind);
    if(It==aMap.end()) {
      const SMDS_MeshElement * elem = aMesh->FindElement(ind);
      aMap.insert( make_pair(ind,elem) );
    }
  }
}


//=============================================================================
/*!
 *
 */
//=============================================================================
CORBA::Boolean SMESH_MeshEditor_i::TriToQuad (const SMESH::long_array &   IDsOfElements,
                                              SMESH::NumericalFunctor_ptr Criterion,
                                              CORBA::Double               MaxAngle)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  SMESHDS_Mesh* aMesh = GetMeshDS();
  map<int,const SMDS_MeshElement*> faces;
  ToMap(IDsOfElements, aMesh, faces);

  SMESH::NumericalFunctor_i* aNumericalFunctor =
    dynamic_cast<SMESH::NumericalFunctor_i*>( SMESH_Gen_i::GetServant( Criterion ).in() );
  SMESH::Controls::NumericalFunctorPtr aCrit;
  if ( !aNumericalFunctor )
    aCrit.reset( new SMESH::Controls::AspectRatio() );
  else
    aCrit = aNumericalFunctor->GetNumericalFunctor();

  // Update Python script
  TPythonDump() << "isDone = " << this << ".TriToQuad( "
                << IDsOfElements << ", None, " << MaxAngle << " )";
#ifdef _DEBUG_
  TPythonDump() << "print 'TriToQuad: ', isDone";
#endif

  ::SMESH_MeshEditor anEditor( _myMesh );

  bool stat = anEditor.TriToQuad( faces, aCrit, MaxAngle );

  UpdateLastResult(anEditor);

  return stat;
}


//=============================================================================
/*!
 *
 */
//=============================================================================
CORBA::Boolean SMESH_MeshEditor_i::TriToQuadObject (SMESH::SMESH_IDSource_ptr   theObject,
                                                    SMESH::NumericalFunctor_ptr Criterion,
                                                    CORBA::Double               MaxAngle)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  SMESH::long_array_var anElementsId = theObject->GetIDs();
  CORBA::Boolean isDone = TriToQuad(anElementsId, Criterion, MaxAngle);

  // Clear python line(s), created by TriToQuad()
  SMESH_Gen_i* aSMESHGen = SMESH_Gen_i::GetSMESHGen();
  aSMESHGen->RemoveLastFromPythonScript(aSMESHGen->GetCurrentStudyID());
#ifdef _DEBUG_
  aSMESHGen->RemoveLastFromPythonScript(aSMESHGen->GetCurrentStudyID());
#endif

  // Update Python script
  TPythonDump() << "isDone = " << this << ".TriToQuadObject("
                << theObject << ", None, " << MaxAngle << " )";
#ifdef _DEBUG_
  TPythonDump() << "print 'TriToQuadObject: ', isDone";
#endif

  return isDone;
}


//=============================================================================
/*!
 *
 */
//=============================================================================
CORBA::Boolean SMESH_MeshEditor_i::QuadToTri (const SMESH::long_array &   IDsOfElements,
                                              SMESH::NumericalFunctor_ptr Criterion)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  SMESHDS_Mesh* aMesh = GetMeshDS();
  map<int,const SMDS_MeshElement*> faces;
  ToMap(IDsOfElements, aMesh, faces);

  SMESH::NumericalFunctor_i* aNumericalFunctor =
    dynamic_cast<SMESH::NumericalFunctor_i*>( SMESH_Gen_i::GetServant( Criterion ).in() );
  SMESH::Controls::NumericalFunctorPtr aCrit;
  if ( !aNumericalFunctor )
    aCrit.reset( new SMESH::Controls::AspectRatio() );
  else
    aCrit = aNumericalFunctor->GetNumericalFunctor();


  // Update Python script
  TPythonDump() << "isDone = " << this << ".QuadToTri( " << IDsOfElements << ", None )";
#ifdef _DEBUG_
  TPythonDump() << "print 'QuadToTri: ', isDone";
#endif

  ::SMESH_MeshEditor anEditor( _myMesh );
  CORBA::Boolean stat = anEditor.QuadToTri( faces, aCrit );

  UpdateLastResult(anEditor);

  return stat;
}


//=============================================================================
/*!
 *
 */
//=============================================================================
CORBA::Boolean SMESH_MeshEditor_i::QuadToTriObject (SMESH::SMESH_IDSource_ptr   theObject,
                                                    SMESH::NumericalFunctor_ptr Criterion)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  SMESH::long_array_var anElementsId = theObject->GetIDs();
  CORBA::Boolean isDone = QuadToTri(anElementsId, Criterion);

  // Clear python line(s), created by QuadToTri()
  SMESH_Gen_i* aSMESHGen = SMESH_Gen_i::GetSMESHGen();
  aSMESHGen->RemoveLastFromPythonScript(aSMESHGen->GetCurrentStudyID());
#ifdef _DEBUG_
  aSMESHGen->RemoveLastFromPythonScript(aSMESHGen->GetCurrentStudyID());
#endif

  // Update Python script
  TPythonDump() << "isDone = " << this << ".QuadToTriObject(" << theObject << ", None )";
#ifdef _DEBUG_
  TPythonDump() << "print 'QuadToTriObject: ', isDone";
#endif

  return isDone;
}


//=============================================================================
/*!
 *
 */
//=============================================================================
CORBA::Boolean SMESH_MeshEditor_i::SplitQuad (const SMESH::long_array & IDsOfElements,
                                              CORBA::Boolean            Diag13)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  SMESHDS_Mesh* aMesh = GetMeshDS();
  map<int,const SMDS_MeshElement*> faces;
  ToMap(IDsOfElements, aMesh, faces);

  // Update Python script
  TPythonDump() << "isDone = " << this << ".SplitQuad( "
                << IDsOfElements << ", " << Diag13 << " )";
#ifdef _DEBUG_
  TPythonDump() << "print 'SplitQuad: ', isDone";
#endif

  ::SMESH_MeshEditor anEditor( _myMesh );
  CORBA::Boolean stat = anEditor.QuadToTri( faces, Diag13 );

  UpdateLastResult(anEditor);

  return stat;
}


//=============================================================================
/*!
 *
 */
//=============================================================================
CORBA::Boolean SMESH_MeshEditor_i::SplitQuadObject (SMESH::SMESH_IDSource_ptr theObject,
                                                    CORBA::Boolean            Diag13)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  SMESH::long_array_var anElementsId = theObject->GetIDs();
  CORBA::Boolean isDone = SplitQuad(anElementsId, Diag13);

  // Clear python line(s), created by SplitQuad()
  SMESH_Gen_i* aSMESHGen = SMESH_Gen_i::GetSMESHGen();
  aSMESHGen->RemoveLastFromPythonScript(aSMESHGen->GetCurrentStudyID());
#ifdef _DEBUG_
  aSMESHGen->RemoveLastFromPythonScript(aSMESHGen->GetCurrentStudyID());
#endif

  // Update Python script
  TPythonDump() << "isDone = " << this << ".SplitQuadObject( "
                << theObject << ", " << Diag13 << " )";
#ifdef _DEBUG_
  TPythonDump() << "print 'SplitQuadObject: ', isDone";
#endif

  return isDone;
}


//=============================================================================
/*!
 *  BestSplit
 */
//=============================================================================
CORBA::Long SMESH_MeshEditor_i::BestSplit (CORBA::Long                 IDOfQuad,
                                           SMESH::NumericalFunctor_ptr Criterion)
{
  const SMDS_MeshElement* quad = GetMeshDS()->FindElement(IDOfQuad);
  if (quad && quad->GetType() == SMDSAbs_Face && quad->NbNodes() == 4)
  {
    SMESH::NumericalFunctor_i* aNumericalFunctor =
      dynamic_cast<SMESH::NumericalFunctor_i*>(SMESH_Gen_i::GetServant(Criterion).in());
    SMESH::Controls::NumericalFunctorPtr aCrit;
    if (aNumericalFunctor)
      aCrit = aNumericalFunctor->GetNumericalFunctor();
    else
      aCrit.reset(new SMESH::Controls::AspectRatio());

    ::SMESH_MeshEditor anEditor (_myMesh);
    return anEditor.BestSplit(quad, aCrit);
  }
  return -1;
}


//=======================================================================
//function : Smooth
//purpose  :
//=======================================================================

CORBA::Boolean
  SMESH_MeshEditor_i::Smooth(const SMESH::long_array &              IDsOfElements,
                             const SMESH::long_array &              IDsOfFixedNodes,
                             CORBA::Long                            MaxNbOfIterations,
                             CORBA::Double                          MaxAspectRatio,
                             SMESH::SMESH_MeshEditor::Smooth_Method Method)
{
  return smooth( IDsOfElements, IDsOfFixedNodes, MaxNbOfIterations,
                MaxAspectRatio, Method, false );
}


//=======================================================================
//function : SmoothParametric
//purpose  :
//=======================================================================

CORBA::Boolean
  SMESH_MeshEditor_i::SmoothParametric(const SMESH::long_array &              IDsOfElements,
                                       const SMESH::long_array &              IDsOfFixedNodes,
                                       CORBA::Long                            MaxNbOfIterations,
                                       CORBA::Double                          MaxAspectRatio,
                                       SMESH::SMESH_MeshEditor::Smooth_Method Method)
{
  return smooth( IDsOfElements, IDsOfFixedNodes, MaxNbOfIterations,
                MaxAspectRatio, Method, true );
}


//=======================================================================
//function : SmoothObject
//purpose  :
//=======================================================================

CORBA::Boolean
  SMESH_MeshEditor_i::SmoothObject(SMESH::SMESH_IDSource_ptr              theObject,
				   const SMESH::long_array &              IDsOfFixedNodes,
				   CORBA::Long                            MaxNbOfIterations,
				   CORBA::Double                          MaxAspectRatio,
				   SMESH::SMESH_MeshEditor::Smooth_Method Method)
{
  return smoothObject (theObject, IDsOfFixedNodes, MaxNbOfIterations,
                       MaxAspectRatio, Method, false);
}


//=======================================================================
//function : SmoothParametricObject
//purpose  :
//=======================================================================

CORBA::Boolean
  SMESH_MeshEditor_i::SmoothParametricObject(SMESH::SMESH_IDSource_ptr              theObject,
				   const SMESH::long_array &              IDsOfFixedNodes,
				   CORBA::Long                            MaxNbOfIterations,
				   CORBA::Double                          MaxAspectRatio,
				   SMESH::SMESH_MeshEditor::Smooth_Method Method)
{
  return smoothObject (theObject, IDsOfFixedNodes, MaxNbOfIterations,
                       MaxAspectRatio, Method, true);
}


//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Boolean
  SMESH_MeshEditor_i::smooth(const SMESH::long_array &              IDsOfElements,
                             const SMESH::long_array &              IDsOfFixedNodes,
                             CORBA::Long                            MaxNbOfIterations,
                             CORBA::Double                          MaxAspectRatio,
                             SMESH::SMESH_MeshEditor::Smooth_Method Method,
                             bool                                   IsParametric)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  SMESHDS_Mesh* aMesh = GetMeshDS();

  map<int,const SMDS_MeshElement*> elements;
  ToMap(IDsOfElements, aMesh, elements);

  set<const SMDS_MeshNode*> fixedNodes;
  for (int i = 0; i < IDsOfFixedNodes.length(); i++) {
    CORBA::Long index = IDsOfFixedNodes[i];
    const SMDS_MeshNode * node = aMesh->FindNode(index);
    if ( node )
      fixedNodes.insert( node );
  }
  ::SMESH_MeshEditor::SmoothMethod method = ::SMESH_MeshEditor::LAPLACIAN;
  if ( Method != SMESH::SMESH_MeshEditor::LAPLACIAN_SMOOTH )
    method = ::SMESH_MeshEditor::CENTROIDAL;

  ::SMESH_MeshEditor anEditor( _myMesh );
  anEditor.Smooth(elements, fixedNodes, method,
                  MaxNbOfIterations, MaxAspectRatio, IsParametric );

  UpdateLastResult(anEditor);

  // Update Python script
  TPythonDump() << "isDone = " << this << "."
                << (IsParametric ? "SmoothParametric( " : "Smooth( ")
                << IDsOfElements << ", "     << IDsOfFixedNodes << ", "
                << MaxNbOfIterations << ", " << MaxAspectRatio << ", "
                << "SMESH.SMESH_MeshEditor."
                << ( Method == SMESH::SMESH_MeshEditor::CENTROIDAL_SMOOTH ?
                     "CENTROIDAL_SMOOTH )" : "LAPLACIAN_SMOOTH )");
#ifdef _DEBUG_
  TPythonDump() << "print 'Smooth: ', isDone";
#endif

  return true;
}


//=============================================================================
/*!
 *
 */
//=============================================================================

CORBA::Boolean
SMESH_MeshEditor_i::smoothObject(SMESH::SMESH_IDSource_ptr              theObject,
                                 const SMESH::long_array &              IDsOfFixedNodes,
                                 CORBA::Long                            MaxNbOfIterations,
                                 CORBA::Double                          MaxAspectRatio,
                                 SMESH::SMESH_MeshEditor::Smooth_Method Method,
                                 bool                                   IsParametric)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  SMESH::long_array_var anElementsId = theObject->GetIDs();
  CORBA::Boolean isDone = smooth (anElementsId, IDsOfFixedNodes, MaxNbOfIterations,
                                  MaxAspectRatio, Method, IsParametric);

  // Clear python line(s), created by Smooth()
  SMESH_Gen_i* aSMESHGen = SMESH_Gen_i::GetSMESHGen();
  aSMESHGen->RemoveLastFromPythonScript(aSMESHGen->GetCurrentStudyID());
#ifdef _DEBUG_
  aSMESHGen->RemoveLastFromPythonScript(aSMESHGen->GetCurrentStudyID());
#endif

  // Update Python script
  TPythonDump() << "isDone = " << this << "."
                << (IsParametric ? "SmoothParametricObject( " : "SmoothObject( ")
                << theObject << ", " << IDsOfFixedNodes << ", "
                << MaxNbOfIterations << ", " << MaxAspectRatio << ", "
                << "SMESH.SMESH_MeshEditor."
                << ( Method == SMESH::SMESH_MeshEditor::CENTROIDAL_SMOOTH ?
                     "CENTROIDAL_SMOOTH )" : "LAPLACIAN_SMOOTH )");
#ifdef _DEBUG_
  TPythonDump() << "print 'SmoothObject: ', isDone";
#endif

  return isDone;
}


//=============================================================================
/*!
 *
 */
//=============================================================================

void SMESH_MeshEditor_i::RenumberNodes()
{
  // Update Python script
  TPythonDump() << this << ".RenumberNodes()";

  GetMeshDS()->Renumber( true );
}


//=============================================================================
/*!
 *
 */
//=============================================================================

void SMESH_MeshEditor_i::RenumberElements()
{
  // Update Python script
  TPythonDump() << this << ".RenumberElements()";

  GetMeshDS()->Renumber( false );
}


//=======================================================================
//function : RotationSweep
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::RotationSweep(const SMESH::long_array & theIDsOfElements,
                                       const SMESH::AxisStruct & theAxis,
                                       CORBA::Double             theAngleInRadians,
                                       CORBA::Long               theNbOfSteps,
                                       CORBA::Double             theTolerance)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  SMESHDS_Mesh* aMesh = GetMeshDS();

  map<int,const SMDS_MeshElement*> elements;
  ToMap(theIDsOfElements, aMesh, elements);

  gp_Ax1 Ax1 (gp_Pnt( theAxis.x, theAxis.y, theAxis.z ),
              gp_Vec( theAxis.vx, theAxis.vy, theAxis.vz ));

  ::SMESH_MeshEditor anEditor( _myMesh );
  anEditor.RotationSweep (elements, Ax1, theAngleInRadians,
                          theNbOfSteps, theTolerance);

  UpdateLastResult(anEditor);

  // Update Python script
  TPythonDump() << "axis = " << theAxis;
  TPythonDump() << this << ".RotationSweep( "
                << theIDsOfElements
                << ", axis, "
                << theAngleInRadians << ", "
                << theNbOfSteps << ", "
                << theTolerance << " )";
}

//=======================================================================
//function : RotationSweepObject
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::RotationSweepObject(SMESH::SMESH_IDSource_ptr theObject,
					     const SMESH::AxisStruct & theAxis,
					     CORBA::Double             theAngleInRadians,
					     CORBA::Long               theNbOfSteps,
					     CORBA::Double             theTolerance)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  SMESH::long_array_var anElementsId = theObject->GetIDs();
  RotationSweep(anElementsId, theAxis, theAngleInRadians, theNbOfSteps, theTolerance);

  // Clear python line, created by RotationSweep()
  SMESH_Gen_i* aSMESHGen = SMESH_Gen_i::GetSMESHGen();
  aSMESHGen->RemoveLastFromPythonScript(aSMESHGen->GetCurrentStudyID());

  // Update Python script
  TPythonDump() << this << ".RotationSweepObject( "
                << theObject
                << ", axis, "
                << theAngleInRadians << ", "
                << theNbOfSteps << ", "
                << theTolerance << " )";
}

//=======================================================================
//function : ExtrusionSweep
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::ExtrusionSweep(const SMESH::long_array & theIDsOfElements,
                                        const SMESH::DirStruct &  theStepVector,
                                        CORBA::Long               theNbOfSteps)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  CASCatch_TRY {   
    SMESHDS_Mesh* aMesh = GetMeshDS();
    
    map<int,const SMDS_MeshElement*> elements;
    ToMap(theIDsOfElements, aMesh, elements);

    const SMESH::PointStruct * P = &theStepVector.PS;
    gp_Vec stepVec( P->x, P->y, P->z );
    
    TElemOfElemListMap aHystory;
    ::SMESH_MeshEditor anEditor( _myMesh );
    anEditor.ExtrusionSweep (elements, stepVec, theNbOfSteps, aHystory);

    UpdateLastResult(anEditor);

    // Update Python script
    TPythonDump() << "stepVector = " << theStepVector;
    TPythonDump() << this << ".ExtrusionSweep( "
		  << theIDsOfElements << ", stepVector, " << theNbOfSteps << " )";

  }
  CASCatch_CATCH(Standard_Failure) {
    Handle(Standard_Failure) aFail = Standard_Failure::Caught();          
    INFOS( "SMESH_MeshEditor_i::ExtrusionSweep fails - "<< aFail->GetMessageString() );
  }
}


//=======================================================================
//function : ExtrusionSweepObject
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::ExtrusionSweepObject(SMESH::SMESH_IDSource_ptr theObject,
					      const SMESH::DirStruct &  theStepVector,
					      CORBA::Long               theNbOfSteps)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  SMESH::long_array_var anElementsId = theObject->GetIDs();
  ExtrusionSweep(anElementsId, theStepVector, theNbOfSteps);

  // Clear python line, created by ExtrusionSweep()
  SMESH_Gen_i* aSMESHGen = SMESH_Gen_i::GetSMESHGen();
  aSMESHGen->RemoveLastFromPythonScript(aSMESHGen->GetCurrentStudyID());

  // Update Python script
  TPythonDump() << this << ".ExtrusionSweepObject( "
                << theObject << ", stepVector, " << theNbOfSteps << " )";
}

//=======================================================================
//function : ExtrusionSweepObject1D
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::ExtrusionSweepObject1D(SMESH::SMESH_IDSource_ptr theObject,
                                                const SMESH::DirStruct &  theStepVector,
                                                CORBA::Long               theNbOfSteps)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  SMESHDS_Mesh* aMesh = GetMeshDS();

  SMESH::long_array_var allElementsId = theObject->GetIDs();

  map<int,const SMDS_MeshElement*> elements;
  ToMap(allElementsId, aMesh, elements);

  const SMESH::PointStruct * P = &theStepVector.PS;
  gp_Vec stepVec( P->x, P->y, P->z );

  ::SMESH_MeshEditor anEditor( _myMesh );
  //anEditor.ExtrusionSweep (elements, stepVec, theNbOfSteps);
  TElemOfElemListMap aHystory;
  anEditor.ExtrusionSweep (elements, stepVec, theNbOfSteps, aHystory);

  UpdateLastResult(anEditor);

  // Update Python script
  TPythonDump() << "stepVector = " << theStepVector;
  TPythonDump() << this << ".ExtrusionSweepObject1D( "
                << theObject << ", stepVector, " << theNbOfSteps << " )";
}

//=======================================================================
//function : ExtrusionSweepObject2D
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::ExtrusionSweepObject2D(SMESH::SMESH_IDSource_ptr theObject,
                                                const SMESH::DirStruct &  theStepVector,
                                                CORBA::Long               theNbOfSteps)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  SMESHDS_Mesh* aMesh = GetMeshDS();

  SMESH::long_array_var allElementsId = theObject->GetIDs();

  map<int,const SMDS_MeshElement*> elements;
  ToMap(allElementsId, aMesh, elements);

  const SMESH::PointStruct * P = &theStepVector.PS;
  gp_Vec stepVec( P->x, P->y, P->z );

  ::SMESH_MeshEditor anEditor( _myMesh );
  //anEditor.ExtrusionSweep (elements, stepVec, theNbOfSteps);
  TElemOfElemListMap aHystory;
  anEditor.ExtrusionSweep (elements, stepVec, theNbOfSteps, aHystory);

  UpdateLastResult(anEditor);

  // Update Python script
  TPythonDump() << "stepVector = " << theStepVector;
  TPythonDump() << this << ".ExtrusionSweepObject2D( "
                << theObject << ", stepVector, " << theNbOfSteps << " )";
}


//=======================================================================
//function : AdvancedExtrusion
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::AdvancedExtrusion(const SMESH::long_array & theIDsOfElements,
					   const SMESH::DirStruct &  theStepVector,
					   CORBA::Long               theNbOfSteps,
					   CORBA::Long               theExtrFlags,
					   CORBA::Double             theSewTolerance)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  SMESHDS_Mesh* aMesh = GetMeshDS();

  map<int,const SMDS_MeshElement*> elements;
  ToMap(theIDsOfElements, aMesh, elements);

  const SMESH::PointStruct * P = &theStepVector.PS;
  gp_Vec stepVec( P->x, P->y, P->z );

  ::SMESH_MeshEditor anEditor( _myMesh );
  TElemOfElemListMap aHystory;
  anEditor.ExtrusionSweep (elements, stepVec, theNbOfSteps, aHystory,
			   theExtrFlags, theSewTolerance);

  UpdateLastResult(anEditor);

  // Update Python script
  TPythonDump() << "stepVector = " << theStepVector;
  TPythonDump() << this << ".AdvancedExtrusion("
                << theIDsOfElements
                << ", stepVector, "
                << theNbOfSteps << ","
                << theExtrFlags << ", "
                << theSewTolerance <<  " )";
}


#define RETCASE(enm) case ::SMESH_MeshEditor::enm: return SMESH::SMESH_MeshEditor::enm;

static SMESH::SMESH_MeshEditor::Extrusion_Error convExtrError( const::SMESH_MeshEditor::Extrusion_Error e )
{
  switch ( e ) {
  RETCASE( EXTR_OK );
  RETCASE( EXTR_NO_ELEMENTS );
  RETCASE( EXTR_PATH_NOT_EDGE );
  RETCASE( EXTR_BAD_PATH_SHAPE );
  RETCASE( EXTR_BAD_STARTING_NODE );
  RETCASE( EXTR_BAD_ANGLES_NUMBER );
  RETCASE( EXTR_CANT_GET_TANGENT );
  }
  return SMESH::SMESH_MeshEditor::EXTR_OK;
}

//=======================================================================
//function : ExtrusionAlongPath
//purpose  :
//=======================================================================

SMESH::SMESH_MeshEditor::Extrusion_Error
  SMESH_MeshEditor_i::ExtrusionAlongPath(const SMESH::long_array &   theIDsOfElements,
					 SMESH::SMESH_Mesh_ptr       thePathMesh,
					 GEOM::GEOM_Object_ptr       thePathShape,
					 CORBA::Long                 theNodeStart,
					 CORBA::Boolean              theHasAngles,
					 const SMESH::double_array & theAngles,
					 CORBA::Boolean              theHasRefPoint,
					 const SMESH::PointStruct &  theRefPoint)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  SMESHDS_Mesh*  aMesh = GetMeshDS();

  if ( thePathMesh->_is_nil() || thePathShape->_is_nil() )
    return SMESH::SMESH_MeshEditor::EXTR_BAD_PATH_SHAPE;

  SMESH_Mesh_i* aMeshImp = dynamic_cast<SMESH_Mesh_i*>( SMESH_Gen_i::GetServant( thePathMesh ).in() );
  TopoDS_Shape aShape = SMESH_Gen_i::GetSMESHGen()->GeomObjectToShape( thePathShape );
  SMESH_subMesh* aSubMesh = aMeshImp->GetImpl().GetSubMesh( aShape );

  if ( !aSubMesh || !aSubMesh->GetSubMeshDS())
    return SMESH::SMESH_MeshEditor::EXTR_BAD_PATH_SHAPE;

  SMDS_MeshNode* nodeStart = (SMDS_MeshNode*)aMeshImp->GetImpl().GetMeshDS()->FindNode(theNodeStart);
  if ( !nodeStart )
    return SMESH::SMESH_MeshEditor::EXTR_BAD_STARTING_NODE;

  map<int,const SMDS_MeshElement*> elements;
  ToMap(theIDsOfElements, aMesh, elements);

  list<double> angles;
  for (int i = 0; i < theAngles.length(); i++) {
    angles.push_back( theAngles[i] );
  }

  gp_Pnt refPnt( theRefPoint.x, theRefPoint.y, theRefPoint.z );

  // Update Python script
  TPythonDump() << "refPoint = SMESH.PointStruct( "
                << refPnt.X() << ", "
                << refPnt.Y() << ", "
                << refPnt.Z() << " )";
  TPythonDump() << "error = " << this << ".ExtrusionAlongPath( "
                << theIDsOfElements << ", "
                << thePathMesh  <<  ", "
                << thePathShape <<  ", "
                << theNodeStart << ", "
                << theHasAngles << ", "
                << theAngles << ", "
                << theHasRefPoint << ", refPoint )";

  ::SMESH_MeshEditor anEditor( _myMesh );
  SMESH::SMESH_MeshEditor::Extrusion_Error error = 
    convExtrError( anEditor.ExtrusionAlongTrack( elements, aSubMesh, nodeStart,
                                                theHasAngles, angles,
                                                theHasRefPoint, refPnt ) );

  UpdateLastResult(anEditor);

  return error;
}

//=======================================================================
//function : ExtrusionAlongPathObject
//purpose  :
//=======================================================================

SMESH::SMESH_MeshEditor::Extrusion_Error
SMESH_MeshEditor_i::ExtrusionAlongPathObject(SMESH::SMESH_IDSource_ptr   theObject,
                                             SMESH::SMESH_Mesh_ptr       thePathMesh,
                                             GEOM::GEOM_Object_ptr       thePathShape,
                                             CORBA::Long                 theNodeStart,
                                             CORBA::Boolean              theHasAngles,
                                             const SMESH::double_array & theAngles,
                                             CORBA::Boolean              theHasRefPoint,
                                             const SMESH::PointStruct &  theRefPoint)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  SMESH::long_array_var anElementsId = theObject->GetIDs();
  SMESH::SMESH_MeshEditor::Extrusion_Error error = ExtrusionAlongPath
    (anElementsId, thePathMesh, thePathShape, theNodeStart,
     theHasAngles, theAngles, theHasRefPoint, theRefPoint);

  // Clear python line, created by ExtrusionAlongPath()
  SMESH_Gen_i* aSMESHGen = SMESH_Gen_i::GetSMESHGen();
  aSMESHGen->RemoveLastFromPythonScript(aSMESHGen->GetCurrentStudyID());

  // Update Python script
  TPythonDump() << "error = " << this << ".ExtrusionAlongPathObject( "
                << theObject    << ", "
                << thePathMesh  << ", "
                << thePathShape << ", "
                << theNodeStart << ", "
                << theHasAngles << ", "
                << theAngles << ", "
                << theHasRefPoint << ", refPoint )";

  return error;
}

//=======================================================================
//function : Mirror
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::Mirror(const SMESH::long_array &           theIDsOfElements,
                                const SMESH::AxisStruct &           theAxis,
                                SMESH::SMESH_MeshEditor::MirrorType theMirrorType,
                                CORBA::Boolean                      theCopy)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  SMESHDS_Mesh* aMesh = GetMeshDS();

  map<int,const SMDS_MeshElement*> elements;
  ToMap(theIDsOfElements, aMesh, elements);

  gp_Pnt P ( theAxis.x, theAxis.y, theAxis.z );
  gp_Vec V ( theAxis.vx, theAxis.vy, theAxis.vz );

  gp_Trsf aTrsf;
  TCollection_AsciiString typeStr;
  switch ( theMirrorType ) {
  case  SMESH::SMESH_MeshEditor::POINT:
    aTrsf.SetMirror( P );
    typeStr = "SMESH.SMESH_MeshEditor.POINT";
    break;
  case  SMESH::SMESH_MeshEditor::AXIS:
    aTrsf.SetMirror( gp_Ax1( P, V ));
    typeStr = "SMESH.SMESH_MeshEditor.AXIS";
    break;
  default:
    aTrsf.SetMirror( gp_Ax2( P, V ));
    typeStr = "SMESH.SMESH_MeshEditor.PLANE";
  }

  // Update Python script
  TPythonDump() << this << ".Mirror( "
                << theIDsOfElements << ", "
                << theAxis           << ", "
                << typeStr           << ", "
                << theCopy           << " )";

  ::SMESH_MeshEditor anEditor( _myMesh );
  anEditor.Transform (elements, aTrsf, theCopy);

  if(theCopy) {
    UpdateLastResult(anEditor);
  }
}


//=======================================================================
//function : MirrorObject
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::MirrorObject(SMESH::SMESH_IDSource_ptr           theObject,
				      const SMESH::AxisStruct &           theAxis,
				      SMESH::SMESH_MeshEditor::MirrorType theMirrorType,
				      CORBA::Boolean                      theCopy)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  SMESH::long_array_var anElementsId = theObject->GetIDs();
  Mirror(anElementsId, theAxis, theMirrorType, theCopy);

  // Clear python line, created by Mirror()
  SMESH_Gen_i* aSMESHGen = SMESH_Gen_i::GetSMESHGen();
  aSMESHGen->RemoveLastFromPythonScript(aSMESHGen->GetCurrentStudyID());

  // Update Python script
  TCollection_AsciiString typeStr;
  switch ( theMirrorType ) {
  case  SMESH::SMESH_MeshEditor::POINT:
    typeStr = "SMESH.SMESH_MeshEditor.POINT";
    break;
  case  SMESH::SMESH_MeshEditor::AXIS:
    typeStr = "SMESH.SMESH_MeshEditor.AXIS";
    break;
  default:
    typeStr = "SMESH.SMESH_MeshEditor.PLANE";
  }
  TPythonDump() << "axis = " << theAxis;
  TPythonDump() << this << ".MirrorObject( "
                << theObject << ", "
                << "axis, "
                << typeStr << ", "
                << theCopy << " )";
}

//=======================================================================
//function : Translate
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::Translate(const SMESH::long_array & theIDsOfElements,
                                   const SMESH::DirStruct &  theVector,
                                   CORBA::Boolean            theCopy)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  SMESHDS_Mesh* aMesh = GetMeshDS();

  map<int,const SMDS_MeshElement*> elements;
  ToMap(theIDsOfElements, aMesh, elements);

  gp_Trsf aTrsf;
  const SMESH::PointStruct * P = &theVector.PS;
  aTrsf.SetTranslation( gp_Vec( P->x, P->y, P->z ));

  ::SMESH_MeshEditor anEditor( _myMesh );
  anEditor.Transform (elements, aTrsf, theCopy);

  if(theCopy) {
    UpdateLastResult(anEditor);
  }

  // Update Python script
  TPythonDump() << "vector = " << theVector;
  TPythonDump() << this << ".Translate( "
                << theIDsOfElements
                << ", vector, "
                << theCopy << " )";
}

//=======================================================================
//function : TranslateObject
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::TranslateObject(SMESH::SMESH_IDSource_ptr theObject,
					 const SMESH::DirStruct &  theVector,
					 CORBA::Boolean            theCopy)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  SMESH::long_array_var anElementsId = theObject->GetIDs();
  Translate(anElementsId, theVector, theCopy);

  // Clear python line, created by Translate()
  SMESH_Gen_i* aSMESHGen = SMESH_Gen_i::GetSMESHGen();
  aSMESHGen->RemoveLastFromPythonScript(aSMESHGen->GetCurrentStudyID());

  // Update Python script
  TPythonDump() << this << ".TranslateObject( "
                << theObject
                << ", vector, "
                << theCopy << " )";
}

//=======================================================================
//function : Rotate
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::Rotate(const SMESH::long_array & theIDsOfElements,
                                const SMESH::AxisStruct & theAxis,
                                CORBA::Double             theAngle,
                                CORBA::Boolean            theCopy)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  SMESHDS_Mesh* aMesh = GetMeshDS();

  map<int,const SMDS_MeshElement*> elements;
  ToMap(theIDsOfElements, aMesh, elements);

  gp_Pnt P ( theAxis.x, theAxis.y, theAxis.z );
  gp_Vec V ( theAxis.vx, theAxis.vy, theAxis.vz );

  gp_Trsf aTrsf;
  aTrsf.SetRotation( gp_Ax1( P, V ), theAngle);

  ::SMESH_MeshEditor anEditor( _myMesh );
  anEditor.Transform (elements, aTrsf, theCopy);

  if(theCopy) {
    UpdateLastResult(anEditor);
  }

  // Update Python script
  TPythonDump() << "axis = " << theAxis;
  TPythonDump() << this << ".Rotate( "
                << theIDsOfElements
                << ", axis, "
                << theAngle << ", "
                << theCopy << " )";
}

//=======================================================================
//function : RotateObject
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::RotateObject(SMESH::SMESH_IDSource_ptr theObject,
				      const SMESH::AxisStruct & theAxis,
				      CORBA::Double             theAngle,
				      CORBA::Boolean            theCopy)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  SMESH::long_array_var anElementsId = theObject->GetIDs();
  Rotate(anElementsId, theAxis, theAngle, theCopy);

  // Clear python line, created by Rotate()
  SMESH_Gen_i* aSMESHGen = SMESH_Gen_i::GetSMESHGen();
  aSMESHGen->RemoveLastFromPythonScript(aSMESHGen->GetCurrentStudyID());

  // Update Python script
  TPythonDump() << this << ".RotateObject( "
                << theObject
                << ", axis, "
                << theAngle << ", "
                << theCopy << " )";
}

//=======================================================================
//function : FindCoincidentNodes
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::FindCoincidentNodes (CORBA::Double                  Tolerance,
                                              SMESH::array_of_long_array_out GroupsOfNodes)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  ::SMESH_MeshEditor::TListOfListOfNodes aListOfListOfNodes;
  ::SMESH_MeshEditor anEditor( _myMesh );
  set<const SMDS_MeshNode*> nodes; // no input nodes
  anEditor.FindCoincidentNodes( nodes, Tolerance, aListOfListOfNodes );

  GroupsOfNodes = new SMESH::array_of_long_array;
  GroupsOfNodes->length( aListOfListOfNodes.size() );
  ::SMESH_MeshEditor::TListOfListOfNodes::iterator llIt = aListOfListOfNodes.begin();
  for ( CORBA::Long i = 0; llIt != aListOfListOfNodes.end(); llIt++, i++ ) {
    list< const SMDS_MeshNode* >& aListOfNodes = *llIt;
    list< const SMDS_MeshNode* >::iterator lIt = aListOfNodes.begin();;
    SMESH::long_array& aGroup = GroupsOfNodes[ i ];
    aGroup.length( aListOfNodes.size() );
    for ( int j = 0; lIt != aListOfNodes.end(); lIt++, j++ )
      aGroup[ j ] = (*lIt)->GetID();
  }
  // Update Python script
  TPythonDump() << "coincident_nodes = " << this << ".FindCoincidentNodes( "
                << Tolerance << " )";
}

//=======================================================================
//function : MergeNodes
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::MergeNodes (const SMESH::array_of_long_array& GroupsOfNodes)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  SMESHDS_Mesh* aMesh = GetMeshDS();

  TPythonDump aTPythonDump;
  aTPythonDump << this << ".MergeNodes([";
  ::SMESH_MeshEditor::TListOfListOfNodes aListOfListOfNodes;
  for (int i = 0; i < GroupsOfNodes.length(); i++)
  {
    const SMESH::long_array& aNodeGroup = GroupsOfNodes[ i ];
    aListOfListOfNodes.push_back( list< const SMDS_MeshNode* >() );
    list< const SMDS_MeshNode* >& aListOfNodes = aListOfListOfNodes.back();
    for ( int j = 0; j < aNodeGroup.length(); j++ )
    {
      CORBA::Long index = aNodeGroup[ j ];
      const SMDS_MeshNode * node = aMesh->FindNode(index);
      if ( node )
        aListOfNodes.push_back( node );
    }
    if ( aListOfNodes.size() < 2 )
      aListOfListOfNodes.pop_back();

    if ( i > 0 ) aTPythonDump << ", ";
    aTPythonDump << aNodeGroup;
  }
  ::SMESH_MeshEditor anEditor( _myMesh );
  anEditor.MergeNodes( aListOfListOfNodes );

  // Update Python script
  aTPythonDump <<  "])";
}

//=======================================================================
//function : MergeEqualElements
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::MergeEqualElements()
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  ::SMESH_MeshEditor anEditor( _myMesh );
  anEditor.MergeEqualElements();

  // Update Python script
  TPythonDump() << this << ".MergeEqualElements()";
}

//=======================================================================
//function : operator
//purpose  :
//=======================================================================

#define RETCASE(enm) case ::SMESH_MeshEditor::enm: return SMESH::SMESH_MeshEditor::enm;

static SMESH::SMESH_MeshEditor::Sew_Error convError( const::SMESH_MeshEditor::Sew_Error e )
{
  switch ( e ) {
  RETCASE( SEW_OK );
  RETCASE( SEW_BORDER1_NOT_FOUND );
  RETCASE( SEW_BORDER2_NOT_FOUND );
  RETCASE( SEW_BOTH_BORDERS_NOT_FOUND );
  RETCASE( SEW_BAD_SIDE_NODES );
  RETCASE( SEW_VOLUMES_TO_SPLIT );
  RETCASE( SEW_DIFF_NB_OF_ELEMENTS );
  RETCASE( SEW_TOPO_DIFF_SETS_OF_ELEMENTS );
  RETCASE( SEW_BAD_SIDE1_NODES );
  RETCASE( SEW_BAD_SIDE2_NODES );
  }
  return SMESH::SMESH_MeshEditor::SEW_OK;
}

//=======================================================================
//function : SewFreeBorders
//purpose  :
//=======================================================================

SMESH::SMESH_MeshEditor::Sew_Error
  SMESH_MeshEditor_i::SewFreeBorders(CORBA::Long FirstNodeID1,
                                     CORBA::Long SecondNodeID1,
                                     CORBA::Long LastNodeID1,
                                     CORBA::Long FirstNodeID2,
                                     CORBA::Long SecondNodeID2,
                                     CORBA::Long LastNodeID2,
                                     CORBA::Boolean CreatePolygons,
                                     CORBA::Boolean CreatePolyedrs)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  SMESHDS_Mesh* aMesh = GetMeshDS();

  const SMDS_MeshNode* aBorderFirstNode  = aMesh->FindNode( FirstNodeID1  );
  const SMDS_MeshNode* aBorderSecondNode = aMesh->FindNode( SecondNodeID1 );
  const SMDS_MeshNode* aBorderLastNode   = aMesh->FindNode( LastNodeID1   );
  const SMDS_MeshNode* aSide2FirstNode   = aMesh->FindNode( FirstNodeID2  );
  const SMDS_MeshNode* aSide2SecondNode  = aMesh->FindNode( SecondNodeID2 );
  const SMDS_MeshNode* aSide2ThirdNode   = aMesh->FindNode( LastNodeID2   );

  if (!aBorderFirstNode ||
      !aBorderSecondNode||
      !aBorderLastNode)
    return SMESH::SMESH_MeshEditor::SEW_BORDER1_NOT_FOUND;
  if (!aSide2FirstNode  ||
      !aSide2SecondNode ||
      !aSide2ThirdNode)
    return SMESH::SMESH_MeshEditor::SEW_BORDER2_NOT_FOUND;

  // Update Python script
  TPythonDump() << "error = " << this << ".SewFreeBorders( "
                << FirstNodeID1  << ", "
                << SecondNodeID1 << ", "
                << LastNodeID1   << ", "
                << FirstNodeID2  << ", "
                << SecondNodeID2 << ", "
                << LastNodeID2   << ", "
                << CreatePolygons<< ", "
                << CreatePolyedrs<< " )";

  ::SMESH_MeshEditor anEditor( _myMesh );
  SMESH::SMESH_MeshEditor::Sew_Error error =
    convError( anEditor.SewFreeBorder (aBorderFirstNode,
                                       aBorderSecondNode,
                                       aBorderLastNode,
                                       aSide2FirstNode,
                                       aSide2SecondNode,
                                       aSide2ThirdNode,
                                       true,
                                       CreatePolygons,
                                       CreatePolyedrs) );

  UpdateLastResult(anEditor);

  return error;
}


//=======================================================================
//function : SewConformFreeBorders
//purpose  :
//=======================================================================

SMESH::SMESH_MeshEditor::Sew_Error
SMESH_MeshEditor_i::SewConformFreeBorders(CORBA::Long FirstNodeID1,
                                          CORBA::Long SecondNodeID1,
                                          CORBA::Long LastNodeID1,
                                          CORBA::Long FirstNodeID2,
                                          CORBA::Long SecondNodeID2)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  SMESHDS_Mesh* aMesh = GetMeshDS();

  const SMDS_MeshNode* aBorderFirstNode  = aMesh->FindNode( FirstNodeID1  );
  const SMDS_MeshNode* aBorderSecondNode = aMesh->FindNode( SecondNodeID1 );
  const SMDS_MeshNode* aBorderLastNode   = aMesh->FindNode( LastNodeID1   );
  const SMDS_MeshNode* aSide2FirstNode   = aMesh->FindNode( FirstNodeID2  );
  const SMDS_MeshNode* aSide2SecondNode  = aMesh->FindNode( SecondNodeID2 );
  const SMDS_MeshNode* aSide2ThirdNode   = 0;

  if (!aBorderFirstNode ||
      !aBorderSecondNode||
      !aBorderLastNode )
    return SMESH::SMESH_MeshEditor::SEW_BORDER1_NOT_FOUND;
  if (!aSide2FirstNode  ||
      !aSide2SecondNode)
    return SMESH::SMESH_MeshEditor::SEW_BORDER2_NOT_FOUND;

  // Update Python script
  TPythonDump() << "error = " << this << ".SewConformFreeBorders( "
                << FirstNodeID1  << ", "
                << SecondNodeID1 << ", "
                << LastNodeID1   << ", "
                << FirstNodeID2  << ", "
                << SecondNodeID2 << " )";

  ::SMESH_MeshEditor anEditor( _myMesh );
  SMESH::SMESH_MeshEditor::Sew_Error error =
    convError( anEditor.SewFreeBorder (aBorderFirstNode,
                                       aBorderSecondNode,
                                       aBorderLastNode,
                                       aSide2FirstNode,
                                       aSide2SecondNode,
                                       aSide2ThirdNode,
                                       true,
                                       false, false) );

  UpdateLastResult(anEditor);

  return error;
}


//=======================================================================
//function : SewBorderToSide
//purpose  :
//=======================================================================

SMESH::SMESH_MeshEditor::Sew_Error
SMESH_MeshEditor_i::SewBorderToSide(CORBA::Long FirstNodeIDOnFreeBorder,
                                    CORBA::Long SecondNodeIDOnFreeBorder,
                                    CORBA::Long LastNodeIDOnFreeBorder,
                                    CORBA::Long FirstNodeIDOnSide,
                                    CORBA::Long LastNodeIDOnSide,
                                    CORBA::Boolean CreatePolygons,
                                    CORBA::Boolean CreatePolyedrs)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  SMESHDS_Mesh* aMesh = GetMeshDS();

  const SMDS_MeshNode* aBorderFirstNode  = aMesh->FindNode( FirstNodeIDOnFreeBorder  );
  const SMDS_MeshNode* aBorderSecondNode = aMesh->FindNode( SecondNodeIDOnFreeBorder );
  const SMDS_MeshNode* aBorderLastNode   = aMesh->FindNode( LastNodeIDOnFreeBorder   );
  const SMDS_MeshNode* aSide2FirstNode   = aMesh->FindNode( FirstNodeIDOnSide  );
  const SMDS_MeshNode* aSide2SecondNode  = aMesh->FindNode( LastNodeIDOnSide );
  const SMDS_MeshNode* aSide2ThirdNode   = 0;

  if (!aBorderFirstNode ||
      !aBorderSecondNode||
      !aBorderLastNode  )
    return SMESH::SMESH_MeshEditor::SEW_BORDER1_NOT_FOUND;
  if (!aSide2FirstNode  ||
      !aSide2SecondNode)
    return SMESH::SMESH_MeshEditor::SEW_BAD_SIDE_NODES;

  // Update Python script
  TPythonDump() << "error = " << this << ".SewBorderToSide( "
                << FirstNodeIDOnFreeBorder  << ", "
                << SecondNodeIDOnFreeBorder << ", "
                << LastNodeIDOnFreeBorder   << ", "
                << FirstNodeIDOnSide        << ", "
                << LastNodeIDOnSide         << ", "
                << CreatePolygons           << ", "
                << CreatePolyedrs           << ") ";

  ::SMESH_MeshEditor anEditor( _myMesh );
  SMESH::SMESH_MeshEditor::Sew_Error error =
    convError( anEditor.SewFreeBorder (aBorderFirstNode,
                                       aBorderSecondNode,
                                       aBorderLastNode,
                                       aSide2FirstNode,
                                       aSide2SecondNode,
                                       aSide2ThirdNode,
                                       false,
                                       CreatePolygons,
                                       CreatePolyedrs) );

  UpdateLastResult(anEditor);

  return error;
}


//=======================================================================
//function : SewSideElements
//purpose  :
//=======================================================================

SMESH::SMESH_MeshEditor::Sew_Error
SMESH_MeshEditor_i::SewSideElements(const SMESH::long_array& IDsOfSide1Elements,
                                    const SMESH::long_array& IDsOfSide2Elements,
                                    CORBA::Long NodeID1OfSide1ToMerge,
                                    CORBA::Long NodeID1OfSide2ToMerge,
                                    CORBA::Long NodeID2OfSide1ToMerge,
                                    CORBA::Long NodeID2OfSide2ToMerge)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  SMESHDS_Mesh* aMesh = GetMeshDS();

  const SMDS_MeshNode* aFirstNode1ToMerge  = aMesh->FindNode( NodeID1OfSide1ToMerge );
  const SMDS_MeshNode* aFirstNode2ToMerge  = aMesh->FindNode( NodeID1OfSide2ToMerge );
  const SMDS_MeshNode* aSecondNode1ToMerge = aMesh->FindNode( NodeID2OfSide1ToMerge );
  const SMDS_MeshNode* aSecondNode2ToMerge = aMesh->FindNode( NodeID2OfSide2ToMerge );

  if (!aFirstNode1ToMerge ||
      !aFirstNode2ToMerge )
    return SMESH::SMESH_MeshEditor::SEW_BAD_SIDE1_NODES;
  if (!aSecondNode1ToMerge||
      !aSecondNode2ToMerge)
    return SMESH::SMESH_MeshEditor::SEW_BAD_SIDE2_NODES;

  map<int,const SMDS_MeshElement*> aSide1Elems, aSide2Elems;
  ToMap(IDsOfSide1Elements, aMesh, aSide1Elems);
  ToMap(IDsOfSide2Elements, aMesh, aSide2Elems);

  // Update Python script
  TPythonDump() << "error = " << this << ".SewSideElements( "
                << IDsOfSide1Elements << ", "
                << IDsOfSide2Elements << ", "
                << NodeID1OfSide1ToMerge << ", "
                << NodeID1OfSide2ToMerge << ", "
                << NodeID2OfSide1ToMerge << ", "
                << NodeID2OfSide2ToMerge << ")";

  ::SMESH_MeshEditor anEditor( _myMesh );
  SMESH::SMESH_MeshEditor::Sew_Error error =
    convError( anEditor.SewSideElements (aSide1Elems, aSide2Elems,
                                         aFirstNode1ToMerge,
                                         aFirstNode2ToMerge,
                                         aSecondNode1ToMerge,
                                         aSecondNode2ToMerge));

  UpdateLastResult(anEditor);

  return error;
}

//================================================================================
/*!
 * \brief Set new nodes for given element
  * \param ide - element id
  * \param newIDs - new node ids
  * \retval CORBA::Boolean - true if result is OK
 */
//================================================================================

CORBA::Boolean SMESH_MeshEditor_i::ChangeElemNodes(CORBA::Long ide,
                                                   const SMESH::long_array& newIDs)
{
  myLastCreatedElems = new SMESH::long_array();
  myLastCreatedNodes = new SMESH::long_array();

  const SMDS_MeshElement* elem = GetMeshDS()->FindElement(ide);
  if(!elem) return false;

  int nbn = newIDs.length();
  int i=0;
  const SMDS_MeshNode* aNodes [nbn];
  int nbn1=-1;
  for(; i<nbn; i++) {
    const SMDS_MeshNode* aNode = GetMeshDS()->FindNode(newIDs[i]);
    if(aNode) {
      nbn1++;
      aNodes[nbn1] = aNode;
    }
  }
  // Update Python script
  TPythonDump() << "isDone = " << this << ".ChangeElemNodes( "
                << ide << ", " << newIDs << " )";
#ifdef _DEBUG_
  TPythonDump() << "print 'ChangeElemNodes: ', isDone";
#endif

  return GetMeshDS()->ChangeElementNodes( elem, aNodes, nbn1+1 );
}
  
//================================================================================
/*!
 * \brief Update myLastCreatedNodes and myLastCreatedElems
  * \param anEditor - it contains last modification results
 */
//================================================================================

void SMESH_MeshEditor_i::UpdateLastResult(::SMESH_MeshEditor& anEditor)
{
  // add new elements into myLastCreatedNodes
  SMESH_SequenceOfElemPtr aSeq = anEditor.GetLastCreatedNodes();
  SMESH::long_array_var aResult = new SMESH::long_array;
  aResult->length(aSeq.Length());
  int i=0;
  for(; i<aSeq.Length(); i++) {
    aResult[i] = aSeq.Value(i+1)->GetID();
  }
  myLastCreatedNodes = aResult._retn();
  // add new elements into myLastCreatedElems
  aSeq = anEditor.GetLastCreatedElems();
  aResult = new SMESH::long_array;
  aResult->length(aSeq.Length());
  i=0;
  for(; i<aSeq.Length(); i++) {
    aResult[i] = aSeq.Value(i+1)->GetID();
  }
  myLastCreatedElems = aResult._retn();
}

//================================================================================
/*!
 * \brief Returns list of it's IDs of created nodes
  * \retval SMESH::long_array* - list of node ID
 */
//================================================================================

SMESH::long_array* SMESH_MeshEditor_i::GetLastCreatedNodes()
{
  return myLastCreatedNodes;
}

//================================================================================
/*!
 * \brief Returns list of it's IDs of created elements
  * \retval SMESH::long_array* - list of elements' ID
 */
//================================================================================

SMESH::long_array* SMESH_MeshEditor_i::GetLastCreatedElems()
{
  return myLastCreatedElems;
}


//=======================================================================
//function : ConvertToQuadratic
//purpose  :
//=======================================================================

void SMESH_MeshEditor_i::ConvertToQuadratic(CORBA::Boolean theForce3d)
{
  ::SMESH_MeshEditor anEditor( _myMesh );
  anEditor.ConvertToQuadratic(theForce3d);
 // Update Python script
  TPythonDump() << this << ".ConvertToQuadratic( "
                << theForce3d << " )";
}

//=======================================================================
//function : ConvertFromQuadratic
//purpose  :
//=======================================================================

CORBA::Boolean SMESH_MeshEditor_i::ConvertFromQuadratic()
{
  ::SMESH_MeshEditor anEditor( _myMesh );
  CORBA::Boolean isDone = anEditor.ConvertFromQuadratic();
  // Update Python script
  TPythonDump() << this << ".ConvertFromQuadratic()";
  return isDone;
}
