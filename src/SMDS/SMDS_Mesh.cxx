//  SMESH SMDS : implementaion of Salome mesh data structure
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

#include "utilities.h"
#include "SMDS_Mesh.hxx"
#include "SMDS_VolumeOfNodes.hxx"
#include "SMDS_VolumeOfFaces.hxx"
#include "SMDS_FaceOfNodes.hxx"
#include "SMDS_FaceOfEdges.hxx"

///////////////////////////////////////////////////////////////////////////////
/// Create a new mesh object
///////////////////////////////////////////////////////////////////////////////
SMDS_Mesh::SMDS_Mesh()
	:myNodeIDFactory(new SMDS_MeshElementIDFactory()),
	myElementIDFactory(new SMDS_MeshElementIDFactory()),
	myHasConstructionEdges(false), myHasConstructionFaces(false),
	myHasInverseElements(true)
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SMDS_Mesh::SMDS_Mesh(SMDS_Mesh * parent)
	:myParent(parent), myNodeIDFactory(parent->myNodeIDFactory),
	myElementIDFactory(parent->myElementIDFactory),
	myHasConstructionEdges(false), myHasConstructionFaces(false),
	myHasInverseElements(true)
{
}

///////////////////////////////////////////////////////////////////////////////
///Create a submesh and add it to the current mesh
///////////////////////////////////////////////////////////////////////////////

SMDS_Mesh *SMDS_Mesh::AddSubMesh()
{
	SMDS_Mesh *submesh = new SMDS_Mesh(this);
	myChildren.insert(myChildren.end(), submesh);
	return submesh;
}

///////////////////////////////////////////////////////////////////////////////
///create a MeshNode and add it to the current Mesh
///@return : The created node
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshNode * SMDS_Mesh::AddNode(double x, double y, double z)
{
	return AddNodeWithID(x,y,z,myNodeIDFactory->GetFreeID());
}

///////////////////////////////////////////////////////////////////////////////
///create a MeshNode and add it to the current Mesh
///@param ID : The ID of the MeshNode to create
///@return : The created node or NULL if a node with this ID already exists
///////////////////////////////////////////////////////////////////////////////
SMDS_MeshNode * SMDS_Mesh::AddNodeWithID(double x, double y, double z, int ID)
{
	// find the MeshNode corresponding to ID
	const SMDS_MeshElement *node = myNodeIDFactory->MeshElement(ID);

	if (node == NULL)
	{
		SMDS_MeshNode * node=new SMDS_MeshNode(x, y, z);
		myNodes.insert(node);
		myNodeIDFactory->BindID(ID,node);
		return node;
	}
	else
		return NULL;
}

///////////////////////////////////////////////////////////////////////////////
///create a MeshEdge and add it to the current Mesh
///@return : The created MeshEdge
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshEdge* SMDS_Mesh::AddEdge(int idnode1, int idnode2)
{
	return AddEdgeWithID(idnode1, idnode2, myElementIDFactory->GetFreeID());
}

///////////////////////////////////////////////////////////////////////////////
///Create a new edge and at it to the mesh
///@param idnode1 ID of the first node
///@param idnode2 ID of the second node
///@param ID ID of the edge to create
///@return The created edge or NULL if an edge with this ID already exists or
///if input nodes are not found.
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshEdge* SMDS_Mesh::AddEdgeWithID(int idnode1, int idnode2, int ID)
{
	SMDS_MeshNode * node1 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode1);
	SMDS_MeshNode * node2 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode2);
	if((node1==NULL)||(node2==NULL)) return NULL;
	SMDS_MeshEdge * edge=new SMDS_MeshEdge(node1, node2);
	if(myElementIDFactory->BindID(ID, edge))
	{
		node1->AddInverseElement(edge);
		node2->AddInverseElement(edge);		
		myEdges.insert(edge);
		return edge;
	} 
	else
	{
		delete edge;
		return NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// Add a triangle defined by its nodes IDs
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshFace* SMDS_Mesh::AddFace(int idnode1, int idnode2, int idnode3)
{
	return AddFaceWithID(idnode1,idnode2,idnode3,
		myElementIDFactory->GetFreeID());
}

///////////////////////////////////////////////////////////////////////////////
/// Add a quandrangle defined by its nodes IDs
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshFace* SMDS_Mesh::AddFaceWithID(int idnode1, int idnode2, int idnode3, int ID)
{
	SMDS_MeshNode * node1 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode1);
	SMDS_MeshNode * node2 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode2);
	SMDS_MeshNode * node3 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode3);

	if((node1==NULL)||(node2==NULL)||(node3==NULL)) return NULL;
    SMDS_MeshFace * face=createTriangle(node1,node2,node3);

	if(myElementIDFactory->BindID(ID, face))
	{
		node1->AddInverseElement(face);
		node2->AddInverseElement(face);
        node3->AddInverseElement(face);
		return face;
	}	
	else
	{
		RemoveFace(face);
		return NULL;
	}
}

