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
//
//
//
//  File   : SMDS_Mesh.hxx
//  Module : SMESH

#ifndef _SMDS_Mesh_HeaderFile
#define _SMDS_Mesh_HeaderFile

#include "SMDS_MeshNode.hxx"
#include "SMDS_MeshEdge.hxx"
#include "SMDS_MeshFace.hxx"
#include "SMDS_MeshVolume.hxx"
#include "SMDS_MeshElementIDFactory.hxx"
#include "SMDS_ElemIterator.hxx"
#include <NCollection_Map.hxx>

#include <boost/shared_ptr.hpp>
#include <set>
#include <list>

typedef SMDS_Iterator<const SMDS_MeshNode *> SMDS_NodeIterator;
typedef boost::shared_ptr<SMDS_Iterator<const SMDS_MeshNode *> > SMDS_NodeIteratorPtr;
typedef SMDS_Iterator<const SMDS_MeshEdge *> SMDS_EdgeIterator;
typedef boost::shared_ptr<SMDS_Iterator<const SMDS_MeshEdge *> > SMDS_EdgeIteratorPtr;
typedef SMDS_Iterator<const SMDS_MeshFace *> SMDS_FaceIterator;
typedef boost::shared_ptr<SMDS_Iterator<const SMDS_MeshFace *> > SMDS_FaceIteratorPtr;
typedef SMDS_Iterator<const SMDS_MeshVolume *> SMDS_VolumeIterator;
typedef boost::shared_ptr<SMDS_Iterator<const SMDS_MeshVolume *> > SMDS_VolumeIteratorPtr;

class SMDS_Mesh:public SMDS_MeshObject{
public:
  
  SMDS_Mesh();
  
  SMDS_NodeIteratorPtr nodesIterator() const;
  SMDS_EdgeIteratorPtr edgesIterator() const;
  SMDS_FaceIteratorPtr facesIterator() const;
  SMDS_VolumeIteratorPtr volumesIterator() const;
  SMDS_ElemIteratorPtr elementsIterator() const;  

  SMDS_Mesh *AddSubMesh();
  
  virtual SMDS_MeshNode* AddNodeWithID(double x, double y, double z, int ID);
  virtual SMDS_MeshNode* AddNode(double x, double y, double z);
  
  virtual SMDS_MeshEdge* AddEdgeWithID(int n1, int n2, int ID);
  virtual SMDS_MeshEdge* AddEdgeWithID(const SMDS_MeshNode * n1,
				       const SMDS_MeshNode * n2, 
				       int ID);
  virtual SMDS_MeshEdge* AddEdge(const SMDS_MeshNode * n1,
				 const SMDS_MeshNode * n2);
  
  virtual SMDS_MeshFace* AddFaceWithID(int n1, int n2, int n3, int ID);
  virtual SMDS_MeshFace* AddFaceWithID(const SMDS_MeshNode * n1,
				       const SMDS_MeshNode * n2,
				       const SMDS_MeshNode * n3, 
				       int ID);
  virtual SMDS_MeshFace* AddFace(const SMDS_MeshNode * n1,
				 const SMDS_MeshNode * n2,
				 const SMDS_MeshNode * n3);
  
  virtual SMDS_MeshFace* AddFaceWithID(int n1, int n2, int n3, int n4, int ID);
  virtual SMDS_MeshFace* AddFaceWithID(const SMDS_MeshNode * n1,
				       const SMDS_MeshNode * n2,
				       const SMDS_MeshNode * n3,
				       const SMDS_MeshNode * n4, 
				       int ID);
  virtual SMDS_MeshFace* AddFace(const SMDS_MeshNode * n1,
				 const SMDS_MeshNode * n2,
				 const SMDS_MeshNode * n3,
				 const SMDS_MeshNode * n4);
  
  virtual SMDS_MeshFace* AddFaceWithID(const SMDS_MeshEdge * e1,
                                       const SMDS_MeshEdge * e2,
                                       const SMDS_MeshEdge * e3, int ID);
  virtual SMDS_MeshFace* AddFace(const SMDS_MeshEdge * e1,
                                 const SMDS_MeshEdge * e2,
                                 const SMDS_MeshEdge * e3);

  virtual SMDS_MeshFace* AddFaceWithID(const SMDS_MeshEdge * e1,
                                       const SMDS_MeshEdge * e2,
                                       const SMDS_MeshEdge * e3,
                                       const SMDS_MeshEdge * e4, int ID);
  virtual SMDS_MeshFace* AddFace(const SMDS_MeshEdge * e1,
                                 const SMDS_MeshEdge * e2,
                                 const SMDS_MeshEdge * e3,
                                 const SMDS_MeshEdge * e4);

