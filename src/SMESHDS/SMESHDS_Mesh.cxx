using namespace std;
//=============================================================================
// File      : SMESH_Mesh.cxx
// Created   : 
// Author    : Yves FRICAUD, OCC
// Project   : SALOME
// Copyright : OCC 2002
// $Header: 
//=============================================================================

#include "SMESHDS_Mesh.ixx"
#include "SMESHDS_Hypothesis.hxx"
#include "SMESHDS_DataMapOfShapeListOfPtrHypothesis.hxx"
#include "SMESHDS_ListIteratorOfListOfPtrHypothesis.hxx"
#include "SMDS_VertexPosition.hxx"
#include "SMDS_EdgePosition.hxx"
#include "SMDS_FacePosition.hxx"
#include <TopExp_Explorer.hxx>
#include <TopExp.hxx>

#include <Standard_NullObject.hxx>
#include "utilities.h"

//=======================================================================
//function : Create
//purpose  : 
//=======================================================================
SMESHDS_Mesh::SMESHDS_Mesh(const Standard_Integer MeshID) : myMeshID( MeshID)
{
  myScript = new SMESHDS_Script();
}

//=======================================================================
//function : ShapeToMesh
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::ShapeToMesh(const TopoDS_Shape& S) 
{
  myShape = S;
  TopExp::MapShapes(myShape,myIndexToShape);
}
  
  
//=======================================================================
//function : AddHypothesis
//purpose  : 
//=======================================================================

Standard_Boolean SMESHDS_Mesh::AddHypothesis(const TopoDS_Shape& SS,
					     const SMESHDS_PtrHypothesis& H) 
{
  if (!myShapeToHypothesis.IsBound(SS)){
    SMESHDS_ListOfPtrHypothesis empty;
    myShapeToHypothesis.Bind(SS,empty);
  }
  else {
    //Check if the Hypothesis is still present
    SMESHDS_ListOfPtrHypothesis& Hypos = myShapeToHypothesis.ChangeFind (SS);

    for (SMESHDS_ListIteratorOfListOfPtrHypothesis it(Hypos); it.More(); it.Next()) {
      if (H == it.Value()) {
	return Standard_False;
      }
    }
  }
  myShapeToHypothesis(SS).Append(H);
  return Standard_True;
}

//=======================================================================
//function : RemoveHypothesis
//purpose  : 
//=======================================================================

Standard_Boolean SMESHDS_Mesh::RemoveHypothesis(const TopoDS_Shape& S,
						const SMESHDS_PtrHypothesis& H) 
{
  if (myShapeToHypothesis.IsBound(S)){
    SMESHDS_ListOfPtrHypothesis& Hypos = myShapeToHypothesis.ChangeFind (S);
    for (SMESHDS_ListIteratorOfListOfPtrHypothesis it(Hypos); it.More(); it.Next()) {
      if (H == it.Value()) {
	Hypos.Remove(it);
	return Standard_True;
      }
    }
  }
  return Standard_False;
}
  
  
//=======================================================================
//function : AddNode
//purpose  : 
//=======================================================================
Standard_Integer SMESHDS_Mesh::AddNode(const Standard_Real x,
				       const Standard_Real y,
				       const Standard_Real z) 
{
  Standard_Integer NodeID = SMDS_Mesh::AddNode(x,y,z);
  myScript->AddNode(NodeID,x,y,z);
  return NodeID;
}

//=======================================================================
//function : MoveNode
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::MoveNode(const Standard_Integer ID,
					const Standard_Real x,
					const Standard_Real y,
					const Standard_Real z) 
{

  Handle(SMDS_MeshNode) Node = Handle(SMDS_MeshNode)::DownCast(FindNode(ID));
  gp_Pnt P(x,y,z);
  Node->SetPnt(P);
  myScript->MoveNode(ID,x,y,z);
}



