//  SMESH SMESHDS : management of mesh data and SMESH document
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
//  Author : Yves FRICAUD, OCC
//  Module : SMESH
//  $Header: 

#include "SMESHDS_Mesh.hxx"

#include "SMESHDS_Group.hxx"
#include "SMDS_VertexPosition.hxx"
#include "SMDS_EdgePosition.hxx"
#include "SMDS_FacePosition.hxx"
#include "SMESHDS_GroupOnGeom.hxx"
#include <TopExp_Explorer.hxx>
#include <TopExp.hxx>
#include <TopoDS_Iterator.hxx>

#include "utilities.h"

using namespace std;

//=======================================================================
//function : Create
//purpose  : 
//=======================================================================
SMESHDS_Mesh::SMESHDS_Mesh(int MeshID):myMeshID(MeshID)
{
	myScript = new SMESHDS_Script();
}

//=======================================================================
//function : ShapeToMesh
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::ShapeToMesh(const TopoDS_Shape & S)
{
  if ( !myShape.IsNull() && S.IsNull() )
  {
    // removal of a shape to mesh, delete ...
    // - hypotheses
    myShapeToHypothesis.clear();
    // - shape indices in SMDS_Position of nodes
    map<int,SMESHDS_SubMesh*>::iterator i_sub = myShapeIndexToSubMesh.begin();
    for ( ; i_sub != myShapeIndexToSubMesh.end(); i_sub++ ) {
      if ( !i_sub->second->IsComplexSubmesh() ) {
        SMDS_NodeIteratorPtr nIt = i_sub->second->GetNodes();
        while ( nIt->more() )
          nIt->next()->GetPosition()->SetShapeId( 0 );
      }
    }
    // - sub-meshes
    myIndexToShape.Clear();
    myShapeIndexToSubMesh.clear();
    // - groups on geometry
    set<SMESHDS_GroupBase*>::iterator gr = myGroups.begin();
    while ( gr != myGroups.end() ) {
      if ( dynamic_cast<SMESHDS_GroupOnGeom*>( *gr ))
        myGroups.erase( gr++ );
      else
        gr++;
    }
  }
  else {
    myShape = S;
    if ( !S.IsNull() )
      TopExp::MapShapes(myShape, myIndexToShape);
  }
}

//=======================================================================
//function : AddHypothesis
//purpose  : 
//=======================================================================

bool SMESHDS_Mesh::AddHypothesis(const TopoDS_Shape & SS,
	const SMESHDS_Hypothesis * H)
{
	list<const SMESHDS_Hypothesis *>& alist=myShapeToHypothesis[SS];

	//Check if the Hypothesis is still present
	list<const SMESHDS_Hypothesis*>::iterator ith=alist.begin();

	for (; ith!=alist.end(); ith++)
		if (H == *ith) return false;

	alist.push_back(H);
	return true;
}

//=======================================================================
//function : RemoveHypothesis
//purpose  : 
//=======================================================================

bool SMESHDS_Mesh::RemoveHypothesis(const TopoDS_Shape & S,
	const SMESHDS_Hypothesis * H)
{
	ShapeToHypothesis::iterator its=myShapeToHypothesis.find(S);
	if(its!=myShapeToHypothesis.end())
	{
		list<const SMESHDS_Hypothesis*>::iterator ith=(*its).second.begin();

		for (; ith!=(*its).second.end(); ith++)
			if (H == *ith)
			{
				(*its).second.erase(ith);
				return true;
			}
	}
	return false;
}

//=======================================================================
//function : AddNode
//purpose  : 
//=======================================================================
SMDS_MeshNode* SMESHDS_Mesh::AddNode(double x, double y, double z){
  SMDS_MeshNode* node = SMDS_Mesh::AddNode(x, y, z);
  if(node!=NULL) myScript->AddNode(node->GetID(), x, y, z);
  return node;
}

SMDS_MeshNode* SMESHDS_Mesh::AddNodeWithID(double x, double y, double z, int ID){
  SMDS_MeshNode* node = SMDS_Mesh::AddNodeWithID(x,y,z,ID);
  if(node!=NULL) myScript->AddNode(node->GetID(), x, y, z);
  return node;
}