//=======================================================================
//function : AddFace
//purpose  :
//=======================================================================

SMDS_MeshFace* SMDS_Mesh::AddFace(int idnode1,
	int idnode2, int idnode3, int idnode4)
{
	int ID=myElementIDFactory->GetFreeID();
	SMDS_MeshFace* f= AddFaceWithID(idnode1, idnode2, idnode3, idnode4, ID);
	if(f==NULL) myElementIDFactory->ReleaseID(ID);
}

//=======================================================================
//function : AddFace
//purpose  :
//=======================================================================

SMDS_MeshFace* SMDS_Mesh::AddFaceWithID(int idnode1,
	int idnode2, int idnode3, int idnode4, int ID)
{
	SMDS_MeshNode * node1 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode1);
	SMDS_MeshNode * node2 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode2);
	SMDS_MeshNode * node3 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode3);
	SMDS_MeshNode * node4 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode4);

	if((node1==NULL)||(node2==NULL)||(node3==NULL)||(node4==NULL)) return NULL;
    SMDS_MeshFace * face=createQuadrangle(node1,node2,node3,node4);

	if(myElementIDFactory->BindID(ID, face))
	{
		node1->AddInverseElement(face);
		node2->AddInverseElement(face);
        node3->AddInverseElement(face);
		node4->AddInverseElement(face);
		return face;
	}	
	else
	{
		RemoveFace(face);
		return NULL;
	}
}

//=======================================================================
//function : AddVolume
//purpose  : Tetrahedra
//=======================================================================

SMDS_MeshVolume * SMDS_Mesh::AddVolume(int idnode1, int idnode2, int idnode3,
	int idnode4)
{
	int ID = myElementIDFactory->GetFreeID();
	SMDS_MeshVolume * v=
		AddVolumeWithID(idnode1, idnode2, idnode3, idnode4, ID);
	if(v==NULL) myElementIDFactory->ReleaseID(ID);
}


