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
#include "SMDS_Tria3OfNodes.hxx"
#include "SMDS_HexahedronOfNodes.hxx"
#include "SMDS_FaceOfEdges.hxx"

#include <algorithm>
using namespace std;

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
/// Create a new child mesh
/// Note that the tree structure of SMDS_Mesh seems to be unused in this version
/// (2003-09-08) of SMESH
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
///An ID is automatically assigned to the node.
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
/// create a MeshEdge and add it to the current Mesh
/// @return : The created MeshEdge
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshEdge* SMDS_Mesh::AddEdgeWithID(int idnode1, int idnode2, int ID) 
{
	SMDS_MeshNode * node1 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode1);
	SMDS_MeshNode * node2 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode2);
	if((node1==NULL)||(node2==NULL)) return NULL;
	return AddEdgeWithID(node1, node2, ID);
}

///////////////////////////////////////////////////////////////////////////////
/// create a MeshEdge and add it to the current Mesh
/// @return : The created MeshEdge
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshEdge* SMDS_Mesh::AddEdge(const SMDS_MeshNode * node1,
	const SMDS_MeshNode * node2)
{
	return AddEdgeWithID(node1, node2, myElementIDFactory->GetFreeID());
}

///////////////////////////////////////////////////////////////////////////////
/// Create a new edge and at it to the mesh
/// @param idnode1 ID of the first node
/// @param idnode2 ID of the second node
/// @param ID ID of the edge to create
/// @return The created edge or NULL if an edge with this ID already exists or
/// if input nodes are not found.
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshEdge* SMDS_Mesh::AddEdgeWithID(const SMDS_MeshNode * n1,
		const SMDS_MeshNode * n2, int ID)
{
	SMDS_MeshNode *node1,*node2;
	node1=const_cast<SMDS_MeshNode*>(n1);
	node2=const_cast<SMDS_MeshNode*>(n2);

	SMDS_MeshEdge * edge=new SMDS_MeshEdge(node1,node2);
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
/// Add a triangle defined by its nodes. An ID is automatically affected to the
/// Created face
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshFace* SMDS_Mesh::AddFace(const SMDS_MeshNode * n1,
	const SMDS_MeshNode * n2,
	const SMDS_MeshNode * n3)
{
	return AddFaceWithID(n1,n2,n3, myElementIDFactory->GetFreeID());
}

///////////////////////////////////////////////////////////////////////////////
/// Add a quadrangle defined by its nodes IDs
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshFace* SMDS_Mesh::AddFaceWithID(int idnode1, int idnode2, int idnode3, int ID)
{
	SMDS_MeshNode * node1 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode1);
	SMDS_MeshNode * node2 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode2);
	SMDS_MeshNode * node3 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode3);
	if((node1==NULL)||(node2==NULL)||(node3==NULL)) return NULL;
	return AddFaceWithID(node1, node2, node3, ID);	
}

///////////////////////////////////////////////////////////////////////////////
/// Add a quadrangle defined by its nodes
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshFace* SMDS_Mesh::AddFaceWithID(
	const SMDS_MeshNode * n1,
	const SMDS_MeshNode * n2,
	const SMDS_MeshNode * n3, int ID)
{
   	SMDS_MeshNode *node1, *node2, *node3;
	node1=const_cast<SMDS_MeshNode*>(n1),
	node2=const_cast<SMDS_MeshNode*>(n2),
	node3=const_cast<SMDS_MeshNode*>(n3);
	SMDS_MeshFace * face=createTriangle(node1, node2, node3);

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

///////////////////////////////////////////////////////////////////////////////
/// Add a triangle defined by its nodes. An ID is automatically affected to the
/// created face
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshFace* SMDS_Mesh::AddFace(const SMDS_MeshNode * n1,
	const SMDS_MeshNode * n2,
	const SMDS_MeshNode * n3,
	const SMDS_MeshNode * n4)
{
	return AddFaceWithID(n1,n2,n3, n4, myElementIDFactory->GetFreeID());
}

///////////////////////////////////////////////////////////////////////////////
/// Add a quadrangle defined by its nodes IDs
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshFace* SMDS_Mesh::AddFaceWithID(int idnode1, int idnode2, int idnode3,
	int idnode4, int ID)
{
	SMDS_MeshNode *node1, *node2, *node3, *node4;
	node1 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode1);
	node2 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode2);
	node3 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode3);
	node4 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode4);
	if((node1==NULL)||(node2==NULL)||(node3==NULL)) return NULL;
	return AddFaceWithID(node1, node2, node3, node4, ID);	
}