//=======================================================================
//function : MoveNode
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::MoveNode(const SMDS_MeshNode *n, double x, double y, double z)
{
  SMDS_MeshNode * node=const_cast<SMDS_MeshNode*>(n);
  node->setXYZ(x,y,z);
  myScript->MoveNode(n->GetID(), x, y, z);
}

//=======================================================================
//function : ChangeElementNodes
//purpose  : 
//=======================================================================

bool SMESHDS_Mesh::ChangeElementNodes(const SMDS_MeshElement * elem,
                                      const SMDS_MeshNode    * nodes[],
                                      const int                nbnodes)
{
  if ( ! SMDS_Mesh::ChangeElementNodes( elem, nodes, nbnodes ))
    return false;

  ASSERT( nbnodes < 9 );
  int i, IDs[ 8 ];
  for ( i = 0; i < nbnodes; i++ )
    IDs [ i ] = nodes[ i ]->GetID();
  myScript->ChangeElementNodes( elem->GetID(), IDs, nbnodes);

  return true;
}

//=======================================================================
//function : Renumber
//purpose  : 
//=======================================================================

void SMESHDS_Mesh::Renumber (const bool isNodes, const int startID, const int deltaID)
{
  SMDS_Mesh::Renumber( isNodes, startID, deltaID );
  myScript->Renumber( isNodes, startID, deltaID );
}

//=======================================================================
//function :AddEdgeWithID
//purpose  : 
//=======================================================================
SMDS_MeshEdge* SMESHDS_Mesh::AddEdgeWithID(int n1, int n2, int ID)
{
  SMDS_MeshEdge* anElem = SMDS_Mesh::AddEdgeWithID(n1,n2,ID);
  if(anElem) myScript->AddEdge(ID,n1,n2);
  return anElem;
}

SMDS_MeshEdge* SMESHDS_Mesh::AddEdgeWithID(const SMDS_MeshNode * n1,
					   const SMDS_MeshNode * n2, 
					   int ID)
{
  return AddEdgeWithID(n1->GetID(),
		       n2->GetID(),
		       ID);
}

SMDS_MeshEdge* SMESHDS_Mesh::AddEdge(const SMDS_MeshNode * n1,
				     const SMDS_MeshNode * n2)
{
  SMDS_MeshEdge* anElem = SMDS_Mesh::AddEdge(n1,n2);
  if(anElem) myScript->AddEdge(anElem->GetID(), 
			       n1->GetID(), 
			       n2->GetID());
  return anElem;
}

//=======================================================================
//function :AddFace
//purpose  : 
//=======================================================================
SMDS_MeshFace* SMESHDS_Mesh::AddFaceWithID(int n1, int n2, int n3, int ID)
{
  SMDS_MeshFace *anElem = SMDS_Mesh::AddFaceWithID(n1, n2, n3, ID);
  if(anElem) myScript->AddFace(ID,n1,n2,n3);
  return anElem;
}

SMDS_MeshFace* SMESHDS_Mesh::AddFaceWithID(const SMDS_MeshNode * n1,
					   const SMDS_MeshNode * n2,
					   const SMDS_MeshNode * n3, 
					   int ID)
{
  return AddFaceWithID(n1->GetID(),
		       n2->GetID(),
		       n3->GetID(),
		       ID);
}

SMDS_MeshFace* SMESHDS_Mesh::AddFace( const SMDS_MeshNode * n1,
				      const SMDS_MeshNode * n2,
				      const SMDS_MeshNode * n3)
{
  SMDS_MeshFace *anElem = SMDS_Mesh::AddFace(n1, n2, n3);
  if(anElem) myScript->AddFace(anElem->GetID(), 
			       n1->GetID(), 
			       n2->GetID(),
			       n3->GetID());
  return anElem;
}

//=======================================================================
//function :AddFace
//purpose  : 
//=======================================================================
SMDS_MeshFace* SMESHDS_Mesh::AddFaceWithID(int n1, int n2, int n3, int n4, int ID)
{
  SMDS_MeshFace *anElem = SMDS_Mesh::AddFaceWithID(n1, n2, n3, n4, ID);
  if(anElem) myScript->AddFace(ID, n1, n2, n3, n4);
  return anElem;
}

