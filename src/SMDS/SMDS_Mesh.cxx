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
  return SMDS_Mesh::AddNodeWithID(x,y,z,myNodeIDFactory->GetFreeID());
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
  if(!node){
    SMDS_MeshNode * node=new SMDS_MeshNode(x, y, z);
    myNodes.insert(node);
    myNodeIDFactory->BindID(ID,node);
    return node;
  }else
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
  if(!node1 || !node2) return NULL;
  return SMDS_Mesh::AddEdgeWithID(node1, node2, ID);
}

///////////////////////////////////////////////////////////////////////////////
/// create a MeshEdge and add it to the current Mesh
/// @return : The created MeshEdge
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshEdge* SMDS_Mesh::AddEdge(const SMDS_MeshNode * node1,
				  const SMDS_MeshNode * node2)
{
  return SMDS_Mesh::AddEdgeWithID(node1, node2, myElementIDFactory->GetFreeID());
}

///////////////////////////////////////////////////////////////////////////////
/// Create a new edge and at it to the mesh
/// @param idnode1 ID of the first node
/// @param idnode2 ID of the second node
/// @param ID ID of the edge to create
/// @return The created edge or NULL if an element with this ID already exists or
/// if input nodes are not found.
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshEdge* SMDS_Mesh::AddEdgeWithID(const SMDS_MeshNode * n1,
					const SMDS_MeshNode * n2, 
					int ID)
{
  SMDS_MeshEdge * edge=new SMDS_MeshEdge(n1,n2);
  if(myElementIDFactory->BindID(ID, edge)) {
    SMDS_MeshNode *node1,*node2;
    node1=const_cast<SMDS_MeshNode*>(n1);
    node2=const_cast<SMDS_MeshNode*>(n2);
    node1->AddInverseElement(edge);
    node2->AddInverseElement(edge);		
    myEdges.insert(edge);
    return edge;
  } 
  else {
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
  return SMDS_Mesh::AddFaceWithID(n1,n2,n3, myElementIDFactory->GetFreeID());
}

///////////////////////////////////////////////////////////////////////////////
/// Add a triangle defined by its nodes IDs
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshFace* SMDS_Mesh::AddFaceWithID(int idnode1, int idnode2, int idnode3, int ID)
{
  SMDS_MeshNode * node1 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode1);
  SMDS_MeshNode * node2 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode2);
  SMDS_MeshNode * node3 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode3);
  if(!node1 || !node2 || !node3) return NULL;
  return SMDS_Mesh::AddFaceWithID(node1, node2, node3, ID);	
}

///////////////////////////////////////////////////////////////////////////////
/// Add a triangle defined by its nodes
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshFace* SMDS_Mesh::AddFaceWithID(const SMDS_MeshNode * n1,
                                        const SMDS_MeshNode * n2,
                                        const SMDS_MeshNode * n3,
                                        int ID)
{
  SMDS_MeshFace * face=createTriangle(n1, n2, n3);

  if (!registerElement(ID, face)) {
    RemoveElement(face, false);
    face = NULL;
  }
  return face;
}

///////////////////////////////////////////////////////////////////////////////
/// Add a quadrangle defined by its nodes. An ID is automatically affected to the
/// created face
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshFace* SMDS_Mesh::AddFace(const SMDS_MeshNode * n1,
				  const SMDS_MeshNode * n2,
				  const SMDS_MeshNode * n3,
				  const SMDS_MeshNode * n4)
{
  return SMDS_Mesh::AddFaceWithID(n1,n2,n3, n4, myElementIDFactory->GetFreeID());
}

///////////////////////////////////////////////////////////////////////////////
/// Add a quadrangle defined by its nodes IDs
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshFace* SMDS_Mesh::AddFaceWithID(int idnode1, 
					int idnode2, 
					int idnode3,
					int idnode4, 
					int ID)
{
  SMDS_MeshNode *node1, *node2, *node3, *node4;
  node1 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode1);
  node2 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode2);
  node3 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode3);
  node4 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode4);
  if(!node1 || !node2 || !node3 || !node4) return NULL;
  return SMDS_Mesh::AddFaceWithID(node1, node2, node3, node4, ID);	
}

///////////////////////////////////////////////////////////////////////////////
/// Add a quadrangle defined by its nodes
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshFace* SMDS_Mesh::AddFaceWithID(const SMDS_MeshNode * n1,
                                        const SMDS_MeshNode * n2,
                                        const SMDS_MeshNode * n3,
                                        const SMDS_MeshNode * n4,
                                        int ID)
{
  SMDS_MeshFace * face=createQuadrangle(n1, n2, n3, n4);

  if (!registerElement(ID, face)) {
    RemoveElement(face, false);
    face = NULL;
  }
  return face;
}

///////////////////////////////////////////////////////////////////////////////
/// Add a triangle defined by its edges. An ID is automatically assigned to the
/// Created face
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshFace* SMDS_Mesh::AddFace(const SMDS_MeshEdge * e1,
                                  const SMDS_MeshEdge * e2,
                                  const SMDS_MeshEdge * e3)
{
  if (!hasConstructionEdges())
    return NULL;
  return AddFaceWithID(e1,e2,e3, myElementIDFactory->GetFreeID());
}

///////////////////////////////////////////////////////////////////////////////
/// Add a triangle defined by its edges
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshFace* SMDS_Mesh::AddFaceWithID(const SMDS_MeshEdge * e1,
                                        const SMDS_MeshEdge * e2,
                                        const SMDS_MeshEdge * e3,
                                        int ID)
{
  if (!hasConstructionEdges())
    return NULL;
  SMDS_MeshFace * face = new SMDS_FaceOfEdges(e1,e2,e3);
  myFaces.insert(face);

  if (!registerElement(ID, face)) {
    RemoveElement(face, false);
    face = NULL;
  }
  return face;
}