///////////////////////////////////////////////////////////////////////////////
/// Add a quadrangle defined by its nodes
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshFace* SMDS_Mesh::AddFaceWithID(const SMDS_MeshNode * n1,
	const SMDS_MeshNode * n2, const SMDS_MeshNode * n3,
	const SMDS_MeshNode * n4, int ID)
{
   	SMDS_MeshNode *node1, *node2, *node3, *node4;
	node1=const_cast<SMDS_MeshNode*>(n1),
	node2=const_cast<SMDS_MeshNode*>(n2),
	node3=const_cast<SMDS_MeshNode*>(n3);
	node4=const_cast<SMDS_MeshNode*>(n4);
	SMDS_MeshFace * face=createQuadrangle(node1, node2, node3, node4);

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

///////////////////////////////////////////////////////////////////////////////
///Create a new tetrahedron and add it to the mesh. 
///@return The created tetrahedron 
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolume(const SMDS_MeshNode * n1,
	const SMDS_MeshNode * n2, const SMDS_MeshNode * n3,
	const SMDS_MeshNode * n4)
{
	int ID = myElementIDFactory->GetFreeID();
	SMDS_MeshVolume * v = AddVolumeWithID(n1, n2, n3, n4, ID);
	if(v==NULL) myElementIDFactory->ReleaseID(ID);
	return v;
}

///////////////////////////////////////////////////////////////////////////////
///Create a new tetrahedron and add it to the mesh. 
///@param ID The ID of the new volume
///@return The created tetrahedron or NULL if an edge with this ID already exists
///or if input nodes are not found.
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume * SMDS_Mesh::AddVolumeWithID(int idnode1, int idnode2,
	int idnode3, int idnode4, int ID)
{
	SMDS_MeshNode *node1, *node2, *node3, *node4;
	node1 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode1);
	node2 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode2);
	node3 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode3);
	node4 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode4);
	if((node1==NULL)||(node2==NULL)||(node3==NULL)||(node4=NULL)) return NULL;
	return AddVolumeWithID(node1, node2, node3, node4, ID);
}
	
///////////////////////////////////////////////////////////////////////////////
///Create a new tetrahedron and add it to the mesh. 
///@param ID The ID of the new volume
///@return The created tetrahedron 
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolumeWithID(
		const SMDS_MeshNode * n1,
		const SMDS_MeshNode * n2,
		const SMDS_MeshNode * n3,
		const SMDS_MeshNode * n4, int ID)
{
	SMDS_MeshNode *node1, *node2, *node3, *node4;
	node1=const_cast<SMDS_MeshNode*>(n1),
	node2=const_cast<SMDS_MeshNode*>(n2),
	node3=const_cast<SMDS_MeshNode*>(n3);
	node4=const_cast<SMDS_MeshNode*>(n4);
	SMDS_MeshVolume* volume;
	if(hasConstructionFaces())
	{
		SMDS_MeshFace * f1=createTriangle(node1,node2,node3);
		SMDS_MeshFace * f2=createTriangle(node1,node2,node4);
		SMDS_MeshFace * f3=createTriangle(node1,node3,node4);
		SMDS_MeshFace * f4=createTriangle(node2,node3,node4);
		volume=new SMDS_VolumeOfFaces(f1,f2,f3,f4);
		myVolumes.insert(volume);
	}
	else if(hasConstructionEdges())
	{
		MESSAGE("Error : Not implemented");
		return NULL;
	}
	else
	{
		volume=new SMDS_VolumeOfNodes(node1,node2,node3,node4);
		myVolumes.insert(volume);
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
///Create a new pyramid and add it to the mesh. 
///Nodes 1,2,3 and 4 define the base of the pyramid
///@return The created pyramid 
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolume(const SMDS_MeshNode * n1,
	const SMDS_MeshNode * n2, const SMDS_MeshNode * n3,
	const SMDS_MeshNode * n4, const SMDS_MeshNode * n5)
{
	int ID = myElementIDFactory->GetFreeID();
	SMDS_MeshVolume * v = AddVolumeWithID(n1, n2, n3, n4, n5, ID);
	if(v==NULL) myElementIDFactory->ReleaseID(ID);
	return v;
}

///////////////////////////////////////////////////////////////////////////////
///Create a new pyramid and add it to the mesh. 
///Nodes 1,2,3 and 4 define the base of the pyramid
///@param ID The ID of the new volume
///@return The created pyramid or NULL if a pyramid with this ID already exists
///or if input nodes are not found.
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume * SMDS_Mesh::AddVolumeWithID(int idnode1, int idnode2,
	int idnode3, int idnode4, int idnode5, int ID)
{
	SMDS_MeshNode *node1, *node2, *node3, *node4, *node5;
	node1 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode1);
	node2 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode2);
	node3 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode3);
	node4 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode4);
	node5 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode5);
	if((node1==NULL)||(node2==NULL)||(node3==NULL)||(node4=NULL)||
		(node5=NULL))
		return NULL;
	return AddVolumeWithID(node1, node2, node3, node4, node5, ID);
}
	