///////////////////////////////////////////////////////////////////////////////
///Create a new tetrahedron and add it to the mesh. 
///@return The created tetrahedron or NULL if an edge with this ID already exists
///or if input nodes are not found.
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume * SMDS_Mesh::AddVolumeWithID(int idnode1, int idnode2,
	int idnode3, int idnode4, int ID)
{
	SMDS_MeshNode * node1 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode1);
	SMDS_MeshNode * node2 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode2);
	SMDS_MeshNode * node3 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode3);
	SMDS_MeshNode * node4 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode4);

	if((node1==NULL)||(node2==NULL)||(node3==NULL)||(node4=NULL)) return NULL;
	
	SMDS_MeshVolume* volume;
	if(hasConstructionFaces())
	{
		SMDS_MeshFace * f1=createTriangle(node1,node2,node3);
		SMDS_MeshFace * f2=createTriangle(node1,node2,node4);
		SMDS_MeshFace * f3=createTriangle(node1,node3,node4);
		SMDS_MeshFace * f4=createTriangle(node2,node3,node4);
		SMDS_MeshVolume* volume=
			new SMDS_VolumeOfFaces(f1,f2,f3,f4);
		myVolumes.insert(volume);
	}
	else if(hasConstructionEdges())
	{
		/** @todo */
	}
	else
	{
		/** @todo */
	}

	if(myElementIDFactory->BindID(ID, volume))
	{
		node1->AddInverseElement(volume);
		node2->AddInverseElement(volume);
        node3->AddInverseElement(volume);
		node4->AddInverseElement(volume);
		return volume;
	}	
	else
	{
		RemoveVolume(volume);
		return NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// Add a pyramid to the mesh. node 1,2,3 and 4 define the base of the pyramid
///////////////////////////////////////////////////////////////////////////////
SMDS_MeshVolume * SMDS_Mesh::AddVolume(int idnode1,
	int idnode2, int idnode3, int idnode4, int idnode5)
{
	int ID = myElementIDFactory->GetFreeID();
	SMDS_MeshVolume * v=
		AddVolumeWithID(idnode1, idnode2, idnode3, idnode4, idnode5, ID);
	if(v==NULL) myElementIDFactory->ReleaseID(ID);
}

///////////////////////////////////////////////////////////////////////////////
/// Add a pyramid to the mesh. node 1,2,3 and 4 define the base of the pyramid
///////////////////////////////////////////////////////////////////////////////
SMDS_MeshVolume* SMDS_Mesh::AddVolumeWithID(int idnode1, int idnode2,
	int idnode3, int idnode4, int idnode5, int ID)
{
	SMDS_MeshNode * node1 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode1);
	SMDS_MeshNode * node2 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode2);
	SMDS_MeshNode * node3 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode3);
	SMDS_MeshNode * node4 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode4);
	SMDS_MeshNode * node5 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode5);

	if((node1==NULL)||(node2==NULL)||(node3==NULL)||(node4==NULL)||(node5==NULL))
		return NULL;
	
	SMDS_MeshVolume* volume;
	if(hasConstructionFaces())
	{
		SMDS_MeshFace * f1=createQuadrangle(node1,node2,node3,node4);
		SMDS_MeshFace * f2=createTriangle(node1,node2,node5);
		SMDS_MeshFace * f3=createTriangle(node2,node3,node5);
		SMDS_MeshFace * f4=createTriangle(node3,node4,node5);
		SMDS_MeshFace * f5=createTriangle(node4,node1,node4);
		SMDS_MeshVolume* volume=new SMDS_VolumeOfFaces(f1,f2,f3,f4,f5);
		myVolumes.insert(volume);
	}
	else if(hasConstructionEdges())
	{
		/** @todo */
	}
	else
	{
		/** @todo */
	}

	if(myElementIDFactory->BindID(ID, volume))
	{
		node1->AddInverseElement(volume);
		node2->AddInverseElement(volume);
        node3->AddInverseElement(volume);
		node4->AddInverseElement(volume);
		node5->AddInverseElement(volume);
		return volume;
	}	
	else
	{
		RemoveVolume(volume);
		return NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// Add a prism. Nodes 1,2,3 is a triangle and 1,2,5,4 a quadrangle.
///////////////////////////////////////////////////////////////////////////////
SMDS_MeshVolume* SMDS_Mesh::AddVolume(int idnode1, int idnode2, int idnode3,
	int idnode4, int idnode5, int idnode6)
{
	int ID = myElementIDFactory->GetFreeID();
	SMDS_MeshVolume * v=
		AddVolumeWithID(idnode1, idnode2, idnode3, idnode4, idnode5, idnode6,
		ID);
	if(v==NULL) myElementIDFactory->ReleaseID(ID);
}

//=======================================================================
//function : AddVolume
//purpose  : Prism
//=======================================================================

SMDS_MeshVolume* SMDS_Mesh::AddVolumeWithID(int idnode1, int idnode2, int idnode3,
	int idnode4, int idnode5, int idnode6, int ID)
{
	SMDS_MeshNode * node1 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode1);
	SMDS_MeshNode * node2 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode2);
	SMDS_MeshNode * node3 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode3);
	SMDS_MeshNode * node4 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode4);
	SMDS_MeshNode * node5 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode5);
	SMDS_MeshNode * node6 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode6);

	if((node1==NULL)||(node2==NULL)||(node3==NULL)||(node4==NULL)||(node5==NULL)||
		(node6==NULL)) return NULL;
	
	SMDS_MeshVolume* volume;
	if(hasConstructionFaces())

	{
		SMDS_MeshFace * f1=createTriangle(node1,node2,node3);
		SMDS_MeshFace * f2=createTriangle(node4,node5,node6);
		SMDS_MeshFace * f3=createQuadrangle(node1,node4,node5,node2);
		SMDS_MeshFace * f4=createQuadrangle(node2,node5,node6,node3);
		SMDS_MeshFace * f5=createQuadrangle(node3,node6,node4,node1);
		
		SMDS_MeshVolume* volume=new SMDS_VolumeOfFaces(f1,f2,f3,f4,f5);
		myVolumes.insert(volume);
	}
	else if(hasConstructionEdges())
	{
		/** @todo */
	}
	else
	{
		/** @todo */
	}

	if(myElementIDFactory->BindID(ID, volume))
	{
		node1->AddInverseElement(volume);
		node2->AddInverseElement(volume);
        node3->AddInverseElement(volume);
		node4->AddInverseElement(volume);
		node5->AddInverseElement(volume);
		node6->AddInverseElement(volume);
		return volume;
	}	
	else
	{
		RemoveVolume(volume);
		return NULL;
	}
}

//=======================================================================
//function : AddVolume
//purpose  : Hexahedra
//=======================================================================

SMDS_MeshVolume* SMDS_Mesh::AddVolume(int idnode1, int idnode2, int idnode3,
	int idnode4, int idnode5, int idnode6, int idnode7, int idnode8)
{
	int ID = myElementIDFactory->GetFreeID();
	SMDS_MeshVolume * v=
		AddVolumeWithID(idnode1, idnode2, idnode3, idnode4, idnode5, idnode6,
		idnode7, idnode8, ID);
	if(v==NULL) myElementIDFactory->ReleaseID(ID);
}