///////////////////////////////////////////////////////////////////////////////
/// Add a quadrangle defined by its edges. An ID is automatically assigned to the
/// Created face
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshFace* SMDS_Mesh::AddFace(const SMDS_MeshEdge * e1,
                                  const SMDS_MeshEdge * e2,
                                  const SMDS_MeshEdge * e3,
                                  const SMDS_MeshEdge * e4)
{
  if (!hasConstructionEdges())
    return NULL;
  return AddFaceWithID(e1,e2,e3,e4, myElementIDFactory->GetFreeID());
}

///////////////////////////////////////////////////////////////////////////////
/// Add a quadrangle defined by its edges
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshFace* SMDS_Mesh::AddFaceWithID(const SMDS_MeshEdge * e1,
                                        const SMDS_MeshEdge * e2,
                                        const SMDS_MeshEdge * e3,
                                        const SMDS_MeshEdge * e4,
                                        int ID)
{
  if (!hasConstructionEdges())
    return NULL;
  SMDS_MeshFace * face = new SMDS_FaceOfEdges(e1,e2,e3,e4);
  myFaces.insert(face);

  if (!registerElement(ID, face))
  {
    RemoveElement(face, false);
    face = NULL;
  }
  return face;
}

///////////////////////////////////////////////////////////////////////////////
///Create a new tetrahedron and add it to the mesh. 
///@return The created tetrahedron 
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolume(const SMDS_MeshNode * n1,
				      const SMDS_MeshNode * n2, 
				      const SMDS_MeshNode * n3,
				      const SMDS_MeshNode * n4)
{
  int ID = myElementIDFactory->GetFreeID();
  SMDS_MeshVolume * v = SMDS_Mesh::AddVolumeWithID(n1, n2, n3, n4, ID);
  if(v==NULL) myElementIDFactory->ReleaseID(ID);
  return v;
}

///////////////////////////////////////////////////////////////////////////////
///Create a new tetrahedron and add it to the mesh. 
///@param ID The ID of the new volume
///@return The created tetrahedron or NULL if an element with this ID already exists
///or if input nodes are not found.
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume * SMDS_Mesh::AddVolumeWithID(int idnode1, 
					     int idnode2,
					     int idnode3, 
					     int idnode4, 
					     int ID)
{
  SMDS_MeshNode *node1, *node2, *node3, *node4;
  node1 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode1);
  node2 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode2);
  node3 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode3);
  node4 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode4);
  if(!node1 || !node2 || !node3 || !node4) return NULL;
  return SMDS_Mesh::AddVolumeWithID(node1, node2, node3, node4, ID);
}
	
///////////////////////////////////////////////////////////////////////////////
///Create a new tetrahedron and add it to the mesh. 
///@param ID The ID of the new volume
///@return The created tetrahedron 
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolumeWithID(const SMDS_MeshNode * n1,
                                            const SMDS_MeshNode * n2,
                                            const SMDS_MeshNode * n3,
                                            const SMDS_MeshNode * n4,
                                            int ID)
{
  SMDS_MeshVolume* volume;
  if(hasConstructionFaces()) {
    SMDS_MeshFace * f1=FindFaceOrCreate(n1,n2,n3);
    SMDS_MeshFace * f2=FindFaceOrCreate(n1,n2,n4);
    SMDS_MeshFace * f3=FindFaceOrCreate(n1,n3,n4);
    SMDS_MeshFace * f4=FindFaceOrCreate(n2,n3,n4);
    volume=new SMDS_VolumeOfFaces(f1,f2,f3,f4);
    myVolumes.insert(volume);
  }
  else if(hasConstructionEdges()) {
    MESSAGE("Error : Not implemented");
    return NULL;
  }
  else {
    volume=new SMDS_VolumeOfNodes(n1,n2,n3,n4);
    myVolumes.insert(volume);
  }

  if (!registerElement(ID, volume)) {
    RemoveElement(volume, false);
    volume = NULL;
  }
  return volume;
}

///////////////////////////////////////////////////////////////////////////////
///Create a new pyramid and add it to the mesh. 
///Nodes 1,2,3 and 4 define the base of the pyramid
///@return The created pyramid 
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolume(const SMDS_MeshNode * n1,
				      const SMDS_MeshNode * n2, 
				      const SMDS_MeshNode * n3,
				      const SMDS_MeshNode * n4, 
				      const SMDS_MeshNode * n5)
{
  int ID = myElementIDFactory->GetFreeID();
  SMDS_MeshVolume * v = SMDS_Mesh::AddVolumeWithID(n1, n2, n3, n4, n5, ID);
  if(v==NULL) myElementIDFactory->ReleaseID(ID);
  return v;
}

///////////////////////////////////////////////////////////////////////////////
///Create a new pyramid and add it to the mesh. 
///Nodes 1,2,3 and 4 define the base of the pyramid
///@param ID The ID of the new volume
///@return The created pyramid or NULL if an element with this ID already exists
///or if input nodes are not found.
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume * SMDS_Mesh::AddVolumeWithID(int idnode1, 
					     int idnode2,
					     int idnode3, 
					     int idnode4, 
					     int idnode5, 
					     int ID)
{
  SMDS_MeshNode *node1, *node2, *node3, *node4, *node5;
  node1 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode1);
  node2 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode2);
  node3 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode3);
  node4 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode4);
  node5 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode5);
  if(!node1 || !node2 || !node3 || !node4 || !node5) return NULL;
  return SMDS_Mesh::AddVolumeWithID(node1, node2, node3, node4, node5, ID);
}
	