  virtual SMDS_MeshVolume* AddVolumeWithID(int n1, int n2, int n3, int n4, int ID);
  virtual SMDS_MeshVolume* AddVolumeWithID(const SMDS_MeshNode * n1,
					   const SMDS_MeshNode * n2,
					   const SMDS_MeshNode * n3,
					   const SMDS_MeshNode * n4, 
					   int ID);
  virtual SMDS_MeshVolume* AddVolume(const SMDS_MeshNode * n1,
				     const SMDS_MeshNode * n2,
				     const SMDS_MeshNode * n3,
				     const SMDS_MeshNode * n4);
  
  virtual SMDS_MeshVolume* AddVolumeWithID(int n1, int n2, int n3, int n4,
                                           int n5, int ID);
  virtual SMDS_MeshVolume* AddVolumeWithID(const SMDS_MeshNode * n1,
					   const SMDS_MeshNode * n2,
					   const SMDS_MeshNode * n3,
					   const SMDS_MeshNode * n4,
					   const SMDS_MeshNode * n5, 
					   int ID);
  virtual SMDS_MeshVolume* AddVolume(const SMDS_MeshNode * n1,
				     const SMDS_MeshNode * n2,
				     const SMDS_MeshNode * n3,
				     const SMDS_MeshNode * n4,
				     const SMDS_MeshNode * n5);
  
  virtual SMDS_MeshVolume* AddVolumeWithID(int n1, int n2, int n3, int n4,
                                           int n5, int n6, int ID);
  virtual SMDS_MeshVolume* AddVolumeWithID(const SMDS_MeshNode * n1,
					   const SMDS_MeshNode * n2,
					   const SMDS_MeshNode * n3,
					   const SMDS_MeshNode * n4,
					   const SMDS_MeshNode * n5,
					   const SMDS_MeshNode * n6, 
					   int ID);
  virtual SMDS_MeshVolume* AddVolume(const SMDS_MeshNode * n1,
				     const SMDS_MeshNode * n2,
				     const SMDS_MeshNode * n3,
				     const SMDS_MeshNode * n4,
				     const SMDS_MeshNode * n5,
				     const SMDS_MeshNode * n6);
  
  virtual SMDS_MeshVolume* AddVolumeWithID(int n1, int n2, int n3, int n4,
                                           int n5, int n6, int n7, int n8, int ID);
  virtual SMDS_MeshVolume* AddVolumeWithID(const SMDS_MeshNode * n1,
					   const SMDS_MeshNode * n2,
					   const SMDS_MeshNode * n3,
					   const SMDS_MeshNode * n4,
					   const SMDS_MeshNode * n5,
					   const SMDS_MeshNode * n6,
					   const SMDS_MeshNode * n7,
					   const SMDS_MeshNode * n8, 
					   int ID);
  virtual SMDS_MeshVolume* AddVolume(const SMDS_MeshNode * n1,
				     const SMDS_MeshNode * n2,
				     const SMDS_MeshNode * n3,
				     const SMDS_MeshNode * n4,
				     const SMDS_MeshNode * n5,
				     const SMDS_MeshNode * n6,
				     const SMDS_MeshNode * n7,
				     const SMDS_MeshNode * n8);

  virtual SMDS_MeshVolume* AddVolumeWithID(const SMDS_MeshFace * f1,
                                           const SMDS_MeshFace * f2,
                                           const SMDS_MeshFace * f3,
                                           const SMDS_MeshFace * f4, int ID);
  virtual SMDS_MeshVolume* AddVolume(const SMDS_MeshFace * f1,
                                     const SMDS_MeshFace * f2,
                                     const SMDS_MeshFace * f3,
                                     const SMDS_MeshFace * f4);

  virtual SMDS_MeshVolume* AddVolumeWithID(const SMDS_MeshFace * f1,
                                           const SMDS_MeshFace * f2,
                                           const SMDS_MeshFace * f3,
                                           const SMDS_MeshFace * f4,
                                           const SMDS_MeshFace * f5, int ID);
  virtual SMDS_MeshVolume* AddVolume(const SMDS_MeshFace * f1,
                                     const SMDS_MeshFace * f2,
                                     const SMDS_MeshFace * f3,
                                     const SMDS_MeshFace * f4,
                                     const SMDS_MeshFace * f5);

  virtual SMDS_MeshVolume* AddVolumeWithID(const SMDS_MeshFace * f1,
                                           const SMDS_MeshFace * f2,
                                           const SMDS_MeshFace * f3,
                                           const SMDS_MeshFace * f4,
                                           const SMDS_MeshFace * f5,
                                           const SMDS_MeshFace * f6, int ID);
  virtual SMDS_MeshVolume* AddVolume(const SMDS_MeshFace * f1,
                                     const SMDS_MeshFace * f2,
                                     const SMDS_MeshFace * f3,
                                     const SMDS_MeshFace * f4,
                                     const SMDS_MeshFace * f5,
                                     const SMDS_MeshFace * f6);