SMDS_MeshFace* SMESHDS_Mesh::AddFaceWithID(const SMDS_MeshNode * n1,
					   const SMDS_MeshNode * n2,
					   const SMDS_MeshNode * n3,
					   const SMDS_MeshNode * n4, 
					   int ID)
{
  return AddFaceWithID(n1->GetID(),
		       n2->GetID(),
		       n3->GetID(),
		       n4->GetID(),
		       ID);
}

SMDS_MeshFace* SMESHDS_Mesh::AddFace(const SMDS_MeshNode * n1,
				     const SMDS_MeshNode * n2,
				     const SMDS_MeshNode * n3,
				     const SMDS_MeshNode * n4)
{
  SMDS_MeshFace *anElem = SMDS_Mesh::AddFace(n1, n2, n3, n4);
  if(anElem) myScript->AddFace(anElem->GetID(), 
			       n1->GetID(), 
			       n2->GetID(), 
			       n3->GetID(),
			       n4->GetID());
  return anElem;
}

//=======================================================================
//function :AddVolume
//purpose  : 
//=======================================================================
SMDS_MeshVolume* SMESHDS_Mesh::AddVolumeWithID(int n1, int n2, int n3, int n4, int ID)
{
  SMDS_MeshVolume *anElem = SMDS_Mesh::AddVolumeWithID(n1, n2, n3, n4, ID);
  if(anElem) myScript->AddVolume(ID, n1, n2, n3, n4);
  return anElem;
}

SMDS_MeshVolume* SMESHDS_Mesh::AddVolumeWithID(const SMDS_MeshNode * n1,
					       const SMDS_MeshNode * n2,
					       const SMDS_MeshNode * n3,
					       const SMDS_MeshNode * n4, 
					       int ID)
{
  return AddVolumeWithID(n1->GetID(), 
			 n2->GetID(), 
			 n3->GetID(),
			 n4->GetID(),
			 ID);
}

SMDS_MeshVolume* SMESHDS_Mesh::AddVolume(const SMDS_MeshNode * n1,
					 const SMDS_MeshNode * n2,
					 const SMDS_MeshNode * n3,
					 const SMDS_MeshNode * n4)
{
  SMDS_MeshVolume *anElem = SMDS_Mesh::AddVolume(n1, n2, n3, n4);
  if(anElem) myScript->AddVolume(anElem->GetID(), 
				 n1->GetID(), 
				 n2->GetID(), 
				 n3->GetID(),
				 n4->GetID());
  return anElem;
}

//=======================================================================
//function :AddVolume
//purpose  : 
//=======================================================================
SMDS_MeshVolume* SMESHDS_Mesh::AddVolumeWithID(int n1, int n2, int n3, int n4, int n5, int ID)
{
  SMDS_MeshVolume *anElem = SMDS_Mesh::AddVolumeWithID(n1, n2, n3, n4, n5, ID);
  if(anElem) myScript->AddVolume(ID, n1, n2, n3, n4, n5);
  return anElem;
}

SMDS_MeshVolume* SMESHDS_Mesh::AddVolumeWithID(const SMDS_MeshNode * n1,
					       const SMDS_MeshNode * n2,
					       const SMDS_MeshNode * n3,
					       const SMDS_MeshNode * n4,
					       const SMDS_MeshNode * n5, 
					       int ID)
{
  return AddVolumeWithID(n1->GetID(), 
			 n2->GetID(), 
			 n3->GetID(),
			 n4->GetID(), 
			 n5->GetID(),
			 ID);
}

SMDS_MeshVolume* SMESHDS_Mesh::AddVolume(const SMDS_MeshNode * n1,
					 const SMDS_MeshNode * n2,
					 const SMDS_MeshNode * n3,
					 const SMDS_MeshNode * n4,
					 const SMDS_MeshNode * n5)
{
  SMDS_MeshVolume *anElem = SMDS_Mesh::AddVolume(n1, n2, n3, n4, n5);
  if(anElem) myScript->AddVolume(anElem->GetID(), 
				 n1->GetID(), 
				 n2->GetID(), 
				 n3->GetID(),
				 n4->GetID(), 
				 n5->GetID());
  return anElem;
}