///////////////////////////////////////////////////////////////////////////////
///Create a new pyramid and add it to the mesh.
///Nodes 1,2,3 and 4 define the base of the pyramid
///@param ID The ID of the new volume
///@return The created pyramid
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolumeWithID(const SMDS_MeshNode * n1,
                                            const SMDS_MeshNode * n2,
                                            const SMDS_MeshNode * n3,
                                            const SMDS_MeshNode * n4,
                                            const SMDS_MeshNode * n5,
                                            int ID)
{
  SMDS_MeshVolume* volume;
  if(hasConstructionFaces()) {
    SMDS_MeshFace * f1=FindFaceOrCreate(n1,n2,n3,n4);
    SMDS_MeshFace * f2=FindFaceOrCreate(n1,n2,n5);
    SMDS_MeshFace * f3=FindFaceOrCreate(n2,n3,n5);
    SMDS_MeshFace * f4=FindFaceOrCreate(n3,n4,n5);
    volume=new SMDS_VolumeOfFaces(f1,f2,f3,f4);
    myVolumes.insert(volume);
  }
  else if(hasConstructionEdges()) {
    MESSAGE("Error : Not implemented");
    return NULL;
  }
  else {
    volume=new SMDS_VolumeOfNodes(n1,n2,n3,n4,n5);
    myVolumes.insert(volume);
  }

  if (!registerElement(ID, volume)) {
    RemoveElement(volume, false);
    volume = NULL;
  }
  return volume;
}

///////////////////////////////////////////////////////////////////////////////
///Create a new prism and add it to the mesh. 
///Nodes 1,2,3 is a triangle and 1,2,5,4 a quadrangle.
///@return The created prism 
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolume(const SMDS_MeshNode * n1,
				      const SMDS_MeshNode * n2, 
				      const SMDS_MeshNode * n3,
				      const SMDS_MeshNode * n4, 
				      const SMDS_MeshNode * n5,
				      const SMDS_MeshNode * n6)
{
  int ID = myElementIDFactory->GetFreeID();
  SMDS_MeshVolume * v = SMDS_Mesh::AddVolumeWithID(n1, n2, n3, n4, n5, n6, ID);
  if(v==NULL) myElementIDFactory->ReleaseID(ID);
  return v;
}

///////////////////////////////////////////////////////////////////////////////
///Create a new prism and add it to the mesh. 
///Nodes 1,2,3 is a triangle and 1,2,5,4 a quadrangle.
///@param ID The ID of the new volume
///@return The created prism or NULL if an element with this ID already exists
///or if input nodes are not found.
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume * SMDS_Mesh::AddVolumeWithID(int idnode1, 
					     int idnode2,
					     int idnode3, 
					     int idnode4, 
					     int idnode5, 
					     int idnode6, 
					     int ID)
{
  SMDS_MeshNode *node1, *node2, *node3, *node4, *node5, *node6;
  node1 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode1);
  node2 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode2);
  node3 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode3);
  node4 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode4);
  node5 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode5);
  node6 = (SMDS_MeshNode *)myNodeIDFactory->MeshElement(idnode6);
  if(!node1 || !node2 || !node3 || !node4 || !node5 || !node6) return NULL;
  return SMDS_Mesh::AddVolumeWithID(node1, node2, node3, node4, node5, node6, ID);
}
	
///////////////////////////////////////////////////////////////////////////////
///Create a new prism and add it to the mesh.
///Nodes 1,2,3 is a triangle and 1,2,5,4 a quadrangle.
///@param ID The ID of the new volume
///@return The created prism
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolumeWithID(const SMDS_MeshNode * n1,
                                            const SMDS_MeshNode * n2,
                                            const SMDS_MeshNode * n3,
                                            const SMDS_MeshNode * n4,
                                            const SMDS_MeshNode * n5,
                                            const SMDS_MeshNode * n6,
                                            int ID)
{
  SMDS_MeshVolume* volume;
  if(hasConstructionFaces()) {
    SMDS_MeshFace * f1=FindFaceOrCreate(n1,n2,n3);
    SMDS_MeshFace * f2=FindFaceOrCreate(n4,n5,n6);
    SMDS_MeshFace * f3=FindFaceOrCreate(n1,n4,n5,n2);
    SMDS_MeshFace * f4=FindFaceOrCreate(n2,n5,n6,n3);
    SMDS_MeshFace * f5=FindFaceOrCreate(n3,n6,n4,n1);
    volume=new SMDS_VolumeOfFaces(f1,f2,f3,f4,f5);
    myVolumes.insert(volume);
  }
  else if(hasConstructionEdges()) {
    MESSAGE("Error : Not implemented");
    return NULL;
  }
  else {
    volume=new SMDS_VolumeOfNodes(n1,n2,n3,n4,n5,n6);
    myVolumes.insert(volume);
  }

  if (!registerElement(ID, volume)) {
    RemoveElement(volume, false);
    volume = NULL;
  }
  return volume;
}

///////////////////////////////////////////////////////////////////////////////
///Create a new hexahedron and add it to the mesh. 
///Nodes 1,2,3,4 and 5,6,7,8 are quadrangle and 5,1 and 7,3 are an edges.
///@return The created hexahedron 
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolume(const SMDS_MeshNode * n1,
				      const SMDS_MeshNode * n2, 
				      const SMDS_MeshNode * n3,
				      const SMDS_MeshNode * n4, 
				      const SMDS_MeshNode * n5,
				      const SMDS_MeshNode * n6, 
				      const SMDS_MeshNode * n7,
				      const SMDS_MeshNode * n8)
{
  int ID = myElementIDFactory->GetFreeID();
  SMDS_MeshVolume * v = SMDS_Mesh::AddVolumeWithID(n1, n2, n3, n4, n5, n6, n7, n8, ID);
  if(v==NULL) myElementIDFactory->ReleaseID(ID);
  return v;
}

///////////////////////////////////////////////////////////////////////////////
///Create a new hexahedron and add it to the mesh. 
///Nodes 1,2,3,4 and 5,6,7,8 are quadrangle and 5,1 and 7,3 are an edges.
///@param ID The ID of the new volume
///@return The created hexahedron or NULL if an element with this ID already
///exists or if input nodes are not found.
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume * SMDS_Mesh::AddVolumeWithID(int idnode1, 
					     int idnode2,
					     int idnode3, 
					     int idnode4, 
					     int idnode5, 
					     int idnode6, 
					     int idnode7,
					     int idnode8, 
					     int ID)
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
  if(!node1 || !node2 || !node3 || !node4 || !node5 || !node6 || !node7 || !node8)
    return NULL;
  return SMDS_Mesh::AddVolumeWithID(node1, node2, node3, node4, node5, node6,
                                    node7, node8, ID);
}
	