//=======================================================================
//function : AddEdge
//purpose  : 
//=======================================================================
Standard_Integer SMESHDS_Mesh::AddEdge(const Standard_Integer idnode1,
				       const Standard_Integer idnode2) 
{
  Standard_Integer ID = SMDS_Mesh::AddEdge(idnode1,idnode2);
  myScript->AddEdge (ID,idnode1,idnode2);
  return ID;
}


//=======================================================================
//function :AddFace
//purpose  : 
//=======================================================================
Standard_Integer SMESHDS_Mesh::AddFace(const Standard_Integer idnode1,
				       const Standard_Integer idnode2,
				       const Standard_Integer idnode3) 
{
  Standard_Integer ID = SMDS_Mesh::AddFace(idnode1,idnode2,idnode3);
  myScript->AddFace (ID,idnode1,idnode2,idnode3);
  return ID;
}

//=======================================================================
//function :AddFace
//purpose  : 
//=======================================================================
Standard_Integer SMESHDS_Mesh::AddFace(const Standard_Integer idnode1,
				       const Standard_Integer idnode2,
				       const Standard_Integer idnode3,
				       const Standard_Integer idnode4) 
{
  Standard_Integer ID = SMDS_Mesh::AddFace(idnode1,idnode2,idnode3,idnode4);
  myScript->AddFace (ID,idnode1,idnode2,idnode3,idnode4);
  return ID;
}


//=======================================================================
//function :AddVolume
//purpose  : 
//=======================================================================
Standard_Integer SMESHDS_Mesh::AddVolume(const Standard_Integer idnode1,
					 const Standard_Integer idnode2,
					 const Standard_Integer idnode3,
					 const Standard_Integer idnode4) 
{
  Standard_Integer ID = SMDS_Mesh::AddVolume(idnode1,idnode2,idnode3,idnode4);
  myScript->AddVolume (ID,idnode1,idnode2,idnode3,idnode4);
  return ID;
}


//=======================================================================
//function :AddVolume
//purpose  : 
//=======================================================================
Standard_Integer SMESHDS_Mesh::AddVolume(const Standard_Integer idnode1,
					 const Standard_Integer idnode2,
					 const Standard_Integer idnode3,
					 const Standard_Integer idnode4,
					 const Standard_Integer idnode5) 
{
  Standard_Integer ID = SMDS_Mesh::AddVolume(idnode1,idnode2,idnode3,idnode4,idnode5);
  myScript->AddVolume (ID,idnode1,idnode2,idnode3,idnode4,idnode5);
  return ID;
}


//=======================================================================
//function :AddVolume
//purpose  : 
//=======================================================================
Standard_Integer SMESHDS_Mesh::AddVolume(const Standard_Integer idnode1,
					 const Standard_Integer idnode2,
					 const Standard_Integer idnode3,
					 const Standard_Integer idnode4,
					 const Standard_Integer idnode5,
					 const Standard_Integer idnode6) 
{
  Standard_Integer ID = SMDS_Mesh::AddVolume(idnode1,idnode2,idnode3,idnode4,idnode5,idnode6);
  myScript->AddVolume (ID,idnode1,idnode2,idnode3,idnode4,idnode5,idnode6);
  return ID;
}

//=======================================================================
//function :AddVolume
//purpose  : 
//=======================================================================
Standard_Integer SMESHDS_Mesh::AddVolume(const Standard_Integer idnode1,
					 const Standard_Integer idnode2,
					 const Standard_Integer idnode3,
					 const Standard_Integer idnode4,
					 const Standard_Integer idnode5,
					 const Standard_Integer idnode6,
					 const Standard_Integer idnode7,
					 const Standard_Integer idnode8) 
{
  Standard_Integer ID = SMDS_Mesh::AddVolume(idnode1,idnode2,idnode3,idnode4,idnode5,idnode6,idnode7,idnode8);
  myScript->AddVolume (ID,idnode1,idnode2,idnode3,idnode4,idnode5,idnode6,idnode7,idnode8);
  return ID;
}


//=======================================================================
//function : RemoveNode
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::RemoveNode(const Standard_Integer ID) 
{
  SMDS_Mesh::RemoveNode (ID);
  myScript->RemoveNode  (ID);
}