//=======================================================================
//function :AddVolume
//purpose  : 
//=======================================================================
SMDS_MeshVolume* SMESHDS_Mesh::AddVolumeWithID(int n1, int n2, int n3, int n4, int n5, int n6, int ID)
{
  SMDS_MeshVolume *anElem= SMDS_Mesh::AddVolumeWithID(n1, n2, n3, n4, n5, n6, ID);
  if(anElem) myScript->AddVolume(ID, n1, n2, n3, n4, n5, n6);
  return anElem;
}

SMDS_MeshVolume* SMESHDS_Mesh::AddVolumeWithID(const SMDS_MeshNode * n1,
					       const SMDS_MeshNode * n2,
					       const SMDS_MeshNode * n3,
					       const SMDS_MeshNode * n4,
					       const SMDS_MeshNode * n5,
					       const SMDS_MeshNode * n6, 
					       int ID)
{
  return AddVolumeWithID(n1->GetID(), 
			 n2->GetID(), 
			 n3->GetID(),
			 n4->GetID(), 
			 n5->GetID(), 
			 n6->GetID(),
			 ID);
}

SMDS_MeshVolume* SMESHDS_Mesh::AddVolume(const SMDS_MeshNode * n1,
					 const SMDS_MeshNode * n2,
					 const SMDS_MeshNode * n3,
					 const SMDS_MeshNode * n4,
					 const SMDS_MeshNode * n5,
					 const SMDS_MeshNode * n6)
{
  SMDS_MeshVolume *anElem = SMDS_Mesh::AddVolume(n1, n2, n3, n4, n5, n6);
  if(anElem) myScript->AddVolume(anElem->GetID(), 
				 n1->GetID(), 
				 n2->GetID(), 
				 n3->GetID(),
				 n4->GetID(), 
				 n5->GetID(), 
				 n6->GetID());
  return anElem;
}

//=======================================================================
//function :AddVolume
//purpose  : 
//=======================================================================
SMDS_MeshVolume* SMESHDS_Mesh::AddVolumeWithID(int n1, int n2, int n3, int n4, int n5, int n6, int n7, int n8, int ID)
{
  SMDS_MeshVolume *anElem= SMDS_Mesh::AddVolumeWithID(n1, n2, n3, n4, n5, n6, n7, n8, ID);
  if(anElem) myScript->AddVolume(ID, n1, n2, n3, n4, n5, n6, n7, n8);
  return anElem;
}

SMDS_MeshVolume* SMESHDS_Mesh::AddVolumeWithID(const SMDS_MeshNode * n1,
					       const SMDS_MeshNode * n2,
					       const SMDS_MeshNode * n3,
					       const SMDS_MeshNode * n4,
					       const SMDS_MeshNode * n5,
					       const SMDS_MeshNode * n6,
					       const SMDS_MeshNode * n7,
					       const SMDS_MeshNode * n8, 
					       int ID)
{
  return AddVolumeWithID(n1->GetID(), 
			 n2->GetID(), 
			 n3->GetID(),
			 n4->GetID(), 
			 n5->GetID(), 
			 n6->GetID(), 
			 n7->GetID(), 
			 n8->GetID(),
			 ID);
}

SMDS_MeshVolume* SMESHDS_Mesh::AddVolume(const SMDS_MeshNode * n1,
					 const SMDS_MeshNode * n2,
					 const SMDS_MeshNode * n3,
					 const SMDS_MeshNode * n4,
					 const SMDS_MeshNode * n5,
					 const SMDS_MeshNode * n6,
					 const SMDS_MeshNode * n7,
					 const SMDS_MeshNode * n8)
{
  SMDS_MeshVolume *anElem = SMDS_Mesh::AddVolume(n1, n2, n3, n4, n5, n6, n7, n8);
  if(anElem) myScript->AddVolume(anElem->GetID(), 
				 n1->GetID(), 
				 n2->GetID(), 
				 n3->GetID(),
				 n4->GetID(), 
				 n5->GetID(), 
				 n6->GetID(), 
				 n7->GetID(), 
				 n8->GetID());
  return anElem;
}
//=======================================================================
//function : removeFromContainers
//purpose  : 
//=======================================================================