///////////////////////////////////////////////////////////////////////////////
///Create a new hexahedron and add it to the mesh.
///Nodes 1,2,3,4 and 5,6,7,8 are quadrangle and 5,1 and 7,3 are an edges.
///@param ID The ID of the new volume
///@return The created prism or NULL if an element with this ID already exists
///or if input nodes are not found.
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolumeWithID(const SMDS_MeshNode * n1,
                                            const SMDS_MeshNode * n2,
                                            const SMDS_MeshNode * n3,
                                            const SMDS_MeshNode * n4,
                                            const SMDS_MeshNode * n5,
                                            const SMDS_MeshNode * n6,
                                            const SMDS_MeshNode * n7,
                                            const SMDS_MeshNode * n8,
                                            int ID)
{
  SMDS_MeshVolume* volume;
  if(hasConstructionFaces()) {
    SMDS_MeshFace * f1=FindFaceOrCreate(n1,n2,n3,n4);
    SMDS_MeshFace * f2=FindFaceOrCreate(n5,n6,n7,n8);
    SMDS_MeshFace * f3=FindFaceOrCreate(n1,n4,n8,n5);
    SMDS_MeshFace * f4=FindFaceOrCreate(n1,n2,n6,n5);
    SMDS_MeshFace * f5=FindFaceOrCreate(n2,n3,n7,n6);
    SMDS_MeshFace * f6=FindFaceOrCreate(n3,n4,n8,n7);
    volume=new SMDS_VolumeOfFaces(f1,f2,f3,f4,f5,f6);
    myVolumes.insert(volume);
  }
  else if(hasConstructionEdges()) {
    MESSAGE("Error : Not implemented");
    return NULL;
  }
  else {
//    volume=new SMDS_HexahedronOfNodes(n1,n2,n3,n4,n5,n6,n7,n8);
    volume=new SMDS_VolumeOfNodes(n1,n2,n3,n4,n5,n6,n7,n8);
    myVolumes.insert(volume);
  }

  if (!registerElement(ID, volume)) {
    RemoveElement(volume, false);
    volume = NULL;
  }
  return volume;
}

///////////////////////////////////////////////////////////////////////////////
///Create a new tetrahedron defined by its faces and add it to the mesh.
///@return The created tetrahedron
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolume(const SMDS_MeshFace * f1,
                                      const SMDS_MeshFace * f2,
                                      const SMDS_MeshFace * f3,
                                      const SMDS_MeshFace * f4)
{
  if (!hasConstructionFaces())
    return NULL;
  return AddVolumeWithID(f1,f2,f3,f4, myElementIDFactory->GetFreeID());
}

///////////////////////////////////////////////////////////////////////////////
///Create a new tetrahedron defined by its faces and add it to the mesh.
///@param ID The ID of the new volume
///@return The created tetrahedron 
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolumeWithID(const SMDS_MeshFace * f1,
                                            const SMDS_MeshFace * f2,
                                            const SMDS_MeshFace * f3,
                                            const SMDS_MeshFace * f4,
                                            int ID)
{
  if (!hasConstructionFaces())
    return NULL;
  SMDS_MeshVolume * volume = new SMDS_VolumeOfFaces(f1,f2,f3,f4);
  myVolumes.insert(volume);

  if (!registerElement(ID, volume)) {
    RemoveElement(volume, false);
    volume = NULL;
  }
  return volume;
}

///////////////////////////////////////////////////////////////////////////////
///Create a new pyramid defined by its faces and add it to the mesh.
///@return The created pyramid
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolume(const SMDS_MeshFace * f1,
                                      const SMDS_MeshFace * f2,
                                      const SMDS_MeshFace * f3,
                                      const SMDS_MeshFace * f4,
                                      const SMDS_MeshFace * f5)
{
  if (!hasConstructionFaces())
    return NULL;
  return AddVolumeWithID(f1,f2,f3,f4,f5, myElementIDFactory->GetFreeID());
}

///////////////////////////////////////////////////////////////////////////////
///Create a new pyramid defined by its faces and add it to the mesh.
///@param ID The ID of the new volume
///@return The created pyramid 
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolumeWithID(const SMDS_MeshFace * f1,
                                            const SMDS_MeshFace * f2,
                                            const SMDS_MeshFace * f3,
                                            const SMDS_MeshFace * f4,
                                            const SMDS_MeshFace * f5,
                                            int ID)
{
  if (!hasConstructionFaces())
    return NULL;
  SMDS_MeshVolume * volume = new SMDS_VolumeOfFaces(f1,f2,f3,f4,f5);
  myVolumes.insert(volume);

  if (!registerElement(ID, volume)) {
    RemoveElement(volume, false);
    volume = NULL;
  }
  return volume;
}

///////////////////////////////////////////////////////////////////////////////
///Create a new prism defined by its faces and add it to the mesh.
///@return The created prism
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolume(const SMDS_MeshFace * f1,
                                      const SMDS_MeshFace * f2,
                                      const SMDS_MeshFace * f3,
                                      const SMDS_MeshFace * f4,
                                      const SMDS_MeshFace * f5,
                                      const SMDS_MeshFace * f6)
{
  if (!hasConstructionFaces())
    return NULL;
  return AddVolumeWithID(f1,f2,f3,f4,f5,f6, myElementIDFactory->GetFreeID());
}

///////////////////////////////////////////////////////////////////////////////
///Create a new prism defined by its faces and add it to the mesh.
///@param ID The ID of the new volume
///@return The created prism 
///////////////////////////////////////////////////////////////////////////////