///////////////////////////////////////////////////////////////////////////////
/// Add an hexahedron to the mesh. node 1,2,3,4 and 5,6,7,8 are quadrangle and
/// 5,1 and 7,3 are an edges.
///////////////////////////////////////////////////////////////////////////////
SMDS_MeshVolume*  SMDS_Mesh::AddVolumeWithID(int idnode1, int idnode2, int idnode3,
	int idnode4, int idnode5, int idnode6, int idnode7, int idnode8, int ID)
{
	SMDS_MeshNode * node1 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode1);
	SMDS_MeshNode * node2 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode2);
	SMDS_MeshNode * node3 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode3);
	SMDS_MeshNode * node4 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode4);
	SMDS_MeshNode * node5 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode5);
	SMDS_MeshNode * node6 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode6);
	SMDS_MeshNode * node7 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode7);
	SMDS_MeshNode * node8 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode8);

	if((node1==NULL)||(node2==NULL)||(node3==NULL)||(node4==NULL)||(node5==NULL)||
		(node6==NULL)||(node7==NULL)||(node8==NULL)) return NULL;
	
	SMDS_MeshVolume* volume;
	if(hasConstructionFaces())
	{
		SMDS_MeshFace * f1=FindFaceOrCreate(node1,node2,node3,node4);
		SMDS_MeshFace * f2=FindFaceOrCreate(node5,node6,node7,node8);
		SMDS_MeshFace * f3=FindFaceOrCreate(node1,node4,node8,node5);
		SMDS_MeshFace * f4=FindFaceOrCreate(node1,node2,node6,node5);
		SMDS_MeshFace * f5=FindFaceOrCreate(node2,node3,node7,node6);
		SMDS_MeshFace * f6=FindFaceOrCreate(node3,node4,node8,node7);
		volume=new SMDS_VolumeOfFaces(f1,f2,f3,f4,f5,f6);
		myVolumes.insert(volume);
	}
	else if(hasConstructionEdges())
	{
		MESSAGE("Error : Not Implemented");
	}
	else
	{
		volume=new SMDS_VolumeOfNodes(node1,node2,node3,node4,node5,node6,node7,node8);
		myVolumes.insert(volume);
	}

	if(myElementIDFactory->BindID(ID, volume))
	{
		//MESSAGE("SMDS_Mesh::AddVolumeWithID: update inverse elements");
		node1->AddInverseElement(volume);
		node2->AddInverseElement(volume);
        node3->AddInverseElement(volume);
		node4->AddInverseElement(volume);
		node5->AddInverseElement(volume);
		node6->AddInverseElement(volume);
		node7->AddInverseElement(volume);
		node8->AddInverseElement(volume);
		return volume;
	}	
	else
	{
		RemoveVolume(volume);
		return NULL;
	}
}

//=======================================================================
//function : FindNode
//purpose  :
//=======================================================================

const SMDS_MeshNode * SMDS_Mesh::FindNode(int ID) const
{
	return (const SMDS_MeshNode *)myNodeIDFactory->MeshElement(ID);
}

///////////////////////////////////////////////////////////////////////////////
///Create a triangle and add it to the current mesh. This methode do not bind a
///ID to the create triangle.
///////////////////////////////////////////////////////////////////////////////
SMDS_MeshFace * SMDS_Mesh::createTriangle(SMDS_MeshNode * node1,
	SMDS_MeshNode * node2, SMDS_MeshNode * node3)
{
	if(hasConstructionEdges())
	{
		SMDS_MeshEdge *edge1, *edge2, *edge3;
		edge1=FindEdgeOrCreate(node1,node2);
		edge2=FindEdgeOrCreate(node2,node3);
		edge3=FindEdgeOrCreate(node3,node1);

		SMDS_MeshFace * face = new SMDS_FaceOfEdges(edge1,edge2,edge3);
		myFaces.insert(face);
		return face;
	}
	else
	{
		SMDS_MeshFace * face = new SMDS_FaceOfNodes(node1,node2,node3);
		myFaces.insert(face);
		return face;
	}
}

///////////////////////////////////////////////////////////////////////////////
///Create a quadrangle and add it to the current mesh. This methode do not bind
///a ID to the create triangle.
///////////////////////////////////////////////////////////////////////////////
SMDS_MeshFace * SMDS_Mesh::createQuadrangle(SMDS_MeshNode * node1,
	SMDS_MeshNode * node2, SMDS_MeshNode * node3, SMDS_MeshNode * node4)
{
	if(hasConstructionEdges())
	{
		SMDS_MeshEdge *edge1, *edge2, *edge3, *edge4;
		edge1=FindEdgeOrCreate(node1,node2);
		edge2=FindEdgeOrCreate(node2,node3);
		edge3=FindEdgeOrCreate(node3,node4);
		edge4=FindEdgeOrCreate(node4,node1);

		SMDS_MeshFace * face = new SMDS_FaceOfEdges(edge1,edge2,edge3,edge4);
		myFaces.insert(face);
		return face;
	}
	else
	{
		SMDS_MeshFace * face = new SMDS_FaceOfNodes(node1,node2,node3,node4);
		myFaces.insert(face);
		return face;
	}
}

//=======================================================================
//function : RemoveNode
//purpose  :
//=======================================================================

void SMDS_Mesh::RemoveNode(int IDnode)
{
	RemoveNode(FindNode(IDnode));
}

//=======================================================================
//function : RemoveNode
//purpose  :
//=======================================================================