///////////////////////////////////////////////////////////////////////////////
///Create a new pyramid and add it to the mesh.
///Nodes 1,2,3 and 4 define the base of the pyramid
///@param ID The ID of the new volume
///@return The created pyramid
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolumeWithID(
		const SMDS_MeshNode * n1,
		const SMDS_MeshNode * n2,
		const SMDS_MeshNode * n3,
		const SMDS_MeshNode * n4,
		const SMDS_MeshNode * n5, int ID)
{
	SMDS_MeshNode *node1, *node2, *node3, *node4, *node5;
	node1=const_cast<SMDS_MeshNode*>(n1),
	node2=const_cast<SMDS_MeshNode*>(n2),
	node3=const_cast<SMDS_MeshNode*>(n3);
	node4=const_cast<SMDS_MeshNode*>(n4);
	node5=const_cast<SMDS_MeshNode*>(n5);
	SMDS_MeshVolume* volume;
	if(hasConstructionFaces())
	{
		SMDS_MeshFace * f1=createQuadrangle(node1,node2,node3,node4);
		SMDS_MeshFace * f2=createTriangle(node1,node2,node5);
		SMDS_MeshFace * f3=createTriangle(node2,node3,node5);
		SMDS_MeshFace * f4=createTriangle(node3,node4,node5);
		volume=new SMDS_VolumeOfFaces(f1,f2,f3,f4);
		myVolumes.insert(volume);
	}
	else if(hasConstructionEdges())
	{
		MESSAGE("Error : Not implemented");
		return NULL;
	}
	else
	{
		volume=new SMDS_VolumeOfNodes(node1,node2,node3,node4,node5);
		myVolumes.insert(volume);
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
///Create a new prism and add it to the mesh. 
///Nodes 1,2,3 is a triangle and 1,2,5,4 a quadrangle.
///@return The created prism 
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolume(const SMDS_MeshNode * n1,
	const SMDS_MeshNode * n2, const SMDS_MeshNode * n3,
	const SMDS_MeshNode * n4, const SMDS_MeshNode * n5,
	const SMDS_MeshNode * n6)
{
	int ID = myElementIDFactory->GetFreeID();
	SMDS_MeshVolume * v = AddVolumeWithID(n1, n2, n3, n4, n5, n6, ID);
	if(v==NULL) myElementIDFactory->ReleaseID(ID);
	return v;
}

///////////////////////////////////////////////////////////////////////////////
///Create a new prism and add it to the mesh. 
///Nodes 1,2,3 is a triangle and 1,2,5,4 a quadrangle.
///@param ID The ID of the new volume
///@return The created prism or NULL if a prism with this ID already exists
///or if input nodes are not found.
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume * SMDS_Mesh::AddVolumeWithID(int idnode1, int idnode2,
	int idnode3, int idnode4, int idnode5, int idnode6, int ID)
{
	SMDS_MeshNode *node1, *node2, *node3, *node4, *node5, *node6;
	node1 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode1);
	node2 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode2);
	node3 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode3);
	node4 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode4);
	node5 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode5);
	node6 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode6);
	if((node1==NULL)||(node2==NULL)||(node3==NULL)||(node4=NULL)||
		(node5==NULL)||(node6=NULL))
		return NULL;
	return AddVolumeWithID(node1, node2, node3, node4, node5, node6, ID);
}
	