  virtual void RemoveElement(const SMDS_MeshElement *        elem,
                             std::list<const SMDS_MeshElement *>& removedElems,
                             std::list<const SMDS_MeshElement *>& removedNodes,
                             const bool                      removenodes = false);
  virtual void RemoveElement(const SMDS_MeshElement * elem, bool removenodes = false);
  virtual void RemoveNode(const SMDS_MeshNode * node);
  virtual void RemoveEdge(const SMDS_MeshEdge * edge);
  virtual void RemoveFace(const SMDS_MeshFace * face);
  virtual void RemoveVolume(const SMDS_MeshVolume * volume);
  
  virtual bool RemoveFromParent();
  virtual bool RemoveSubMesh(const SMDS_Mesh * aMesh);

  static bool ChangeElementNodes(const SMDS_MeshElement * elem,
                                 const SMDS_MeshNode    * nodes[],
                                 const int                nbnodes);

  virtual void Renumber (const bool isNodes, const int startID = 1, const int deltaID = 1);
  // Renumber all nodes or elements.

  const SMDS_MeshNode *FindNode(int idnode) const;
  const SMDS_MeshEdge *FindEdge(int idnode1, int idnode2) const;
  const SMDS_MeshFace *FindFace(int idnode1, int idnode2, int idnode3) const;
  const SMDS_MeshFace *FindFace(int idnode1, int idnode2, int idnode3, int idnode4) const;
  const SMDS_MeshElement *FindElement(int IDelem) const;
  static const SMDS_MeshEdge* FindEdge(const SMDS_MeshNode * n1,
                                       const SMDS_MeshNode * n2);
  static const SMDS_MeshFace* FindFace(const SMDS_MeshNode *n1,
                                       const SMDS_MeshNode *n2,
                                       const SMDS_MeshNode *n3);
  static const SMDS_MeshFace* FindFace(const SMDS_MeshNode *n1,
                                       const SMDS_MeshNode *n2,
                                       const SMDS_MeshNode *n3,
                                       const SMDS_MeshNode *n4);
  int MaxNodeID() const;
  int MinNodeID() const;
  int MaxElementID() const;
  int MinElementID() const;


  int NbNodes() const;
  int NbEdges() const;
  int NbFaces() const;
  int NbVolumes() const;
  int NbSubMesh() const;
  void DumpNodes() const;
  void DumpEdges() const;
  void DumpFaces() const;
  void DumpVolumes() const;
  void DebugStats() const;
  SMDS_Mesh *boundaryFaces();
  SMDS_Mesh *boundaryEdges();
  virtual ~SMDS_Mesh();
  bool hasConstructionEdges();
  bool hasConstructionFaces();
  bool hasInverseElements();
  void setConstructionEdges(bool);
  void setConstructionFaces(bool);
  void setInverseElements(bool);

  /*!
   * Checks if the element is present in mesh.
   * Useful to determine dead pointers.
   * Use this function for debug purpose only! Do not check in the code
   * using it even in _DEBUG_ mode
   */
  bool Contains (const SMDS_MeshElement* elem) const;

  typedef NCollection_Map<SMDS_MeshNode *> SetOfNodes;
  typedef NCollection_Map<SMDS_MeshEdge *> SetOfEdges;
  typedef NCollection_Map<SMDS_MeshFace *> SetOfFaces;
  typedef NCollection_Map<SMDS_MeshVolume *> SetOfVolumes;

private:
  SMDS_Mesh(SMDS_Mesh * parent);

  SMDS_MeshFace * createTriangle(const SMDS_MeshNode * node1, 
				 const SMDS_MeshNode * node2, 
				 const SMDS_MeshNode * node3);
  SMDS_MeshFace * createQuadrangle(const SMDS_MeshNode * node1,
				   const SMDS_MeshNode * node2, 
				   const SMDS_MeshNode * node3, 
				   const SMDS_MeshNode * node4);
  SMDS_MeshEdge* FindEdgeOrCreate(const SMDS_MeshNode * n1,
				  const SMDS_MeshNode * n2);
  SMDS_MeshFace* FindFaceOrCreate(const SMDS_MeshNode *n1,
				  const SMDS_MeshNode *n2,
				  const SMDS_MeshNode *n3);
  SMDS_MeshFace* FindFaceOrCreate(const SMDS_MeshNode *n1,
				  const SMDS_MeshNode *n2,
				  const SMDS_MeshNode *n3,
				  const SMDS_MeshNode *n4);

  bool registerElement(int ID, SMDS_MeshElement * element);

  void addChildrenWithNodes(std::set<const SMDS_MeshElement*>& setOfChildren, 
			    const SMDS_MeshElement * element, 
			    std::set<const SMDS_MeshElement*>& nodes);

  // Fields PRIVATE
  
  SetOfNodes myNodes;
  SetOfEdges myEdges;
  SetOfFaces myFaces;
  SetOfVolumes myVolumes;
  SMDS_Mesh *myParent;
  std::list<SMDS_Mesh *> myChildren;
  SMDS_MeshElementIDFactory *myNodeIDFactory;
  SMDS_MeshElementIDFactory *myElementIDFactory;
  
  bool myHasConstructionEdges;
  bool myHasConstructionFaces;
  bool myHasInverseElements;
};


#endif