//=======================================================================
//function : RemoveElement
//purpose  : 
//========================================================================
void SMESHDS_Mesh::RemoveElement(const Standard_Integer ID) 
{
  SMDS_Mesh::RemoveElement (ID);
  myScript->RemoveElement  (ID);
}

//=======================================================================
//function : SetNodeOnVolume
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::SetNodeInVolume (const Handle(SMDS_MeshNode)& aNode,
				    const TopoDS_Shell& S) 
{
  if (myShape.IsNull()) 
    Standard_NullObject::Raise("SMESHDS_Mesh::SetNodeOnVolume");
  
  Standard_Integer Index = myIndexToShape.FindIndex(S);
  
  //Set Position on Node
  //Handle (SMDS_FacePosition) aPos = new SMDS_FacePosition (myFaceToId(S),0.,0.);;
  //aNode->SetPosition(aPos);
  
  //Update or build submesh
  if (!myShapeIndexToSubMesh.IsBound(Index)) {
    Handle(SMESHDS_SubMesh) SM = new  SMESHDS_SubMesh (this);
    myShapeIndexToSubMesh.Bind(Index,SM);
  }
  myShapeIndexToSubMesh(Index)->AddNode (aNode);
}

//=======================================================================
//function : SetNodeOnFace
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::SetNodeOnFace (const Handle(SMDS_MeshNode)& aNode,
				  const TopoDS_Face& S) 
{
  if (myShape.IsNull()) 
    Standard_NullObject::Raise("SMESHDS_Mesh::SetNodeOnFace");
  
  Standard_Integer Index = myIndexToShape.FindIndex(S);
  
  //Set Position on Node
  Handle (SMDS_FacePosition) aPos = new SMDS_FacePosition (Index,0.,0.);;
  aNode->SetPosition(aPos);
  
  //Update or build submesh
  if (!myShapeIndexToSubMesh.IsBound(Index)) {
    Handle(SMESHDS_SubMesh) SM = new  SMESHDS_SubMesh (this);
    myShapeIndexToSubMesh.Bind(Index,SM);
  }
  myShapeIndexToSubMesh(Index)->AddNode (aNode);
}

//=======================================================================
//function : SetNodeOnEdge
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::SetNodeOnEdge (const Handle(SMDS_MeshNode)& aNode,
				  const TopoDS_Edge& S) 
{
  if (myShape.IsNull()) 
    Standard_NullObject::Raise("SMESHDS_Mesh::SetNodeOnEdge");
  
  Standard_Integer Index = myIndexToShape.FindIndex(S);
  
  //Set Position on Node
  Handle (SMDS_EdgePosition) aPos = new SMDS_EdgePosition (Index,0.);;
  aNode->SetPosition(aPos);
  
  //Update or build submesh
  if (!myShapeIndexToSubMesh.IsBound(Index)) {
    Handle(SMESHDS_SubMesh) SM = new  SMESHDS_SubMesh (this);
    myShapeIndexToSubMesh.Bind(Index,SM);
  }
  myShapeIndexToSubMesh(Index)->AddNode (aNode);
  
}

//=======================================================================
//function : SetNodeOnVertex
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::SetNodeOnVertex (const Handle(SMDS_MeshNode)& aNode,
				    const TopoDS_Vertex& S) 
{ 
  if (myShape.IsNull()) 
    Standard_NullObject::Raise("SMESHDS_Mesh::SetNodeOnVertex");
  
  Standard_Integer Index = myIndexToShape.FindIndex(S);
  
  //Set Position on Node
  Handle (SMDS_VertexPosition) aPos = new SMDS_VertexPosition (Index);;
  aNode->SetPosition(aPos);
  
  //Update or build submesh
  if (!myShapeIndexToSubMesh.IsBound(Index)) {
    Handle(SMESHDS_SubMesh) SM = new  SMESHDS_SubMesh (this);
    myShapeIndexToSubMesh.Bind(Index,SM);
  }
  myShapeIndexToSubMesh(Index)->AddNode (aNode);
}