///////////////////////////////////////////////////////////////////////////////
///Create a new prism and add it to the mesh.
///Nodes 1,2,3 is a triangle and 1,2,5,4 a quadrangle.
///@param ID The ID of the new volume
///@return The created prism
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolumeWithID(
		const SMDS_MeshNode * n1,
		const SMDS_MeshNode * n2,
		const SMDS_MeshNode * n3,
		const SMDS_MeshNode * n4,
		const SMDS_MeshNode * n5,
		const SMDS_MeshNode * n6, int ID)
{
	SMDS_MeshNode *node1, *node2, *node3, *node4, *node5, *node6;
	node1=const_cast<SMDS_MeshNode*>(n1),
	node2=const_cast<SMDS_MeshNode*>(n2),
	node3=const_cast<SMDS_MeshNode*>(n3);
	node4=const_cast<SMDS_MeshNode*>(n4);
	node5=const_cast<SMDS_MeshNode*>(n5);
	node6=const_cast<SMDS_MeshNode*>(n6);
	SMDS_MeshVolume* volume;
	if(hasConstructionFaces())
	{
		SMDS_MeshFace * f1=createTriangle(node1,node2,node3);
		SMDS_MeshFace * f2=createTriangle(node4,node5,node6);
		SMDS_MeshFace * f3=createQuadrangle(node1,node4,node5,node2);
		SMDS_MeshFace * f4=createQuadrangle(node2,node5,node6,node3);
		SMDS_MeshFace * f5=createQuadrangle(node3,node6,node4,node1);
		volume=new SMDS_VolumeOfFaces(f1,f2,f3,f4,f5);
		myVolumes.insert(volume);
	}
	else if(hasConstructionEdges())
	{
		MESSAGE("Error : Not implemented");
		return NULL;
	}
	else
	{
		volume=new SMDS_VolumeOfNodes(node1,node2,node3,node4,node5,node6);
		myVolumes.insert(volume);
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

///////////////////////////////////////////////////////////////////////////////
///Create a new hexahedron and add it to the mesh. 
///Nodes 1,2,3,4 and 5,6,7,8 are quadrangle and 5,1 and 7,3 are an edges.
///@return The created hexahedron 
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolume(const SMDS_MeshNode * n1,
	const SMDS_MeshNode * n2, const SMDS_MeshNode * n3,
	const SMDS_MeshNode * n4, const SMDS_MeshNode * n5,
	const SMDS_MeshNode * n6, const SMDS_MeshNode * n7,
	const SMDS_MeshNode * n8)
{
	int ID = myElementIDFactory->GetFreeID();
	SMDS_MeshVolume * v = AddVolumeWithID(n1, n2, n3, n4, n5, n6, n7, n8, ID);
	if(v==NULL) myElementIDFactory->ReleaseID(ID);
	return v;
}

///////////////////////////////////////////////////////////////////////////////
///Create a new hexahedron and add it to the mesh. 
///Nodes 1,2,3,4 and 5,6,7,8 are quadrangle and 5,1 and 7,3 are an edges.
///@param ID The ID of the new volume
///@return The created hexahedron or NULL if an hexahedron with this ID already
///exists or if input nodes are not found.
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume * SMDS_Mesh::AddVolumeWithID(int idnode1, int idnode2,
	int idnode3, int idnode4, int idnode5, int idnode6, int idnode7,
	int idnode8, int ID)
{
	SMDS_MeshNode *node1, *node2, *node3, *node4, *node5, *node6, *node7, *node8;
	node1 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode1);
	node2 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode2);
	node3 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode3);
	node4 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode4);
	node5 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode5);
	node6 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode6);
	node7 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode7);
	node8 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode8);
	if((node1==NULL)||(node2==NULL)||(node3==NULL)||(node4==NULL)||
		(node5==NULL)||(node6==NULL)||(node7==NULL)||(node8==NULL))
		return NULL;
	return AddVolumeWithID(node1, node2, node3, node4, node5, node6, node7,
		node8, ID);
}
	