static void removeFromContainers (map<int,SMESHDS_SubMesh*> &      theSubMeshes,
                                  set<SMESHDS_GroupBase*>&             theGroups,
                                  list<const SMDS_MeshElement *> & theElems,
                                  const bool                       isNode)
{
  if ( theElems.empty() )
    return;

  // Rm from group
  // Element can belong to several groups
  if ( !theGroups.empty() )
  {
    set<SMESHDS_GroupBase*>::iterator GrIt = theGroups.begin();
    for ( ; GrIt != theGroups.end(); GrIt++ )
    {
      SMESHDS_Group* group = dynamic_cast<SMESHDS_Group*>( *GrIt );
      if ( !group || group->IsEmpty() ) continue;

      list<const SMDS_MeshElement *>::iterator elIt = theElems.begin();
      for ( ; elIt != theElems.end(); elIt++ )
      {
        group->SMDSGroup().Remove( *elIt );
        if ( group->IsEmpty() ) break;
      }
    }
  }

  // Rm from sub-meshes
  // Element should belong to only one sub-mesh
  map<int,SMESHDS_SubMesh*>::iterator SubIt = theSubMeshes.begin();
  for ( ; SubIt != theSubMeshes.end(); SubIt++ )
  {
    int size = isNode ? (*SubIt).second->NbNodes() : (*SubIt).second->NbElements();
    if ( size == 0 ) continue;

    list<const SMDS_MeshElement *>::iterator elIt = theElems.begin();
    while ( elIt != theElems.end() )
    {
      bool removed = false;
      if ( isNode )
        removed = (*SubIt).second->RemoveNode( static_cast<const SMDS_MeshNode*> (*elIt) );
      else
        removed = (*SubIt).second->RemoveElement( *elIt );

      if (removed)
      {
        elIt = theElems.erase( elIt );
        if ( theElems.empty() )
          return; // all elements are found and removed
      }
      else
      {
        elIt++ ;
      }
    }
  }
}
  
//=======================================================================
//function : RemoveNode
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::RemoveNode(const SMDS_MeshNode * n)
{
  myScript->RemoveNode(n->GetID());
  
  list<const SMDS_MeshElement *> removedElems;
  list<const SMDS_MeshElement *> removedNodes;

  SMDS_Mesh::RemoveElement( n, removedElems, removedNodes, true );

  removeFromContainers( myShapeIndexToSubMesh, myGroups, removedElems, false );
  removeFromContainers( myShapeIndexToSubMesh, myGroups, removedNodes, true );
}

//=======================================================================
//function : RemoveElement
//purpose  : 
//========================================================================
void SMESHDS_Mesh::RemoveElement(const SMDS_MeshElement * elt)
{
  if (elt->GetType() == SMDSAbs_Node)
  {
    RemoveNode( static_cast<const SMDS_MeshNode*>( elt ));
    return;
  }

  myScript->RemoveElement(elt->GetID());

  list<const SMDS_MeshElement *> removedElems;
  list<const SMDS_MeshElement *> removedNodes;

  SMDS_Mesh::RemoveElement(elt, removedElems, removedNodes, false);
  
  removeFromContainers( myShapeIndexToSubMesh, myGroups, removedElems, false );
}

//=======================================================================
//function : SetNodeOnVolume
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::SetNodeInVolume(SMDS_MeshNode * aNode,
	const TopoDS_Shell & S)
{
	if (myShape.IsNull()) MESSAGE("myShape is NULL");

	int Index = myIndexToShape.FindIndex(S);

	//Set Position on Node
	//Handle (SMDS_FacePosition) aPos = new SMDS_FacePosition (myFaceToId(S),0.,0.);;
	//aNode->SetPosition(aPos);

	//Update or build submesh
	map<int,SMESHDS_SubMesh*>::iterator it=myShapeIndexToSubMesh.find(Index);
	if (it==myShapeIndexToSubMesh.end())
		myShapeIndexToSubMesh[Index]= new SMESHDS_SubMesh();

	myShapeIndexToSubMesh[Index]->AddNode(aNode);
}