//=======================================================================
//function : UnSetNodeOnShape
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::UnSetNodeOnShape(const Handle(SMDS_MeshNode)& aNode) 
{ 
  MESSAGE("not implemented");
}


//=======================================================================
//function : SetMeshElementOnShape
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::SetMeshElementOnShape (const Handle(SMDS_MeshElement)& anElement,
					   const TopoDS_Shape& S) 
{
  if (myShape.IsNull()) 
    Standard_NullObject::Raise("SMESHDS_Mesh::SetMeshElementOnShape");
  
  Standard_Integer Index = myIndexToShape.FindIndex(S);
  

  if (!myShapeIndexToSubMesh.IsBound(Index)) {
    Handle(SMESHDS_SubMesh) SM = new  SMESHDS_SubMesh (this);
    myShapeIndexToSubMesh.Bind(Index,SM);
  }
  myShapeIndexToSubMesh(Index)->AddElement (anElement);
}

//=======================================================================
//function : UnSetMeshElementOnShape
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::UnSetMeshElementOnShape (const Handle(SMDS_MeshElement)& anElement,
					     const TopoDS_Shape& S)
  
{
  if (myShape.IsNull()) 
    Standard_NullObject::Raise("SMESHDS_Mesh::UnSetMeshElementOnShape");
  
  Standard_Integer Index = myIndexToShape.FindIndex(S);

  if (myShapeIndexToSubMesh.IsBound(Index)) 
    myShapeIndexToSubMesh(Index)->RemoveElement (anElement);
}

//=======================================================================
//function : ShapeToMesh
//purpose  : 
//=======================================================================
TopoDS_Shape SMESHDS_Mesh::ShapeToMesh() 
{
  return myShape;
}

//=======================================================================
//function : MeshElements
//purpose  : 
//=======================================================================
Handle_SMESHDS_SubMesh SMESHDS_Mesh::MeshElements(const TopoDS_Shape& S) 
{
  if (myShape.IsNull()) 
    Standard_NullObject::Raise("SMESHDS_Mesh::MeshElements");
  
  Standard_Integer Index = myIndexToShape.FindIndex(S);

  if (myShapeIndexToSubMesh.IsBound(Index)) 
    return myShapeIndexToSubMesh(Index);
  Handle(SMESHDS_SubMesh) SM;
  return SM;
}

//=======================================================================
//function : GetHypothesis
//purpose  : 
//=======================================================================
const SMESHDS_ListOfPtrHypothesis&  SMESHDS_Mesh::GetHypothesis(const TopoDS_Shape& S) 
{
  if (myShapeToHypothesis.IsBound(S))
    return myShapeToHypothesis(S);
 
  static SMESHDS_ListOfPtrHypothesis empty;
  return empty;
}

//=======================================================================
//function : GetScript
//purpose  : 
//=======================================================================
const Handle (SMESHDS_Script)&  SMESHDS_Mesh::GetScript() 
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
Standard_Boolean SMESHDS_Mesh::HasMeshElements(const TopoDS_Shape& S) 
{
  if (myShape.IsNull()) 
    Standard_NullObject::Raise("SMESHDS_Mesh::MeshElements");
  
  Standard_Integer Index = myIndexToShape.FindIndex(S);

  return myShapeIndexToSubMesh.IsBound(Index);
}

//=======================================================================
//function : HasHypothesis
//purpose  : 
//=======================================================================
Standard_Boolean SMESHDS_Mesh::HasHypothesis(const TopoDS_Shape& S) 
{
  return myShapeToHypothesis.IsBound(S);
}

//=======================================================================
//function : NewSubMesh 
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::NewSubMesh(const Standard_Integer Index) 
{
  if (!myShapeIndexToSubMesh.IsBound(Index)) {
    Handle(SMESHDS_SubMesh) SM = new  SMESHDS_SubMesh (this);
    myShapeIndexToSubMesh.Bind(Index,SM);
  }
}