///////////////////////////////////////////////////////////////////////////////
///Create a new hexahedron and add it to the mesh.
///Nodes 1,2,3,4 and 5,6,7,8 are quadrangle and 5,1 and 7,3 are an edges.
///@param ID The ID of the new volume
///@return The created prism or NULL if an hexadron with this ID already exists
///or if input nodes are not found.
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolumeWithID(
		const SMDS_MeshNode * n1,
		const SMDS_MeshNode * n2,
		const SMDS_MeshNode * n3,
		const SMDS_MeshNode * n4,
		const SMDS_MeshNode * n5,
		const SMDS_MeshNode * n6,
		const SMDS_MeshNode * n7,
		const SMDS_MeshNode * n8, int ID)
{
	SMDS_MeshNode *node1, *node2, *node3, *node4, *node5, *node6, *node7, *node8;
	node1=const_cast<SMDS_MeshNode*>(n1),
	node2=const_cast<SMDS_MeshNode*>(n2),
	node3=const_cast<SMDS_MeshNode*>(n3);
	node4=const_cast<SMDS_MeshNode*>(n4);
	node5=const_cast<SMDS_MeshNode*>(n5);
	node6=const_cast<SMDS_MeshNode*>(n6);
	node7=const_cast<SMDS_MeshNode*>(n7);
	node8=const_cast<SMDS_MeshNode*>(n8);
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
		MESSAGE("Error : Not implemented");
		return NULL;
	}
	else
	{
		volume=new SMDS_HexahedronOfNodes(node1,node2,node3,node4,node5,node6,
			node7,node8);
		myVolumes.insert(volume);
	}

	if(myElementIDFactory->BindID(ID, volume))
	{
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

///////////////////////////////////////////////////////////////////////////////
/// Return the node whose ID is 'ID'.
///////////////////////////////////////////////////////////////////////////////
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
		SMDS_MeshFace * face = new SMDS_Tria3OfNodes(node1,node2,node3);
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

///////////////////////////////////////////////////////////////////////////////
/// Remove a node and all the elements which own this node
///////////////////////////////////////////////////////////////////////////////

void SMDS_Mesh::RemoveNode(const SMDS_MeshNode * node)
{
	RemoveElement(node, true);
}

///////////////////////////////////////////////////////////////////////////////
/// Remove an edge and all the elements which own this edge
///////////////////////////////////////////////////////////////////////////////

void SMDS_Mesh::RemoveEdge(const SMDS_MeshEdge * edge)
{
	RemoveElement(edge,true);
}

///////////////////////////////////////////////////////////////////////////////
/// Remove an face and all the elements which own this face
///////////////////////////////////////////////////////////////////////////////

void SMDS_Mesh::RemoveFace(const SMDS_MeshFace * face)
{
	RemoveElement(face, true);
}

///////////////////////////////////////////////////////////////////////////////
/// Remove a volume
///////////////////////////////////////////////////////////////////////////////

void SMDS_Mesh::RemoveVolume(const SMDS_MeshVolume * volume)
{
	RemoveElement(volume, true);
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

///////////////////////////////////////////////////////////////////////////////
/// Return the number of nodes
///////////////////////////////////////////////////////////////////////////////
int SMDS_Mesh::NbNodes() const
{
	return myNodes.size();
}

///////////////////////////////////////////////////////////////////////////////
/// Return the number of edges (including construction edges)
///////////////////////////////////////////////////////////////////////////////
int SMDS_Mesh::NbEdges() const
{
	return myEdges.size();
}

///////////////////////////////////////////////////////////////////////////////
/// Return the number of faces (including construction faces)
///////////////////////////////////////////////////////////////////////////////
int SMDS_Mesh::NbFaces() const
{
	return myFaces.size();
}

///////////////////////////////////////////////////////////////////////////////
/// Return the number of volumes
///////////////////////////////////////////////////////////////////////////////
int SMDS_Mesh::NbVolumes() const
{
	return myVolumes.size();
}

///////////////////////////////////////////////////////////////////////////////
/// Return the number of child mesh of this mesh.
/// Note that the tree structure of SMDS_Mesh seems to be unused in this version
/// (2003-09-08) of SMESH
///////////////////////////////////////////////////////////////////////////////
int SMDS_Mesh::NbSubMesh() const
{
	return myChildren.size();
}

///////////////////////////////////////////////////////////////////////////////
/// Destroy the mesh and all its elements
/// All pointer on elements owned by this mesh become illegals.
///////////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////////
/// Return true if this mesh create faces with edges.
/// A false returned value mean that faces are created with nodes. A concequence
/// is, iteration on edges (SMDS_Element::edgesIterator) will be unavailable.
///////////////////////////////////////////////////////////////////////////////
bool SMDS_Mesh::hasConstructionEdges()
{
	return myHasConstructionEdges;
}

///////////////////////////////////////////////////////////////////////////////
/// Return true if this mesh create volumes with faces
/// A false returned value mean that volumes are created with nodes or edges.
/// (see hasConstructionEdges)
/// A concequence is, iteration on faces (SMDS_Element::facesIterator) will be
/// unavailable.
///////////////////////////////////////////////////////////////////////////////
bool SMDS_Mesh::hasConstructionFaces()
{
	return myHasConstructionFaces;
}

///////////////////////////////////////////////////////////////////////////////
/// Return true if nodes are linked to the finit elements, they are belonging to.
/// Currently, It always return true.
///////////////////////////////////////////////////////////////////////////////
bool SMDS_Mesh::hasInverseElements()
{
	return myHasInverseElements;
}

///////////////////////////////////////////////////////////////////////////////
/// Make this mesh creating construction edges (see hasConstructionEdges)
/// @param b true to have construction edges, else false.
///////////////////////////////////////////////////////////////////////////////
void SMDS_Mesh::setConstructionEdges(bool b)
{
	myHasConstructionEdges=b;
}

///////////////////////////////////////////////////////////////////////////////
/// Make this mesh creating construction faces (see hasConstructionFaces)
/// @param b true to have construction faces, else false.
///////////////////////////////////////////////////////////////////////////////
void SMDS_Mesh::setConstructionFaces(bool b)
{
	 myHasConstructionFaces=b;
}

///////////////////////////////////////////////////////////////////////////////
/// Make this mesh creating link from nodes to elements (see hasInverseElements)
/// @param b true to link nodes to elements, else false.
///////////////////////////////////////////////////////////////////////////////
void SMDS_Mesh::setInverseElements(bool b)
{
	if(!b) MESSAGE("Error : inverseElement=false not implemented");
	myHasInverseElements=b;
}

///////////////////////////////////////////////////////////////////////////////
/// Return an iterator on nodes of the current mesh. Once used this iterator
/// must be free by the caller
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
			return myIterator!=mySet.end();
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
///Return an iterator on egdes of the current mesh. Once used this iterator
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
			return myIterator!=mySet.end();
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

///////////////////////////////////////////////////////////////////////////////
/// Do intersection of sets (more than 2)
///////////////////////////////////////////////////////////////////////////////
set<const SMDS_MeshElement*> * intersectionOfSets(
	set<const SMDS_MeshElement*> vs[], int numberOfSets)
{
	set<const SMDS_MeshElement*>* rsetA=new set<const SMDS_MeshElement*>(vs[0]);
	set<const SMDS_MeshElement*>* rsetB;

	for(int i=0; i<numberOfSets-1; i++)
	{
		rsetB=new set<const SMDS_MeshElement*>();
		set_intersection(
			rsetA->begin(), rsetA->end(),
			vs[i+1].begin(), vs[i+1].end(),
			inserter(*rsetB, rsetB->begin()));
		delete rsetA;
		rsetA=rsetB;
	}
	return rsetA;
}

///////////////////////////////////////////////////////////////////////////////
/// Return the list of finit elements owning the given element
///////////////////////////////////////////////////////////////////////////////
set<const SMDS_MeshElement*> * getFinitElements(const SMDS_MeshElement * element)
{
	int numberOfSets=element->NbNodes();
	set<const SMDS_MeshElement*> initSet[numberOfSets];

	SMDS_Iterator<const SMDS_MeshElement*> * itNodes=element->nodesIterator();

	int i=0;
	while(itNodes->more())
	{
		const SMDS_MeshNode * n=static_cast<const SMDS_MeshNode*>(itNodes->next());
		SMDS_Iterator<const SMDS_MeshElement*> * itFe = n->GetInverseElementIterator();

		//initSet[i]=set<const SMDS_MeshElement*>();
		while(itFe->more()) initSet[i].insert(itFe->next());

		i++;
		delete itFe;
	}
	delete itNodes;
	
	return intersectionOfSets(initSet, numberOfSets);
}

///////////////////////////////////////////////////////////////////////////////
/// Return the list of nodes used only by the given elements
///////////////////////////////////////////////////////////////////////////////
set<const SMDS_MeshElement*> * getExclusiveNodes(
	set<const SMDS_MeshElement*>& elements)
{
	set<const SMDS_MeshElement*> * toReturn=new set<const SMDS_MeshElement*>();
	set<const SMDS_MeshElement*>::iterator itElements=elements.begin();

	while(itElements!=elements.end())
	{
		SMDS_Iterator<const SMDS_MeshElement*> * itNodes=
			(*itElements)->nodesIterator();
		itElements++;
	
		while(itNodes->more())
		{
			const SMDS_MeshNode * n=static_cast<const SMDS_MeshNode*>(itNodes->next());
			SMDS_Iterator<const SMDS_MeshElement*> * itFe = n->GetInverseElementIterator();
			set<const SMDS_MeshElement*> s;
			while(itFe->more()) s.insert(itFe->next());
			delete itFe;
			if(s==elements) toReturn->insert(n);
		}
		delete itNodes;
	}
	return toReturn;	
}

///////////////////////////////////////////////////////////////////////////////
///Find the children of an element that are made of given nodes 
///@param setOfChildren The set in which matching children will be inserted
///@param element The element were to search matching children
///@param nodes The nodes that the children must have to be selected
///////////////////////////////////////////////////////////////////////////////
void SMDS_Mesh::addChildrenWithNodes(set<const SMDS_MeshElement*>&	setOfChildren, 
	const SMDS_MeshElement * element, set<const SMDS_MeshElement*>& nodes)
{
	
	switch(element->GetType())
	{
	case SMDSAbs_Node:
		MESSAGE("Internal Error: This should not append");
		break;
	case SMDSAbs_Edge:
	{
		SMDS_Iterator<const SMDS_MeshElement*> * itn=element->nodesIterator();
		while(itn->more())
		{
			const SMDS_MeshElement * e=itn->next();
			if(nodes.find(e)!=nodes.end()) setOfChildren.insert(element);
		}
		delete itn;
	} break;	
	case SMDSAbs_Face:
	{
		SMDS_Iterator<const SMDS_MeshElement*> * itn=element->nodesIterator();
		while(itn->more())
		{
			const SMDS_MeshElement * e=itn->next();
			if(nodes.find(e)!=nodes.end()) setOfChildren.insert(element);
		}
		delete itn;
		if(hasConstructionEdges())
		{
			SMDS_Iterator<const SMDS_MeshElement*>* ite=element->edgesIterator();
			while(ite->more())
				addChildrenWithNodes(setOfChildren, ite->next(), nodes);
			delete ite;
		}
	} break;	
	case SMDSAbs_Volume:
	{
		if(hasConstructionFaces())
		{
			SMDS_Iterator<const SMDS_MeshElement*> * ite=element->facesIterator();
			while(ite->more())
				addChildrenWithNodes(setOfChildren, ite->next(), nodes);
			delete ite;
		}
		else if(hasConstructionEdges())
		{
			SMDS_Iterator<const SMDS_MeshElement*> * ite=element->edgesIterator();
			while(ite->more())
				addChildrenWithNodes(setOfChildren, ite->next(), nodes);
			delete ite;
		}
	}
	}
}

///////////////////////////////////////////////////////////////////////////////
///@param elem The element to delete
///@param removenodes if true remaining nodes will be removed
///////////////////////////////////////////////////////////////////////////////
void SMDS_Mesh::RemoveElement(const SMDS_MeshElement * elem,
	const bool removenodes)
{
	set<const SMDS_MeshElement*> * s1=getFinitElements(elem);
	
	set<const SMDS_MeshElement*> * s2=getExclusiveNodes(*s1);
	set<const SMDS_MeshElement*> s3;
	set<const SMDS_MeshElement*>::iterator it=s1->begin();
	while(it!=s1->end())
	{
		addChildrenWithNodes(s3, *it ,*s2);
		s3.insert(*it);
		it++;
	}
	if(elem->GetType()!=SMDSAbs_Node) s3.insert(elem);	
	it=s3.begin();
	while(it!=s3.end())
	{
		switch((*it)->GetType())
		{
		case SMDSAbs_Node:
			MESSAGE("Internal Error: This should not happen");
			break;
		case SMDSAbs_Edge:
			myEdges.erase(static_cast<SMDS_MeshEdge*>(
				const_cast<SMDS_MeshElement*>(*it)));	
			break;
		case SMDSAbs_Face:
			myFaces.erase(static_cast<SMDS_MeshFace*>(
				const_cast<SMDS_MeshElement*>(*it)));
			break;
		case SMDSAbs_Volume:
			myVolumes.erase(static_cast<SMDS_MeshVolume*>(
				const_cast<SMDS_MeshElement*>(*it)));	
			break;
		}
		delete (*it);
		it++;
	}	
	if(removenodes)
	{
		it=s2->begin();
		while(it!=s2->end())
		{
			myNodes.erase(static_cast<SMDS_MeshNode*>(
				const_cast<SMDS_MeshElement*>(*it)));
			delete *it;
			it++;
		}
	}
		
	delete s2;
	delete s1;
}

/**
 * Concat the coordinates of all nodes in an array.
 * Its used to display the mesh.
 * @return A array of size 3*NbNodes() containing the coordinates of nodes.
 */
double * SMDS_Mesh::getNodesCoordinates()
{
	double * toReturn=new double[3*NbNodes()];
	SMDS_Iterator<const SMDS_MeshNode*> * it=nodesIterator();
	int i=0;
	while(it->more())
	{
		const SMDS_MeshNode * n=it->next();
		toReturn[i]=n->X();
		i++;
		toReturn[i]=n->Y();
		i++;
		toReturn[i]=n->Z();
		i++;
	}
	delete it;
	return toReturn;
}

/**
 * Concat the id of all nodes in an array.
 * Its used to display the mesh.
 * @return A array of size NbNodes() containing the ids of nodes.
 */
long * SMDS_Mesh::getNodesID()
{
	long * toReturn=new long[NbNodes()];
	SMDS_Iterator<const SMDS_MeshNode*> * it=nodesIterator();
	int i=0;
	while(it->more())
	{
		const SMDS_MeshNode * n=it->next();		
		toReturn[i]=n->GetID();
		i++;
	}
	delete it;
	return toReturn;
}

/**
 * Concat the id of nodes of edges in an array.
 * Array format is {edge_id, node1_id, node2_id}
 * Its used to display the mesh.
 * @return A array of size 3*NbEdges() containing the edges.
 */
long * SMDS_Mesh::getEdgesIndices()
{
	long * toReturn=new long[NbEdges()*3];
	SMDS_Iterator<const SMDS_MeshEdge*> * it=edgesIterator();
	int i=0;
	
	while(it->more())
	{
		const SMDS_MeshEdge * e=it->next();
		toReturn[i]=e->GetID();
		i++;
		SMDS_Iterator<const SMDS_MeshElement*> * itn=e->nodesIterator();
		while(itn->more())
		{
			const SMDS_MeshElement * n=itn->next();
			toReturn[i]=n->GetID();
			i++;
		}
		delete itn;
	}
	delete it;
	return toReturn;
}

/**
 * Concat the id of nodes of triangles in an array.
 * Array format is {tria_id, node1_id, node2_id, node3_id}
 * Its used to display the mesh.
 * @return A array of size 4*NbTriangles() containing the edges.
 */
long * SMDS_Mesh::getTrianglesIndices()
{
	long * toReturn=new long[NbTriangles()*4];
	SMDS_Iterator<const SMDS_MeshFace*> * it=facesIterator();
	int i=0;
	while(it->more())
	{
		const SMDS_MeshFace * f=it->next();
		if(f->NbNodes()==3)
		{
			toReturn[i]=f->GetID();
			i++;		
			SMDS_Iterator<const SMDS_MeshElement*> * itn=f->nodesIterator();
			while(itn->more())
			{
				const SMDS_MeshElement * n=itn->next();
				toReturn[i]=n->GetID();
				i++;
			}
			delete itn;
		}
	}
	delete it;
	return toReturn;
}

/**
 * Return the number of 3 nodes faces in the mesh.
 * This method run in O(n).
 * @return The number of face whose number of nodes is 3
 */
int SMDS_Mesh::NbTriangles() const
{
	SMDS_Iterator<const SMDS_MeshFace*> * it=facesIterator();
	int toReturn=0;
	while(it->more())
	{
		const SMDS_MeshFace * f=it->next();
		if(f->NbNodes()==3) toReturn++;
	}
	return toReturn;
}