//=======================================================================
//function : SetNodeOnFace
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::SetNodeOnFace(SMDS_MeshNode * aNode,
	const TopoDS_Face & S)
{
	if (myShape.IsNull()) MESSAGE("myShape is NULL");

	int Index = myIndexToShape.FindIndex(S);

	//Set Position on Node
	aNode->SetPosition(SMDS_PositionPtr(new SMDS_FacePosition(Index, 0., 0.)));

	//Update or build submesh
	map<int,SMESHDS_SubMesh*>::iterator it=myShapeIndexToSubMesh.find(Index);
	if (it==myShapeIndexToSubMesh.end())
		myShapeIndexToSubMesh[Index]= new SMESHDS_SubMesh();

	myShapeIndexToSubMesh[Index]->AddNode(aNode);
}

//=======================================================================
//function : SetNodeOnEdge
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::SetNodeOnEdge(SMDS_MeshNode * aNode,
	const TopoDS_Edge & S)
{
	if (myShape.IsNull()) MESSAGE("myShape is NULL");

	int Index = myIndexToShape.FindIndex(S);

	//Set Position on Node
	aNode->SetPosition(SMDS_PositionPtr(new SMDS_EdgePosition(Index, 0.)));

	//Update or build submesh
	map<int,SMESHDS_SubMesh*>::iterator it=myShapeIndexToSubMesh.find(Index);
	if (it==myShapeIndexToSubMesh.end())
		myShapeIndexToSubMesh[Index]= new SMESHDS_SubMesh();

	myShapeIndexToSubMesh[Index]->AddNode(aNode);
}

//=======================================================================
//function : SetNodeOnVertex
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::SetNodeOnVertex(SMDS_MeshNode * aNode,
	const TopoDS_Vertex & S)
{
	if (myShape.IsNull()) MESSAGE("myShape is NULL");

	int Index = myIndexToShape.FindIndex(S);

	//Set Position on Node
	aNode->SetPosition(SMDS_PositionPtr(new SMDS_VertexPosition(Index)));

	//Update or build submesh
	map<int,SMESHDS_SubMesh*>::iterator it=myShapeIndexToSubMesh.find(Index);
	if (it==myShapeIndexToSubMesh.end())
		myShapeIndexToSubMesh[Index]= new SMESHDS_SubMesh();

	myShapeIndexToSubMesh[Index]->AddNode(aNode);
}

//=======================================================================
//function : UnSetNodeOnShape
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::UnSetNodeOnShape(const SMDS_MeshNode* aNode)
{
	MESSAGE("not implemented");
}

//=======================================================================
//function : SetMeshElementOnShape
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::SetMeshElementOnShape(const SMDS_MeshElement * anElement,
	const TopoDS_Shape & S)
{
	if (myShape.IsNull()) MESSAGE("myShape is NULL");

	int Index = myIndexToShape.FindIndex(S);

	if (myShapeIndexToSubMesh.find(Index)==myShapeIndexToSubMesh.end())
		myShapeIndexToSubMesh[Index]=new SMESHDS_SubMesh();

	myShapeIndexToSubMesh[Index]->AddElement(anElement);
}

//=======================================================================
//function : UnSetMeshElementOnShape
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::
UnSetMeshElementOnShape(const SMDS_MeshElement * anElement,
	const TopoDS_Shape & S)
{
	if (myShape.IsNull()) MESSAGE("myShape is NULL");

	int Index = myIndexToShape.FindIndex(S);

	if (myShapeIndexToSubMesh.find(Index)!=myShapeIndexToSubMesh.end())
		myShapeIndexToSubMesh[Index]->RemoveElement(anElement);
}

//=======================================================================
//function : ShapeToMesh
//purpose  : 
//=======================================================================
TopoDS_Shape SMESHDS_Mesh::ShapeToMesh() const
{
	return myShape;
}

//=======================================================================
//function : IsGroupOfSubShapes
//purpose  : return true if at least one subshape of theShape is a subshape
//           of myShape or theShape == myShape
//=======================================================================

bool SMESHDS_Mesh::IsGroupOfSubShapes (const TopoDS_Shape& theShape) const
{
  if ( myShape.IsSame( theShape ))
    return true;

  for ( TopoDS_Iterator it( theShape ); it.More(); it.Next() ) {
    if (myIndexToShape.Contains( it.Value() ) ||
        IsGroupOfSubShapes( it.Value() ))
      return true;
  }
  
  return false;
}