bool SMDS_Mesh::RemoveNode(const SMDS_MeshNode * node)
{
	SMDS_Iterator<const SMDS_MeshElement *> * it=
		node->GetInverseElementIterator();
	while(it->more()) RemoveElement(it->next(),true);
	myNodeIDFactory->ReleaseID(node->GetID());
	myNodes.erase(const_cast<SMDS_MeshNode*>(node));
}

//=======================================================================
//function : RemoveEdge
//purpose  :
//=======================================================================

void SMDS_Mesh::RemoveEdge(int idnode1, int idnode2)
{
	RemoveEdge(FindEdge(idnode1, idnode2));
}

//=======================================================================
//function : RemoveEdge
//purpose  :
//=======================================================================

void SMDS_Mesh::RemoveEdge(const SMDS_MeshEdge * edge)
{
	/** @todo to be fix */
	myEdges.erase(const_cast<SMDS_MeshEdge*>(edge));
	//removeElementDependencies(edge);
	delete edge;
}

//=======================================================================
//function : RemoveFace
//purpose  :
//=======================================================================

void SMDS_Mesh::RemoveFace(int idnode1, int idnode2, int idnode3)
{
	RemoveFace(FindFace(idnode1, idnode2, idnode3));
}

//=======================================================================
//function : RemoveFace
//purpose  :
//=======================================================================

void SMDS_Mesh::RemoveFace(int idnode1, int idnode2, int idnode3, int idnode4)
{
	RemoveFace(FindFace(idnode1, idnode2, idnode3, idnode4));
}

//=======================================================================
//function : RemoveFace
//purpose  :
//=======================================================================

void SMDS_Mesh::RemoveFace(const SMDS_MeshFace * face)
{
	/** @todo to be fix */
	myFaces.erase(const_cast<SMDS_MeshFace*>(face));
	//removeElementDependencies(face);
	delete face;
}

//=======================================================================
//function : RemoveVolume
//purpose  :
//=======================================================================

void SMDS_Mesh::RemoveVolume(const SMDS_MeshVolume * volume)
{
	/** @todo to be fix */
	myVolumes.erase(const_cast<SMDS_MeshVolume*>(volume));
	//removeElementDependencies(volume);
	delete volume;
}
///////////////////////////////////////////////////////////////////////////////
/// Remove no longer used sub element of an element. Unbind the element ID
///////////////////////////////////////////////////////////////////////////////
void SMDS_Mesh::removeElementDependencies(SMDS_MeshElement * element)
{
	/** @todo to be fix */
	myElementIDFactory->ReleaseID(element->GetID());
	SMDS_Iterator<const SMDS_MeshElement*> * it=element->nodesIterator();
	while(it->more())
	{
		SMDS_MeshNode * node=static_cast<SMDS_MeshNode*>(
			const_cast<SMDS_MeshElement*>(it->next()));
		node->RemoveInverseElement(element);
		if(node->emptyInverseElements()) RemoveNode(node);
	}
}
//=======================================================================
//function : RemoveElement
//purpose  :
//=======================================================================

void SMDS_Mesh::RemoveElement(int IDelem, const bool removenodes)
{
	RemoveElement(myElementIDFactory->MeshElement(IDelem), removenodes);
}

//=======================================================================
//function : RemoveElement
//purpose  :
//=======================================================================

void SMDS_Mesh::RemoveElement(const SMDS_MeshElement * elem,
	const bool removenodes)
{
	/** @todo to be fix */
	switch(elem->GetType())
	{
    case SMDSAbs_Node:
		RemoveNode((const SMDS_MeshNode*)elem);
		return;
    case SMDSAbs_Edge:
		RemoveEdge((const SMDS_MeshEdge*)elem);
		break;
    case SMDSAbs_Face:
		RemoveFace((const SMDS_MeshFace*)elem);
		break;
    case SMDSAbs_Volume:
		RemoveVolume((const SMDS_MeshVolume*)elem);
		break;
    default :
		MESSAGE("remove function : unknown type");
		return;
	}
/*	
	SMDS_Iterator<const SMDS_MeshNode*> * it=elem->nodesIterator();
	while(it->more())
	{
		const SMDS_MeshNode * node=it->next();
		
	}*/
}

//=======================================================================
//function : RemoveFromParent
//purpose  :
//=======================================================================

bool SMDS_Mesh::RemoveFromParent()
{
	if (myParent==NULL) return false;
	else return (myParent->RemoveSubMesh(this));
}

//=======================================================================
//function : RemoveSubMesh
//purpose  :
//=======================================================================

bool SMDS_Mesh::RemoveSubMesh(const SMDS_Mesh * aMesh)
{
	bool found = false;

	list<SMDS_Mesh *>::iterator itmsh=myChildren.begin();
	for (; itmsh!=myChildren.end() && !found; itmsh++)
	{
		SMDS_Mesh * submesh = *itmsh;
		if (submesh == aMesh)
		{
			found = true;
			myChildren.erase(itmsh);
		}
	}

	return found;
}


//=======================================================================
//function : FindEdge
//purpose  :
//=======================================================================

