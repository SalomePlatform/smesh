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
#include "SMDS_VertexPosition.hxx"
#include "SMDS_EdgePosition.hxx"
#include "SMDS_FacePosition.hxx"
#include <TopExp_Explorer.hxx>
#include <TopExp.hxx>

#include "utilities.h"
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
	myShape = S;
	TopExp::MapShapes(myShape, myIndexToShape);
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
SMDS_MeshNode* SMESHDS_Mesh::AddNode(double x, double y, double z)
{
	SMDS_MeshNode* node = SMDS_Mesh::AddNode(x, y, z);
	if(node!=NULL) myScript->AddNode(node->GetID(), x, y, z);
	return node;
}

//=======================================================================
//function : MoveNode
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::MoveNode(int ID, double x, double y, double z)
{
	SMDS_MeshNode * node=const_cast<SMDS_MeshNode*>(FindNode(ID));
	node->setXYZ(x,y,z);
	myScript->MoveNode(ID, x, y, z);
}

//=======================================================================
//function : AddEdge
//purpose  : 
//=======================================================================
SMDS_MeshEdge* SMESHDS_Mesh::AddEdge(int idnode1, int idnode2)
{
	SMDS_MeshEdge* e = SMDS_Mesh::AddEdge(idnode1, idnode2);
	if(e!=NULL) myScript->AddEdge(e->GetID(), idnode1, idnode2);
	return e;
}

//=======================================================================
//function :AddFace
//purpose  : 
//=======================================================================
SMDS_MeshFace* SMESHDS_Mesh::AddFace(int idnode1, int idnode2, int idnode3)
{
	SMDS_MeshFace *f = SMDS_Mesh::AddFace(idnode1, idnode2, idnode3);
	if(f!=NULL) myScript->AddFace(f->GetID(), idnode1, idnode2, idnode3);
	return f;
}

//=======================================================================
//function :AddFace
//purpose  : 
//=======================================================================
SMDS_MeshFace* SMESHDS_Mesh::AddFace(int idnode1, int idnode2, int idnode3,
	int idnode4)
{
	SMDS_MeshFace *f = SMDS_Mesh::AddFace(idnode1, idnode2, idnode3, idnode4);
	if(f!=NULL)
		myScript->AddFace(f->GetID(), idnode1, idnode2, idnode3, idnode4);
	return f;
}

//=======================================================================
//function :AddVolume
//purpose  : 
//=======================================================================
SMDS_MeshVolume* SMESHDS_Mesh::AddVolume(int idnode1, int idnode2, int idnode3,
	int idnode4)
{
 	SMDS_MeshVolume *f = SMDS_Mesh::AddVolume(idnode1, idnode2, idnode3,
		idnode4);
	if(f!=NULL)
		myScript->AddVolume(f->GetID(), idnode1, idnode2, idnode3, idnode4);
	return f;
}

//=======================================================================
//function :AddVolume
//purpose  : 
//=======================================================================
SMDS_MeshVolume* SMESHDS_Mesh::AddVolume(int idnode1, int idnode2, int idnode3,
	int idnode4, int idnode5)
{
	SMDS_MeshVolume *v = SMDS_Mesh::AddVolume(idnode1, idnode2, idnode3,
		idnode4, idnode5);
	if(v!=NULL)
		myScript->AddVolume(v->GetID(), idnode1, idnode2, idnode3, idnode4,
			idnode5);
	return v;
}

//=======================================================================
//function :AddVolume
//purpose  : 
//=======================================================================
SMDS_MeshVolume* SMESHDS_Mesh::AddVolume(int idnode1, int idnode2, int idnode3,
	int idnode4, int idnode5, int idnode6)
{
	SMDS_MeshVolume *v=
		SMDS_Mesh::AddVolume(idnode1, idnode2, idnode3, idnode4, idnode5,
		idnode6);
	if(v!=NULL) 
		myScript->AddVolume(v->GetID(), idnode1, idnode2, idnode3, idnode4,
			idnode5, idnode6);
	return v;
}

//=======================================================================
//function :AddVolume
//purpose  : 
//=======================================================================
SMDS_MeshVolume* SMESHDS_Mesh::AddVolume(int idnode1, int idnode2, int idnode3,
	int idnode4, int idnode5, int idnode6, int idnode7, int idnode8)
{
	SMDS_MeshVolume *v=
		SMDS_Mesh::AddVolume(idnode1, idnode2, idnode3, idnode4, idnode5,
		idnode6, idnode7, idnode8);
	if(v!=NULL)
		myScript->AddVolume(v->GetID(), idnode1, idnode2, idnode3, idnode4,
			idnode5, idnode6, idnode7, idnode8);
	return v;
}