//=======================================================================
//function : IndexToShape
//purpose  : 
//=======================================================================
TopoDS_Shape SMESHDS_Mesh::IndexToShape(const Standard_Integer ShapeIndex)
{
  return  myIndexToShape.FindKey(ShapeIndex);
} 

//=======================================================================
//function : ShapeToIndex
//purpose  : 
//=======================================================================
Standard_Integer SMESHDS_Mesh::ShapeToIndex(const TopoDS_Shape& S) 
{
  if (myShape.IsNull()) 
    Standard_NullObject::Raise("SMESHDS_Mesh::SetNodeOnVolume");
  
  return  myIndexToShape.FindIndex(S);
}

//=======================================================================
//function : SetNodeOnVolume
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::SetNodeInVolume (const Handle(SMDS_MeshNode)& aNode,
				    const Standard_Integer Index) 
{
  
  //Set Position on Node
  //Handle (SMDS_FacePosition) aPos = new SMDS_FacePosition (myFaceToId(S),0.,0.);;
  //aNode->SetPosition(aPos);
  
  //Update or build submesh
  if (!myShapeIndexToSubMesh.IsBound(Index)) {
    Handle(SMESHDS_SubMesh) SM = new  SMESHDS_SubMesh (this);
    myShapeIndexToSubMesh.Bind(Index,SM);
  }
  myShapeIndexToSubMesh(Index)->AddNode (aNode);
}

//=======================================================================
//function : SetNodeOnFace
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::SetNodeOnFace (const Handle(SMDS_MeshNode)& aNode,
				  const Standard_Integer Index) 
{
  
  //Set Position on Node
  Handle (SMDS_FacePosition) aPos = new SMDS_FacePosition (Index,0.,0.);;
  aNode->SetPosition(aPos);
  
  //Update or build submesh
  if (!myShapeIndexToSubMesh.IsBound(Index)) {
    Handle(SMESHDS_SubMesh) SM = new  SMESHDS_SubMesh (this);
    myShapeIndexToSubMesh.Bind(Index,SM);
  }
  myShapeIndexToSubMesh(Index)->AddNode (aNode);
}

//=======================================================================
//function : SetNodeOnEdge
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::SetNodeOnEdge (const Handle(SMDS_MeshNode)& aNode,
				  const Standard_Integer Index) 
{
  
  //Set Position on Node
  Handle (SMDS_EdgePosition) aPos = new SMDS_EdgePosition (Index,0.);;
  aNode->SetPosition(aPos);
  
  //Update or build submesh
  if (!myShapeIndexToSubMesh.IsBound(Index)) {
    Handle(SMESHDS_SubMesh) SM = new  SMESHDS_SubMesh (this);
    myShapeIndexToSubMesh.Bind(Index,SM);
  }
  myShapeIndexToSubMesh(Index)->AddNode (aNode);
  
}

//=======================================================================
//function : SetNodeOnVertex
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::SetNodeOnVertex (const Handle(SMDS_MeshNode)& aNode,
				    const Standard_Integer Index) 
{ 
  //Set Position on Node
  Handle (SMDS_VertexPosition) aPos = new SMDS_VertexPosition (Index);;
  aNode->SetPosition(aPos);
  
  //Update or build submesh
  if (!myShapeIndexToSubMesh.IsBound(Index)) {
    Handle(SMESHDS_SubMesh) SM = new  SMESHDS_SubMesh (this);
    myShapeIndexToSubMesh.Bind(Index,SM);
  }
  myShapeIndexToSubMesh(Index)->AddNode (aNode);
}

//=======================================================================
//function : SetMeshElementOnShape
//purpose  : 
//=======================================================================
void SMESHDS_Mesh::SetMeshElementOnShape (const Handle(SMDS_MeshElement)& anElement,
					  const Standard_Integer Index) 
{
  if (!myShapeIndexToSubMesh.IsBound(Index)) {
    Handle(SMESHDS_SubMesh) SM = new  SMESHDS_SubMesh (this);
    myShapeIndexToSubMesh.Bind(Index,SM);
  }
  myShapeIndexToSubMesh(Index)->AddElement (anElement);
}