SMDS_MeshVolume* SMDS_Mesh::AddVolumeWithID(const SMDS_MeshFace * f1,
                                            const SMDS_MeshFace * f2,
                                            const SMDS_MeshFace * f3,
                                            const SMDS_MeshFace * f4,
                                            const SMDS_MeshFace * f5,
                                            const SMDS_MeshFace * f6,
                                            int ID)
{
  if (!hasConstructionFaces())
    return NULL;
  SMDS_MeshVolume * volume = new SMDS_VolumeOfFaces(f1,f2,f3,f4,f5,f6);
  myVolumes.insert(volume);

  if (!registerElement(ID, volume)) {
    RemoveElement(volume, false);
    volume = NULL;
  }
  return volume;
}

///////////////////////////////////////////////////////////////////////////////
/// Registers element with the given ID, maintains inverse connections
///////////////////////////////////////////////////////////////////////////////
bool SMDS_Mesh::registerElement(int ID, SMDS_MeshElement * element)
{
  if (myElementIDFactory->BindID(ID, element)) {
    SMDS_ElemIteratorPtr it = element->nodesIterator();
    while (it->more()) {
      SMDS_MeshNode *node = static_cast<SMDS_MeshNode*>
        (const_cast<SMDS_MeshElement*>(it->next()));
      node->AddInverseElement(element);
    }
    return true;
  }
  return false;
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
SMDS_MeshFace * SMDS_Mesh::createTriangle(const SMDS_MeshNode * node1,
                                          const SMDS_MeshNode * node2,
                                          const SMDS_MeshNode * node3)
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
SMDS_MeshFace * SMDS_Mesh::createQuadrangle(const SMDS_MeshNode * node1,
					    const SMDS_MeshNode * node2,
					    const SMDS_MeshNode * node3,
					    const SMDS_MeshNode * node4)
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
//function : ChangeElementNodes
//purpose  : 
//=======================================================================

bool SMDS_Mesh::ChangeElementNodes(const SMDS_MeshElement * elem,
                                   const SMDS_MeshNode    * nodes[],
                                   const int                nbnodes)
{
  // keep current nodes of elem
  set<const SMDS_MeshElement*> oldNodes;
  SMDS_ElemIteratorPtr itn = elem->nodesIterator();
  while(itn->more())
    oldNodes.insert(  itn->next() );

  // change nodes
  bool Ok = false;
  switch ( elem->GetType() )
  {
  case SMDSAbs_Edge: {
    if ( nbnodes == 2 ) {
      const SMDS_MeshEdge* edge = dynamic_cast<const SMDS_MeshEdge*>( elem );
      if ( edge )
        Ok = const_cast<SMDS_MeshEdge*>( edge )->ChangeNodes( nodes[0], nodes[1] );
    }
    break;
  }
  case SMDSAbs_Face: {
    const SMDS_FaceOfNodes* face = dynamic_cast<const SMDS_FaceOfNodes*>( elem );
    if ( face )
      Ok = const_cast<SMDS_FaceOfNodes*>( face )->ChangeNodes( nodes, nbnodes );
    break;
  }
  case SMDSAbs_Volume: {
    const SMDS_VolumeOfNodes* vol = dynamic_cast<const SMDS_VolumeOfNodes*>( elem );
    if ( vol )
      Ok = const_cast<SMDS_VolumeOfNodes*>( vol )->ChangeNodes( nodes, nbnodes );
    break;
  }
  default:
    MESSAGE ( "WRONG ELEM TYPE");
  }

  if ( Ok ) { // update InverseElements

    // AddInverseElement to new nodes
    for ( int i = 0; i < nbnodes; i++ )
      if ( oldNodes.find( nodes[i] ) == oldNodes.end() )
        // new node
        const_cast<SMDS_MeshNode*>( nodes[i] )->AddInverseElement( elem );
      else
        // remove from oldNodes a node that remains in elem
        oldNodes.erase( nodes[i] );


    // RemoveInverseElement from the nodes removed from elem
    set<const SMDS_MeshElement*>::iterator it;
    for ( it = oldNodes.begin(); it != oldNodes.end(); it++ )
    {
      SMDS_MeshNode * n = static_cast<SMDS_MeshNode *>
        (const_cast<SMDS_MeshElement *>( *it ));
      n->RemoveInverseElement( elem );
    }
  }

  //MESSAGE ( "::ChangeNodes() Ok = " << Ok);

  return Ok;
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

//#include "Profiler.h"
const SMDS_MeshEdge* SMDS_Mesh::FindEdge(const SMDS_MeshNode * node1,
                                         const SMDS_MeshNode * node2)
{
	const SMDS_MeshEdge * toReturn=NULL;
	//PROFILER_Init();
	//PROFILER_Set();
	SMDS_ElemIteratorPtr it1=node1->edgesIterator();
	//PROFILER_Get(0);
	//PROFILER_Set();
	while(it1->more())
	{
		const SMDS_MeshEdge * e=static_cast<const SMDS_MeshEdge *>
			(it1->next());
		SMDS_ElemIteratorPtr it2=e->nodesIterator();
		while(it2->more())
		{
			if(it2->next()->GetID()==node2->GetID())
			{
				toReturn=e;
				break;
			}
		}
	}
	//PROFILER_Get(1);
	return toReturn;
}


SMDS_MeshEdge* SMDS_Mesh::FindEdgeOrCreate(const SMDS_MeshNode * node1,
	const SMDS_MeshNode * node2) 
{
	SMDS_MeshEdge * toReturn=NULL;
	toReturn=const_cast<SMDS_MeshEdge*>(FindEdge(node1,node2));
	if(toReturn==NULL)	
	{
          toReturn=new SMDS_MeshEdge(node1,node2);
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
	if((node1==NULL)||(node2==NULL)||(node3==NULL)) return NULL;
	return FindFace(node1, node2, node3);
}

const SMDS_MeshFace* SMDS_Mesh::FindFace(
		const SMDS_MeshNode *node1,
		const SMDS_MeshNode *node2,
		const SMDS_MeshNode *node3)
{
	const SMDS_MeshFace * face;
	const SMDS_MeshElement * node;
	bool node2found, node3found;

	SMDS_ElemIteratorPtr it1=node1->facesIterator();
	while(it1->more())
	{
		face=static_cast<const SMDS_MeshFace*>(it1->next());
		if(face->NbNodes()!=3) continue;
		SMDS_ElemIteratorPtr it2=face->nodesIterator();
		node2found=false;
		node3found=false;
		while(it2->more())
		{
			node=it2->next();
			if(node->GetID()==node2->GetID()) node2found=true;
			if(node->GetID()==node3->GetID()) node3found=true;
		}
		if(node2found&&node3found)
			return face;
	}
	return NULL;
}

SMDS_MeshFace* SMDS_Mesh::FindFaceOrCreate(
		const SMDS_MeshNode *node1,
		const SMDS_MeshNode *node2,
		const SMDS_MeshNode *node3)
{
	SMDS_MeshFace * toReturn=NULL;
	toReturn=const_cast<SMDS_MeshFace*>(FindFace(node1,node2,node3));
	if(toReturn==NULL)
	{
	  toReturn=createTriangle(node1,node2,node3);
	}
	return toReturn;
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
		const SMDS_MeshNode *node4)
{
	const SMDS_MeshFace * face;
	const SMDS_MeshElement * node;
	bool node2found, node3found, node4found;
	SMDS_ElemIteratorPtr it1=node1->facesIterator();
	while(it1->more())
	{
		face=static_cast<const SMDS_MeshFace *>(it1->next());
		if(face->NbNodes()!=4) continue;
		SMDS_ElemIteratorPtr it2=face->nodesIterator();
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
		if(node2found&&node3found&&node4found)
			return face;
	}
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
	  toReturn=createQuadrangle(node1,node2,node3,node4);
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
	SMDS_NodeIteratorPtr itnode=nodesIterator();
	while(itnode->more()) MESSAGE(itnode->next());
}

//=======================================================================
//function : DumpEdges
//purpose  : 
//=======================================================================

void SMDS_Mesh::DumpEdges() const
{
	MESSAGE("dump edges of mesh : ");
	SMDS_EdgeIteratorPtr itedge=edgesIterator();
	while(itedge->more()) MESSAGE(itedge->next());
}

//=======================================================================
//function : DumpFaces
//purpose  : 
//=======================================================================

void SMDS_Mesh::DumpFaces() const
{
	MESSAGE("dump faces of mesh : ");
	SMDS_FaceIteratorPtr itface=facesIterator();
	while(itface->more()) MESSAGE(itface->next());
}

//=======================================================================
//function : DumpVolumes
//purpose  : 
//=======================================================================

void SMDS_Mesh::DumpVolumes() const
{
	MESSAGE("dump volumes of mesh : ");
	SMDS_VolumeIteratorPtr itvol=volumesIterator();
	while(itvol->more()) MESSAGE(itvol->next());
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
	
	SMDS_NodeIteratorPtr itnode=nodesIterator();
	int sizeofnodes = 0;
	int sizeoffaces = 0;

	while(itnode->more())
	{
		const SMDS_MeshNode *node = itnode->next();

		sizeofnodes += sizeof(*node);
		
		SMDS_ElemIteratorPtr it = node->GetInverseElementIterator();
		while(it->more())
		{
			const SMDS_MeshElement *me = it->next();
			sizeofnodes += sizeof(me);
		}

	}

	SMDS_FaceIteratorPtr itface=facesIterator();
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
	
	SMDS_NodeIteratorPtr itn=nodesIterator();
	while(itn->more())
	{
		delete itn->next();
	}

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
/// Return an iterator on nodes of the current mesh
///////////////////////////////////////////////////////////////////////////////
class SMDS_Mesh_MyNodeIterator:public SMDS_NodeIterator
{
  const SMDS_IdElementMap&          myIdElemMap;
  SMDS_IdElementMap::const_iterator myIterator;
 public:
  SMDS_Mesh_MyNodeIterator(const SMDS_IdElementMap& s):myIdElemMap(s)
  {
    myIterator=myIdElemMap.begin();
  }

  bool more()
  {
    return myIterator!=myIdElemMap.end();
  }

  const SMDS_MeshNode* next()
  {
    const SMDS_MeshElement* current=(*myIterator).second;
    myIterator++;
    return static_cast<const SMDS_MeshNode*>( current );
  }
};

SMDS_NodeIteratorPtr SMDS_Mesh::nodesIterator() const
{
  return SMDS_NodeIteratorPtr
    (new SMDS_Mesh_MyNodeIterator(myNodeIDFactory->GetIdElementMap()));
}
///////////////////////////////////////////////////////////////////////////////
/// Return an iterator on nodes of the current mesh
///////////////////////////////////////////////////////////////////////////////
class SMDS_Mesh_MyElemIterator:public SMDS_ElemIterator
{
  const SMDS_IdElementMap&          myIdElemMap;
  SMDS_IdElementMap::const_iterator myIterator;
 public:
  SMDS_Mesh_MyElemIterator(const SMDS_IdElementMap& s):myIdElemMap(s)
  {
    myIterator=myIdElemMap.begin();
  }

  bool more()
  {
    return myIterator!=myIdElemMap.end();
  }

  const SMDS_MeshElement* next()
  {
    const SMDS_MeshElement* current=(*myIterator).second;
    myIterator++;
    return current;	
  }
};

SMDS_ElemIteratorPtr SMDS_Mesh::elementsIterator() const
{
  return SMDS_ElemIteratorPtr
    (new SMDS_Mesh_MyElemIterator(myElementIDFactory->GetIdElementMap()));
}

///////////////////////////////////////////////////////////////////////////////
///Return an iterator on volumes of the current mesh.
///////////////////////////////////////////////////////////////////////////////
class SMDS_Mesh_MyEdgeIterator:public SMDS_EdgeIterator
{
  typedef SMDS_Mesh::SetOfEdges SetOfEdges;
  const SetOfEdges& mySet;
  const SMDS_MeshEdge * myEdge;
  SetOfEdges::iterator myIterator;
 public:
  SMDS_Mesh_MyEdgeIterator(const SetOfEdges& s):mySet(s)
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

SMDS_EdgeIteratorPtr SMDS_Mesh::edgesIterator() const
{
  return SMDS_EdgeIteratorPtr(new SMDS_Mesh_MyEdgeIterator(myEdges));
}

///////////////////////////////////////////////////////////////////////////////
///Return an iterator on faces of the current mesh. Once used this iterator
///must be free by the caller
///////////////////////////////////////////////////////////////////////////////
class SMDS_Mesh_MyFaceIterator:public SMDS_FaceIterator
{
  typedef SMDS_Mesh::SetOfFaces SetOfFaces;
  const SetOfFaces& mySet;
  SetOfFaces::iterator myIterator;
 public:
  SMDS_Mesh_MyFaceIterator(const SetOfFaces& s):mySet(s)
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

SMDS_FaceIteratorPtr SMDS_Mesh::facesIterator() const
{
  return SMDS_FaceIteratorPtr(new SMDS_Mesh_MyFaceIterator(myFaces));
}

///////////////////////////////////////////////////////////////////////////////
///Return an iterator on volumes of the current mesh. Once used this iterator
///must be free by the caller
///////////////////////////////////////////////////////////////////////////////
class SMDS_Mesh_MyVolumeIterator:public SMDS_VolumeIterator
{
  typedef SMDS_Mesh::SetOfVolumes SetOfVolumes;
  const SetOfVolumes& mySet;
  SetOfVolumes::iterator myIterator;
 public:
  SMDS_Mesh_MyVolumeIterator(const SetOfVolumes& s):mySet(s)
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

SMDS_VolumeIteratorPtr SMDS_Mesh::volumesIterator() const
{
  return SMDS_VolumeIteratorPtr(new SMDS_Mesh_MyVolumeIterator(myVolumes));
}

///////////////////////////////////////////////////////////////////////////////
/// Do intersection of sets (more than 2)
///////////////////////////////////////////////////////////////////////////////
static set<const SMDS_MeshElement*> * intersectionOfSets(
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
static set<const SMDS_MeshElement*> * getFinitElements(const SMDS_MeshElement * element)
{
	int numberOfSets=element->NbNodes();
	set<const SMDS_MeshElement*> initSet[numberOfSets];

	SMDS_ElemIteratorPtr itNodes=element->nodesIterator();

	int i=0;
	while(itNodes->more())
	{
		const SMDS_MeshNode * n=static_cast<const SMDS_MeshNode*>(itNodes->next());
		SMDS_ElemIteratorPtr itFe = n->GetInverseElementIterator();

		//initSet[i]=set<const SMDS_MeshElement*>();
		while(itFe->more())
                  initSet[i].insert(itFe->next());

		i++;
	}
	
	return intersectionOfSets(initSet, numberOfSets);
}

///////////////////////////////////////////////////////////////////////////////
/// Return the list of nodes used only by the given elements
///////////////////////////////////////////////////////////////////////////////
static set<const SMDS_MeshElement*> * getExclusiveNodes(
	set<const SMDS_MeshElement*>& elements)
{
	set<const SMDS_MeshElement*> * toReturn=new set<const SMDS_MeshElement*>();
	set<const SMDS_MeshElement*>::iterator itElements=elements.begin();

	while(itElements!=elements.end())
	{
		SMDS_ElemIteratorPtr itNodes = (*itElements)->nodesIterator();
		itElements++;
	
		while(itNodes->more())
		{
			const SMDS_MeshNode * n=static_cast<const SMDS_MeshNode*>(itNodes->next());
			SMDS_ElemIteratorPtr itFe = n->GetInverseElementIterator();
			set<const SMDS_MeshElement*> s;
			while(itFe->more())
                          s.insert(itFe->next());
			if(s==elements) toReturn->insert(n);
		}
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
		SMDS_ElemIteratorPtr itn=element->nodesIterator();
		while(itn->more())
		{
			const SMDS_MeshElement * e=itn->next();
			if(nodes.find(e)!=nodes.end())
                        {
                          setOfChildren.insert(element);
                          break;
                        }
		}
	} break;
	case SMDSAbs_Face:
	{
		SMDS_ElemIteratorPtr itn=element->nodesIterator();
		while(itn->more())
		{
			const SMDS_MeshElement * e=itn->next();
			if(nodes.find(e)!=nodes.end())
                        {
                          setOfChildren.insert(element);
                          break;
                        }
		}
		if(hasConstructionEdges())
		{
			SMDS_ElemIteratorPtr ite=element->edgesIterator();
			while(ite->more())
				addChildrenWithNodes(setOfChildren, ite->next(), nodes);
		}
	} break;	
	case SMDSAbs_Volume:
	{
		if(hasConstructionFaces())
		{
			SMDS_ElemIteratorPtr ite=element->facesIterator();
			while(ite->more())
				addChildrenWithNodes(setOfChildren, ite->next(), nodes);
		}
		else if(hasConstructionEdges())
		{
			SMDS_ElemIteratorPtr ite=element->edgesIterator();
			while(ite->more())
				addChildrenWithNodes(setOfChildren, ite->next(), nodes);
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
  list<const SMDS_MeshElement *> removedElems;
  list<const SMDS_MeshElement *> removedNodes;
  RemoveElement( elem, removedElems, removedNodes, removenodes );
}
  
///////////////////////////////////////////////////////////////////////////////
///@param elem The element to delete
///@param removedElems contains all removed elements
///@param removedNodes contains all removed nodes
///@param removenodes if true remaining nodes will be removed
///////////////////////////////////////////////////////////////////////////////
void SMDS_Mesh::RemoveElement(const SMDS_MeshElement *        elem,
                              list<const SMDS_MeshElement *>& removedElems,
                              list<const SMDS_MeshElement *>& removedNodes,
                              bool                            removenodes)
{
  // get finite elements built on elem
  set<const SMDS_MeshElement*> * s1;
  if (!hasConstructionEdges() && elem->GetType() == SMDSAbs_Edge ||
      !hasConstructionFaces() && elem->GetType() == SMDSAbs_Face)
  {
    s1 = new set<const SMDS_MeshElement*>();
    s1->insert(elem);
  }
  else
    s1 = getFinitElements(elem);

  // get exclusive nodes (which would become free afterwards)
  set<const SMDS_MeshElement*> * s2;
  if (elem->GetType() == SMDSAbs_Node) // a node is removed
  {
    // do not remove nodes except elem
    s2 = new set<const SMDS_MeshElement*>();
    s2->insert(elem);
    removenodes = true;
  }
  else
    s2 = getExclusiveNodes(*s1);

  // form the set of finite and construction elements to remove
  set<const SMDS_MeshElement*> s3;
  set<const SMDS_MeshElement*>::iterator it=s1->begin();
  while(it!=s1->end())
  {
    addChildrenWithNodes(s3, *it ,*s2);
    s3.insert(*it);
    it++;
  }
  if(elem->GetType()!=SMDSAbs_Node) s3.insert(elem);

  // remove finite and construction elements
  it=s3.begin();
  while(it!=s3.end())
  {
    // Remove element from <InverseElements> of its nodes
    SMDS_ElemIteratorPtr itn=(*it)->nodesIterator();
    while(itn->more())
    {
      SMDS_MeshNode * n = static_cast<SMDS_MeshNode *>
        (const_cast<SMDS_MeshElement *>(itn->next()));
      n->RemoveInverseElement( (*it) );
    }

    switch((*it)->GetType())
    {
    case SMDSAbs_Node:
      MESSAGE("Internal Error: This should not happen");
      break;
    case SMDSAbs_Edge:
      myEdges.erase(static_cast<SMDS_MeshEdge*>
                    (const_cast<SMDS_MeshElement*>(*it)));
      break;
    case SMDSAbs_Face:
      myFaces.erase(static_cast<SMDS_MeshFace*>
                    (const_cast<SMDS_MeshElement*>(*it)));
      break;
    case SMDSAbs_Volume:
      myVolumes.erase(static_cast<SMDS_MeshVolume*>
                      (const_cast<SMDS_MeshElement*>(*it)));
      break;
    }
    //MESSAGE( "SMDS: RM elem " << (*it)->GetID() );
    removedElems.push_back( (*it) );
    myElementIDFactory->ReleaseID((*it)->GetID());
    delete (*it);
    it++;
  }

  // remove exclusive (free) nodes
  if(removenodes)
  {
    it=s2->begin();
    while(it!=s2->end())
    {
      //MESSAGE( "SMDS: RM node " << (*it)->GetID() );
      myNodes.erase(static_cast<SMDS_MeshNode*>
                    (const_cast<SMDS_MeshElement*>(*it)));
      myNodeIDFactory->ReleaseID((*it)->GetID());
      removedNodes.push_back( (*it) );
      delete *it;
      it++;
    }
  }

  delete s2;
  delete s1;
}

/*!
 * Checks if the element is present in mesh.
 * Useful to determine dead pointers.
 */
bool SMDS_Mesh::Contains (const SMDS_MeshElement* elem) const
{
  // we should not imply on validity of *elem, so iterate on containers
  // of all types in the hope of finding <elem> somewhere there
  SMDS_NodeIteratorPtr itn = nodesIterator();
  while (itn->more())
    if (elem == itn->next())
      return true;
  SMDS_EdgeIteratorPtr ite = edgesIterator();
  while (ite->more())
    if (elem == ite->next())
      return true;
  SMDS_FaceIteratorPtr itf = facesIterator();
  while (itf->more())
    if (elem == itf->next())
      return true;
  SMDS_VolumeIteratorPtr itv = volumesIterator();
  while (itv->more())
    if (elem == itv->next())
      return true;
  return false;
}

//=======================================================================
//function : MaxNodeID
//purpose  : 
//=======================================================================

int SMDS_Mesh::MaxNodeID() const
{
  return myNodeIDFactory->GetMaxID();
}

//=======================================================================
//function : MinNodeID
//purpose  : 
//=======================================================================

int SMDS_Mesh::MinNodeID() const
{
  return myNodeIDFactory->GetMinID();
}

//=======================================================================
//function : MaxElementID
//purpose  : 
//=======================================================================

int SMDS_Mesh::MaxElementID() const
{
  return myElementIDFactory->GetMaxID();
}

//=======================================================================
//function : MinElementID
//purpose  : 
//=======================================================================

int SMDS_Mesh::MinElementID() const
{
  return myElementIDFactory->GetMinID();
}

//=======================================================================
//function : Renumber
//purpose  : Renumber all nodes or elements.
//=======================================================================

void SMDS_Mesh::Renumber (const bool isNodes, const int  startID, const int  deltaID)
{
  if ( deltaID == 0 )
    return;

  SMDS_MeshElementIDFactory * idFactory =
    isNodes ? myNodeIDFactory : myElementIDFactory;

  // get existing elements in the order of ID increasing and release their ids
  list< SMDS_MeshElement * > elemList;
  const SMDS_IdElementMap& idElemMap = idFactory->GetIdElementMap();
  SMDS_IdElementMap::const_iterator idElemIt = idElemMap.begin();
  while ( idElemIt != idElemMap.end() ) {
    SMDS_MeshElement* elem = (*idElemIt).second;
    int id = (*idElemIt).first;
    idElemIt++;
    elemList.push_back( elem );
    idFactory->ReleaseID( id );
  }
  // set new IDs
  int ID = startID;
  list< SMDS_MeshElement * >::iterator elemIt = elemList.begin();
  for ( ; elemIt != elemList.end(); elemIt++ )
  {
    idFactory->BindID( ID, *elemIt );
    ID += deltaID;
  }
}