const SMDS_MeshEdge* SMDS_Mesh::FindEdge(int idnode1, int idnode2) const
{
	const SMDS_MeshNode * node1=FindNode(idnode1);
	const SMDS_MeshNode * node2=FindNode(idnode2);
	if((node1==NULL)||(node2==NULL)) return NULL;
	return FindEdge(node1,node2);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////
//#include "Profiler.h"
const SMDS_MeshEdge* SMDS_Mesh::FindEdge(const SMDS_MeshNode * node1,
	const SMDS_MeshNode * node2) const
{
	const SMDS_MeshEdge * toReturn=NULL;
	//PROFILER_Init();
	//PROFILER_Set();
	SMDS_Iterator<const SMDS_MeshElement *>*  it1=node1->edgesIterator();
	//PROFILER_Get(0);
	//PROFILER_Set();
	while(it1->more())
	{
		const SMDS_MeshEdge * e=static_cast<const SMDS_MeshEdge *>
			(it1->next());
		SMDS_Iterator<const SMDS_MeshElement *>* it2=e->nodesIterator();
		while(it2->more())
		{
			if(it2->next()->GetID()==node2->GetID())
			{
				toReturn=e;
				break;
			}
		}
		delete it2;
	}
	//PROFILER_Get(1);
	delete it1;
	return toReturn;
}


SMDS_MeshEdge* SMDS_Mesh::FindEdgeOrCreate(const SMDS_MeshNode * node1,
	const SMDS_MeshNode * node2) 
{
	SMDS_MeshEdge * toReturn=NULL;
	toReturn=const_cast<SMDS_MeshEdge*>(FindEdge(node1,node2));
	if(toReturn==NULL)	
	{
		toReturn=new SMDS_MeshEdge(const_cast<SMDS_MeshNode*>(node1),
			const_cast<SMDS_MeshNode*>(node2));
		myEdges.insert(toReturn);
	} 
	return toReturn;
}

//=======================================================================
//function : FindFace
//purpose  :
//=======================================================================

const SMDS_MeshFace* SMDS_Mesh::FindFace(int idnode1, int idnode2,
	int idnode3) const
{
	const SMDS_MeshNode * node1=FindNode(idnode1);
	const SMDS_MeshNode * node2=FindNode(idnode2);
	const SMDS_MeshNode * node3=FindNode(idnode3);
	const SMDS_MeshFace * face;
	const SMDS_MeshElement * node;
	bool node2found, node3found;
	if((node1==NULL)||(node2==NULL)||(node3==NULL)) return NULL;

	SMDS_Iterator<const SMDS_MeshElement *>*  it1=node1->facesIterator();
	while(it1->more())
	{
		face=static_cast<const SMDS_MeshFace*>(it1->next());
		if(face->NbNodes()!=3) continue;
		SMDS_Iterator<const SMDS_MeshElement *>* it2=face->nodesIterator();
		node2found=false;
		node3found=false;
		while(it2->more())
		{
			node=it2->next();
			if(node->GetID()==idnode2) node2found=true;
			if(node->GetID()==idnode3) node3found=true;
		}
		delete it2;
		if(node2found&&node3found)
		{
			delete it1;
			return face;
		}
	}
	delete it1;
	return NULL;
}

//=======================================================================
//function : FindFace
//purpose  :
//=======================================================================

const SMDS_MeshFace* SMDS_Mesh::FindFace(int idnode1, int idnode2, int idnode3,
	int idnode4) const
{
	const SMDS_MeshNode * node1=FindNode(idnode1);
	const SMDS_MeshNode * node2=FindNode(idnode2);
	const SMDS_MeshNode * node3=FindNode(idnode3);
	const SMDS_MeshNode * node4=FindNode(idnode4);
	if((node1==NULL)||(node2==NULL)||(node3==NULL)||(node4==NULL)) return NULL;
	return FindFace(node1, node2, node3, node4);
}

const SMDS_MeshFace* SMDS_Mesh::FindFace(
		const SMDS_MeshNode *node1,
		const SMDS_MeshNode *node2,
		const SMDS_MeshNode *node3,
		const SMDS_MeshNode *node4) const
{
	const SMDS_MeshFace * face;
	const SMDS_MeshElement * node;
	bool node2found, node3found, node4found;
	SMDS_Iterator<const SMDS_MeshElement *>*  it1=node1->facesIterator();
	while(it1->more())
	{
		face=static_cast<const SMDS_MeshFace *>(it1->next());
		if(face->NbNodes()!=4) continue;
		SMDS_Iterator<const SMDS_MeshElement *>* it2=face->nodesIterator();
		node2found=false;
		node3found=false;
		node4found=false;
		while(it2->more())
		{
			node=it2->next();
			if(node->GetID()==node2->GetID()) node2found=true;
			if(node->GetID()==node3->GetID()) node3found=true;
			if(node->GetID()==node4->GetID()) node4found=true;
		}
		delete it2;
		if(node2found&&node3found&&node4found)
		{
			delete it1;
			return face;
		}
	}
	delete it1;
	return NULL;
}

SMDS_MeshFace* SMDS_Mesh::FindFaceOrCreate(
		const SMDS_MeshNode *node1,
		const SMDS_MeshNode *node2,
		const SMDS_MeshNode *node3,
		const SMDS_MeshNode *node4)
{
	SMDS_MeshFace * toReturn=NULL;
	toReturn=const_cast<SMDS_MeshFace*>(FindFace(node1,node2,node3,node4));
	if(toReturn==NULL)	
	{
		toReturn=createQuadrangle(
			const_cast<SMDS_MeshNode *>(node1),
			const_cast<SMDS_MeshNode *>(node2),
			const_cast<SMDS_MeshNode *>(node3),
			const_cast<SMDS_MeshNode *>(node4)
		);
	} 
	return toReturn;
}

//=======================================================================
//function : FindElement
//purpose  :
//=======================================================================

const SMDS_MeshElement* SMDS_Mesh::FindElement(int IDelem) const
{
	return myElementIDFactory->MeshElement(IDelem);
}

//=======================================================================
//function : DumpNodes
//purpose  : 
//=======================================================================

void SMDS_Mesh::DumpNodes() const
{
	MESSAGE("dump nodes of mesh : ");
	SMDS_Iterator<const SMDS_MeshNode *> * itnode=nodesIterator();
	while(itnode->more()) MESSAGE(itnode->next());
	delete itnode;
}

//=======================================================================
//function : DumpEdges
//purpose  : 
//=======================================================================

void SMDS_Mesh::DumpEdges() const
{
	MESSAGE("dump edges of mesh : ");
	SMDS_Iterator<const SMDS_MeshEdge *> * itedge=edgesIterator();
	while(itedge->more()) MESSAGE(itedge->next());
	delete itedge;
}

//=======================================================================
//function : DumpFaces
//purpose  : 
//=======================================================================

void SMDS_Mesh::DumpFaces() const
{
	MESSAGE("dump faces of mesh : ");
	SMDS_Iterator<const SMDS_MeshFace *> * itface=facesIterator();
	while(itface->more()) MESSAGE(itface->next());
	delete itface;
}

//=======================================================================
//function : DumpVolumes
//purpose  : 
//=======================================================================

void SMDS_Mesh::DumpVolumes() const
{
	MESSAGE("dump volumes of mesh : ");
	SMDS_Iterator<const SMDS_MeshVolume *> * itvol=volumesIterator();
	while(itvol->more()) MESSAGE(itvol->next());
	delete itvol;
}

//=======================================================================
//function : DebugStats
//purpose  : 
//=======================================================================

void SMDS_Mesh::DebugStats() const
{
	MESSAGE("Debug stats of mesh : ");

	MESSAGE("===== NODES ====="<<NbNodes());
	MESSAGE("===== EDGES ====="<<NbEdges());
	MESSAGE("===== FACES ====="<<NbFaces());
	MESSAGE("===== VOLUMES ====="<<NbVolumes());

	MESSAGE("End Debug stats of mesh ");

	//#ifdef DEB
	
	SMDS_Iterator<const SMDS_MeshNode *> * itnode=nodesIterator();
	int sizeofnodes = 0;
	int sizeoffaces = 0;

	while(itnode->more())
	{
		const SMDS_MeshNode *node = itnode->next();

		sizeofnodes += sizeof(*node);
		
		SMDS_Iterator<const SMDS_MeshElement *> * it=
			node->GetInverseElementIterator();
		while(it->more())
		{
			const SMDS_MeshElement *me = it->next();
			sizeofnodes += sizeof(me);
		}
		delete it;

	}
	delete itnode;
	SMDS_Iterator<const SMDS_MeshFace*>* itface=facesIterator();
	
	while(itface->more())
	{
		const SMDS_MeshElement *face = itface->next();		
		sizeoffaces += sizeof(*face);

	}
	MESSAGE("total size of node elements = " << sizeofnodes);;
	MESSAGE("total size of face elements = " << sizeoffaces);;

	//#endif

}

int SMDS_Mesh::NbNodes() const
{
	myNodes.size();
}

int SMDS_Mesh::NbEdges() const
{
	return myEdges.size();
}

int SMDS_Mesh::NbFaces() const
{
	return myFaces.size();
}

int SMDS_Mesh::NbVolumes() const
{
	return myVolumes.size();
}

int SMDS_Mesh::NbSubMesh() const
{
	return myChildren.size();
}

SMDS_Mesh::~SMDS_Mesh()
{
	if(myParent==NULL)
	{
		delete myNodeIDFactory;
		delete myElementIDFactory;
	}

	list<SMDS_Mesh*>::iterator itc=myChildren.begin();
	while(itc!=myChildren.end())
	{
		delete *itc;
		itc++;
	}
	
	SMDS_Iterator<const SMDS_MeshNode*> * itn=nodesIterator();
	while(itn->more())
	{
		delete itn->next();
	}
	delete itn;

	set<SMDS_MeshEdge*>::iterator ite=myEdges.begin();
	while(ite!=myEdges.end())
	{
		delete *ite;
		ite++;
	}

	set<SMDS_MeshFace*>::iterator itf=myFaces.begin();
	while(itf!=myFaces.end())
	{
		delete *itf;
		itf++;
	}

	set<SMDS_MeshVolume*>::iterator itv=myVolumes.begin();
	while(itv!=myVolumes.end())
	{
		delete *itv;
		itv++;
	}

}

bool SMDS_Mesh::hasConstructionEdges()
{
	return myHasConstructionEdges;
}

bool SMDS_Mesh::hasConstructionFaces()
{
	return myHasConstructionFaces;
}

bool SMDS_Mesh::hasInverseElements()
{
	return myHasInverseElements;
}

void SMDS_Mesh::setConstructionEdges(bool b)
{
	myHasConstructionEdges=b;
}

void SMDS_Mesh::setConstructionFaces(bool b)
{
	 myHasConstructionFaces=b;
}

void SMDS_Mesh::setInverseElements(bool b)
{
	if(!b) MESSAGE("Error : inverseElement=false not implemented");
	myHasInverseElements=b;
}

///////////////////////////////////////////////////////////////////////////////
///Return an iterator on nodes of the current mesh. Once used this iterator
///must be free by the caller
///////////////////////////////////////////////////////////////////////////////
SMDS_Iterator<const SMDS_MeshNode *> * SMDS_Mesh::nodesIterator() const
{
	class MyIterator:public SMDS_Iterator<const SMDS_MeshNode*>
	{
		const SetOfNodes& mySet;
		SetOfNodes::iterator myIterator;
	  public:
		MyIterator(const SetOfNodes& s):mySet(s)
		{
			myIterator=mySet.begin();
		}

		bool more()
		{
			myIterator!=mySet.end();
		}

		const SMDS_MeshNode* next()
		{
			const SMDS_MeshNode* current=*myIterator;
			myIterator++;
			return current;	
		}	
	};
	return new MyIterator(myNodes);
}

///////////////////////////////////////////////////////////////////////////////
///Return an iterator on volumes of the current mesh. Once used this iterator
///must be free by the caller
///////////////////////////////////////////////////////////////////////////////
SMDS_Iterator<const SMDS_MeshEdge *> * SMDS_Mesh::edgesIterator() const
{
	class MyIterator:public SMDS_Iterator<const SMDS_MeshEdge*>
	{
		const SetOfEdges& mySet;
		const SMDS_MeshEdge * myEdge;
		SetOfEdges::iterator myIterator;
	  public:
		MyIterator(const SetOfEdges& s):mySet(s)
		{
			myIterator=mySet.begin();
		}

		bool more()
		{
			while((myIterator!=mySet.end()))
			{
				if((*myIterator)->GetID()!=-1)
					return true;
				myIterator++;
			}
			return false;
		}

		const SMDS_MeshEdge* next()
		{
			const SMDS_MeshEdge* current=*myIterator;
			myIterator++;
			return current;	
		}	
	};
	return new MyIterator(myEdges);
}

///////////////////////////////////////////////////////////////////////////////
///Return an iterator on faces of the current mesh. Once used this iterator
///must be free by the caller
///////////////////////////////////////////////////////////////////////////////
SMDS_Iterator<const SMDS_MeshFace *> * SMDS_Mesh::facesIterator() const
{
	class MyIterator:public SMDS_Iterator<const SMDS_MeshFace*>
	{
		const SetOfFaces& mySet;
		set<SMDS_MeshFace*>::iterator myIterator;
	  public:
		MyIterator(const SetOfFaces& s):mySet(s)
		{
			myIterator=mySet.begin();
		}

		bool more()
		{
			while((myIterator!=mySet.end()))
			{
				if((*myIterator)->GetID()!=-1)
					return true;
				myIterator++;
			}
			return false;
		}

		const SMDS_MeshFace* next()
		{
			const SMDS_MeshFace* current=*myIterator;
			myIterator++;
			return current;	
		}	
	};
	return new MyIterator(myFaces);
}

///////////////////////////////////////////////////////////////////////////////
///Return an iterator on volumes of the current mesh. Once used this iterator
///must be free by the caller
///////////////////////////////////////////////////////////////////////////////
SMDS_Iterator<const SMDS_MeshVolume *> * SMDS_Mesh::volumesIterator() const
{
	class MyIterator:public SMDS_Iterator<const SMDS_MeshVolume*>
	{
		const SetOfVolumes& mySet;
		SetOfVolumes::iterator myIterator;
	  public:
		MyIterator(const SetOfVolumes& s):mySet(s)
		{
			myIterator=mySet.begin();
		}

		bool more()
		{
			myIterator!=mySet.end();
		}

		const SMDS_MeshVolume* next()
		{
			const SMDS_MeshVolume* current=*myIterator;
			myIterator++;
			return current;	
		}	
	};
	return new MyIterator(myVolumes);
}