///////////////////////////////////////////////////////////////////////////////
/// Return the sub mesh linked to the a given TopoDS_Shape or NULL if the given
/// TopoDS_Shape is unknown
///////////////////////////////////////////////////////////////////////////////
SMESHDS_SubMesh * SMESHDS_Mesh::MeshElements(const TopoDS_Shape & S)
{
  if (myShape.IsNull()) MESSAGE("myShape is NULL");

  int Index = ShapeToIndex(S);
  if (myShapeIndexToSubMesh.find(Index)!=myShapeIndexToSubMesh.end())
    return myShapeIndexToSubMesh[Index];
  else
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
/// Return the sub mesh by Id of shape it is linked to
///////////////////////////////////////////////////////////////////////////////
SMESHDS_SubMesh * SMESHDS_Mesh::MeshElements(const int Index)
{
  if (myShape.IsNull()) MESSAGE("myShape is NULL");

  if (myShapeIndexToSubMesh.find(Index)!=myShapeIndexToSubMesh.end())
    return myShapeIndexToSubMesh[Index];
  else
    return NULL;
}

//=======================================================================
//function : SubMeshIndices
//purpose  : 
//=======================================================================
list<int> SMESHDS_Mesh::SubMeshIndices()
{
  list<int> anIndices;
  std::map<int,SMESHDS_SubMesh*>::iterator anIter = myShapeIndexToSubMesh.begin();
  for (; anIter != myShapeIndexToSubMesh.end(); anIter++) {
    anIndices.push_back((*anIter).first);
  }
  return anIndices;
}

//=======================================================================
//function : GetHypothesis
//purpose  : 
//=======================================================================

const list<const SMESHDS_Hypothesis*>& SMESHDS_Mesh::GetHypothesis(
	const TopoDS_Shape & S) const
{
	if (myShapeToHypothesis.find(S)!=myShapeToHypothesis.end())
		return myShapeToHypothesis.find(S)->second;

	static list<const SMESHDS_Hypothesis*> empty;
	return empty;
}

//=======================================================================
//function : GetScript
//purpose  : 
//=======================================================================
SMESHDS_Script* SMESHDS_Mesh::GetScript()
{
	return myScript;
}

//=======================================================================
//function : ClearScript
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::ClearScript()
{
	myScript->Clear();
}

//=======================================================================
//function : HasMeshElements
//purpose  : 
//=======================================================================
bool SMESHDS_Mesh::HasMeshElements(const TopoDS_Shape & S)
{
	if (myShape.IsNull()) MESSAGE("myShape is NULL");
	int Index = myIndexToShape.FindIndex(S);
	return myShapeIndexToSubMesh.find(Index)!=myShapeIndexToSubMesh.end();
}

//=======================================================================
//function : HasHypothesis
//purpose  : 
//=======================================================================
bool SMESHDS_Mesh::HasHypothesis(const TopoDS_Shape & S)
{
	return myShapeToHypothesis.find(S)!=myShapeToHypothesis.end();
}

//=======================================================================
//function : NewSubMesh 
//purpose  : 
//=======================================================================
SMESHDS_SubMesh * SMESHDS_Mesh::NewSubMesh(int Index)
{
  SMESHDS_SubMesh* SM = 0;
  if (myShapeIndexToSubMesh.find(Index)==myShapeIndexToSubMesh.end())
  {
    SM = new SMESHDS_SubMesh();
    myShapeIndexToSubMesh[Index]=SM;
  }
  else
    SM = myShapeIndexToSubMesh[Index];
  return SM;
}

//=======================================================================
//function : AddCompoundSubmesh
//purpose  : 
//=======================================================================

int SMESHDS_Mesh::AddCompoundSubmesh(const TopoDS_Shape& S,
                                     TopAbs_ShapeEnum    type)
{
  int aMainIndex = 0;
  if ( IsGroupOfSubShapes( S ) || (S.ShapeType() == TopAbs_VERTEX && myIndexToShape.Contains(S)) )
  {
    aMainIndex = myIndexToShape.Add( S );
    bool all = ( type == TopAbs_SHAPE );
    if ( all ) // corresponding simple submesh may exist
      aMainIndex = -aMainIndex;
    //MESSAGE("AddCompoundSubmesh index = " << aMainIndex );
    SMESHDS_SubMesh * aNewSub = NewSubMesh( aMainIndex );
    if ( !aNewSub->IsComplexSubmesh() ) // is empty
    {
      int shapeType = all ? myShape.ShapeType() : type;
      int typeLimit = all ? TopAbs_VERTEX : type;
      for ( ; shapeType <= typeLimit; shapeType++ )
      {
        TopExp_Explorer exp( S, TopAbs_ShapeEnum( shapeType ));
        for ( ; exp.More(); exp.Next() )
        {
          int index = myIndexToShape.FindIndex( exp.Current() );
          if ( index )
            aNewSub->AddSubMesh( NewSubMesh( index ));
        }
      }
    }
  }
  return aMainIndex;
}

//=======================================================================
//function : IndexToShape
//purpose  : 
//=======================================================================
TopoDS_Shape SMESHDS_Mesh::IndexToShape(int ShapeIndex)
{
	return myIndexToShape.FindKey(ShapeIndex);
}

//=======================================================================
//function : ShapeToIndex
//purpose  : 
//=======================================================================
int SMESHDS_Mesh::ShapeToIndex(const TopoDS_Shape & S)
{
  if (myShape.IsNull())
    MESSAGE("myShape is NULL");

  int index = myIndexToShape.FindIndex(S);
  
  return index;
}

//=======================================================================
//function : SetNodeOnVolume
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::SetNodeInVolume(const SMDS_MeshNode* aNode, int Index)
{
	//Set Position on Node
	//Handle (SMDS_FacePosition) aPos = new SMDS_FacePosition (myFaceToId(S),0.,0.);;
	//aNode->SetPosition(aPos);

	//Update or build submesh
	if (myShapeIndexToSubMesh.find(Index)==myShapeIndexToSubMesh.end())
		myShapeIndexToSubMesh[Index]=new SMESHDS_SubMesh();

	myShapeIndexToSubMesh[Index]->AddNode(aNode);
}

//=======================================================================
//function : SetNodeOnFace
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::SetNodeOnFace(SMDS_MeshNode* aNode, int Index)
{
	//Set Position on Node
	aNode->SetPosition(SMDS_PositionPtr(new SMDS_FacePosition(Index, 0., 0.)));

	//Update or build submesh
	if (myShapeIndexToSubMesh.find(Index)==myShapeIndexToSubMesh.end())
		myShapeIndexToSubMesh[Index]=new SMESHDS_SubMesh();

	myShapeIndexToSubMesh[Index]->AddNode(aNode);
}

//=======================================================================
//function : SetNodeOnEdge
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::SetNodeOnEdge(SMDS_MeshNode* aNode, int Index)
{
	//Set Position on Node
	aNode->SetPosition(SMDS_PositionPtr(new SMDS_EdgePosition(Index, 0.)));

	//Update or build submesh
	if (myShapeIndexToSubMesh.find(Index)==myShapeIndexToSubMesh.end())
		myShapeIndexToSubMesh[Index]=new SMESHDS_SubMesh();

	myShapeIndexToSubMesh[Index]->AddNode(aNode);
}

//=======================================================================
//function : SetNodeOnVertex
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::SetNodeOnVertex(SMDS_MeshNode* aNode, int Index)
{
	//Set Position on Node
	aNode->SetPosition(SMDS_PositionPtr(new SMDS_VertexPosition(Index)));

	//Update or build submesh
	if (myShapeIndexToSubMesh.find(Index)==myShapeIndexToSubMesh.end())
		myShapeIndexToSubMesh[Index]=new SMESHDS_SubMesh();

	myShapeIndexToSubMesh[Index]->AddNode(aNode);
}

//=======================================================================
//function : SetMeshElementOnShape
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::SetMeshElementOnShape(const SMDS_MeshElement* anElement,
	int Index)
{
	if (myShapeIndexToSubMesh.find(Index)==myShapeIndexToSubMesh.end())
		myShapeIndexToSubMesh[Index]=new SMESHDS_SubMesh();

	myShapeIndexToSubMesh[Index]->AddElement(anElement);
}

SMESHDS_Mesh::~SMESHDS_Mesh()
{
}