//=======================================================================
//function : RemoveNode
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::RemoveNode(int ID)
{
	SMDS_Mesh::RemoveNode(ID);
	myScript->RemoveNode(ID);
}

//=======================================================================
//function : RemoveElement
//purpose  : 
//========================================================================
void SMESHDS_Mesh::RemoveElement(int ID)
{
	SMDS_Mesh::RemoveElement(ID);
	myScript->RemoveElement(ID);
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
		myShapeIndexToSubMesh[Index]= new SMESHDS_SubMesh(this);

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
	aNode->SetPosition(new SMDS_FacePosition(Index, 0., 0.));

	//Update or build submesh
	map<int,SMESHDS_SubMesh*>::iterator it=myShapeIndexToSubMesh.find(Index);
	if (it==myShapeIndexToSubMesh.end())
		myShapeIndexToSubMesh[Index]= new SMESHDS_SubMesh(this);

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
	aNode->SetPosition(new SMDS_EdgePosition(Index, 0.));

	//Update or build submesh
	map<int,SMESHDS_SubMesh*>::iterator it=myShapeIndexToSubMesh.find(Index);
	if (it==myShapeIndexToSubMesh.end())
		myShapeIndexToSubMesh[Index]= new SMESHDS_SubMesh(this);

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
	aNode->SetPosition(new SMDS_VertexPosition(Index));

	//Update or build submesh
	map<int,SMESHDS_SubMesh*>::iterator it=myShapeIndexToSubMesh.find(Index);
	if (it==myShapeIndexToSubMesh.end())
		myShapeIndexToSubMesh[Index]= new SMESHDS_SubMesh(this);

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
		myShapeIndexToSubMesh[Index]=new SMESHDS_SubMesh(this);

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

///////////////////////////////////////////////////////////////////////////////
/// Return the sub mesh linked to the a given TopoDS_Shape or NULL if the given
/// TopoDS_Shape is unknown
///////////////////////////////////////////////////////////////////////////////
SMESHDS_SubMesh * SMESHDS_Mesh::MeshElements(const TopoDS_Shape & S)
{
	if (myShape.IsNull()) MESSAGE("myShape is NULL");

	int Index = myIndexToShape.FindIndex(S);
	if (myShapeIndexToSubMesh.find(Index)!=myShapeIndexToSubMesh.end())
		return myShapeIndexToSubMesh[Index];
	else
		return NULL;
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
void SMESHDS_Mesh::NewSubMesh(int Index)
{
	if (myShapeIndexToSubMesh.find(Index)==myShapeIndexToSubMesh.end())
	{
		SMESHDS_SubMesh* SM = new SMESHDS_SubMesh(this);
		myShapeIndexToSubMesh[Index]=SM;
	}
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
	if (myShape.IsNull()) MESSAGE("myShape is NULL");
	return myIndexToShape.FindIndex(S);
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
		myShapeIndexToSubMesh[Index]=new SMESHDS_SubMesh(this);

	myShapeIndexToSubMesh[Index]->AddNode(aNode);
}

//=======================================================================
//function : SetNodeOnFace
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::SetNodeOnFace(SMDS_MeshNode* aNode, int Index)
{
	//Set Position on Node
	aNode->SetPosition(new SMDS_FacePosition(Index, 0., 0.));

	//Update or build submesh
	if (myShapeIndexToSubMesh.find(Index)==myShapeIndexToSubMesh.end())
		myShapeIndexToSubMesh[Index]=new SMESHDS_SubMesh(this);

	myShapeIndexToSubMesh[Index]->AddNode(aNode);
}

//=======================================================================
//function : SetNodeOnEdge
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::SetNodeOnEdge(SMDS_MeshNode* aNode, int Index)
{
	//Set Position on Node
	aNode->SetPosition(new SMDS_EdgePosition(Index, 0.));

	//Update or build submesh
	if (myShapeIndexToSubMesh.find(Index)==myShapeIndexToSubMesh.end())
		myShapeIndexToSubMesh[Index]=new SMESHDS_SubMesh(this);

	myShapeIndexToSubMesh[Index]->AddNode(aNode);
}

//=======================================================================
//function : SetNodeOnVertex
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::SetNodeOnVertex(SMDS_MeshNode* aNode, int Index)
{
	//Set Position on Node
	aNode->SetPosition(new SMDS_VertexPosition(Index));

	//Update or build submesh
	if (myShapeIndexToSubMesh.find(Index)==myShapeIndexToSubMesh.end())
		myShapeIndexToSubMesh[Index]=new SMESHDS_SubMesh(this);

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
		myShapeIndexToSubMesh[Index]=new SMESHDS_SubMesh(this);

	myShapeIndexToSubMesh[Index]->AddElement(anElement);
}
